#ifndef ACTIONFAMILY_H
#define ACTIONFAMILY_H

#include <optional>

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtGui/QKeySequence>
#include <QtGui/QIcon>

#include <QAKCore/qakglobal.h>

namespace QAK {

    // clang-format off
    class ActionIcon {
    public:
        enum Type {
            ID,
            LocalFile,
        };
        inline ActionIcon(Type type = ID, const QString &data = {}) : m_type(type), m_data(data){};
        inline Type type() const { return m_type; }
        inline QString data() const { return m_data; }
        inline bool isLocalFile() const { return m_type == LocalFile; }
        inline bool isNull() const { return !m_data.isEmpty(); }

    protected:
        Type m_type;
        QString m_data;
    };
    // clang-format on

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
        void addIcon(const QString &theme, const QString &id, const QString &fileName);
        void addIconManifest(const QString &fileName);
        void removeIcon(const QString &theme, const QString &id);
        void removeIconManifest(const QString &fileName);
        void removeAllIcons();

        QStringList iconThemes() const;
        QStringList iconIds(const QString &theme);
        QIcon icon(const QString &theme, const QString &iconId) const;

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

    public:
        static QJsonArray shortcutsFamilyToJson(const ShortcutsFamily &shortcutsFamily);
        static ShortcutsFamily shortcutsFamilyFromJson(const QJsonArray &arr);

        static QJsonArray iconFamilyToJson(const IconFamily &iconFamily);
        static IconFamily iconFamilyFromJson(const QJsonArray &arr);

    protected:
        ActionFamily(ActionFamilyPrivate &d, QObject *parent = nullptr);

        std::unique_ptr<ActionFamilyPrivate> d_ptr;
    };

}

#endif // ACTIONFAMILY_H