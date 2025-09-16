#ifndef ACTIONFAMILY_H
#define ACTIONFAMILY_H

#include <optional>

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtGui/QKeySequence>

#include <QAKCore/actionicon.h>

namespace QAK {

    class ActionFamilyPrivate;

    /// \class ActionFamily
    /// \brief The ActionFamily class represents a group of actions which are normally functioning
    /// in a window.
    class QAK_CORE_EXPORT ActionFamily : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionFamily)
    public:
        explicit ActionFamily(QObject *parent = nullptr);
        ~ActionFamily();

        using ShortcutsOverride = std::optional<QList<QKeySequence>>;
        using IconOverride = std::optional<ActionIcon>;
        using ShortcutsFamily = QMap<QString, ShortcutsOverride>;
        using IconFamily = QMap<QString, IconOverride>;

    public:
        /// This set of functions is used to query or modify the base icon collection of the
        /// \c ActionFamily.
        void addIcon(const QString &theme, const QString &id, const ActionIcon &icon);
        void addIconManifest(const QString &fileName);
        void removeIcon(const QString &theme, const QString &id);
        void removeIconManifest(const QString &fileName);
        void removeAllIcons();
        QStringList iconThemes() const;
        QStringList iconIds(const QString &theme);
        ActionIcon icon(const QString &theme, const QString &iconId) const;

    public:
        /// Returns the current keymap.
        ShortcutsFamily shortcutsFamily() const;
        /// Sets the keymap.
        void setShortcutsFamily(const ShortcutsFamily &shortcutsFamily);
        /// Resets the shortcuts for all actions.
        void resetShortcuts();
        /// Returns the shortcuts for the given action id.
        ShortcutsOverride shortcuts(const QString &id) const;
        /// Sets the shortcuts for the given action id.
        void setShortcuts(const QString &id, const ShortcutsOverride &shortcuts);

        /// Returns the current overridden icon family.
        IconFamily iconFamily() const;
        /// Sets the overridden icon family.
        void setIconFamily(const IconFamily &iconFamily);
        /// Returns the overridden icon for the given action id.
        IconOverride icon(const QString &id) const;
        /// Sets the overridden icon for the given action id.
        void setIcon(const QString &id, const IconOverride &icon);
        /// Resets the overridden icons for all actions.
        void resetIcons();

        inline ActionIcon actionIcon(const QString &theme, const QString &id) const;

    public:
        static QJsonArray shortcutsFamilyToJson(const ShortcutsFamily &shortcutsFamily);
        static ShortcutsFamily shortcutsFamilyFromJson(const QJsonArray &arr);

        static QJsonArray iconFamilyToJson(const IconFamily &iconFamily);
        static IconFamily iconFamilyFromJson(const QJsonArray &arr);

    protected:
        explicit ActionFamily(ActionFamilyPrivate &d, QObject *parent = nullptr);

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
