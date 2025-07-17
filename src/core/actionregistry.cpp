#include "actionregistry.h"
#include "actionregistry_p.h"

#include <utility>

#include <QtCore/QStack>
#include <QtCore/QQueue>

#include "qakglobal_p.h"
#include "actioncontext_p.h"

namespace QAK::Helper::Catalog {

    using LinkedParentMap = stdc::linked_map<QString, QString, std::map>;

    enum NodeStatus {
        Unchecked,
        Checking,
        Checked,
    };

    // Scans from leaf-side nodes towards the root, marking encountered nodes as 'Checking'.
    //
    // During scanning, if node A is found whose parent B is already marked as 'Checking':
    // - This indicates B is both a descendant of A (in the scanned portion)
    //   and A's parent (in the unscanned portion)
    // We will,
    // - Remove node B from the unscanned tree structure
    // - Promote node A as a new root node
    static void processNode(const QString &startId,
                            // The parent map
                            const LinkedParentMap &parentMap,
                            // The node status will be updated
                            QMap<QString, NodeStatus> &nodeStatus,
                            // The list of root nodes
                            QVector<QString> &rootNodes) {
        QStack<QString> stack;
        stack.push(startId);
        nodeStatus[startId] = Checking;

        while (true) {
            // Find parent
            const auto &currentId = stack.top();
            auto it = parentMap.find(currentId);

            // No parent, marked as root
            if (it == parentMap.end() || it.value().isEmpty()) {
                rootNodes.append(currentId);
                while (!stack.isEmpty()) {
                    nodeStatus[stack.pop()] = Checked;
                }
                return;
            }

            const QString &parentId = it.value();
            NodeStatus parentStatus = nodeStatus.value(parentId, Unchecked);
            switch (parentStatus) {
                case Checking: // Cycle detected, mark the current as root and the rest as non-root
                {
                    qCWarning(qActionKitLog).noquote().nospace()
                        << "Recursive catalog chain: " << stack.join(", ");

                    // Disassociates the node from its parent, this link will be removed later
                    rootNodes.append(currentId);

                    // Note: the parent is demoted to a descendant of the current node as the one
                    // we first encountered in the cycle.

                    // Example:
                    // A -> B -> C -> D -> B
                    //
                    // Scanning:
                    // 0.        (Mark A as Checking)
                    // 1. A -> B (Mark B as Checking)
                    // 2. B -> C (Mark C as Checking)
                    // 3. C -> D (Mark D as Checking)
                    // 4. D -> B (Cycle detected)
                    //
                    // In this case, we will mark D as a new root, and remove the 4th edge (D -> B),
                    // so that the final graph becomes:
                    //
                    // A -> B -> C -> D
                    // D is the new root node, and B, C, and A are its descendants.

                    Q_FALLTHROUGH();
                }
                case Checked: {
                    while (!stack.isEmpty()) {
                        nodeStatus[stack.pop()] = Checked;
                    }
                    return;
                }
                default: // Parent is unchecked, add to stack and mark as Checking
                {
                    stack.push(parentId);
                    nodeStatus[parentId] = Checking;
                    break;
                }
            }
        }
    }

    // Convert a list of pairs (child-id, parent-id) to a map of adjacency lists and a map of parent
    // links
    //
    // input: a list of pairs (child-id, parent-id)
    //   - note: child-id is non-empty, parent-id can be empty if child is root
    // output:
    //   - adjacency list: for each node, a list of its children
    //   - parent link: for each node, its parent (or empty string if root)
    static void extractNodeParentLinks(const QVector<QPair<QString, QString>> &input,
                                       QMap<QString, QStringList> &outAdjacencyMap,
                                       QMap<QString, QString> &outParentMap) {
        LinkedParentMap parentMap;

        // Build `parentMap`
        for (const QPair<QString, QString> &pair : input) {
            const QString &childId = pair.first;
            const QString &parentId = pair.second;
            if (childId.isEmpty()              // empty child id is not allowed
                || childId == parentId         // self-reference is not allowed
                || parentMap.contains(childId) // child id already exists in the map
            ) {
                continue;
            }
            parentMap[childId] = parentId;
        }

        // Traverse and fill `nodeStatus`
        QMap<QString, NodeStatus> nodeStatus;
        QStringList rootNodes;
        {
            for (const auto &[child, parent] : std::as_const(parentMap)) {
                if (parent.isEmpty()) {
                    nodeStatus[child] = Checked;
                    rootNodes.append(child);
                    continue;
                }
                if (nodeStatus.value(parent, Unchecked) == Unchecked) {
                    processNode(parent, parentMap, nodeStatus, rootNodes);
                }
            }

            // We remove all root nodes from `parentMap` and add them to the end of the list.
            // A part of the links should be removed because we have discovered they are in cycles.
            for (const auto &link : std::as_const(rootNodes)) {
                Q_ASSERT(nodeStatus.value(link) == Checked);
                parentMap.erase(link);
            }

            for (const auto &root : std::as_const(rootNodes)) {
                parentMap.append(root, {});
            }
        }

        // Build `childMap`
        QMap<QString, QStringList> adjacencyMap;
        QMap<QString, QString> parentMap2;
        for (const auto &pair : std::as_const(parentMap)) {
            adjacencyMap[pair.second].append(pair.first);
            parentMap2[pair.first] = pair.second;
        }

        outAdjacencyMap = std::move(adjacencyMap);
        outParentMap = std::move(parentMap2);
    }

}

namespace QAK::Helper::Layouts {

    class TopologicalSorter {
    private:
        QMap<QString, QSet<QString>> graph;
        QMap<QString, int> inDegree;

    public:
        QStringList result;

        void addEdge(const QString &u, const QString &v) {
            // Avoid duplicated edge
            if (!graph[u].contains(v)) {
                graph[u].insert(v);
                if (!inDegree.contains(u)) {
                    inDegree[u] = 0;
                }
                inDegree[u];
                inDegree[v]++;
            }
        }

        bool sort() {
            QQueue<QString> queue;
            QStringList sorted;

            // Add all non-in degrees
            for (auto it = inDegree.begin(); it != inDegree.end(); ++it) {
                if (it.value() == 0) {
                    queue.enqueue(it.key());
                }
            }

            while (!queue.isEmpty()) {
                QString u = queue.dequeue();
                sorted.append(u);
                for (const QString &v : std::as_const(graph[u])) {

                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        queue.enqueue(v);
                    }
                }
            }

            // Loop exists if there are degrees that hasn't been processed
            if (sorted.size() != graph.size()) {
                return false;
            }

            result = sorted;
            return true;
        }
    };

}

namespace QAK {

    void ActionCatalog::setAdjacencyTable(const QMap<QString, QStringList> &input) {
        QVector<QPair<QString, QString>> parentLinks;
        for (auto it = input.begin(); it != input.end(); ++it) {
            auto &parentId = it.key();
            auto &children = it.value();
            for (const auto &childId : children) {
                parentLinks.emplace_back(childId, parentId);
            }
        }
        Helper::Catalog::extractNodeParentLinks(parentLinks, m_adjacencyMap, m_parentMap);
    }

    void ActionCatalog::setNodeParentLinks(const QVector<QPair<QString, QString>> &input) {
        Helper::Catalog::extractNodeParentLinks(input, m_adjacencyMap, m_parentMap);
    }

    void ActionRegistryPrivate::flushActionItems() {
        actionItems.clear();
        for (const auto &pair : std::as_const(extensions)) {
            auto &e = pair.second;
            for (int i = 0; i < e->itemCount(); ++i) {
                const auto &item = e->item(i);
                QString id = item.id();
                if (actionItems.contains(id)) {
                    continue;
                }
                actionItems.append(id, item);
            }
        }
        catalog = defaultCatalog();
        layouts = defaultLayouts();
    }

    ActionCatalog ActionRegistryPrivate::defaultCatalog() const {
        QVector<QPair<QString, QString>> nodeParentLinks;
        for (auto it = actionItems.begin(); it != actionItems.end(); ++it) {
            nodeParentLinks.emplace_back(it.key(), it.value().catalog());
        }
        return ActionCatalog(nodeParentLinks);
    }

    ActionLayouts ActionRegistryPrivate::defaultLayouts() const {
        // TODO: implement default layouts
        return {};
    }

    ActionLayouts ActionRegistryPrivate::correctLayouts(const ActionLayouts &layouts) const {
        // TODO: implement correct layouts
        return layouts;
    }

    ActionRegistry::ActionRegistry(QObject *parent)
        : ActionFamily(*new ActionRegistryPrivate(), parent) {
    }

    ActionRegistry::~ActionRegistry() = default;

    QList<const ActionExtension *> ActionRegistry::extensions() const {
        Q_D(const ActionRegistry);
        return d->extensions.values_qlist();
    }

    void ActionRegistry::setExtensions(const QList<const ActionExtension *> &extensions) {
        Q_D(ActionRegistry);

        d->extensions.clear();
        for (const auto &ext : extensions) {
            if (d->extensions.contains(ext->id())) {
                qCWarning(qActionKitLog).noquote().nospace()
                    << "Action extension with id \"" << ext->id() << "\" already exists";
                continue;
            }
            d->extensions.append(ext->id(), ext);
        }
        d->flushActionItems();
    }

    QStringList ActionRegistry::actionIds() const {
        Q_D(const ActionRegistry);
        return d->actionItems.keys_qlist();
    }

    ActionItemInfo ActionRegistry::actionInfo(const QString &id) const {
        Q_D(const ActionRegistry);
        return d->actionItems.value(id);
    }

    ActionCatalog ActionRegistry::catalog() const {
        Q_D(const ActionRegistry);
        return d->catalog;
    }

    ActionLayouts ActionRegistry::layouts() const {
        Q_D(const ActionRegistry);
        return d->layouts;
    }

    void ActionRegistry::setLayouts(const ActionLayouts &layouts) {
        Q_D(ActionRegistry);
        d->layouts = d->correctLayouts(layouts);
    }

    void ActionRegistry::resetLayouts() {
        Q_D(ActionRegistry);
        d->layouts = d->defaultLayouts();
    }

    QJsonObject ActionRegistry::layoutsToJson(const ActionLayouts &layouts) {
        // TODO
        return {};
    }

    ActionLayouts ActionRegistry::layoutsFromJson(const QJsonObject &obj) {
        // TODO
        return {};
    }

    ActionRegistry::ActionRegistry(ActionRegistryPrivate &d, QObject *parent)
        : ActionFamily(d, parent) {
    }

    void ActionRegistry::addContext(ActionContext *ctx) {
        Q_D(ActionRegistry);
        d->contexts.removeAll(nullptr);
        d->contexts.removeAll(ctx);
        d->contexts.append(ctx);

        auto &reg = ctx->d_func()->registry;
        if (reg) {
            reg->removeContext(ctx);
        }
        reg = this;
    }

    void ActionRegistry::removeContext(ActionContext *ctx) {
        Q_D(ActionRegistry);
        for (auto &item : d->contexts) {
            if (item == ctx) {
                item = nullptr;
            }
        }
        ctx->d_func()->registry = nullptr;
    }

    void ActionRegistry::updateContextLayouts() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateLayouts();
            }
        }
    }

    void ActionRegistry::updateContextTexts() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateTexts();
            }
        }
    }

    void ActionRegistry::updateContextKeymap() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateKeymap();
            }
        }
    }

    void ActionRegistry::updateContextIcons() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateIcons();
            }
        }
    }

}