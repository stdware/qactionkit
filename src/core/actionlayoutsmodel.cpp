#include "actionlayoutsmodel.h"

#include <QHash>
#include <QQueue>
#include <QSet>

namespace QAK {

    // Path represents a path from root to a node in the DAG tree view
    using NodePath = QStringList;

    class ActionLayoutsModelPrivate {
        Q_DECLARE_PUBLIC(ActionLayoutsModel)
    public:
        ActionLayoutsModel *q_ptr;
        
        // Original DAG adjacency map
        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        QStringList hashList;
        
        // Explicit top-level nodes list
        QVector<ActionLayoutEntry> topLevelNodes;
        
        // Cache for path-based tree view
        mutable QHash<quintptr, NodePath> pathCache;
        mutable quintptr nextPathId = 1;
        
        // Reverse mapping: nodeId -> list of paths containing this node
        mutable QHash<QString, QList<NodePath>> nodePathsMap;
        mutable bool cacheValid = false;

        NodePath pathFromIndex(const QModelIndex &index) const;
        QModelIndex indexFromPath(const NodePath &path) const;
        quintptr cachePathId(const NodePath &path) const;
        NodePath pathFromCacheId(quintptr cacheId) const;
        void rebuildCache() const;
        void clearCache();
        
        ActionLayoutEntry entryAtPath(const NodePath &path) const;
        QVector<ActionLayoutEntry> childrenAtPath(const NodePath &path) const;
        NodePath parentPath(const NodePath &path) const;
        int findRowInParent(const NodePath &path) const;
        
        // DAG manipulation
        bool wouldCreateCycle(const QString &sourceNodeId, const QString &targetNodeId) const;
        bool hasPath(const QString &from, const QString &to) const;
        void synchronizeNodeChanges(const QString &nodeId, const QVector<ActionLayoutEntry> &newChildren);
        
        // Validation helpers
        bool validateActionLayoutEntry(const ActionLayoutEntry &entry) const;
        bool validateEntryChange(const NodePath &path, const ActionLayoutEntry &oldEntry, const ActionLayoutEntry &newEntry) const;
    };

    NodePath ActionLayoutsModelPrivate::pathFromIndex(const QModelIndex &index) const {
        if (!index.isValid()) {
            return NodePath(); // Root path (empty)
        }
        return pathFromCacheId(index.internalId());
    }

    QModelIndex ActionLayoutsModelPrivate::indexFromPath(const NodePath &path) const {
        Q_Q(const ActionLayoutsModel);
        
        if (path.isEmpty()) {
            return QModelIndex(); // Root index
        }

        NodePath parentPath = this->parentPath(path);
        int row = findRowInParent(path);
        
        if (row == -1) {
            return QModelIndex();
        }

        quintptr cacheId = cachePathId(path);
        return q->createIndex(row, 0, cacheId);
    }

    quintptr ActionLayoutsModelPrivate::cachePathId(const NodePath &path) const {
        // Find existing cache entry
        for (auto it = pathCache.begin(); it != pathCache.end(); ++it) {
            if (it.value() == path) {
                return it.key();
            }
        }
        
        // Create new cache entry
        quintptr cacheId = nextPathId++;
        pathCache.insert(cacheId, path);
        return cacheId;
    }

    NodePath ActionLayoutsModelPrivate::pathFromCacheId(quintptr cacheId) const {
        return pathCache.value(cacheId);
    }

    void ActionLayoutsModelPrivate::rebuildCache() const {
        if (cacheValid) {
            return;
        }
        
        nodePathsMap.clear();
        
        // Build all possible paths in the DAG
        QQueue<NodePath> queue;
        QSet<NodePath> visited;
        
        // Start from top-level nodes
        NodePath rootPath;
        
        for (const auto &entry : topLevelNodes) {
            if (!entry.id().isEmpty()) {
                NodePath childPath = rootPath;
                childPath.append(entry.id());
                queue.enqueue(childPath);
                nodePathsMap[entry.id()].append(childPath);
            }
        }
        
        while (!queue.isEmpty()) {
            NodePath currentPath = queue.dequeue();
            if (visited.contains(currentPath)) {
                continue; // Avoid infinite loops in case of cycles
            }
            visited.insert(currentPath);
            
            QString currentNodeId = currentPath.last();
            auto children = adjacencyMap.value(currentNodeId);
            
            for (const auto &entry : children) {
                if (!entry.id().isEmpty()) {
                    NodePath childPath = currentPath;
                    childPath.append(entry.id());
                    
                    // Check if this would create too deep nesting (cycle detection)
                    if (childPath.count(entry.id()) <= 1) { // Allow node to appear at most once in a path
                        queue.enqueue(childPath);
                        nodePathsMap[entry.id()].append(childPath);
                    }
                }
            }
        }
        
        cacheValid = true;
    }

    void ActionLayoutsModelPrivate::clearCache() {
        pathCache.clear();
        nodePathsMap.clear();
        nextPathId = 1;
        cacheValid = false;
    }

    ActionLayoutEntry ActionLayoutsModelPrivate::entryAtPath(const NodePath &path) const {
        if (path.isEmpty()) {
            return ActionLayoutEntry(); // Invalid entry for root
        }
        
        QString nodeId = path.last();
        
        // Find the entry by traversing the path
        NodePath parentPath = this->parentPath(path);
        auto parentChildren = childrenAtPath(parentPath);
        
        for (const auto &entry : parentChildren) {
            if (entry.id() == nodeId) {
                return entry;
            }
        }
        
        return ActionLayoutEntry(); // Not found
    }

    QVector<ActionLayoutEntry> ActionLayoutsModelPrivate::childrenAtPath(const NodePath &path) const {
        if (path.isEmpty()) {
            // Return top-level nodes instead of adjacency map lookup
            return topLevelNodes;
        }
        
        QString nodeId = path.last();
        return adjacencyMap.value(nodeId);
    }

    NodePath ActionLayoutsModelPrivate::parentPath(const NodePath &path) const {
        if (path.isEmpty()) {
            return NodePath(); // Root has no parent
        }
        
        NodePath parent = path;
        parent.removeLast();
        return parent;
    }

    int ActionLayoutsModelPrivate::findRowInParent(const NodePath &path) const {
        if (path.isEmpty()) {
            return -1; // Root has no row
        }
        
        NodePath parentPath = this->parentPath(path);
        auto siblings = childrenAtPath(parentPath);
        QString nodeId = path.last();
        
        for (int i = 0; i < siblings.size(); ++i) {
            if (siblings[i].id() == nodeId) {
                return i;
            }
        }
        
        return -1;
    }

    bool ActionLayoutsModelPrivate::wouldCreateCycle(const QString &sourceNodeId, const QString &targetNodeId) const {
        return hasPath(targetNodeId, sourceNodeId);
    }

    bool ActionLayoutsModelPrivate::hasPath(const QString &from, const QString &to) const {
        if (from == to) {
            return true;
        }
        
        QSet<QString> visited;
        QQueue<QString> queue;
        queue.enqueue(from);
        
        while (!queue.isEmpty()) {
            QString current = queue.dequeue();
            if (visited.contains(current)) {
                continue;
            }
            visited.insert(current);
            
            auto children = adjacencyMap.value(current);
            for (const auto &entry : children) {
                QString childId = entry.id();
                if (childId == to) {
                    return true;
                }
                if (!childId.isEmpty() && !visited.contains(childId)) {
                    queue.enqueue(childId);
                }
            }
        }
        
        return false;
    }

    void ActionLayoutsModelPrivate::synchronizeNodeChanges(const QString &nodeId, const QVector<ActionLayoutEntry> &newChildren) {
        if (nodeId.isEmpty()) {
            // Special case: updating top-level nodes
            topLevelNodes = newChildren;
        } else {
            adjacencyMap[nodeId] = newChildren;
        }
        clearCache(); // Invalidate cache after changes
    }

    bool ActionLayoutsModelPrivate::validateActionLayoutEntry(const ActionLayoutEntry &entry) const {
        // Check: (type == Stretch || type == Separator) == (id is empty)
        bool isSpecialType = (entry.type() == ActionLayoutEntry::Stretch || 
                             entry.type() == ActionLayoutEntry::Separator);
        bool hasEmptyId = entry.id().isEmpty();
        
        return isSpecialType == hasEmptyId;
    }

    bool ActionLayoutsModelPrivate::validateEntryChange(const NodePath &path, const ActionLayoutEntry &oldEntry, const ActionLayoutEntry &newEntry) const {
        // First check basic entry validity
        if (!validateActionLayoutEntry(newEntry)) {
            return false;
        }
        
        // If id changed, check for cycles
        if (oldEntry.id() != newEntry.id()) {
            // Get the parent path to check cycle from parent to new id
            NodePath parentPath = this->parentPath(path);
            QString parentNodeId = parentPath.isEmpty() ? QString() : parentPath.last();
            
            // If new entry has non-empty id, check if it would create a cycle
            if (!newEntry.id().isEmpty()) {
                // Check if adding this id as child of parent would create cycle
                if (wouldCreateCycle(newEntry.id(), parentNodeId)) {
                    return false;
                }
            }
        }
        
        return true;
    }

    ActionLayoutsModel::ActionLayoutsModel(QObject *parent)
        : QAbstractItemModel(parent), d_ptr(new ActionLayoutsModelPrivate) {
        Q_D(ActionLayoutsModel);
        d->q_ptr = this;
    }

    ActionLayoutsModel::~ActionLayoutsModel() = default;

    ActionLayouts ActionLayoutsModel::actionLayouts() const {
        Q_D(const ActionLayoutsModel);
        
        return ActionLayouts(d->adjacencyMap, d->hashList);
    }

    void ActionLayoutsModel::setActionLayouts(const ActionLayouts &layouts) {
        Q_D(ActionLayoutsModel);
        
        beginResetModel();
        d->adjacencyMap = layouts.adjacencyMap();
        d->hashList = layouts.hashList();
        
        d->clearCache();
        endResetModel();
    }

    QVector<ActionLayoutEntry> ActionLayoutsModel::topLevelNodes() const {
        Q_D(const ActionLayoutsModel);
        return d->topLevelNodes;
    }

    void ActionLayoutsModel::setTopLevelNodes(const QVector<ActionLayoutEntry> &nodes) {
        Q_D(ActionLayoutsModel);
        
        beginResetModel();
        d->topLevelNodes = nodes;
        d->clearCache();
        endResetModel();
    }

    QModelIndex ActionLayoutsModel::index(int row, int column, const QModelIndex &parent) const {
        Q_D(const ActionLayoutsModel);
        
        if (!hasIndex(row, column, parent)) {
            return QModelIndex();
        }

        d->rebuildCache();
        
        NodePath parentPath = d->pathFromIndex(parent);
        auto children = d->childrenAtPath(parentPath);
        
        if (row >= children.size()) {
            return QModelIndex();
        }

        const auto &entry = children[row];
        NodePath childPath = parentPath;
        
        if (entry.id().isEmpty()) {
            // For Separator/Stretch, use index as identifier
            childPath.append(QString("__sep_stretch_%1").arg(row));
        } else {
            childPath.append(entry.id());
        }
        
        quintptr cacheId = d->cachePathId(childPath);
        return createIndex(row, column, cacheId);
    }

    QModelIndex ActionLayoutsModel::parent(const QModelIndex &child) const {
        Q_D(const ActionLayoutsModel);
        
        if (!child.isValid()) {
            return QModelIndex();
        }

        NodePath childPath = d->pathFromIndex(child);
        NodePath parentPath = d->parentPath(childPath);
        
        return d->indexFromPath(parentPath);
    }

    int ActionLayoutsModel::rowCount(const QModelIndex &parent) const {
        Q_D(const ActionLayoutsModel);
        
        NodePath parentPath = d->pathFromIndex(parent);
        return d->childrenAtPath(parentPath).size();
    }

    int ActionLayoutsModel::columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent)
        return 1;
    }

    QVariant ActionLayoutsModel::data(const QModelIndex &index, int role) const {
        Q_D(const ActionLayoutsModel);
        
        if (!index.isValid()) {
            return QVariant();
        }

        NodePath path = d->pathFromIndex(index);
        
        if (path.isEmpty()) {
            return QVariant();
        }
        
        // Handle Separator/Stretch specially
        if (path.last().startsWith("__sep_stretch_")) {
            NodePath parentPath = d->parentPath(path);
            auto siblings = d->childrenAtPath(parentPath);
            int row = index.row();
            
            if (row < siblings.size()) {
                const auto &entry = siblings[row];
                
                switch (role) {
                case Qt::DisplayRole:
                    return QString(); // Empty string for Separator/Stretch
                case Qt::UserRole:
                    return QVariant::fromValue(entry);
                }
            }
            return QVariant();
        }
        
        ActionLayoutEntry entry = d->entryAtPath(path);
        
        switch (role) {
        case Qt::DisplayRole:
            return entry.id();
        case Qt::UserRole:
            return QVariant::fromValue(entry);
        }

        return QVariant();
    }

    QHash<int, QByteArray> ActionLayoutsModel::roleNames() const {
        QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
        roles[Qt::UserRole] = "entry";
        return roles;
    }

    Qt::ItemFlags ActionLayoutsModel::flags(const QModelIndex &index) const {
        Q_D(const ActionLayoutsModel);
        
        if (!index.isValid()) {
            return Qt::ItemIsDropEnabled;
        }
        
        NodePath path = d->pathFromIndex(index);
        NodePath parentPath = d->parentPath(path);
        
        // Top-level nodes are read-only
        if (parentPath.isEmpty()) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
        
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
    }

    bool ActionLayoutsModel::validateSetData(const QModelIndex &index, const QVariant &value, int role) const {
        Q_D(const ActionLayoutsModel);
        
        if (!index.isValid()) {
            return false;
        }
        
        if (role != Qt::UserRole) {
            return false;
        }
        
        if (!value.canConvert<ActionLayoutEntry>()) {
            return false;
        }
        
        ActionLayoutEntry newEntry = value.value<ActionLayoutEntry>();
        NodePath path = d->pathFromIndex(index);
        
        if (path.isEmpty()) {
            return false;
        }
        
        // Prevent modification of top-level nodes
        NodePath parentPath = d->parentPath(path);
        if (parentPath.isEmpty()) {
            return false;
        }
        
        // Get current entry
        ActionLayoutEntry currentEntry;
        if (path.last().startsWith("__sep_stretch_")) {
            // Handle Separator/Stretch specially
            auto siblings = d->childrenAtPath(parentPath);
            int row = index.row();
            
            if (row >= siblings.size()) {
                return false;
            }
            currentEntry = siblings[row];
        } else {
            currentEntry = d->entryAtPath(path);
        }
        
        return d->validateEntryChange(path, currentEntry, newEntry);
    }

    bool ActionLayoutsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        Q_D(ActionLayoutsModel);
        
        // First validate the change
        if (!validateSetData(index, value, role)) {
            return false;
        }
        
        ActionLayoutEntry newEntry = value.value<ActionLayoutEntry>();
        NodePath path = d->pathFromIndex(index);
        NodePath parentPath = d->parentPath(path);
        
        // Get current children list
        auto currentChildren = d->childrenAtPath(parentPath);
        int row = index.row();
        
        if (row >= currentChildren.size()) {
            return false;
        }
        
        ActionLayoutEntry oldEntry = currentChildren[row];
        
        // Update the entry
        currentChildren[row] = newEntry;
        
        // Update the adjacency map
        QString parentNodeId = parentPath.isEmpty() ? QString() : parentPath.last();
        d->adjacencyMap[parentNodeId] = currentChildren;
        
        // If the entry ID changed, we need to synchronize all instances of the old ID
        if (oldEntry.id() != newEntry.id() && !oldEntry.id().isEmpty()) {
            d->rebuildCache(); // Ensure cache is up to date
            
            // Find all paths that contain the old ID and update them
            auto pathsWithOldId = d->nodePathsMap.value(oldEntry.id());
            
            for (const auto &pathToUpdate : pathsWithOldId) {
                if (pathToUpdate == path) {
                    continue; // Already updated above
                }
                
                // Update this path's parent's children list
                NodePath updateParentPath = d->parentPath(pathToUpdate);
                auto updateParentChildren = d->childrenAtPath(updateParentPath);
                
                // Find the entry in the parent's children list
                for (int i = 0; i < updateParentChildren.size(); ++i) {
                    if (updateParentChildren[i].id() == oldEntry.id()) {
                        updateParentChildren[i] = newEntry;
                    }
                }
                
                // Update the adjacency map for this parent
                QString updateParentNodeId = updateParentPath.isEmpty() ? QString() : updateParentPath.last();
                d->adjacencyMap[updateParentNodeId] = updateParentChildren;
            }
        }
        
        // Clear cache to reflect changes
        d->clearCache();
        
        // Emit data changed
        emit dataChanged(index, index, {role});
        
        return true;
    }

    bool ActionLayoutsModel::insertRows(int row, int count, const QModelIndex &parent) {
        Q_D(ActionLayoutsModel);
        
        if (count <= 0) {
            return false;
        }
        
        NodePath parentPath = d->pathFromIndex(parent);
        
        // Prevent insertion at top level
        if (parentPath.isEmpty()) {
            return false;
        }
        
        auto currentChildren = d->childrenAtPath(parentPath);
        
        if (row < 0 || row > currentChildren.size()) {
            return false;
        }
        
        beginInsertRows(parent, row, row + count - 1);
        
        // Insert empty entries
        for (int i = 0; i < count; ++i) {
            currentChildren.insert(row + i, ActionLayoutEntry());
        }
        
        QString parentNodeId = parentPath.isEmpty() ? QString() : parentPath.last();
        d->synchronizeNodeChanges(parentNodeId, currentChildren);
        
        endInsertRows();
        return true;
    }

    bool ActionLayoutsModel::removeRows(int row, int count, const QModelIndex &parent) {
        Q_D(ActionLayoutsModel);
        
        if (count <= 0) {
            return false;
        }
        
        NodePath parentPath = d->pathFromIndex(parent);
        
        // Prevent removal of top-level nodes
        if (parentPath.isEmpty()) {
            return false;
        }
        
        auto currentChildren = d->childrenAtPath(parentPath);
        
        if (row < 0 || row + count > currentChildren.size()) {
            return false;
        }
        
        beginRemoveRows(parent, row, row + count - 1);
        
        // Remove entries
        for (int i = 0; i < count; ++i) {
            currentChildren.removeAt(row);
        }
        
        QString parentNodeId = parentPath.isEmpty() ? QString() : parentPath.last();
        d->synchronizeNodeChanges(parentNodeId, currentChildren);
        
        endRemoveRows();
        return true;
    }

    bool ActionLayoutsModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                      const QModelIndex &destinationParent, int destinationChild) {
        Q_D(ActionLayoutsModel);
        
        if (count <= 0) {
            return false;
        }
        
        NodePath srcParentPath = d->pathFromIndex(sourceParent);
        NodePath dstParentPath = d->pathFromIndex(destinationParent);
        
        // Prevent moving top-level nodes (source)
        if (srcParentPath.isEmpty()) {
            return false;
        }
        
        // Prevent moving to top level (destination)
        if (dstParentPath.isEmpty()) {
            return false;
        }
        
        auto srcChildren = d->childrenAtPath(srcParentPath);
        auto dstChildren = d->childrenAtPath(dstParentPath);
        
        if (sourceRow < 0 || sourceRow + count > srcChildren.size()) {
            return false;
        }
        
        if (destinationChild < 0 || destinationChild > dstChildren.size()) {
            return false;
        }
        
        // Extract entries to move
        QVector<ActionLayoutEntry> entriesToMove;
        for (int i = 0; i < count; ++i) {
            entriesToMove.append(srcChildren[sourceRow + i]);
        }
        
        // Check for cycles when moving between different parents
        if (srcParentPath != dstParentPath) {
            QString dstParentNodeId = dstParentPath.isEmpty() ? QString() : dstParentPath.last();
            
            for (const auto &entry : entriesToMove) {
                if (!entry.id().isEmpty() && d->wouldCreateCycle(entry.id(), dstParentNodeId)) {
                    return false; // Would create cycle
                }
            }
        }
        
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
        
        // Remove from source
        for (int i = 0; i < count; ++i) {
            srcChildren.removeAt(sourceRow);
        }
        
        // Insert to destination
        if (srcParentPath == dstParentPath) {
            // Moving within same parent, adjust destination index
            if (destinationChild > sourceRow) {
                destinationChild -= count;
            }
            dstChildren = srcChildren; // Same list after removal
        }
        
        for (int i = 0; i < count; ++i) {
            dstChildren.insert(destinationChild + i, entriesToMove[i]);
        }
        
        // Update both parents
        QString srcParentNodeId = srcParentPath.isEmpty() ? QString() : srcParentPath.last();
        QString dstParentNodeId = dstParentPath.isEmpty() ? QString() : dstParentPath.last();
        
        d->synchronizeNodeChanges(srcParentNodeId, srcChildren);
        if (srcParentPath != dstParentPath) {
            d->synchronizeNodeChanges(dstParentNodeId, dstChildren);
        }
        
        endMoveRows();
        return true;
    }

} // QAK
