#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <QtCore/QMap>
#include <QtCore/QSharedData>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionfamily.h>

namespace QAK {

    class ActionContext;

    class ActionRegistryPrivate;

    /// The logical hierarchy of the action items, from which a settings page presents them to the
    /// user. The hierarchy is a forest whose root is the empty id, and it is independent of the
    /// layout of the items in the menus.
    class ActionCatalog {
    public:
        /// Default constructor.
        inline ActionCatalog() = default;
        /// Constructs from a list of child-parent pairs.
        inline explicit ActionCatalog(const QVector<QPair<QString, QString>> &input) {
            setParentMap(input);
        }
        /// Constructs from an adjacency table.
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

    /// The composition of the menus, tool bars and groups of a view, stored as the adjacency map
    /// of a directed acyclic graph, together with the hash of every \c ActionExtension from which
    /// it was built. \c ActionRegistry compares the hashes to update a layout saved by the user
    /// after an extension has been added or changed.
    class ActionLayouts {
        Q_GADGET
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

    /// The central repository of the action extensions of an application, holding the catalog, the
    /// layouts and the customizations made by the user. An application normally has one registry,
    /// with one \c ActionContext per window registered with it.
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
        /// Registers \a ctx with the registry, and removes it from the registry it was registered
        /// with before. A destroyed context is removed from its registry.
        void addContext(ActionContext *ctx);
        /// Unregisters \a ctx from the registry.
        void removeContext(ActionContext *ctx);
        /// Rebuilds \a element of every registered context from the current state of the
        /// registry.
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
