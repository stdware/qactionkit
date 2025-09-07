#include "parser.h"

#include <algorithm>
#include <utility>

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringView>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVersionNumber>

#include <qmxmladaptor/qmxmladaptor.h>
#include <stdcorelib/linked_map.h>
#include <util/util.h>

static const QVersionNumber &parserVersion() {
    static const QVersionNumber version{1, 0};
    return version;
}

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

static QString parseItemId(const QString &id) {
    QString parsed(id.size(), QChar::Null);
    int idx = 0;
    bool previousSectionContainsSpecialCharactor = false;
    bool currentSectionContainsSpecialCharactor = false;
    for (int pos = 0; pos < id.size(); pos++) {
        QChar ch = id[pos];
        if (ch.unicode() > 0x7f) {
            return {};
        }
        if (ch == QLatin1Char('&')) {
            if (pos != 0 && id[pos - 1] == QLatin1Char('&')) {
                return {};
            }
            currentSectionContainsSpecialCharactor = true;
            continue;
        }
        if (ch == QLatin1Char('^')) {
            currentSectionContainsSpecialCharactor = true;
            if (pos == id.size() - 1) {
                continue;
            } else {
                return {};
            }
        }
        if (ch == QLatin1Char('.')) {
            if (idx == 0 || parsed[idx - 1] == QLatin1Char('.')) {
                return {};
            }

            previousSectionContainsSpecialCharactor = previousSectionContainsSpecialCharactor || currentSectionContainsSpecialCharactor;
            currentSectionContainsSpecialCharactor = false;
            parsed[idx++] = QLatin1Char('.');
            continue;
        }
        if (ch.isLetterOrNumber() || ch == QLatin1Char('_')) {
            parsed[idx++] = ch;
            continue;
        }
        return {};
    }
    if (previousSectionContainsSpecialCharactor) {
        return {};
    }
    parsed.truncate(idx);
    return parsed;
}

static QString itemIdToText(const QString &id, int sectionIndex = -1) {
    static QSet<QString> lowerCase {

        QStringLiteral("a"),
        QStringLiteral("an"),
        QStringLiteral("the"),

        QStringLiteral("for"),
        QStringLiteral("and"),
        QStringLiteral("nor"),
        QStringLiteral("but"),
        QStringLiteral("or"),

        QStringLiteral("amid"),
        QStringLiteral("as"),
        QStringLiteral("at"),
        QStringLiteral("by"),
        QStringLiteral("down"),
        QStringLiteral("from"),
        QStringLiteral("in"),
        QStringLiteral("into"),
        QStringLiteral("near"),
        QStringLiteral("of"),
        QStringLiteral("off"),
        QStringLiteral("on"),
        QStringLiteral("onto"),
        QStringLiteral("out"),
        QStringLiteral("over"),
        QStringLiteral("per"),
        QStringLiteral("than"),
        QStringLiteral("till"),
        QStringLiteral("to"),
        QStringLiteral("up"),
        QStringLiteral("upon"),
        QStringLiteral("via"),
        QStringLiteral("with"),
    };
    QStringView idView(id);
    auto sections = idView.split(QLatin1Char('.'));
    if (sections.size() + sectionIndex < 0) {
        return {};
    }
    idView = sections[sections.size() + sectionIndex];
    static const auto rx = QRegularExpression("(?<=.)(?=(?:[A-Z]|&[A-Z]))(?<!(?:^|[^&])&)");
    QList<QStringView> parts = idView.split(rx, Qt::SkipEmptyParts);
    QStringList convertedParts;
    for (int i = 0; i < parts.size(); ++i) {
        auto part = parts[i];
        QString prefix;
        QString suffix;
        if (part.startsWith(QLatin1Char('&'))) {
            prefix = QStringLiteral("&");
            part = part.mid(1);
        }
        if (part.endsWith(QLatin1Char('^'))) {
            suffix = QStringLiteral("...");
            part = part.mid(0, part.size() - 1);
        }
        auto s = part.toString().toLower();
        if (i == 0 || i == parts.size() - 1 || !lowerCase.contains(s)) {
            if (!s.isEmpty())
                s[0] = s[0].toUpper();
        }
        convertedParts.append(prefix + s + suffix);
    }
    return convertedParts.join(" ");
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
    ParserPrivate(Parser &q) : q(q) {
    }

    // Configuration
    QString defaultCatalog;

    // Parse configuration, fix parser members
    void parseConfiguration(const QMXmlAdaptorElement &e,
                            const QHash<QString, QString> &reservedVars) {
        QHash<QString, QString> vars;
        for (const auto &item : e.children) {
            if (item->name == QStringLiteral("defaultCatalog") && item->namespaceUri.isEmpty()) {
                defaultCatalog = resolve(item->value);
                continue;
            }

            if (item->name == QStringLiteral("translationContext") && item->namespaceUri.isEmpty()) {
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

            if (item->name == QStringLiteral("vars") && item->namespaceUri.isEmpty()) {
                QSet<QString> commandLineKeys;
                for (auto it = q.variables.begin(); it != q.variables.end(); ++it) {
                    commandLineKeys.insert(it.key());
                }

                for (const auto &subItem : item->children) {
                    const auto &key = subItem->properties.value(QStringLiteral("key"));
                    if (commandLineKeys.contains(key))
                        continue;

                    auto value = resolve(subItem->properties.value(QStringLiteral("value")));
                    if (!key.isEmpty() && !reservedVars.contains(key)) {
                        q.variables.insert(key, value);
                    }
                }
            }
        }
    }

    // Intermediate data
    stdc::linked_map<QString, ActionItemInfoMessage> itemInfoMap;

    ParseResult result;

    inline QString resolve(const QString &s) const {
        return Util::parseExpression(s, q.variables);
    }

    // Infer item type when parsing item info
    void parseItemAttrs(const QMXmlAdaptorElement &e, ActionItemInfoMessage &info,
                        const char *field) const {
        const auto &readTopLevel = [&]() {
            if (resolve(e.properties.value(QStringLiteral("topLevel"))) == QStringLiteral("true")) {
                info.topLevel = true;
            }
        };

        info.id = parseItemId(info.rawId);
        if (info.id.isEmpty()) {
            error("%s: item \"%s\" has an invalid \"id\" value \"%s\"\n", qPrintable(q.fileName),
                  qPrintable(e.name), qPrintable(info.rawId));
            std::exit(1);
        }

        const auto &name = e.name;
        const auto &namespaceUri = e.namespaceUri;
        if (name == QStringLiteral("action") && namespaceUri.isEmpty()) {
            info.type = QAK::ActionItemInfo::Action;
        } else if (name == QStringLiteral("group") && namespaceUri.isEmpty()) {
            info.type = QAK::ActionItemInfo::Group;
            readTopLevel();
        } else if (name == QStringLiteral("menu") && namespaceUri.isEmpty()) {
            info.type = QAK::ActionItemInfo::Menu;
            readTopLevel();
        } else if ((name == QStringLiteral("menuBar") || name == QStringLiteral("toolBar")) && namespaceUri.isEmpty()) {
            info.type = QAK::ActionItemInfo::Menu;
            info.topLevel = true;
        } else if (name == QStringLiteral("phony") && namespaceUri.isEmpty()) {
            info.type = QAK::ActionItemInfo::Phony;
        } else {
            error("%s: %s item \"%s\" has an unknown tag \"%s\"\n", qPrintable(q.fileName), field,
                  qPrintable(info.id), qPrintable(e.name));
            std::exit(1);
        }

        info.tag = e.name;

        // text
        if (auto text = resolve(e.properties.value(QStringLiteral("text"))); !text.isEmpty()) {
            info.text = text;
        } else {
            info.text = itemIdToText(info.rawId);
        }

        // class
        if (info.type == QAK::ActionItemInfo::Action) {
            if (auto actionClass = resolve(e.properties.value(QStringLiteral("class")));
                !actionClass.isEmpty()) {
                info.actionClass = actionClass;
            } else {
                info.actionClass = itemIdToText(info.rawId, -2);
            }
        }

        // description
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
        if (info.type == QAK::ActionItemInfo::Action) {
            if (auto shortcuts = resolve(e.properties.value(QStringLiteral("shortcuts")));
                !shortcuts.isEmpty()) {
                info.shortcutTokens = parseStringList(shortcuts);
            } else if (shortcuts = resolve(e.properties.value(QStringLiteral("shortcut")));
                       !shortcuts.isEmpty()) {
                info.shortcutTokens = parseStringList(shortcuts);
            }
        }

        // catalog
        if (auto catalog = resolve(e.properties.value(QStringLiteral("catalog")));
            !catalog.isEmpty()) {
            info.catalog = resolve(catalog);
        }

        // attributes
        for (auto it = e.properties.begin(); it != e.properties.end(); ++it) {
            static const QMap<QMXmlAdaptorAttributeKey, int> reservedKeys = {
                {QMXmlAdaptorAttributeKey(QStringLiteral("id")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("text")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("class")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("description")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("catalog")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("shortcuts")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("shortcut")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("topLevel")), {}},
                {QMXmlAdaptorAttributeKey(QStringLiteral("icon")), {}},
            };
            const auto &key = it.key();
            if (reservedKeys.contains(key)) {
                continue;
            }
            info.attributes.insert(QAK::ActionAttributeKey(key.name, key.namespaceUri), resolve(it.value()));
        }
    }

    ActionItemInfoMessage parseItem(const QMXmlAdaptorElement &e) const {
        ActionItemInfoMessage info;
        auto id = resolve(e.properties.value(QStringLiteral("id")));
        if (id.isEmpty()) {
            error("%s: item element \"%s\" doesn't have an \"id\" field\n", qPrintable(q.fileName),
                  qPrintable(e.name));
            std::exit(1);
        }
        info.rawId = id;

        parseItemAttrs(e, info, "item");

        if (!e.children.isEmpty()) {
            error("%s: item declaration element \"%s\" shouldn't have children\n",
                  qPrintable(q.fileName), qPrintable(id));
            std::exit(1);
        }
        return info;
    }

    // Create new item when encountering a new ID in layouts or insertions
    ActionItemInfoMessage &findOrInsertItemInfo(const QMXmlAdaptorElement *e,
                                                const QString &upperCatalog, const char *field) {
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
        if (auto it = itemInfoMap.find(id); it != itemInfoMap.end()) {
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
                    if ((e->name != QStringLiteral("action") && e->name != QStringLiteral("item")) || !e->namespaceUri.isEmpty()) {
                        typeMismatch = true;
                    }
                    break;
                }
                case QAK::ActionItemInfo::Group:
                case QAK::ActionItemInfo::Menu: {
                    static const QSet<QString> allowedTags = {
                        QStringLiteral("group"),   QStringLiteral("menu"),
                        QStringLiteral("menuBar"), QStringLiteral("toolBar"),
                        QStringLiteral("item"),
                    };
                    if (!allowedTags.contains(e->name) || !e->namespaceUri.isEmpty()) {
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
                info.catalog = upperCatalog;
            }
            pInfo = &info;
        } else {
            // Create one
            ActionItemInfoMessage info;
            info.rawId = id;
            parseItemAttrs(*e, info, field);
            if (info.type == QAK::ActionItemInfo::Phony) {
                errorPhony();
            }
            if (info.catalog.isEmpty()) {
                info.catalog = upperCatalog;
            }

            auto insertResult = itemInfoMap.append(id, info);
            pInfo = &insertResult.first.value();
        }
        return *pInfo;
    }

    ActionLayoutEntryMessage
        parseLayoutRecursively(const QMXmlAdaptorElement *e, const QString &upperCatalog,
                               stdc::linked_map<QString, int /*NOT USED*/> &path) {
        const auto &checkChildren = [this, e](const char *typeName) {
            if (!e->children.isEmpty()) {
                error("%s: layout element of %s type shouldn't have children\n",
                      qPrintable(q.fileName), typeName);
                std::exit(1);
            }
        };

        ActionLayoutEntryMessage entry;
        if (e->name == QStringLiteral("separator") && e->namespaceUri.isEmpty()) {
            checkChildren("separator");
            entry.type = QAK::ActionLayoutEntry::Separator;
            return entry;
        } else if (e->name == QStringLiteral("stretch") && e->namespaceUri.isEmpty()) {
            checkChildren("stretch");
            entry.type = QAK::ActionLayoutEntry::Stretch;
            return entry;
        }

        auto &info = findOrInsertItemInfo(e, upperCatalog, "layout");
        QString id = info.id;

        // Recursive path chain detected?
        if (path.contains(id)) {
            error("%s: recursive chain in layout: %s\n", qPrintable(q.fileName),
                  qPrintable((QStringList(path.keys_qlist()) << id).join(", ")));
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
                if (e->name == QStringLiteral("menu") && e->namespaceUri.isEmpty()) {
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

            path.append(id, {});

            QVector<ActionLayoutEntryMessage> children;
            for (const auto &child : e->children) {
                children.append(parseLayoutRecursively(child.data(), id, path));
            }
            path.erase(std::prev(path.end()));

            info.children = std::move(children);
        }
        return entry;
    }

    ActionInsertionMessage parseInsertion(const QMXmlAdaptorElement &root) {
        if (const auto &rootName = root.name; rootName != QStringLiteral("insertion") || !root.namespaceUri.isEmpty()) {
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
            if (e.name == QStringLiteral("separator") && e.namespaceUri.isEmpty()) {
                entry.type = QAK::ActionLayoutEntry::Separator;
            } else if (e.name == QStringLiteral("stretch") && e.namespaceUri.isEmpty()) {
                entry.type = QAK::ActionLayoutEntry::Stretch;
            } else {
                auto &info = findOrInsertItemInfo(&e, {}, "insertion");
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
                        if (e.name == QStringLiteral("menu") && e.namespaceUri.isEmpty()) {
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

        // Check root name and namespace
        const auto &root = xml.root;
        if (const auto &rootName = root.name; rootName != QStringLiteral("actionExtension") || !root.namespaceUri.isEmpty()) {
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
        const QMXmlAdaptorElement *configElement = nullptr;
        for (const auto &item : std::as_const(root.children)) {
            if (item->name == QStringLiteral("items") && item->namespaceUri.isEmpty()) {
                for (const auto &subItem : std::as_const(item->children)) {
                    objElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("layouts") && item->namespaceUri.isEmpty()) {
                for (const auto &subItem : std::as_const(item->children)) {
                    layoutElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("insertions") && item->namespaceUri.isEmpty()) {
                for (const auto &subItem : std::as_const(item->children)) {
                    insertionElements.append(subItem.data());
                }
                continue;
            }
            if (item->name == QStringLiteral("id") && item->namespaceUri.isEmpty()) {
                id = item->value;
                continue;
            }
            if (item->name == QStringLiteral("version") && item->namespaceUri.isEmpty()) {
                version = item->value;
                continue;
            }
            if (item->name == QStringLiteral("configuration") && item->namespaceUri.isEmpty()) {
                if (configElement) {
                    error("%s: duplicated configuration elements\n", qPrintable(q.fileName));
                    std::exit(1);
                }
                configElement = item.data();
                continue;
            }
        }

        if (version.isEmpty()) {
            error("%s: extension version is not specified\n", qPrintable(q.fileName));
            std::exit(1);
        }
        if (parserVersion() < QVersionNumber::fromString(version)) {
            error("%s: extension version \"%s\" is not supported\n", qPrintable(q.fileName),
                  qPrintable(version));
            std::exit(1);
        }

        if (id.isEmpty()) {
            error("%s: extension id is not specified\n", qPrintable(q.fileName));
            std::exit(1);
        }

        // Build result
        result.extension.version = version;
        result.extension.id = id;
        result.extension.hash = calculateContentSha256(data);

        // Parse configuration
        const QHash<QString, QString> reservedVars = {
            {QStringLiteral("_ID_"),           id                              },
            {QStringLiteral("_VERSION_"),      version                         },
            {QStringLiteral("_IDENTIFIER_"),   q.identifier                    },
            {QStringLiteral("_FILENAME_"),     QFileInfo(q.fileName).fileName()},
            {QStringLiteral("_FILEBASENAME_"), QFileInfo(q.fileName).baseName()},
        };
        q.variables.insert(reservedVars);
        if (configElement) {
            parseConfiguration(*configElement, reservedVars);
        }

        // Parse items
        for (const auto &item : std::as_const(objElements)) {
            auto entity = parseItem(*item);
            if (itemInfoMap.contains(entity.id)) {
                error("%s: duplicated item id %s\n", qPrintable(q.fileName), qPrintable(entity.id));
                std::exit(1);
            }
            std::ignore = itemInfoMap.append(entity.id, entity);
        }

        // Parse layouts
        for (const auto &item : std::as_const(layoutElements)) {
            stdc::linked_map<QString, int /*NOT USED*/> path;
            std::ignore = parseLayoutRecursively(item, {}, path);
        }

        // Parse insertions
        for (const auto &item : std::as_const(insertionElements)) {
            result.extension.insertions.append(parseInsertion(*item));
        }

        // Add default catalog as a phony item if not specified
        if (!defaultCatalog.isEmpty() && !itemInfoMap.contains(defaultCatalog)) {
            ActionItemInfoMessage info;
            info.type = QAK::ActionItemInfo::Phony;
            info.id = defaultCatalog;
            info.text = itemIdToText(info.id);
            itemInfoMap.prepend(info.id, info);
        }

        // Collect items
        for (auto &pair : itemInfoMap) {
            auto &info = pair.second;
            if (info.type != QAK::ActionItemInfo::Phony && !info.topLevel &&
                info.catalog.isEmpty()) {
                info.catalog = defaultCatalog; // fallback to default catalog
            }
            result.extension.items.append(info);
        }
    }
};

ParseResult Parser::parse(const QByteArray &data) {
    ParserPrivate d(*this);
    d.parse(data);
    return d.result;
}