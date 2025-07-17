#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <QtCore/QMap>
#include <QtCore/QSharedData>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionfamily.h>

namespace QAK {

    class ActionContext;

    class ActionRegistryPrivate;

    class ActionCatalogData;

    class ActionCatalog {
    public:
        inline ActionCatalog() = default;
        inline explicit ActionCatalog(const QVector<QPair<QString, QString>> &input) {
            setNodeParentLinks(input);
        }
        inline explicit ActionCatalog(const QMap<QString, QStringList> &input) {
            setAdjacencyTable(input);
        }

    public:
        inline QMap<QString, QStringList> adjacencyTable() const {
            return m_adjacencyMap;
        }

        inline QString parent(const QString &id) const {
            return m_parentMap.value(id);
        }

        inline QStringList children(const QString &id) const {
            return m_adjacencyMap.value(id);
        }

        inline void clear() {
            m_adjacencyMap.clear();
            m_parentMap.clear();
        }

        QAK_CORE_EXPORT void setAdjacencyTable(const QMap<QString, QStringList> &input);
        QAK_CORE_EXPORT void setNodeParentLinks(const QVector<QPair<QString, QString>> &input);

    protected:
        QMap<QString, QStringList> m_adjacencyMap;
        QMap<QString, QString> m_parentMap;
    };

    using ActionLayouts = QMap<QString, QVector<ActionLayoutEntry>>;

    class QAK_CORE_EXPORT ActionRegistry : public ActionFamily {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionRegistry)
    public:
        explicit ActionRegistry(QObject *parent = nullptr);
        ~ActionRegistry();

    public:
        QList<const ActionExtension *> extensions() const;
        void setExtensions(const QList<const ActionExtension *> &extensions);
        
        QStringList actionIds() const;
        ActionItemInfo actionInfo(const QString &id) const;
        ActionCatalog catalog() const;

    public:
        ActionLayouts layouts() const;
        void setLayouts(const ActionLayouts &layouts);
        void resetLayouts();

        inline QList<QKeySequence> actionShortcuts(const QString &id) const;

    public:
        static QJsonObject layoutsToJson(const ActionLayouts &layouts);
        static ActionLayouts layoutsFromJson(const QJsonObject &obj);

    public:
        void addContext(ActionContext *ctx);
        void removeContext(ActionContext *ctx);

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

}

#endif // ACTIONREGISTRY_H