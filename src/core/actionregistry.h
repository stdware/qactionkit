#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <QtCore/QMap>
#include <QtCore/QSharedData>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionfamily.h>

namespace QAK {

    class ActionContext;

    class ActionRegistryPrivate;

    /// \class ActionCatalog
    /// \brief ActionCatalog is a multi-way tree that defines the logical hierarchy of Actions,
    /// Groups, or Menus within a window. It can be constructed using either an adjacency map or a
    /// child-parent mapping table.​
    class ActionCatalog {
    public:
        /// Default constructor.
        inline ActionCatalog() = default;
        /// Constructs from an adjacency map.
        inline explicit ActionCatalog(const QVector<QPair<QString, QString>> &input) {
            setParentMap(input);
        }
        /// Constructs from a child-parent mapping table.
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
        QAK_CORE_EXPORT void setParentMap(const QVector<QPair<QString, QString>> &input);

    protected:
        QMap<QString, QStringList> m_adjacencyMap;
        QMap<QString, QString> m_parentMap;
    };

    /// \class ActionLayouts
    /// \brief ActionLayouts is a directed acyclic graph (DAG) that defines the compositional
    /// relationships between action, group, menu, and other menu elements within a view. It is
    /// stored as an adjacency map and also retains hashes of each \c ActionExtension participating
    /// in its construction.​
    class ActionLayouts {
    public:
        /// Default constructor.
        inline ActionLayouts() = default;
        /// Constructs from an adjacency map and a list of \c ActionExtension hashes.
        inline explicit ActionLayouts(const QMap<QString, QVector<ActionLayoutEntry>> &input,
                                      const QStringList &hashList)
            : m_adjacencyMap(input), m_hashList(hashList) {
        }

    public:
        inline QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap() const {
            return m_adjacencyMap;
        }
        inline QStringList hashList() const {
            return m_hashList;
        }
        QAK_CORE_EXPORT QJsonObject toJsonObject() const;
        QAK_CORE_EXPORT static ActionLayouts fromJsonObject(const QJsonObject &obj);

    protected:
        QMap<QString, QVector<ActionLayoutEntry>> m_adjacencyMap;
        QStringList m_hashList; // hash of extensions
    };

    /// \class ActionRegistry
    /// \brief ActionRegistry is a central repository of all \c ActionExtension instances and
    /// manages the catalog and layouts.
    class QAK_CORE_EXPORT ActionRegistry : public ActionFamily {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionRegistry)
    public:
        explicit ActionRegistry(QObject *parent = nullptr);
        ~ActionRegistry();

    public:
        QList<const ActionExtension *> extensions() const;
        void setExtensions(const QList<const ActionExtension *> &extensions);
        void addExtension(const ActionExtension *extension);

        QStringList actionIds() const;
        ActionItemInfo actionInfo(const QString &id) const;
        ActionCatalog catalog() const;

    public:
        ActionLayouts layouts() const;
        void setLayouts(const ActionLayouts &layouts);
        void resetLayouts();

        inline QList<QKeySequence> actionShortcuts(const QString &id) const;

    public:
        /// Registers a context with the registry.
        void addContext(ActionContext *ctx);
        /// Unregisters a context from the registry.
        void removeContext(ActionContext *ctx);
        /// Updates the context of all contexts that are registered with the registry.
        void updateContext(ActionElement element);

    protected:
        explicit ActionRegistry(ActionRegistryPrivate &d, QObject *parent = nullptr);
    };

    inline QList<QKeySequence> ActionRegistry::actionShortcuts(const QString &id) const {
        if (const auto o = shortcuts(id); o) {
            return o.value();
        }
        return actionInfo(id).shortcuts();
    }

}

#endif // ACTIONREGISTRY_H