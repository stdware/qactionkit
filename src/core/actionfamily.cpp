#include "actionfamily.h"
#include "actionfamily_p.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <utility>

#include <qmxmladaptor/qmxmladaptor.h>
#include <util/util.h>

#include "qakglobal_p.h"

namespace QAK {

    ActionIcon ActionIconFromJson(const QJsonValue &json, const QString &baseDir);

    class IconConfigParser {
    public:
        IconConfigParser(QString fileName) : fileName(std::move(fileName)) {
        }

        QHash<QString, QHash<QString, ActionIcon>> parse() {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                qCWarning(qActionKitLog).nospace().noquote()
                    << "QAK::ActionFamily: " << fileName
                    << ": failed to read icon configuration file";
                return {};
            }

            QByteArray data(file.readAll());
            file.close();

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                qCWarning(qActionKitLog).nospace().noquote()
                    << "QAK::ActionFamily: " << fileName
                    << ": failed to parse icon configuration file";
                return {};
            }

            auto docObj = doc.object();
            baseDirectory = QFileInfo(fileName).absolutePath();

            if (auto it = docObj.find(QStringLiteral("configuration"));
                it != docObj.end() && it->isObject()) {
                parseConfig(it->toObject());
            }

            // Get results
            QJsonArray themeArr;
            if (auto it = docObj.find(QStringLiteral("themes"));
                it == docObj.end() || !it->isArray()) {
                return {};
            } else {
                themeArr = it->toArray();
            }

            QHash<QString, QHash<QString, ActionIcon>> result;
            for (const auto &themeItem : std::as_const(themeArr)) {
                if (!themeItem.isObject()) {
                    continue;
                }

                auto themeObj = themeItem.toObject();
                QString themeId;
                QHash<QString, ActionIcon> icons;
                if (auto it = themeObj.find(QStringLiteral("id"));
                    it == themeObj.end() || !it->isString()) {
                    continue;
                } else {
                    themeId = it->toString();
                }

                QJsonArray iconArr;
                if (auto it = themeObj.find(QStringLiteral("icons"));
                    it == themeObj.end() || !it->isArray()) {
                    continue;
                } else {
                    iconArr = it->toArray();
                }

                for (const auto &iconItem : std::as_const(iconArr)) {
                    if (!iconItem.isObject()) {
                        continue;
                    }

                    auto iconObj = iconItem.toObject();
                    QString iconId;
                    if (auto it = iconObj.find(QStringLiteral("id"));
                        it == iconObj.end() || !it->isString()) {
                        continue;
                    } else {
                        iconId = it->toString();
                    }

                    ActionIcon icon;
                    if (auto it = iconObj.find(QStringLiteral("icon")); it == iconObj.end()) {
                        continue;
                    } else {
                        icon = ActionIconFromJson(it.value(), baseDirectory);
                    }
                    icons.insert(iconId, icon);
                }
                result.insert(themeId, icons);
            }
            return result;
        }

        void parseConfig(const QJsonObject &obj) {
            if (auto it = obj.find(QStringLiteral("baseDir")); it != obj.end() && it->isString()) {
                baseDirectory =
                    Util::absolutePath(it->toString(), QFileInfo(fileName).absolutePath());
            }
        }

    public:
        QString fileName;

        // config
        QString baseDirectory;
    };

}

namespace QAK {

    ActionFamilyPrivate::ActionFamilyPrivate() {
    }

    ActionFamilyPrivate::~ActionFamilyPrivate() = default;

    void ActionFamilyPrivate::init() {
    }

    ActionFamily::ActionFamily(QObject *parent) : ActionFamily(*new ActionFamilyPrivate(), parent) {
    }

    ActionFamily::~ActionFamily() = default;

    void ActionFamilyPrivate::flushIcons() const {
        auto &changes = iconChange.items;
        if (changes.empty())
            return;

        for (const auto &pair : std::as_const(changes)) {
            auto &c = pair.second;

            switch (c.index()) {
                case 0: {
                    auto &singles = iconStorage.singles;
                    auto &itemToBeChanged = std::get<0>(c);
                    if (itemToBeChanged.remove) {
                        auto it = singles.find(itemToBeChanged.theme);
                        if (it != singles.end()) {
                            auto &map = it.value();
                            if (map.remove(itemToBeChanged.id)) {
                                if (map.isEmpty()) {
                                    singles.erase(it);
                                }
                                storeOrder.remove({itemToBeChanged.theme, itemToBeChanged.id});
                            }
                        }
                    } else {
                        QStringList keys = {itemToBeChanged.theme, itemToBeChanged.id};
                        singles[itemToBeChanged.theme][itemToBeChanged.id] = itemToBeChanged.icon;
                        storeOrder.remove(keys);
                        storeOrder.append(keys, {});
                    }
                    break;
                }
                case 1: {
                    auto &configFiles = iconStorage.configFiles;
                    auto &itemToBeChanged = std::get<1>(c);
                    if (itemToBeChanged.remove) {
                        if (configFiles.remove(itemToBeChanged.fileName)) {
                            storeOrder.remove({itemToBeChanged.fileName});
                        }
                    } else if (auto iconsFromFile =
                                   IconConfigParser(itemToBeChanged.fileName).parse();
                               !iconsFromFile.isEmpty()) {
                        QStringList keys = {itemToBeChanged.fileName};
                        configFiles[itemToBeChanged.fileName] = iconsFromFile;
                        storeOrder.remove(keys);
                        storeOrder.append(keys, {});
                    }
                    break;
                }
                case 2: {
                    iconStorage = {};
                    break;
                }
                default:
                    break;
            }
        }
        changes.clear();

        // Build map
        {
            auto &storage = iconStorage.storage;
            storage.clear();
            for (const auto &pair : std::as_const(storeOrder)) {
                auto &keys = pair.first;
                if (keys.size() == 1) {
                    auto configMap = iconStorage.configFiles.value(keys[0]);
                    for (auto it = configMap.begin(); it != configMap.end(); ++it) {
                        auto &from = it.value();
                        auto &to = storage[it.key()];
                        for (auto it2 = from.begin(); it2 != from.end(); ++it2) {
                            to.insert(it2.key(), it2.value());
                        }
                    }
                } else {
                    storage[keys[0]][keys[1]] = iconStorage.singles.value(keys[0]).value(keys[1]);
                }
            }
        }
    }

    void ActionFamily::addIcon(const QString &theme, const QString &id, const ActionIcon &icon) {
        Q_D(ActionFamily);
        ActionFamilyPrivate::IconChange::Single itemToBeAdded{
            theme,
            id,
            icon,
            false,
        };
        auto &items = d->iconChange.items;
        QStringList keys = {theme, id};
        items.remove(keys);
        items.append(keys, {itemToBeAdded});
    }

    void ActionFamily::addIconManifest(const QString &fileName) {
        Q_D(ActionFamily);
        QFileInfo info(fileName);
        if (!info.isFile()) {
            return;
        }
        QString canonicalFileName = info.canonicalFilePath();
        ActionFamilyPrivate::IconChange::Config itemToBeAdded{
            canonicalFileName,
            false,
        };
        auto &items = d->iconChange.items;
        QStringList keys = {canonicalFileName};
        items.remove(keys);
        items.append({canonicalFileName}, {itemToBeAdded});
    }

    void ActionFamily::removeIcon(const QString &theme, const QString &id) {
        Q_D(ActionFamily);
        auto &items = d->iconChange.items;
        ActionFamilyPrivate::IconChange::Single itemToBeRemoved{
            theme,
            id,
            {},
            true,
        };
        QStringList keys = {theme, id};
        items.remove(keys);
        items.append(keys, itemToBeRemoved);
    }

    void ActionFamily::removeIconManifest(const QString &fileName) {
        Q_D(ActionFamily);
        QFileInfo info(fileName);
        if (!info.isFile()) {
            return;
        }
        QString canonicalFileName = info.canonicalFilePath();
        auto &items = d->iconChange.items;
        ActionFamilyPrivate::IconChange::Config itemToBeRemoved{
            canonicalFileName,
            true,
        };
        QStringList keys = {canonicalFileName};
        items.remove(keys);
        items.append(keys, itemToBeRemoved);
    }

    void ActionFamily::removeAllIcons() {
        Q_D(ActionFamily);
        static const QStringList keys = {QStringLiteral("%REMOVE_ALL%")};
        auto &items = d->iconChange.items;
        ActionFamilyPrivate::IconChange::All itemToBeRemoved;
        items.remove(keys);
        items.append(keys, itemToBeRemoved);
    }

    QStringList ActionFamily::iconThemes() const {
        Q_D(const ActionFamily);
        d->flushIcons();
        return d->iconStorage.storage.keys();
    }

    QStringList ActionFamily::iconIds(const QString &theme) {
        Q_D(const ActionFamily);
        d->flushIcons();
        return d->iconStorage.storage.value(theme).keys();
    }

    ActionIcon ActionFamily::icon(const QString &theme, const QString &iconId) const {
        Q_D(const ActionFamily);
        d->flushIcons();
        return d->iconStorage.storage.value(theme).value(iconId);
    }

    ActionFamily::ShortcutsFamily ActionFamily::shortcutsFamily() const {
        Q_D(const ActionFamily);
        return d->overriddenShortcuts;
    }

    void ActionFamily::setShortcutsFamily(const ShortcutsFamily &shortcutsFamily) {
        Q_D(ActionFamily);
        d->overriddenShortcuts = shortcutsFamily;
    }

    void ActionFamily::resetShortcuts() {
        Q_D(ActionFamily);
        d->overriddenShortcuts.clear();
    }

    ActionFamily::ShortcutsOverride ActionFamily::shortcuts(const QString &id) const {
        Q_D(const ActionFamily);
        return d->overriddenShortcuts.value(id);
    }

    void ActionFamily::setShortcuts(const QString &id, const ShortcutsOverride &shortcuts) {
        Q_D(ActionFamily);
        d->overriddenShortcuts.insert(id, shortcuts);
    }

    ActionFamily::IconFamily ActionFamily::iconFamily() const {
        Q_D(const ActionFamily);
        return d->overriddenIcons;
    }

    void ActionFamily::setIconFamily(const IconFamily &iconFamily) {
        Q_D(ActionFamily);
        d->overriddenIcons = iconFamily;
    }

    ActionFamily::IconOverride ActionFamily::icon(const QString &id) const {
        Q_D(const ActionFamily);
        return d->overriddenIcons.value(id);
    }

    void ActionFamily::setIcon(const QString &id, const IconOverride &icon) {
        Q_D(ActionFamily);
        d->overriddenIcons.insert(id, icon);
    }

    void ActionFamily::resetIcons() {
        Q_D(ActionFamily);
        d->overriddenIcons.clear();
    }

    QJsonArray ActionFamily::shortcutsFamilyToJson(const ShortcutsFamily &shortcutsFamily) {
        QJsonArray arr;
        for (auto it = shortcutsFamily.begin(); it != shortcutsFamily.end(); ++it) {
            const auto &id = it.key();
            QJsonValue value = QJsonValue::Null;
            if (const auto &val = it.value()) {
                QJsonArray keysArr;
                for (const auto &subItem : val.value()) {
                    keysArr.push_back(subItem.toString());
                }
                value = keysArr;
            }
            arr.append(QJsonObject({
                {QStringLiteral("id"),   id   },
                {QStringLiteral("keys"), value},
            }));
        }
        return arr;
    }

    ActionFamily::ShortcutsFamily ActionFamily::shortcutsFamilyFromJson(const QJsonArray &arr) {
        ShortcutsFamily result;
        for (const auto &item : std::as_const(arr)) {
            if (!item.isObject()) {
                continue;
            }
            const auto obj = item.toObject();
            const auto &id = obj.value(QStringLiteral("id")).toString();
            if (id.isEmpty()) {
                continue;
            }
            const auto &shortcutsValue = obj.value(QStringLiteral("keys"));
            if (shortcutsValue.isNull()) {
                result.insert(id, {});
                continue;
            }
            if (!shortcutsValue.isArray()) {
                continue;
            }
            const auto &shortcutsArr = shortcutsValue.toArray();
            QList<QKeySequence> shortcuts;
            shortcuts.reserve(shortcutsArr.size());
            for (const auto &subItem : std::as_const(shortcutsArr)) {
                if (QKeySequence key = QKeySequence::fromString(subItem.toString());
                    !key.isEmpty()) {
                    shortcuts.append(key);
                }
            }
            result.insert(id, shortcuts);
        }
        return result;
    }

    QJsonArray ActionFamily::iconFamilyToJson(const IconFamily &iconFamily) {
        QJsonArray arr;
        for (auto it = iconFamily.begin(); it != iconFamily.end(); ++it) {
            const auto &id = it.key();
            QJsonValue value = QJsonValue::Null;
            if (const auto &val = it.value()) {
                value = val->toJson();
            }
            arr.append(QJsonObject({
                {QStringLiteral("id"),   id   },
                {QStringLiteral("icon"), value},
            }));
        }
        return arr;
    }

    ActionFamily::IconFamily ActionFamily::iconFamilyFromJson(const QJsonArray &arr) {
        IconFamily result;
        for (const auto &item : std::as_const(arr)) {
            if (!item.isObject()) {
                continue;
            }
            const auto obj = item.toObject();
            const auto &id = obj.value(QStringLiteral("id")).toString();
            if (id.isEmpty()) {
                continue;
            }
            const auto &iconValue = obj.value(QStringLiteral("icon"));
            if (iconValue.isNull()) {
                result.insert(id, {});
                continue;
            }
            result.insert(id, ActionIcon::fromJson(iconValue));
        }
        return result;
    }

    ActionFamily::ActionFamily(ActionFamilyPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.init();
    }

}