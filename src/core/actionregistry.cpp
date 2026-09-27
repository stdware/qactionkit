#include "actionregistry.h"
#include "actionregistry_p.h"

#include <set>
#include <utility>

#include <QtCore/QStack>
#include <QtCore/QQueue>
#include <QtCore/QJsonArray>

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
        static constexpr bool childIsSeparator(const Child &child) {
            return false;
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
        static bool childIsSeparator(const Child &child) {
            return child.type() == ActionLayoutEntry::Separator ||
                   child.type() == ActionLayoutEntry::Stretch;
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
            if (Trait::childIsSeparator(child)) {
                realChildren.append(child);
                continue;
            }

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

        if (!id.isEmpty()) {
            visiting.erase(id);
            // The erasure is not required, because the node is already in the result map, which
            // is checked before the visiting set.
        }
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

    void ActionCatalog::setParentMap(const QVector<QPair<QString, QString>> &input) {
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

    static QJsonObject actionLayoutEntryToJson(const ActionLayoutEntry &entry) {
        QJsonObject obj;
        obj["id"] = entry.id();
        QString typeStr;
        switch (entry.type()) {
            case ActionLayoutEntry::Action:
                typeStr = "Action";
                break;
            case ActionLayoutEntry::Group:
                typeStr = "Group";
                break;
            case ActionLayoutEntry::Menu:
                typeStr = "Menu";
                break;
            case ActionLayoutEntry::Separator:
                typeStr = "Separator";
                break;
            case ActionLayoutEntry::Stretch:
                typeStr = "Stretch";
                break;
            default:
                typeStr = "Action";
                break;
        }
        obj["type"] = typeStr;
        return obj;
    }

    static ActionLayoutEntry actionLayoutEntryFromJson(const QJsonObject &obj) {
        ActionLayoutEntry::Type type = ActionLayoutEntry::Action;
        QString typeStr = obj["type"].toString();
        if (typeStr == "Group") {
            type = ActionLayoutEntry::Group;
        } else if (typeStr == "Menu") {
            type = ActionLayoutEntry::Menu;
        } else if (typeStr == "Separator") {
            type = ActionLayoutEntry::Separator;
        } else if (typeStr == "Stretch") {
            type = ActionLayoutEntry::Stretch;
        }
        // Separators and stretches have no id, and every other type requires one. The caller
        // checks the id through ActionLayoutEntry::isNull().
        return ActionLayoutEntry(obj["id"].toString(), type);
    }

    QJsonObject ActionLayouts::toJsonObject() const {
        QJsonObject rootObj;

        QJsonObject adjacencyMapObj;
        for (auto it = m_adjacencyMap.constBegin(); it != m_adjacencyMap.constEnd(); ++it) {
            QJsonArray entriesArray;
            const QVector<ActionLayoutEntry> &entries = it.value();
            for (const ActionLayoutEntry &entry : entries) {
                entriesArray.append(actionLayoutEntryToJson(entry));
            }
            adjacencyMapObj.insert(it.key(), entriesArray);
        }
        rootObj.insert("adjacencyMap", adjacencyMapObj);

        QJsonArray hashArray;
        for (const QString &hash : m_hashList) {
            hashArray.append(hash);
        }
        rootObj.insert("hashList", hashArray);
        return rootObj;
    }

    ActionLayouts ActionLayouts::fromJsonObject(const QJsonObject &obj) {
        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        QStringList hashList;
        if (auto it = obj.find("adjacencyMap"); it != obj.end() && it->isObject()) {
            const QJsonObject &adjacencyMapObj = it->toObject();
            for (auto it1 = adjacencyMapObj.constBegin(); it1 != adjacencyMapObj.constEnd();
                 ++it1) {
                if (it1->isArray()) {
                    const QJsonArray &entriesArray = it1->toArray();
                    QVector<ActionLayoutEntry> entries;
                    for (const QJsonValue &entryValue : entriesArray) {
                        if (entryValue.isObject()) {
                            auto entry = actionLayoutEntryFromJson(entryValue.toObject());
                            if (!entry.isNull()) {
                                entries.append(entry);
                            }
                        }
                    }
                    adjacencyMap.insert(it1.key(), entries);
                }
            }
        }
        if (auto it = obj.find("hashList"); it != obj.end() && obj["hashList"].isArray()) {
            QJsonArray hashArray = obj["hashList"].toArray();
            for (const QJsonValue &hashValue : hashArray) {
                if (hashValue.isString()) {
                    hashList.append(hashValue.toString());
                }
            }
        }
        return ActionLayouts(adjacencyMap, hashList);
    }

    void ActionRegistryPrivate::flushActionItems() const {
        if (!extensionsDirty)
            return;
        extensionsDirty = false;

        actionItems.clear();
        // The extension whose declaration of each item is used, for the warning on a conflict
        QHash<QString, QString> declaringExtensions;
        for (const auto &pair : std::as_const(extensions)) {
            auto &e = pair.second;
            for (int i = 0; i < e->itemCount(); ++i) {
                const auto &item = e->item(i);
                QString id = item.id();
                auto existing = actionItems.find(id);
                if (existing == actionItems.end()) {
                    actionItems.append(id, item);
                    declaringExtensions.insert(id, e->id());
                    continue;
                }

                // An extension declares a node of another extension again as a phony, which gives
                // way to the declaration of the extension that owns the node, whatever the order
                // of registration
                if (item.type() == ActionItemInfo::Phony) {
                    continue;
                }
                if (existing->second.type() == ActionItemInfo::Phony) {
                    existing->second = item;
                    declaringExtensions.insert(id, e->id());
                    continue;
                }
                qCWarning(qActionKitLog).noquote().nospace()
                    << "Action item \"" << id << "\" is declared by both \""
                    << declaringExtensions.value(id) << "\" and \"" << e->id()
                    << "\", and the first declaration is kept";
            }
        }
        catalog = defaultCatalog();
        layouts = defaultLayouts();
    }

    ActionCatalog ActionRegistryPrivate::defaultCatalog() const {
        QVector<QPair<QString, QString>> nodeParentLinks;
        for (auto it = actionItems.begin(); it != actionItems.end(); ++it) {
            nodeParentLinks.emplace_back(it->first, it->second.catalog());
        }
        return ActionCatalog(nodeParentLinks);
    }

    enum class InsertionResult {
        Applied,
        MissingTarget,
        MissingRelativeTo,
    };

    // Applies the insertion, or skips it if its target or its relative sibling does not exist
    static InsertionResult applyInsertion(const ActionInsertion &insertion,
                                          QMap<QString, QVector<ActionLayoutEntry>> &input) {
        const auto &target = insertion.target();
        auto it = input.find(target);
        if (it == input.end()) {
            return InsertionResult::MissingTarget;
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
                    return InsertionResult::MissingRelativeTo;
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
        return InsertionResult::Applied;
    }

    // Equivalent to:
    //     correctLayouts(ActionLayouts());
    ActionLayouts ActionRegistryPrivate::defaultLayouts() const {
        QMap<QString, QVector<ActionLayoutEntry>> oldAdjacencyMap;
        for (auto it = actionItems.begin(); it != actionItems.end(); ++it) {
            oldAdjacencyMap.insert(it->first, it->second.children());
        }

        QStringList hashList;
        hashList.reserve(extensions.size());
        for (const auto &pair : extensions) {
            const auto &e = pair.second;
            // Apply insertions. A skipped insertion is reported here only: in a layout customized
            // by the user, a missing target or sibling may have been removed on purpose.
            for (int i = 0; i < e->insertionCount(); ++i) {
                const auto insertion = e->insertion(i);
                switch (applyInsertion(insertion, oldAdjacencyMap)) {
                    case InsertionResult::Applied:
                        break;
                    case InsertionResult::MissingTarget:
                        qCWarning(qActionKitLog).noquote().nospace()
                            << "Action extension \"" << e->id() << "\" inserts into \""
                            << insertion.target() << "\", which does not exist";
                        break;
                    case InsertionResult::MissingRelativeTo:
                        qCWarning(qActionKitLog).noquote().nospace()
                            << "Action extension \"" << e->id() << "\" inserts relative to \""
                            << insertion.relativeTo() << "\", which \"" << insertion.target()
                            << "\" does not contain";
                        break;
                }
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

    ActionRegistry::~ActionRegistry() {
        Q_D(ActionRegistry);
        // The remaining contexts are detached, because each would otherwise keep a dangling
        // pointer to the registry. A context is not necessarily destroyed together with its
        // registry, and the destruction order of sibling QObject children is unspecified.
        for (const auto &ctx : std::as_const(d->contexts)) {
            if (ctx) {
                ctx->d_func()->registry = nullptr;
            }
        }
    }

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
        d->extensionsDirty = true;
    }

    void ActionRegistry::addExtension(const ActionExtension *extension) {
        Q_D(ActionRegistry);
        if (d->extensions.contains(extension->id())) {
            qCWarning(qActionKitLog).noquote().nospace()
                << "Action extension with id \"" << extension->id() << "\" already exists";
            return;
        }
        d->extensions.append(extension->id(), extension);
        d->extensionsDirty = true;
    }

    QStringList ActionRegistry::actionIds() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->actionItems.keys_qlist();
    }

    std::optional<ActionItemInfo> ActionRegistry::actionInfo(const QString &id) const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        if (auto it = d->actionItems.find(id); it != d->actionItems.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    ActionCatalog ActionRegistry::catalog() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->catalog;
    }

    ActionLayouts ActionRegistry::layouts() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->layouts;
    }

    void ActionRegistry::setLayouts(const ActionLayouts &layouts) {
        Q_D(ActionRegistry);
        d->flushActionItems();
        d->layouts = d->correctLayouts(layouts);
    }

    void ActionRegistry::resetLayouts() {
        Q_D(ActionRegistry);
        d->flushActionItems();
        d->layouts = d->defaultLayouts();
    }

    ActionRegistry::ActionRegistry(ActionRegistryPrivate &d, QObject *parent)
        : ActionFamily(d, parent) {
    }

    void ActionRegistry::addContext(ActionContext *ctx) {
        Q_D(ActionRegistry);
        if (!ctx) {
            return;
        }

        // Detached from its previous registry before it is appended. In the reverse order, adding
        // a context again to the registry it belongs to would null the entry just appended.
        if (auto reg = ctx->d_func()->registry) {
            reg->removeContext(ctx);
        }

        d->contexts.removeAll(nullptr);
        d->contexts.append(ctx);
        ctx->d_func()->registry = this;
    }

    void ActionRegistry::removeContext(ActionContext *ctx) {
        Q_D(ActionRegistry);
        if (!ctx) {
            return;
        }
        // The entries are nulled rather than erased, and addContext() removes the null entries.
        for (auto &item : d->contexts) {
            if (item == ctx) {
                item = nullptr;
            }
        }
        ctx->d_func()->registry = nullptr;
    }

    void ActionRegistry::updateContext(ActionElement element) {
        Q_D(ActionRegistry);
        // Iterated over a copy, because a context may register or unregister contexts, or be
        // destroyed, while it is updated.
        const auto contexts = d->contexts;
        for (const auto &ctx : contexts) {
            if (ctx) {
                ctx->updateElement(element);
            }
        }
    }

}
