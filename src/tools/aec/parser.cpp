#include "parser.h"

#include <algorithm>
#include <utility>

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringView>
#include <QtCore/QFile>

#include <qmxmladaptor/qmxmladaptor.h>
#include <stdcorelib/linked_map.h>
#include <util/util.h>

void error(const char *fmt, ...) {
    fprintf(stderr, "%s: ", qPrintable(qApp->applicationName()));

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    // fprintf(stderr, "\n");
}

static QString calculateContentSha256(const QByteArray &data) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return hash.result().toHex();
}

static QString itemIdToText(const QString &id) {
    QStringList parts;
    QString currentPart;

    QStringView idView(id);
    auto dotIdx = id.lastIndexOf('.');
    if (dotIdx >= 0 && dotIdx < id.size() - 1) {
        idView = idView.mid(dotIdx + 1);
    }
    for (const auto &ch : std::as_const(idView)) {
        if (ch.isUpper() && !currentPart.isEmpty()) {
            parts.append(currentPart);
            currentPart.clear();
        }
        currentPart += currentPart.isEmpty() ? ch.toUpper() : ch;
    }
    if (!currentPart.isEmpty()) {
        parts.append(currentPart);
    }

    // Lower case special words
    static QSet<QString> specialWords{
        QStringLiteral("and"), QStringLiteral("but"),  QStringLiteral("or"), QStringLiteral("nor"),
        QStringLiteral("for"), QStringLiteral("yet"),  QStringLiteral("so"), QStringLiteral("as"),
        QStringLiteral("at"),  QStringLiteral("by"),   QStringLiteral("in"), QStringLiteral("of"),
        QStringLiteral("on"),  QStringLiteral("to"),   QStringLiteral("up"), QStringLiteral("a"),
        QStringLiteral("an"),  QStringLiteral("the "),
    };
    for (auto &part : parts) {
        if (auto lower = part.toLower(); specialWords.contains(lower)) {
            part = lower;
        }
    }
    return parts.join(QChar(' '));
}

static QString simplifyActionText(const QString &s) {
    QString res;

    for (int i = 0; i < s.size(); ++i) {
        const QChar &ch = s[i];
        if (ch == '&') {
            if (i + 1 < s.size()) {
                i++;
                res += s[i];
            }
            continue;
        }
        res += ch;
    }
    if (res.endsWith(QStringLiteral("..."))) {
        res.chop(3);
    }
    return res;
}

static QStringList parseStringList(const QString &s) {
    QStringList parts;
    QString currentPart;

    for (int i = 0; i < s.size(); ++i) {
        const QChar &ch = s[i];
        if (ch == '\\') {
            if (i + 1 < s.size()) {
                i++;
                currentPart += s[i];
            }
            continue;
        }
        if (ch == ';') {
            parts.append(currentPart);
            currentPart.clear();
            continue;
        }
        currentPart += ch;
    }
    parts.append(currentPart);
    return parts;
}

static inline bool isStringDigits(const QString &s) {
    return std::all_of(s.begin(), s.end(), [](const QChar &ch) { return ch.isDigit(); });
}

struct ParserPrivate {
    Parser &q;
    ParserPrivate(Parser &q) : q(q) {}

    // Configuration
    QString defaultCatalog;

    // Parse configuration, fix parser members
    void parseConfiguration(const QMXmlAdaptorElement &e) {
        for (const auto &item : e.children) {
            if (item->name == QStringLiteral("defaultCatalog")) {
                defaultCatalog = resolve(item->value);
                continue;
            }

            if (item->name == QStringLiteral("translationContext")) {
                auto &properties = item->properties;
                if (auto it = properties.find(QStringLiteral("text")); it != properties.end()) {
                    result.textTranslationContext = resolve(it.value());
                }
                if (auto it = properties.find(QStringLiteral("class")); it != properties.end()) {
                    result.classTranslationContext = resolve(it.value());
                }
                if (auto it = properties.find(QStringLiteral("description"));
                    it != properties.end()) {
                    result.descriptionTranslationContext = resolve(it.value());
                }
                continue;
            }

            if (item->name == QStringLiteral("vars")) {
                for (const auto &subItem : item->children) {
                    auto key = resolve(subItem->properties.value(QStringLiteral("key")));
                    auto value = resolve(subItem->properties.value(QStringLiteral("value")));
                    if (!key.isEmpty()) {
                        q.variables.insert(key, value);
                    }
                }
            }
        }
    }

    // Intermediate data
    stdc::linked_map<QString, ActionItemInfoMessage> itemInfoMap;

    ParseResult result;

    inline QString resolve(const QString &s) const { return Util::parseExpression(s, q.variables); }

    // Infer item type when parsing item info
    void parseItemAttrs(const QMXmlAdaptorElement &e, ActionItemInfoMessage &info,
                        const char *field) const {
        const auto &name = e.name;
        if (name == QStringLiteral("action")) {
            info.type = QAK::ActionItemInfo::Action;
        } else if (name == QStringLiteral("group")) {
            info.type = QAK::ActionItemInfo::Group;
        } else if (name == QStringLiteral("menu")) {
            info.type = QAK::ActionItemInfo::Menu;
            if (resolve(e.properties.value(QStringLiteral("topLevel"))) == QStringLiteral("true")) {
                info.topLevel = true;
            }
        } else if (name == QStringLiteral("menuBar") || name == QStringLiteral("toolBar")) {
            info.type = QAK::ActionItemInfo::Menu;
            info.topLevel = true;
        } else if (name == QStringLiteral("phony")) {
            info.type = QAK::ActionItemInfo::Phony;
        } else {
            error("%s: %s item \"%s\" has an unknown tag \"%s\"\n", qPrintable(q.fileName), field,
                  qPrintable(info.id), qPrintable(e.name));
            std::exit(1);
        }

        info.tag = e.name;

        for (auto it = e.properties.begin(); it != e.properties.end(); ++it) {
            static const QSet<QString> reservedKeys = {
                QStringLiteral("id"),       QStringLiteral("text"),
                QStringLiteral("class"),    QStringLiteral("description"),
                QStringLiteral("catalog"),  QStringLiteral("shortcuts"),
                QStringLiteral("shortcut"), QStringLiteral("topLevel"),
                QStringLiteral("icon"),
            };
            const auto &key = it.key();
            if (reservedKeys.contains(key)) {
                continue;
            }
            info.attributes.insert(key, resolve(it.value()));
        }
    }

    ActionItemInfoMessage parseItem(const QMXmlAdaptorElement &e) {
        ActionItemInfoMessage info;
        auto id = resolve(e.properties.value(QStringLiteral("id")));
        if (id.isEmpty()) {
            error("%s: item element \"%s\" doesn't have an \"id\" field\n", qPrintable(q.fileName),
                  qPrintable(e.name));
            std::exit(1);
        }
        info.id = id;

        // type
        parseItemAttrs(e, info, "item");

        // text
        if (auto text = resolve(e.properties.value(QStringLiteral("text"))); !text.isEmpty()) {
            info.text = text;
        } else {
            info.text = itemIdToText(info.id);
        }

        // class
        if (auto actionClass = resolve(e.properties.value(QStringLiteral("class")));
            !actionClass.isEmpty()) {
            info.actionClass = actionClass;
        }

        // class
        if (auto description = resolve(e.properties.value(QStringLiteral("description")));
            !description.isEmpty()) {
            info.description = description;
        }

        // icon
        if (auto icon = resolve(e.properties.value(QStringLiteral("icon"))); !icon.isEmpty()) {
            info.icon = icon;
        } else {
            info.icon = info.id;
        }

        // shortcuts
        if (auto shortcuts = resolve(e.properties.value(QStringLiteral("shortcuts")));
            !shortcuts.isEmpty()) {
            info.shortcutTokens = parseStringList(shortcuts);
        } else if (shortcuts = resolve(e.properties.value(QStringLiteral("shortcut")));
                   !shortcuts.isEmpty()) {
            info.shortcutTokens = parseStringList(shortcuts);
        }

        // catalog
        if (auto catalog = resolve(e.properties.value(QStringLiteral("catalog")));
            !catalog.isEmpty()) {
            info.catalog = resolve(catalog);
        }

        if (!e.children.isEmpty()) {
            error("%s: item declaration element \"%s\" shouldn't have children\n",
                  qPrintable(q.fileName), qPrintable(id));
            std::exit(1);
        }
        return info;
    }

    // Create new item when encountering a new ID in layouts or insertions
    ActionItemInfoMessage &findOrInsertItemInfo(const QMXmlAdaptorElement *e,
                                                const QString &parentId, const char *field) {
        auto id = resolve(e->properties.value(QStringLiteral("id")));
        if (id.isEmpty()) {
            error("%s: %s element \"%s\" doesn't have an \"id\" field\n", qPrintable(q.fileName),
                  field, qPrintable(e->name));
            std::exit(1);
        }

        const auto &errorPhony = [this, id, field]() {
            error("%s: %s element \"%s\" shouldn't have a phony type\n", qPrintable(q.fileName),
                  field, qPrintable(id));
            std::exit(1);
        };

        ActionItemInfoMessage *pInfo;
        if (auto it = itemInfoMap.find(id.toLower()); it != itemInfoMap.end()) {
            // This layout item has been declared in the items field
            auto &info = it.value();

            // Check if the tag matches
            bool typeMismatch = false;
            switch (info.type) {
                case QAK::ActionItemInfo::Phony: {
                    errorPhony();
                    break;
                }
                case QAK::ActionItemInfo::Action: {
                    if (e->name != QStringLiteral("action")) {
                        typeMismatch = true;
                    }
                    break;
                }
                case QAK::ActionItemInfo::Group:
                case QAK::ActionItemInfo::Menu: {
                    static const QSet<QString> allowedTags = {
                        QStringLiteral("group"),
                        QStringLiteral("menu"),
                        QStringLiteral("menuBar"),
                        QStringLiteral("toolBar"),
                    };
                    if (!allowedTags.contains(e->name)) {
                        typeMismatch = true;
                    }
                    break;
                }
                default:
                    Q_UNREACHABLE();
                    break;
            }
            if (typeMismatch) {
                error("%s: %s element \"%s\" has inconsistent tag \"%s\" with the "
                      "item element \"%s\"\n",
                      qPrintable(q.fileName), field, qPrintable(id), qPrintable(e->name),
                      qPrintable(info.tag));
                std::exit(1);
            }

            if (info.catalog.isEmpty()) {
                // The item doesn't have a specified category, use the current one
                info.catalog = parentId;
            }
            pInfo = &info;
        } else {
            // Create one
            ActionItemInfoMessage info;
            info.id = id;
            parseItemAttrs(*e, info, field);
            if (info.type == QAK::ActionItemInfo::Phony) {
                errorPhony();
            }
            info.text = itemIdToText(id);
            info.catalog = parentId;

            auto insertResult = itemInfoMap.append(id.toLower(), info);
            pInfo = &insertResult.first.value();
        }
        return *pInfo;
    }

    ActionLayoutEntryMessage parseLayoutRecursively(const QMXmlAdaptorElement *e,
                                                    const QString &parentId, QStringList &path) {
        const auto &checkChildren = [this, e](const char *typeName) {
            if (!e->children.isEmpty()) {
                error("%s: layout element of %s type shouldn't have children\n",
                      qPrintable(q.fileName), typeName);
                std::exit(1);
            }
        };

        ActionLayoutEntryMessage entry;
        if (e->name == QStringLiteral("separator")) {
            checkChildren("separator");
            entry.type = QAK::ActionLayoutEntry::Separator;
            return entry;
        } else if (e->name == QStringLiteral("stretch")) {
            checkChildren("stretch");
            entry.type = QAK::ActionLayoutEntry::Stretch;
            return entry;
        }

        auto &info = findOrInsertItemInfo(e, parentId, "layout");
        QString id = info.id;

        // Recursive path chain detected?
        if (path.contains(id)) {
            error("%s: recursive chain in layout: %s\n", qPrintable(q.fileName),
                  qPrintable((QStringList(path) << id).join(", ")));
            std::exit(1);
        }
        entry.id = id;

        switch (info.type) {
            case QAK::ActionItemInfo::Action: {
                entry.type = QAK::ActionLayoutEntry::Action;
                checkChildren(qPrintable(QString(R"("%1")").arg(id)));
                return entry;
            }
            case QAK::ActionItemInfo::Menu:
            case QAK::ActionItemInfo::Group: {
                if (e->name == QStringLiteral("menu")) {
                    entry.type = QAK::ActionLayoutEntry::Menu;
                } else {
                    entry.type = QAK::ActionLayoutEntry::Group;
                }
                break;
            }
            default: {
                Q_UNREACHABLE();
                break;
            }
        }

        if (!e->children.isEmpty()) {
            if (!info.children.isEmpty()) {
                error("%s: layout element \"%s\" has been declared more than once\n",
                      qPrintable(q.fileName), qPrintable(id));
                std::exit(1);
            }

            path << id;

            QVector<ActionLayoutEntryMessage> children;
            for (const auto &child : e->children) {
                children.append(parseLayoutRecursively(child.data(), id, path));
            }

            path.removeLast();

            info.children = std::move(children);
        }
        return entry;
    }

    ActionInsertionMessage parseInsertion(const QMXmlAdaptorElement &root) {
        if (const auto &rootName = root.name; rootName != QStringLiteral("insertion")) {
            error("%s: unknown insertion element tag \"%s\"\n", qPrintable(q.fileName),
                  qPrintable(rootName));
            std::exit(1);
        }

        auto target = resolve(root.properties.value(QStringLiteral("target")));
        if (target.isEmpty()) {
            error("%s: insertion doesn't have a target\n", qPrintable(q.fileName));
            std::exit(1);
        }

        auto anchorToken = root.properties.value(QStringLiteral("anchor"));
        QAK::ActionInsertion::Anchor anchor;
        bool needRelative = false;
        if (anchorToken == QStringLiteral("last") || anchorToken == QStringLiteral("back")) {
            anchor = QAK::ActionInsertion::Last;
        } else if (anchorToken == QStringLiteral("first") ||
                   anchorToken == QStringLiteral("front")) {
            anchor = QAK::ActionInsertion::First;
        } else if (anchorToken == QStringLiteral("before")) {
            anchor = QAK::ActionInsertion::Before;
            needRelative = true;
        } else if (anchorToken.isEmpty() || anchorToken == QStringLiteral("after")) {
            anchor = QAK::ActionInsertion::After;
            needRelative = true;
        } else {
            error("%s: unknown insertion anchor \"%s\"\n", qPrintable(q.fileName),
                  qPrintable(anchorToken));
            std::exit(1);
        }

        auto relative = resolve(root.properties.value(QStringLiteral("relativeTo")));
        if (needRelative && relative.isEmpty()) {
            error("%s: insertion with anchor \"%s\" must have a relative sibling\n",
                  qPrintable(q.fileName), qPrintable(anchorToken));
            std::exit(1);
        }

        ActionInsertionMessage insertion;
        insertion.anchor = anchor;
        insertion.target = target;
        insertion.relativeTo = relative;

        if (root.children.isEmpty()) {
            error("%s: empty insertion\n", qPrintable(q.fileName));
            std::exit(1);
        }

        for (const auto &item : root.children) {
            auto &e = *item;

            ActionLayoutEntryMessage entry;
            if (e.name == QStringLiteral("separator")) {
                entry.type = QAK::ActionLayoutEntry::Separator;
            } else if (e.name == QStringLiteral("stretch")) {
                entry.type = QAK::ActionLayoutEntry::Stretch;
            } else {
                auto &info = findOrInsertItemInfo(&e, defaultCatalog, "insertion");
                auto id = info.id;
                if (!e.children.isEmpty()) {
                    error("%s: insertion element \"%s\" shouldn't have children\n",
                          qPrintable(q.fileName), qPrintable(id));
                    std::exit(1);
                }
                entry.id = id;

                switch (info.type) {
                    case QAK::ActionItemInfo::Action: {
                        entry.type = QAK::ActionLayoutEntry::Action;
                        break;
                    }
                    case QAK::ActionItemInfo::Menu:
                    case QAK::ActionItemInfo::Group: {
                        if (e.name == QStringLiteral("menu")) {
                            entry.type = QAK::ActionLayoutEntry::Menu;
                        } else {
                            entry.type = QAK::ActionLayoutEntry::Group;
                        }
                        break;
                    }
                    default:
                        Q_UNREACHABLE();
                        break;
                }
            }
            insertion.items.append(entry);
        }
        return insertion;
    }

    void parse(const QByteArray &data) {
        QMXmlAdaptor xml;

        // Read file
        if (!xml.loadData(data)) {
            error("%s: invalid format\n", qPrintable(q.fileName));
            std::exit(1);
        }

        // Check root name
        const auto &root = xml.root;
        if (const auto &rootName = root.name; rootName != QStringLiteral("actionExtension")) {
            error("%s: unknown root element tag \"%s\"\n", qPrintable(q.fileName),
                  qPrintable(rootName));
            std::exit(1);
        }

        QList<QMXmlAdaptorElement *> objElements;
        QList<QMXmlAdaptorElement *> layoutElements;
        QList<QMXmlAdaptorElement *> insertionElements;

        // Collect elements and attributes
        QString version;
        QString id;
        bool hasParserConfig = false;
        for (const auto &item : std::as_const(root.children)) {
            if (item->name == QStringLiteral("items")) {
                for (const auto &subItem : std::as_const(item->children)) {
                    objElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("layouts")) {
                for (const auto &subItem : std::as_const(item->children)) {
                    layoutElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("insertions")) {
                for (const auto &subItem : std::as_const(item->children)) {
                    insertionElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("id")) {
                id = item->value;
                continue;
            }
            if (item->name == QStringLiteral("version")) {
                version = item->value;
                continue;
            }
            if (item->name == QStringLiteral("configuration")) {
                if (hasParserConfig) {
                    error("%s: duplicated configuration elements\n", qPrintable(q.fileName));
                    std::exit(1);
                }
                parseConfiguration(*item);
                hasParserConfig = true;
                continue;
            }
        }

        if (version.isEmpty()) {
            version = QStringLiteral(PARSER_VERSION);
        }

        if (id.isEmpty()) {
            error("%s: extension version is not specified\n", qPrintable(q.fileName));
            std::exit(1);
        }

        // Build result
        result.extension.version = version;
        result.extension.id = id;
        result.extension.hash = calculateContentSha256(data);

        // Parse items
        for (const auto &item : std::as_const(objElements)) {
            auto entity = parseItem(*item);
            if (itemInfoMap.contains(entity.id.toLower())) {
                error("%s: duplicated item id %s\n", qPrintable(q.fileName), qPrintable(entity.id));
                std::exit(1);
            }
            std::ignore = itemInfoMap.append(entity.id.toLower(), entity);
        }

        // Parse layouts
        for (const auto &item : std::as_const(layoutElements)) {
            QStringList path;
            std::ignore = parseLayoutRecursively(item, defaultCatalog, path);
        }

        // Parse build routines
        for (const auto &item : std::as_const(insertionElements)) {
            result.extension.insertions.append(parseInsertion(*item));
        }

        // Collect objects
        for (const auto &pair : std::as_const(itemInfoMap)) {
            result.extension.items.append(pair.second);
        }
    }
};

Parser::Parser() = default;

ParseResult Parser::parse(const QByteArray &data) {
    // stdc::linked_map<QString, QString> map;
    // auto it = map.append("1", "1");
    // it = map.append("2", "2");
    // it = map.append("2", "1");
    // qDebug() << map["1"];
    // qDebug() << it.first.value() << it.second;

    ParserPrivate d(*this);
    d.parse(data);
    return d.result;
}