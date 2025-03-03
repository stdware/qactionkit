#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <QtCore/QMap>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionfamily.h>

namespace QAK {

    class ActionContext;

    class ActionRegistryPrivate;

    class QAK_CORE_EXPORT ActionRegistry : public ActionFamily {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionRegistry)
    public:
        explicit ActionRegistry(QObject *parent = nullptr);
        ~ActionRegistry();

        using Catalog = QMap<QString /* id */, QStringList /* child ids */>;
        using Layouts = QMap<QString /* id */, QList<ActionLayoutEntry> /* children */>;

    public:
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

        QStringList actionIds() const;
        ActionItemInfo actionInfo(const QString &id) const;
        Catalog catalog() const;

    public:
        Layouts layouts() const;
        void setLayouts(const Layouts &layouts);
        void resetLayouts();

        inline QList<QKeySequence> actionShortcuts(const QString &id) const;
        inline QIcon actionIcon(const QString &theme, const QString &id) const;

    public:
        static QJsonObject layoutsToJson(const Layouts &shortcutsFamily);
        static Layouts layoutsFromJson(const QJsonObject &obj);

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