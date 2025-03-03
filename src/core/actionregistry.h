#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <QtCore/QSharedData>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionfamily.h>

namespace QAK {

    class ActionContext;

    class ActionCatalogData;

    class QAK_CORE_EXPORT ActionCatalog {
    public:
        ActionCatalog();
        ActionCatalog(const QString &id);
        ActionCatalog(const ActionCatalog &other);
        ActionCatalog &operator=(const ActionCatalog &other);
        ~ActionCatalog();

    public:
        QString id() const;
        void setId(const QString &id);

        QList<ActionCatalog> children() const;
        void addChild(const ActionCatalog &child);
        void setChildren(const QList<ActionCatalog> &children);

    protected:
        QSharedDataPointer<ActionCatalogData> d;

        friend class ActionDomain;
    };

    class ActionLayoutData;

    class QAK_CORE_EXPORT ActionLayout {
    public:
        ActionLayout();
        ActionLayout(const QString &id);
        ActionLayout(const ActionLayout &other);
        ActionLayout &operator=(const ActionLayout &other);
        ~ActionLayout();

    public:
        QString id() const;
        void setId(const QString &id);

        ActionLayoutEntry::Type type() const;
        void setType(ActionLayoutEntry::Type type);

        QList<ActionLayout> children() const;
        void addChild(const ActionLayout &child);
        void setChildren(const QList<ActionLayout> &children);

    protected:
        QSharedDataPointer<ActionLayoutData> d;

        friend class ActionDomain;
    };

    class ActionRegistryPrivate;

    class QAK_CORE_EXPORT ActionRegistry : public ActionFamily {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionRegistry)
    public:
        explicit ActionRegistry(QObject *parent = nullptr);
        ~ActionRegistry();

    public:
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

        QStringList actionIds() const;
        ActionItemInfo actionInfo(const QString &id) const;
        ActionCatalog catalog() const;

    public:
        QList<ActionLayout> layouts() const;
        void setLayouts(const QList<ActionLayout> &layouts);
        void resetLayouts();

        inline QList<QKeySequence> actionShortcuts(const QString &id) const;
        inline QIcon actionIcon(const QString &theme, const QString &id) const;

    public:
        static QJsonObject layoutsToJson(const ShortcutsFamily &shortcutsFamily);
        static ShortcutsFamily layoutsFromJson(const QJsonObject &obj);

    public:
        void addContext(ActionContext *builder);
        void removeContext(ActionContext *builder);

        void updateContextLayouts();
        void updateContextTexts();
        void updateContextKeymap();
        void updateContextIcons();

    protected:
        ActionRegistry(ActionRegistryPrivate &d, QObject *parent = nullptr);
    };

    inline QList<QKeySequence> ActionRegistry::actionShortcuts(const QString &id) const {
        if (const auto o = shortcuts(id); o) {
            return o.value();
        }
        return actionInfo(id).shortcuts();
    }

    inline QIcon ActionRegistry::actionIcon(const QString &theme, const QString &id) const {
        if (const auto o = icon(id); o) {
            return o->isLocalFile() ? QIcon(o->data()) : icon(theme, o->data());
        }
        return icon(theme, id);
    }

}

#endif // ACTIONREGISTRY_H