#include "actionregistry.h"
#include "actionregistry_p.h"

#include <set>
#include <utility>

#include <QtCore/QStack>
#include <QtCore/QQueue>

#include "qakglobal_p.h"
#include "actioncontext_p.h"

namespace QAK {

    struct CatalogTrait {
        static constexpr bool Unique = true;
        using Child = QString;
        using ChildList = QStringList;
        using InputMap = QMap<QString, ChildList>;
        static QString getChildId(const Child &child) {
            return child;
        }
    };

    struct LayoutsTrait {
        static constexpr bool Unique = false;
        using Child = ActionLayoutEntry;
        using ChildList = QVector<ActionLayoutEntry>;
        using InputMap = QMap<QString, QVector<ActionLayoutEntry>>;
        static QString getChildId(const Child &child) {
            return child.id();
        }
    };

    template <class Trait>
    static bool buildGraph(const QString &id,                                //
                           const typename Trait::ChildList &children,        //
                           const typename Trait::InputMap &input,            //
                           QMap<QString, typename Trait::ChildList> &result, //
                           std::set<QString> &visiting                       //
    ) {
        // Empty id is reserved which means the forest, and only appears once
        if (!id.isEmpty()) {
            if (result.contains(id)) {
                // The node is already a valid node, skip it
                return true;
            }

            if (visiting.count(id)) {
                // A cycle is detected, skip it
                return false;
            }

            // Add the node to the visiting set
            visiting.insert(id);
        }

        // Build the real children list
        typename Trait::ChildList realChildren;
        for (const auto &child : children) {
            QString childId = Trait::getChildId(child);
            if (childId.isEmpty()) {
                // Child should not use the reserved forest id
                continue;
            }

            if (!buildGraph<Trait>(childId, input.value(childId), input, result, visiting)) {
                // Ignore invalid child node
                continue;
            }

            if constexpr (Trait::Unique) {
                if (!realChildren.contains(child)) {
                    realChildren.append(child);
                }
            } else {
                realChildren.append(child);
            }
        }
        result.insert(id, std::move(realChildren));
        return true;
    }

    void ActionCatalog::setAdjacencyTable(const QMap<QString, QStringList> &input) {
        QMap<QString, QStringList> adjacencyMap;
        for (auto it = input.begin(); it != input.end(); ++it) {
            std::set<QString> visiting;
            buildGraph<CatalogTrait>(it.key(), it.value(), input, adjacencyMap, visiting);
        }

        QMap<QString, QString> parentMap;
        for (auto it = adjacencyMap.begin(); it != adjacencyMap.end(); ++it) {
            const QString &parentId = it.key();
            const QStringList &children = it.value();
            for (const auto &childId : children) {
                parentMap[childId] = parentId;
            }
        }
        m_adjacencyMap = std::move(adjacencyMap);
        m_parentMap = std::move(parentMap);
    }

    void ActionCatalog::setNodeParentLinks(const QVector<QPair<QString, QString>> &input) {
        QMap<QString, QStringList> inputAdjacencyMap;
        for (const auto &pair : input) {
            const QString &childId = pair.first;
            const QString &parentId = pair.second;
            if (childId.isEmpty()) {
                continue;
            }
            inputAdjacencyMap[parentId].append(childId);
        }
        setAdjacencyTable(inputAdjacencyMap);
    }

    QAK_CORE_EXPORT QJsonObject ActionLayouts::toJsonObject() const {
        // TODO
        return {};
    }

    QAK_CORE_EXPORT ActionLayouts ActionLayouts::fromJsonObject(const QJsonObject &obj) {
        // TODO
        return {};
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

    static void applyInsertion(const ActionInsertion &insertion,
                               QMap<QString, QVector<ActionLayoutEntry>> &input) {
        const auto &target = insertion.target();
        auto it = input.find(target);
        if (it == input.end()) {
            return;
        }

        auto &targetItems = it.value();
        const auto &insertItems = insertion.items();
        switch (insertion.anchor()) {
            case ActionInsertion::Last: {
                targetItems.append(insertItems);
                break;
            }
            case ActionInsertion::First: {
                targetItems.insert(0, insertItems.size(), {});
                std::copy(insertItems.begin(), insertItems.end(), targetItems.begin());
                break;
            }
            case ActionInsertion::After:
            case ActionInsertion::Before: {
                const auto &relativeTo = insertion.relativeTo();
                auto relativeIt = std::find_if(targetItems.begin(), targetItems.end(),
                                               [&relativeTo](const ActionLayoutEntry &entry) {
                                                   return entry.id() == relativeTo;
                                               });
                if (relativeIt == targetItems.end()) {
                    break;
                }

                int index = relativeIt - targetItems.begin();
                if (insertion.anchor() == ActionInsertion::After) {
                    index++;
                }

                targetItems.insert(index, insertItems.size(), {});
                std::copy(insertItems.begin(), insertItems.end(), targetItems.begin() + index);
                break;
            }
        }
    }

    // Equivalent to:
    //     correctLayouts(ActionLayouts());
    ActionLayouts ActionRegistryPrivate::defaultLayouts() const {
        QMap<QString, QVector<ActionLayoutEntry>> oldAdjacencyMap;
        for (auto it = actionItems.begin(); it != actionItems.end(); ++it) {
            oldAdjacencyMap.insert(it.key(), it.value().children());
        }

        QStringList hashList;
        hashList.reserve(extensions.size());
        for (const auto &pair : extensions) {
            const auto &e = pair.second;
            // Apply insertions
            for (int i = 0; i < e->insertionCount(); ++i) {
                applyInsertion(e->insertion(i), oldAdjacencyMap);
            }
            hashList.append(e->hash());
        }

        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        for (auto it = oldAdjacencyMap.begin(); it != oldAdjacencyMap.end(); ++it) {
            std::set<QString> visiting;
            buildGraph<LayoutsTrait>(it.key(), it.value(), oldAdjacencyMap, adjacencyMap, visiting);
        }
        return ActionLayouts(adjacencyMap, hashList);
    }

    ActionLayouts ActionRegistryPrivate::correctLayouts(const ActionLayouts &layouts) const {
        QMap<QString, QVector<ActionLayoutEntry>> oldAdjacencyMap = layouts.adjacencyMap();
        const auto &oldHashList = layouts.hashList();

        std::set<QString> existingExtensionHashSet(oldHashList.begin(), oldHashList.end());
        for (const auto &pair : extensions) {
            const auto &e = pair.second;
            if (existingExtensionHashSet.count(e->hash())) {
                continue;
            }

            // Add items
            for (int i = 0; i < e->itemCount(); ++i) {
                const auto &item = e->item(i);
                QString id = item.id();
                if (oldAdjacencyMap.contains(id)) {
                    continue;
                }
                oldAdjacencyMap.insert(id, item.children());
            }

            // Apply insertions
            for (int i = 0; i < e->insertionCount(); ++i) {
                applyInsertion(e->insertion(i), oldAdjacencyMap);
            }
        }

        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        for (auto it = oldAdjacencyMap.begin(); it != oldAdjacencyMap.end(); ++it) {
            std::set<QString> visiting;
            buildGraph<LayoutsTrait>(it.key(), it.value(), oldAdjacencyMap, adjacencyMap, visiting);
        }

        QStringList hashList;
        hashList.reserve(extensions.size());
        for (const auto &pair : extensions) {
            const auto &e = pair.second;
            hashList.append(e->hash());
        }
        return ActionLayouts(adjacencyMap, hashList);
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

    void ActionRegistry::updateContext(ActionElement element) {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateElement(element);
            }
        }
    }

}