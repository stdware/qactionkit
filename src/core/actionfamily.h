#ifndef ACTIONFAMILY_H
#define ACTIONFAMILY_H

#include <optional>

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtGui/QKeySequence>

#include <QAKCore/actionicon.h>

namespace QAK {

    class ActionFamilyPrivate;

    class QAK_CORE_EXPORT ActionFamily : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionFamily)
    public:
        explicit ActionFamily(QObject *parent = nullptr);
        ~ActionFamily();

        using ShortcutsOverride = std::optional<QList<QKeySequence>>;
        using IconOverride = std::optional<ActionIcon>;
        using ShortcutsFamily = QHash<QString, ShortcutsOverride>;
        using IconFamily = QHash<QString, IconOverride>;

    public:
        void addIcon(const QString &theme, const QString &id, const ActionIcon &icon);
        void addIconManifest(const QString &fileName);
        void removeIcon(const QString &theme, const QString &id);
        void removeIconManifest(const QString &fileName);
        void removeAllIcons();

        QStringList iconThemes() const;
        QStringList iconIds(const QString &theme);
        ActionIcon icon(const QString &theme, const QString &iconId) const;

    public:
        ShortcutsFamily shortcutsFamily() const;
        void setShortcutsFamily(const ShortcutsFamily &shortcutsFamily);

        IconFamily iconFamily() const;
        void setIconFamily(const IconFamily &iconFamily);

        ShortcutsOverride shortcuts(const QString &id) const;
        void setShortcuts(const QString &id, const ShortcutsOverride &shortcuts);
        void resetShortcuts();

        IconOverride icon(const QString &id) const;
        void setIcon(const QString &id, const IconOverride &icon);
        void resetIcons();
        
        inline ActionIcon actionIcon(const QString &theme, const QString &id) const;

    public:
        static QJsonArray shortcutsFamilyToJson(const ShortcutsFamily &shortcutsFamily);
        static ShortcutsFamily shortcutsFamilyFromJson(const QJsonArray &arr);

        static QJsonArray iconFamilyToJson(const IconFamily &iconFamily);
        static IconFamily iconFamilyFromJson(const QJsonArray &arr);

    protected:
        ActionFamily(ActionFamilyPrivate &d, QObject *parent = nullptr);

        std::unique_ptr<ActionFamilyPrivate> d_ptr;
    };

    inline ActionIcon ActionFamily::actionIcon(const QString &theme, const QString &id) const {
        if (const auto o = icon(id); o) {
            return o.value();
        }
        return icon(theme, id);
    }

}

#endif // ACTIONFAMILY_H