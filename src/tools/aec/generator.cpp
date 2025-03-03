#include "generator.h"

#include <QSet>

template <template <class> class Array, class T>
static QString joinNumbers(const Array<T> &arr, const QString &glue) {
    QStringList list;
    list.reserve(arr.size());
    for (const auto &item : arr) {
        list.append(QString::number(item));
    }
    return list.join(glue);
}

static QString itemInfoTypeToString(QAK::ActionItemInfo::Type type) {
    switch (type) {
        case QAK::ActionItemInfo::Action:
            return QStringLiteral("Action");
        case QAK::ActionItemInfo::Group:
            return QStringLiteral("Group");
        case QAK::ActionItemInfo::Menu:
            return QStringLiteral("Menu");
        case QAK::ActionItemInfo::Phony:
            return QStringLiteral("Phony");
        default:
            Q_UNREACHABLE();
            break;
    }
    return {};
}

static QString layoutEntryTypeToString(QAK::ActionLayoutEntry::Type type) {
    switch (type) {
        case QAK::ActionLayoutEntry::Action:
            return QStringLiteral("Action");
        case QAK::ActionLayoutEntry::Group:
            return QStringLiteral("Group");
        case QAK::ActionLayoutEntry::Menu:
            return QStringLiteral("Menu");
        case QAK::ActionLayoutEntry::Separator:
            return QStringLiteral("Separator");
        case QAK::ActionLayoutEntry::Stretch:
            return QStringLiteral("Stretch");
        default:
            Q_UNREACHABLE();
            break;
    }
    return {};
}

static QString insertionAnchorToString(QAK::ActionInsertion::Anchor anchor) {
    switch (anchor) {
        case QAK::ActionInsertion::Last:
            return QStringLiteral("Last");
        case QAK::ActionInsertion::First:
            return QStringLiteral("First");
        case QAK::ActionInsertion::After:
            return QStringLiteral("After");
        case QAK::ActionInsertion::Before:
            return QStringLiteral("Before");
        default:
            Q_UNREACHABLE();
            break;
    }
    return {};
}

Generator::Generator(FILE *out, const QString &inputFileName, const QString &identifier,
                     const ParseResult &parseResult)
    : out(out), inputFileName(inputFileName), identifier(identifier), parseResult(parseResult) {}

static QByteArray escapeString(const QByteArray &bytes) {
    QByteArray res;
    res.reserve(bytes.size());
    for (const auto &ch : bytes) {
        if (ch >= 32 && ch <= 126) {
            res += ch;
            continue;
        }
        switch (ch) {
            case '\\':
                res += R"(\\)";
                continue;
            case '\'':
                res += R"(\')";
                continue;
            case '\"':
                res += R"(\")";
                continue;
            default:
                break;
        }
        QString hexStr = QString::number(static_cast<unsigned char>(ch), 16).toUpper();
        if (hexStr.length() < 2) {
            hexStr.prepend(QChar('0'));
        }
        res += "\\x";
        res += hexStr.toLatin1();
    }
    return res;
}

#define escPrintable(STR) escapeString((STR).toLocal8Bit()).constData()

#define STRING_4_SPACE  "    "
#define STRING_8_SPACE  "        "
#define STRING_12_SPACE "            "
#define STRING_16_SPACE "                "

#define GENERATE_STRING(NAME, VAR)                                                                 \
    fprintf(out, STRING_12_SPACE "// " #NAME "\n");                                                \
    fprintf(out, STRING_12_SPACE "QStringLiteral(\"%s\"),\n", escPrintable(VAR));

#define GENERATE_ENUM(NAME, SCOPE, VALUE)                                                          \
    fprintf(out, STRING_12_SPACE "// " #NAME "\n");                                                \
    fprintf(out, STRING_12_SPACE SCOPE "::%s,\n", qPrintable(VALUE));

#define GENERATE_BOOL(NAME, VALUE)                                                                 \
    fprintf(out, STRING_12_SPACE "// " #NAME "\n");                                                \
    fprintf(out, STRING_12_SPACE "%s,\n", VALUE ? "true" : "false");

class GeneratorPrivate {
public:
    GeneratorPrivate(Generator &q) : q(q) {}

    Generator &q;

    void generateItems(FILE *out, const QVector<ActionItemInfoMessage> &objects) {
        int i = 0;
        for (const auto &item : std::as_const(objects)) {
            fprintf(out, STRING_8_SPACE "{\n");
            fprintf(out, STRING_12_SPACE "// index %d\n", i++);

            GENERATE_STRING(id, item.id);
            GENERATE_ENUM(type, "ActionItemInfo", itemInfoTypeToString(item.type));
            GENERATE_STRING(text, item.text);
            GENERATE_STRING(actionClass, item.actionClass);
            GENERATE_STRING(description, item.description);
            GENERATE_STRING(icon, item.icon);

            // shortcuts
            fprintf(out, STRING_12_SPACE "// shortcuts\n");
            fprintf(out, STRING_12_SPACE "{\n");
            for (const auto &key : std::as_const(item.shortcutTokens)) {
                fprintf(out, STRING_16_SPACE "QKeySequence(QStringLiteral(\"%s\")),\n",
                        escPrintable(key.trimmed()));
            }
            fprintf(out, STRING_12_SPACE "},\n");

            GENERATE_STRING(catalog, item.catalog);
            GENERATE_BOOL(topLevel, item.topLevel);

            // attributes
            fprintf(out, STRING_12_SPACE "// attributes\n");
            fprintf(out, STRING_12_SPACE "{\n");
            for (auto it = item.attributes.begin(); it != item.attributes.end(); ++it) {
                fprintf(out,
                        STRING_16_SPACE "{ QStringLiteral(\"%s\"), QStringLiteral(\"%s\") },\n",
                        escPrintable(it.key()), escPrintable(it.value()));
            }
            fprintf(out, STRING_12_SPACE "},\n");

            // children
            fprintf(out, STRING_12_SPACE "// children\n");
            fprintf(out, STRING_12_SPACE "{\n");
            for (const auto &child : std::as_const(item.children)) {
                fprintf(out,
                        STRING_16_SPACE "{ QStringLiteral(\"%s\"), ActionLayoutEntry::%s },\n",
                        escPrintable(child.id), qPrintable(layoutEntryTypeToString(child.type)));
            }
            fprintf(out, STRING_12_SPACE "},\n");

            fprintf(out, STRING_8_SPACE "},\n");
        }
    }

    void generateInsertions(FILE *out, const QVector<ActionInsertionMessage> &routines) {
        int i = 0;
        for (const auto &item : std::as_const(routines)) {
            fprintf(out, STRING_8_SPACE "{\n");
            fprintf(out, "            // index %d\n", i++);

            GENERATE_ENUM(anchor, "ActionInsertion", insertionAnchorToString(item.anchor));
            GENERATE_STRING(target, item.target);
            GENERATE_STRING(relativeTo, item.relativeTo);

            // children
            fprintf(out, STRING_12_SPACE "// items\n");
            fprintf(out, STRING_12_SPACE "{\n");
            for (const auto &item : std::as_const(item.items)) {
                fprintf(out,
                        STRING_16_SPACE "{ QStringLiteral(\"%s\"), ActionLayoutEntry::%s },\n",
                        escPrintable(item.id), qPrintable(layoutEntryTypeToString(item.type)));
            }
            fprintf(out, STRING_12_SPACE "},\n");

            fprintf(out, STRING_8_SPACE "},\n");
        }
    }

    void generateTranslations(FILE *out, const QVector<ActionItemInfoMessage> &items) {
        {
            QSet<QString> texts{{}};
            fprintf(out, "    // Action Text\n");
            for (const auto &item : std::as_const(items)) {
                if (item.text.isEmpty() || texts.contains(item.text))
                    continue;
                texts.insert(item.text);

                QString ctx;
                if (auto it = item.attributes.find(QStringLiteral("textTr"));
                    it != item.attributes.end()) {
                    ctx = it.value();
                }
                if (ctx.isEmpty()) {
                    ctx = q.parseResult.textTranslationContext;
                }

                fprintf(out, STRING_4_SPACE "QCoreApplication::translate(\"%s\", \"%s\");\n",
                        qPrintable(ctx), escPrintable(item.text));
            }
            fprintf(out, "\n");
        }

        {
            QSet<QString> actionClasses{{}};
            fprintf(out, STRING_4_SPACE "// Action Class\n");
            for (const auto &item : std::as_const(items)) {
                if (item.actionClass.isEmpty() || actionClasses.contains(item.actionClass))
                    continue;
                actionClasses.insert(item.text);

                QString ctx;
                if (auto it = item.attributes.find(QStringLiteral("classTr"));
                    it != item.attributes.end()) {
                    ctx = it.value();
                }
                if (ctx.isEmpty()) {
                    ctx = q.parseResult.classTranslationContext;
                }

                fprintf(out, STRING_4_SPACE "QCoreApplication::translate(\"%s\", \"%s\");\n",
                        qPrintable(ctx), escPrintable(item.actionClass));
            }
            fprintf(out, "\n");
        }

        {
            QSet<QString> descriptions{{}};
            fprintf(out, "    // Action Description\n");
            for (const auto &item : std::as_const(items)) {
                if (item.description.isEmpty() || descriptions.contains(item.description))
                    continue;
                descriptions.insert(item.description);

                QString ctx;
                if (auto it = item.attributes.find(QStringLiteral("descriptionTr"));
                    it != item.attributes.end()) {
                    ctx = it.value();
                }
                if (ctx.isEmpty()) {
                    ctx = q.parseResult.descriptionTranslationContext;
                }

                fprintf(out, STRING_4_SPACE "QCoreApplication::translate(\"%s\", \"%s\");\n",
                        qPrintable(ctx), escPrintable(item.description));
            }
            fprintf(out, "\n");
        }
    }

    void generateExtraInformation(FILE *out, const QVector<ActionItemInfoMessage> &objects) {
        QVector<ActionItemInfoMessage> actions;
        QVector<ActionItemInfoMessage> groups;
        QVector<ActionItemInfoMessage> menus;
        QVector<ActionItemInfoMessage> phonies;

        for (const auto &item : objects) {
            if (item.type == QAK::ActionItemInfo::Action) {
                actions.append(item);
            } else if (item.type == QAK::ActionItemInfo::Group) {
                groups.append(item);
            } else if (item.type == QAK::ActionItemInfo::Menu) {
                menus.append(item);
            } else if (item.type == QAK::ActionItemInfo::Phony) {
                phonies.append(item);
            }
        }

        fprintf(out, "/*\n");

        // Actions
        fprintf(out, STRING_4_SPACE "[Action]\n");
        for (const auto &item : std::as_const(actions)) {
            fprintf(out, STRING_4_SPACE "%s\n", qPrintable(item.id));
        }
        fprintf(out, "\n");

        // // Groups
        fprintf(out, STRING_4_SPACE "[Group]\n");
        for (const auto &item : std::as_const(groups)) {
            fprintf(out, STRING_4_SPACE "%s\n", qPrintable(item.id));
        }
        fprintf(out, "\n");

        // Menus
        fprintf(out, STRING_4_SPACE "[Menu]\n");
        for (const auto &item : std::as_const(menus)) {
            fprintf(out, STRING_4_SPACE "%s\n", qPrintable(item.id));
        }
        fprintf(out, "\n");

        // Phonies
        fprintf(out, STRING_4_SPACE "[Phony]\n");
        for (const auto &item : std::as_const(phonies)) {
            fprintf(out, "    %s\n", item.id.toLocal8Bit().data());
        }
        fprintf(out, "\n");

        fprintf(out, "*/\n");
    }

    void generate() {
        auto &msg = q.parseResult.extension;
        auto out = q.out;

        // Warning
        fprintf(out,
                "/****************************************************************************\n"
                "** Action extension structure code from reading XML file '%s'\n**\n",
                qPrintable(q.inputFileName));
        fprintf(out, "** Created by: QActionKit Action Extension Compiler version %s (Qt %s)\n**\n",
                APP_VERSION, QT_VERSION_STR);
        fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
                     "*************************************************************************"
                     "****/\n");

        // Headers
        fprintf(out, R"(
#include <QtCore/QString>
#include <QtCore/QCoreApplication>

#include <QAKCore/private/actionextension_p.h>

)");

        fprintf(out, "namespace qakStaticActionExtension_%s {\n", qPrintable(q.identifier));

        fprintf(out, R"(
using namespace QAK;

static ActionExtensionData *get_data() {
    static ActionExtensionData data;
)");

        fprintf(out, STRING_4_SPACE "data.version = QStringLiteral(\"%s\");\n",
                escPrintable(msg.version));
        fprintf(out, STRING_4_SPACE "data.id = QStringLiteral(\"%s\");\n", escPrintable(msg.id));
        fprintf(out, STRING_4_SPACE "data.hash = QStringLiteral(\"%s\");\n", qPrintable(msg.hash));
        fprintf(out, "\n");

        if (msg.items.isEmpty()) {
            fprintf(out, STRING_4_SPACE "data.items = nullptr;\n");
            fprintf(out, STRING_4_SPACE "data.itemCount = 0;\n");
        } else {
            fprintf(out, STRING_4_SPACE "static ActionItemInfoData staticItems[] = {\n");
            generateItems(out, msg.items);
            fprintf(out, STRING_4_SPACE "};\n");
            fprintf(out, STRING_4_SPACE "data.items = staticItems;\n");
            fprintf(out, STRING_4_SPACE
                    "data.itemCount = sizeof(staticItems) / sizeof(staticItems[0]);\n");
        }
        fprintf(out, "\n");

        if (msg.insertions.isEmpty()) {
            fprintf(out, STRING_4_SPACE "data.insertions = nullptr;\n");
            fprintf(out, STRING_4_SPACE "data.insertionCount = 0;\n");
        } else {
            fprintf(out, STRING_4_SPACE "static ActionInsertionData staticInsertions[] = {\n");
            generateInsertions(out, msg.insertions);
            fprintf(out, STRING_4_SPACE "};\n");
            fprintf(out, STRING_4_SPACE "data.insertions = staticInsertions;\n");
            fprintf(out, STRING_4_SPACE "data.insertionCount = sizeof(staticInsertions) / "
                                        "sizeof(staticInsertions[0]);\n");
        }
        fprintf(out, "\n");

        fprintf(out, R"(    return &data;
}

}

)");

        fprintf(out,
                "const QAK::ActionExtension "
                "*QT_MANGLE_NAMESPACE(qakGetStaticActionExtension_%s)() {\n",
                qPrintable(q.identifier));
        fprintf(out, STRING_4_SPACE "static QAK::ActionExtension extension{\n");
        fprintf(out, STRING_8_SPACE "{\n");
        fprintf(out, STRING_12_SPACE "qakStaticActionExtension_%s::get_data(),\n",
                qPrintable(q.identifier));
        fprintf(out, STRING_8_SPACE "},\n");
        fprintf(out, STRING_4_SPACE "};\n");

        fprintf(out, R"(    return &extension;
}

#if 0
// This field is only used to generate translation files for the Qt linguist tool
)");

        fprintf(out, "static void qakStaticActionTranslations_%s() {\n", qPrintable(q.identifier));
        generateTranslations(out, msg.items);
        fprintf(out, R"(}
#endif
)");
        fprintf(out, "\n");

        // Extra information
        generateExtraInformation(out, msg.items);
    }
};

void Generator::generate() {
    GeneratorPrivate d(*this);
    d.generate();
}