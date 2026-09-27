#include "actionlayoutsmodel.h"

#include <memory>
#include <vector>

#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QSet>

namespace QAK {

    // A node of the tree that shows the graph. The children of a node are built when they are
    // first needed, so that a large graph with shared menus is not expanded as a whole.
    struct LayoutNode {
        LayoutNode *parent = nullptr;
        ActionLayoutEntry entry; // Null for the root
        bool built = false;
        std::vector<std::unique_ptr<LayoutNode>> children;
    };

    static bool isAnonymous(const ActionLayoutEntry &entry) {
        return entry.type() == ActionLayoutEntry::Separator ||
               entry.type() == ActionLayoutEntry::Stretch;
    }

    static bool isContainer(const ActionLayoutEntry &entry) {
        return entry.type() == ActionLayoutEntry::Menu || entry.type() == ActionLayoutEntry::Group;
    }

    class ActionLayoutsModelPrivate {
        Q_DECLARE_PUBLIC(ActionLayoutsModel)
    public:
        ActionLayoutsModel *q_ptr;

        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        QVector<ActionLayoutEntry> topLevelNodes;
        QPointer<ActionRegistry> registry;

        mutable LayoutNode root;

        void resetTree();
        LayoutNode *nodeOf(const QModelIndex &index) const;
        QModelIndex indexOf(const LayoutNode *node) const;
        int rowOf(const LayoutNode *node) const;
        void buildChildren(LayoutNode *node) const;
        std::unique_ptr<LayoutNode> makeNode(LayoutNode *parent,
                                             const ActionLayoutEntry &entry) const;

        bool isShared(const LayoutNode *node) const;
        bool hasPath(const QString &from, const QString &to) const;
        bool acceptsEntry(const LayoutNode *container, const ActionLayoutEntry &entry) const;
    };

    void ActionLayoutsModelPrivate::resetTree() {
        root.children.clear();
        root.built = false;
    }

    LayoutNode *ActionLayoutsModelPrivate::nodeOf(const QModelIndex &index) const {
        return index.isValid() ? static_cast<LayoutNode *>(index.internalPointer()) : &root;
    }

    QModelIndex ActionLayoutsModelPrivate::indexOf(const LayoutNode *node) const {
        Q_Q(const ActionLayoutsModel);
        if (node == &root) {
            return {};
        }
        return q->createIndex(rowOf(node), 0, const_cast<LayoutNode *>(node));
    }

    int ActionLayoutsModelPrivate::rowOf(const LayoutNode *node) const {
        const auto &siblings = node->parent->children;
        for (int row = 0; row < int(siblings.size()); ++row) {
            if (siblings[row].get() == node) {
                return row;
            }
        }
        Q_UNREACHABLE_RETURN(-1);
    }

    std::unique_ptr<LayoutNode>
        ActionLayoutsModelPrivate::makeNode(LayoutNode *parent,
                                            const ActionLayoutEntry &entry) const {
        auto node = std::make_unique<LayoutNode>();
        node->parent = parent;
        node->entry = entry;
        return node;
    }

    // Each child of the node corresponds to the entry of the same row, so that a row is also the
    // index into the children in the graph. A menu that already encloses the node, which only a
    // graph with a cycle has, is shown without its children.
    void ActionLayoutsModelPrivate::buildChildren(LayoutNode *node) const {
        if (node->built) {
            return;
        }
        node->built = true;
        QVector<ActionLayoutEntry> entries;
        if (node == &root) {
            entries = topLevelNodes;
        } else if (isContainer(node->entry)) {
            entries = adjacencyMap.value(node->entry.id());
        }
        for (const auto &entry : std::as_const(entries)) {
            auto child = makeNode(node, entry);
            for (auto ancestor = node; ancestor != &root && !isAnonymous(entry);
                 ancestor = ancestor->parent) {
                if (ancestor->entry.id() == entry.id()) {
                    child->built = true;
                    break;
                }
            }
            node->children.push_back(std::move(child));
        }
    }

    // Returns whether the children of the container that node shows appear in another built node,
    // which an edit of them changes as well
    bool ActionLayoutsModelPrivate::isShared(const LayoutNode *node) const {
        const auto id = node->entry.id();
        int count = 0;
        QQueue<const LayoutNode *> queue;
        queue.enqueue(&root);
        while (!queue.isEmpty()) {
            const auto current = queue.dequeue();
            if (current != &root && current->built && isContainer(current->entry) &&
                current->entry.id() == id && ++count > 1) {
                return true;
            }
            for (const auto &child : current->children) {
                queue.enqueue(child.get());
            }
        }
        return false;
    }

    bool ActionLayoutsModelPrivate::hasPath(const QString &from, const QString &to) const {
        QSet<QString> visited;
        QQueue<QString> queue;
        queue.enqueue(from);
        while (!queue.isEmpty()) {
            const auto current = queue.dequeue();
            if (current == to) {
                return true;
            }
            if (visited.contains(current)) {
                continue;
            }
            visited.insert(current);
            for (const auto &entry : adjacencyMap.value(current)) {
                if (!isAnonymous(entry)) {
                    queue.enqueue(entry.id());
                }
            }
        }
        return false;
    }

    bool ActionLayoutsModelPrivate::acceptsEntry(const LayoutNode *container,
                                                 const ActionLayoutEntry &entry) const {
        if (container == &root || !isContainer(container->entry)) {
            return false;
        }
        if (isAnonymous(entry)) {
            return entry.id().isEmpty();
        }
        if (entry.id().isEmpty() || hasPath(entry.id(), container->entry.id())) {
            return false;
        }
        if (!registry) {
            return true;
        }
        const auto info = registry->actionInfo(entry.id());
        if (!info) {
            return false;
        }
        switch (info->type()) {
            case ActionItemInfo::Action:
                return entry.type() == ActionLayoutEntry::Action;
            case ActionItemInfo::Menu:
            case ActionItemInfo::Group:
                return isContainer(entry);
            case ActionItemInfo::Phony:
                break;
        }
        return false;
    }

    ActionLayoutsModel::ActionLayoutsModel(QObject *parent)
        : QAbstractItemModel(parent), d_ptr(new ActionLayoutsModelPrivate) {
        Q_D(ActionLayoutsModel);
        d->q_ptr = this;
    }

    ActionLayoutsModel::~ActionLayoutsModel() = default;

    ActionLayouts ActionLayoutsModel::actionLayouts() const {
        Q_D(const ActionLayoutsModel);
        return ActionLayouts(d->adjacencyMap);
    }

    void ActionLayoutsModel::setActionLayouts(const ActionLayouts &layouts) {
        Q_D(ActionLayoutsModel);
        beginResetModel();
        d->adjacencyMap = layouts.adjacencyMap();
        d->resetTree();
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
        d->resetTree();
        endResetModel();
    }

    ActionRegistry *ActionLayoutsModel::registry() const {
        Q_D(const ActionLayoutsModel);
        return d->registry;
    }

    void ActionLayoutsModel::setRegistry(ActionRegistry *registry) {
        Q_D(ActionLayoutsModel);
        d->registry = registry;
    }

    QModelIndex ActionLayoutsModel::index(int row, int column, const QModelIndex &parent) const {
        Q_D(const ActionLayoutsModel);
        if (!hasIndex(row, column, parent)) {
            return {};
        }
        const auto node = d->nodeOf(parent);
        d->buildChildren(node);
        return createIndex(row, column, node->children[row].get());
    }

    QModelIndex ActionLayoutsModel::parent(const QModelIndex &child) const {
        Q_D(const ActionLayoutsModel);
        if (!child.isValid()) {
            return {};
        }
        return d->indexOf(d->nodeOf(child)->parent);
    }

    int ActionLayoutsModel::rowCount(const QModelIndex &parent) const {
        Q_D(const ActionLayoutsModel);
        if (parent.column() > 0) {
            return 0;
        }
        const auto node = d->nodeOf(parent);
        d->buildChildren(node);
        return int(node->children.size());
    }

    int ActionLayoutsModel::columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent)
        return 1;
    }

    QVariant ActionLayoutsModel::data(const QModelIndex &index, int role) const {
        Q_D(const ActionLayoutsModel);
        if (!index.isValid()) {
            return {};
        }
        const auto &entry = d->nodeOf(index)->entry;
        switch (role) {
            case Qt::DisplayRole:
                return entry.id();
            case Qt::UserRole:
                return QVariant::fromValue(entry);
            default:
                break;
        }
        return {};
    }

    QHash<int, QByteArray> ActionLayoutsModel::roleNames() const {
        QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
        roles[Qt::UserRole] = "entry";
        return roles;
    }

    Qt::ItemFlags ActionLayoutsModel::flags(const QModelIndex &index) const {
        Q_D(const ActionLayoutsModel);
        if (!index.isValid()) {
            return Qt::NoItemFlags;
        }
        const auto node = d->nodeOf(index);
        Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (isContainer(node->entry)) {
            flags |= Qt::ItemIsDropEnabled;
        }
        // The top-level nodes are read-only
        if (node->parent != &d->root) {
            flags |= Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
        }
        return flags;
    }

    bool ActionLayoutsModel::validateSetData(const QModelIndex &index, const QVariant &value,
                                             int role) const {
        Q_D(const ActionLayoutsModel);
        if (!index.isValid() || role != Qt::UserRole || !value.canConvert<ActionLayoutEntry>()) {
            return false;
        }
        return d->acceptsEntry(d->nodeOf(index)->parent, value.value<ActionLayoutEntry>());
    }

    // The entry of the row is replaced, and so are the rows under it, which show the children of
    // the new entry
    bool ActionLayoutsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        Q_D(ActionLayoutsModel);
        if (!validateSetData(index, value, role)) {
            return false;
        }
        const auto node = d->nodeOf(index);
        const auto container = node->parent;
        const auto entry = value.value<ActionLayoutEntry>();
        const int row = index.row();

        if (d->isShared(container)) {
            beginResetModel();
            d->adjacencyMap[container->entry.id()][row] = entry;
            d->resetTree();
            endResetModel();
            return true;
        }

        if (!node->children.empty()) {
            beginRemoveRows(index, 0, int(node->children.size()) - 1);
            node->children.clear();
            endRemoveRows();
        }
        d->adjacencyMap[container->entry.id()][row] = entry;
        node->entry = entry;
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::UserRole});

        // The children are built aside and then attached, as the model reports them
        LayoutNode rebuilt;
        rebuilt.parent = node->parent;
        rebuilt.entry = entry;
        d->buildChildren(&rebuilt);
        node->built = true;
        if (!rebuilt.children.empty()) {
            beginInsertRows(index, 0, int(rebuilt.children.size()) - 1);
            for (auto &child : rebuilt.children) {
                child->parent = node;
                node->children.push_back(std::move(child));
            }
            endInsertRows();
        }
        return true;
    }

    bool ActionLayoutsModel::insertRows(int row, int count, const QModelIndex &parent) {
        Q_D(ActionLayoutsModel);
        const auto container = d->nodeOf(parent);
        const ActionLayoutEntry separator({}, ActionLayoutEntry::Separator);
        if (count <= 0 || !d->acceptsEntry(container, separator)) {
            return false;
        }
        d->buildChildren(container);
        auto &children = d->adjacencyMap[container->entry.id()];
        if (row < 0 || row > children.size()) {
            return false;
        }

        if (d->isShared(container)) {
            beginResetModel();
            children.insert(row, count, separator);
            d->resetTree();
            endResetModel();
            return true;
        }

        beginInsertRows(parent, row, row + count - 1);
        children.insert(row, count, separator);
        for (int i = 0; i < count; ++i) {
            container->children.insert(container->children.begin() + row + i,
                                       d->makeNode(container, separator));
        }
        endInsertRows();
        return true;
    }

    bool ActionLayoutsModel::removeRows(int row, int count, const QModelIndex &parent) {
        Q_D(ActionLayoutsModel);
        const auto container = d->nodeOf(parent);
        if (count <= 0 || container == &d->root || !isContainer(container->entry)) {
            return false;
        }
        d->buildChildren(container);
        auto &children = d->adjacencyMap[container->entry.id()];
        if (row < 0 || row + count > children.size()) {
            return false;
        }

        if (d->isShared(container)) {
            beginResetModel();
            children.remove(row, count);
            d->resetTree();
            endResetModel();
            return true;
        }

        beginRemoveRows(parent, row, row + count - 1);
        children.remove(row, count);
        container->children.erase(container->children.begin() + row,
                                  container->children.begin() + row + count);
        endRemoveRows();
        return true;
    }

    bool ActionLayoutsModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                      const QModelIndex &destinationParent, int destinationChild) {
        Q_D(ActionLayoutsModel);
        const auto source = d->nodeOf(sourceParent);
        const auto destination = d->nodeOf(destinationParent);
        if (count <= 0 || source == &d->root || !isContainer(source->entry)) {
            return false;
        }
        d->buildChildren(source);
        d->buildChildren(destination);
        const auto sourceChildren = d->adjacencyMap.value(source->entry.id());
        if (sourceRow < 0 || sourceRow + count > sourceChildren.size() || destinationChild < 0 ||
            destinationChild > d->adjacencyMap.value(destination->entry.id()).size()) {
            return false;
        }

        const bool sameContainer = source->entry.id() == destination->entry.id();
        const auto moved = sourceChildren.mid(sourceRow, count);
        if (!sameContainer) {
            for (const auto &entry : moved) {
                if (!d->acceptsEntry(destination, entry)) {
                    return false;
                }
            }
        } else if (destinationChild >= sourceRow && destinationChild <= sourceRow + count) {
            // Moving rows to where they are changes nothing, and beginMoveRows() rejects it
            return false;
        }

        const auto applyToMap = [&] {
            d->adjacencyMap[source->entry.id()].remove(sourceRow, count);
            auto &destinationChildren = d->adjacencyMap[destination->entry.id()];
            const int insertAt = sameContainer && destinationChild > sourceRow
                                     ? destinationChild - count
                                     : destinationChild;
            for (int i = 0; i < count; ++i) {
                destinationChildren.insert(insertAt + i, moved[i]);
            }
            return insertAt;
        };

        if (d->isShared(source) || d->isShared(destination)) {
            beginResetModel();
            applyToMap();
            d->resetTree();
            endResetModel();
            return true;
        }

        if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent,
                           destinationChild)) {
            return false;
        }
        const int insertAt = applyToMap();
        std::vector<std::unique_ptr<LayoutNode>> nodes;
        for (int i = 0; i < count; ++i) {
            nodes.push_back(std::move(source->children[sourceRow + i]));
        }
        source->children.erase(source->children.begin() + sourceRow,
                               source->children.begin() + sourceRow + count);
        for (int i = 0; i < count; ++i) {
            nodes[i]->parent = destination;
            destination->children.insert(destination->children.begin() + insertAt + i,
                                         std::move(nodes[i]));
        }
        endMoveRows();
        return true;
    }

} // QAK
