#include "generator.h"

#include <QtCore/QSet>

#include <QAKCore/private/actionextension_p.h>

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

static QByteArray escapeString(const QByteArray &bytes) {
    QByteArray res;
    res.reserve(bytes.size());
    for (const auto &ch : bytes) {
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
        if (ch >= 32 && ch <= 126) {
            res += ch;
            continue;
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

// The generated file is compiled as UTF-8, and the escaped bytes are placed in QStringLiteral,
// which decodes them as UTF-8. Encoding in the local 8-bit codec would corrupt non-ASCII text on
// a system whose locale codec is not UTF-8.
#define escPrintable(STR) escapeString((STR).toUtf8()).constData()

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

#define GENERATE_INT(NAME, VALUE)                                                                  \
    fprintf(out, STRING_12_SPACE "// " #NAME "\n");                                                \
    fprintf(out, STRING_12_SPACE "%d,\n", VALUE);

static void writeBanner(FILE *out, const QString &inputFileName) {
    fprintf(out,
            "/****************************************************************************\n"
            "** Action extension structure code from reading XML file '%s'\n**\n",
            qPrintable(inputFileName));
    fprintf(out, "** Created by: QActionKit Action Extension Compiler version %s (Qt %s)\n**\n",
            APP_VERSION, QT_VERSION_STR);
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n"
                 "*************************************************************************"
                 "****/\n");
}

// Writes the declaration of the function that returns the extension, preceded by the header that
// defines the export directive.
static void writeDeclaration(FILE *out, const Generator &q) {
    if (!q.exportFileName.isEmpty()) {
        fprintf(out, "#include <%s>\n\n", qPrintable(q.exportFileName));
    }
    const char *indent = "";
    if (!q.nameSpace.isEmpty()) {
        fprintf(out, "namespace %s {\n", qPrintable(q.nameSpace));
        indent = STRING_4_SPACE;
    }
    fprintf(out, "%s", indent);
    if (!q.exportDirective.isEmpty()) {
        fprintf(out, "%s ", qPrintable(q.exportDirective));
    }
    fprintf(out, "const QAK::ActionExtension *%s();\n", qPrintable(q.function));
    if (!q.nameSpace.isEmpty()) {
        fprintf(out, "}\n");
    }
}

void Generator::generateHeader(FILE *header) const {
    writeBanner(header, inputFileName);
    fprintf(header, "\n#pragma once\n\n");
    fprintf(header, "#include <QAKCore/actionextension.h>\n\n");
    writeDeclaration(header, *this);
}

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
            GENERATE_STRING(category, item.category);
            GENERATE_STRING(description, item.description);
            GENERATE_STRING(textContext, item.textContext);
            GENERATE_STRING(categoryContext, item.categoryContext);
            GENERATE_STRING(descriptionContext, item.descriptionContext);
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
            GENERATE_BOOL(external, item.external);

            // attributes
            fprintf(out, STRING_12_SPACE "// attributes\n");
            fprintf(out, STRING_12_SPACE "{\n");
            for (auto it = item.attributes.begin(); it != item.attributes.end(); ++it) {
                fprintf(out,
                        STRING_16_SPACE "{ ActionAttributeKey(QStringLiteral(\"%s\"), QStringLiteral(\"%s\")), QStringLiteral(\"%s\") },\n",
                        escPrintable(it.key().name), escPrintable(it.key().namespaceUri), escPrintable(it.value()));
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
            GENERATE_INT(priority, item.priority);

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

    // Writes one call of QCoreApplication::translate() for each distinct pair of a string and its
    // context, which is chosen as ActionItemInfo chooses it at run time, so that lupdate extracts
    // the strings in the contexts where they are looked up.
    static void generateTranslationCalls(FILE *out, const char *title,
                                         const QVector<ActionItemInfoMessage> &items,
                                         QString ActionItemInfoMessage::*string,
                                         QString ActionItemInfoMessage::*itemContext,
                                         const QString &extensionContext,
                                         const char *defaultContext) {
        QSet<QPair<QString, QString>> written;
        fprintf(out, STRING_4_SPACE "// %s\n", title);
        for (const auto &item : std::as_const(items)) {
            const QString &source = item.*string;
            if (source.isEmpty())
                continue;

            const QString context = !(item.*itemContext).isEmpty() ? item.*itemContext
                                    : !extensionContext.isEmpty()
                                        ? extensionContext
                                        : QString::fromUtf8(defaultContext);
            if (written.contains({context, source}))
                continue;
            written.insert({context, source});

            fprintf(out, STRING_4_SPACE "QCoreApplication::translate(\"%s\", \"%s\");\n",
                    escPrintable(context), escPrintable(source));
        }
        fprintf(out, "\n");
    }

    void generateTranslations(FILE *out, const QVector<ActionItemInfoMessage> &items) {
        const auto &result = q.parseResult;
        generateTranslationCalls(out, "Action Text", items, &ActionItemInfoMessage::text,
                                 &ActionItemInfoMessage::textContext, result.textTranslationContext,
                                 QAK::ActionExtensionData::defaultTextContext);
        generateTranslationCalls(out, "Action Category", items, &ActionItemInfoMessage::category,
                                 &ActionItemInfoMessage::categoryContext,
                                 result.categoryTranslationContext,
                                 QAK::ActionExtensionData::defaultCategoryContext);
        generateTranslationCalls(
            out, "Action Description", items, &ActionItemInfoMessage::description,
            &ActionItemInfoMessage::descriptionContext, result.descriptionTranslationContext,
            QAK::ActionExtensionData::defaultDescriptionContext);
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

        // Groups
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

        writeBanner(out, q.inputFileName);

        // Headers. Without a generated header, the declaration is written here, because the
        // definition below may be qualified by a namespace.
        fprintf(out, "\n");
        if (!q.headerInclude.isEmpty()) {
            fprintf(out, "#include \"%s\"\n\n", qPrintable(q.headerInclude));
        }
        fprintf(out, R"(#include <QtCore/QString>
#include <QtCore/QCoreApplication>

#include <QAKCore/private/actionextension_p.h>

)");
        if (q.headerInclude.isEmpty()) {
            writeDeclaration(out, q);
            fprintf(out, "\n");
        }

        fprintf(out, "namespace {\n");

        fprintf(out, R"(
using namespace QAK;

static ActionExtensionData *get_data() {
    static ActionExtensionData data;
)");

        fprintf(out, STRING_4_SPACE "data.version = QStringLiteral(\"%s\");\n",
                escPrintable(msg.version));
        fprintf(out, STRING_4_SPACE "data.id = QStringLiteral(\"%s\");\n", escPrintable(msg.id));
        fprintf(out, STRING_4_SPACE "data.hash = QStringLiteral(\"%s\");\n", qPrintable(msg.hash));
        fprintf(out, STRING_4_SPACE "data.textContext = QStringLiteral(\"%s\");\n",
                escPrintable(q.parseResult.textTranslationContext));
        fprintf(out, STRING_4_SPACE "data.categoryContext = QStringLiteral(\"%s\");\n",
                escPrintable(q.parseResult.categoryTranslationContext));
        fprintf(out, STRING_4_SPACE "data.descriptionContext = QStringLiteral(\"%s\");\n",
                escPrintable(q.parseResult.descriptionTranslationContext));
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

        const QString qualifiedFunction =
            q.nameSpace.isEmpty() ? q.function : q.nameSpace + QStringLiteral("::") + q.function;
        fprintf(out, "const QAK::ActionExtension *%s() {\n", qPrintable(qualifiedFunction));
        fprintf(out, STRING_4_SPACE "static QAK::ActionExtension extension{\n");
        fprintf(out, STRING_8_SPACE "{\n");
        fprintf(out, STRING_12_SPACE "get_data(),\n");
        fprintf(out, STRING_8_SPACE "},\n");
        fprintf(out, STRING_4_SPACE "};\n");

        fprintf(out, R"(    return &extension;
}

#if 0
// This field is only used to generate translation files for the Qt linguist tool
)");

        fprintf(out, "static void qakActionTranslations() {\n");
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
