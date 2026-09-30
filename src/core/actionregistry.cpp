#include "actionregistry.h"
#include "actionregistry_p.h"

#include <algorithm>
#include <set>
#include <utility>

#include <QtCore/QStack>
#include <QtCore/QQueue>
#include <QtCore/QSet>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

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
        static constexpr bool childIsSeparator(const Child &) {
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
        return rootObj;
    }

    ActionLayouts ActionLayouts::fromJsonObject(const QJsonObject &obj) {
        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
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
        return ActionLayouts(adjacencyMap);
    }

    static std::optional<ActionLayoutEntry::Type> entryTypeFromString(const QString &s) {
        static const QHash<QString, ActionLayoutEntry::Type> types = {
            {QStringLiteral("Action"),    ActionLayoutEntry::Action   },
            {QStringLiteral("Group"),     ActionLayoutEntry::Group    },
            {QStringLiteral("Menu"),      ActionLayoutEntry::Menu     },
            {QStringLiteral("Separator"), ActionLayoutEntry::Separator},
            {QStringLiteral("Stretch"),   ActionLayoutEntry::Stretch  },
        };
        if (auto it = types.find(s); it != types.end()) {
            return it.value();
        }
        return std::nullopt;
    }

    // The anchors are written as in a manifest
    static const QHash<QString, ActionInsertion::Anchor> &anchorNames() {
        static const QHash<QString, ActionInsertion::Anchor> names = {
            {QStringLiteral("first"),  ActionInsertion::First },
            {QStringLiteral("last"),   ActionInsertion::Last  },
            {QStringLiteral("after"),  ActionInsertion::After },
            {QStringLiteral("before"), ActionInsertion::Before},
        };
        return names;
    }

    QJsonObject ActionLayoutChange::toJsonObject() const {
        QJsonObject obj;
        obj.insert(QStringLiteral("kind"),
                   kind == Add ? QStringLiteral("add") : QStringLiteral("remove"));
        obj.insert(QStringLiteral("container"), container);
        obj.insert(QStringLiteral("entry"), actionLayoutEntryToJson(entry));
        obj.insert(QStringLiteral("anchor"), anchorNames().key(anchor));
        if (!relativeTo.isEmpty()) {
            obj.insert(QStringLiteral("relativeTo"), relativeTo);
        }
        if (offset != 0) {
            obj.insert(QStringLiteral("offset"), offset);
        }
        if (moved) {
            obj.insert(QStringLiteral("moved"), true);
        }
        return obj;
    }

    std::optional<ActionLayoutChange> ActionLayoutChange::fromJsonObject(const QJsonObject &obj) {
        ActionLayoutChange change;

        const auto kind = obj.value(QStringLiteral("kind")).toString();
        if (kind == QStringLiteral("add")) {
            change.kind = Add;
        } else if (kind == QStringLiteral("remove")) {
            change.kind = Remove;
        } else {
            return std::nullopt;
        }

        change.container = obj.value(QStringLiteral("container")).toString();
        if (change.container.isEmpty()) {
            return std::nullopt;
        }

        // A separator or stretch has no id, and any other entry has one
        const auto entryObj = obj.value(QStringLiteral("entry")).toObject();
        const auto type = entryTypeFromString(entryObj.value(QStringLiteral("type")).toString());
        if (!type) {
            return std::nullopt;
        }
        const auto id = entryObj.value(QStringLiteral("id")).toString();
        const bool hasId =
            *type != ActionLayoutEntry::Separator && *type != ActionLayoutEntry::Stretch;
        if (id.isEmpty() == hasId) {
            return std::nullopt;
        }
        change.entry = ActionLayoutEntry(id, *type);

        const auto &names = anchorNames();
        const auto anchor = names.find(obj.value(QStringLiteral("anchor")).toString());
        if (anchor == names.end()) {
            return std::nullopt;
        }
        change.anchor = anchor.value();
        change.relativeTo = obj.value(QStringLiteral("relativeTo")).toString();
        if ((change.anchor == ActionInsertion::After || change.anchor == ActionInsertion::Before) &&
            change.relativeTo.isEmpty()) {
            return std::nullopt;
        }

        const auto offset = obj.value(QStringLiteral("offset")).toDouble(0);
        if (offset < 0 || offset != int(offset)) {
            return std::nullopt;
        }
        change.offset = int(offset);
        change.moved = obj.value(QStringLiteral("moved")).toBool(false);
        return change;
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
        catalog = computeDefaultCatalog();
        defaultLayouts = computeDefaultLayouts();
        replayLayoutChanges();
    }

    ActionCatalog ActionRegistryPrivate::computeDefaultCatalog() const {
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

    struct PendingInsertion {
        const ActionExtension *extension;
        ActionInsertion insertion;
    };

    // Returns the insertions of the extensions in the order in which they are applied. Among the
    // insertions at the same position, those with smaller priorities end up first, and those with
    // the same priority follow the order of registration and of the manifest. An insertion at the
    // first position or after a sibling goes before those applied earlier there, so the insertions
    // at such a position are applied in reverse. Each position is applied where its first insertion
    // occurs, so that an insertion still follows the one that adds its sibling.
    static QVector<PendingInsertion>
        orderInsertions(const QVector<const ActionExtension *> &extensions) {
        QVector<QVector<PendingInsertion>> positions;
        QHash<QString, int> positionIndexes;
        for (const auto &e : extensions) {
            for (int i = 0; i < e->insertionCount(); ++i) {
                const auto insertion = e->insertion(i);
                const auto key = insertion.target() + QChar(u'\0') +
                                 QString::number(insertion.anchor()) + QChar(u'\0') +
                                 insertion.relativeTo();
                auto it = positionIndexes.find(key);
                if (it == positionIndexes.end()) {
                    it = positionIndexes.insert(key, int(positions.size()));
                    positions.emplace_back();
                }
                positions[it.value()].append({e, insertion});
            }
        }

        QVector<PendingInsertion> result;
        for (auto &position : positions) {
            std::stable_sort(position.begin(), position.end(),
                             [](const PendingInsertion &a, const PendingInsertion &b) {
                                 return a.insertion.priority() < b.insertion.priority();
                             });
            const auto anchor = position.first().insertion.anchor();
            if (anchor == ActionInsertion::First || anchor == ActionInsertion::After) {
                std::reverse(position.begin(), position.end());
            }
            result += position;
        }
        return result;
    }

    // Computes the layouts of the registered extensions, which the changes of the user are then
    // replayed on
    ActionLayouts ActionRegistryPrivate::computeDefaultLayouts() const {
        QMap<QString, QVector<ActionLayoutEntry>> oldAdjacencyMap;
        for (auto it = actionItems.begin(); it != actionItems.end(); ++it) {
            oldAdjacencyMap.insert(it->first, it->second.children());
        }

        QVector<const ActionExtension *> extensionList;
        for (const auto &pair : extensions) {
            extensionList.append(pair.second);
        }

        // Apply insertions
        for (const auto &[e, insertion] : orderInsertions(extensionList)) {
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

        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        for (auto it = oldAdjacencyMap.begin(); it != oldAdjacencyMap.end(); ++it) {
            std::set<QString> visiting;
            buildGraph<LayoutsTrait>(it.key(), it.value(), oldAdjacencyMap, adjacencyMap, visiting);
        }
        return ActionLayouts(adjacencyMap);
    }

    static bool isAnonymous(const ActionLayoutEntry &entry) {
        return entry.type() == ActionLayoutEntry::Separator ||
               entry.type() == ActionLayoutEntry::Stretch;
    }

    // A position within the children of a container, and the direction in which its offset passes
    // the separators and stretches
    struct LayoutPosition {
        int index;
        bool forward;

        // Returns the index of the entry at the position, which a removal removes: the one after
        // it when counting forward from the anchor, and the one before it otherwise
        int entryIndex() const {
            return forward ? index : index - 1;
        }
    };

    // Returns the position that the change describes within children, or std::nullopt if the id
    // that it refers to is missing. With fewer separators and stretches than the offset, the
    // position stops at the last of them.
    static std::optional<LayoutPosition> positionOf(const ActionLayoutChange &change,
                                                    const QVector<ActionLayoutEntry> &children) {
        LayoutPosition position{0, true};
        switch (change.anchor) {
            case ActionInsertion::First:
                break;
            case ActionInsertion::Last:
                position = {int(children.size()), false};
                break;
            case ActionInsertion::After:
            case ActionInsertion::Before: {
                const auto it = std::find_if(
                    children.begin(), children.end(), [&change](const ActionLayoutEntry &entry) {
                        return !isAnonymous(entry) && entry.id() == change.relativeTo;
                    });
                if (it == children.end()) {
                    return std::nullopt;
                }
                const int index = int(it - children.begin());
                position = change.anchor == ActionInsertion::After ? LayoutPosition{index + 1, true}
                                                                   : LayoutPosition{index, false};
                break;
            }
        }
        for (int passed = 0; passed < change.offset; ++passed) {
            const int next = position.entryIndex();
            if (next < 0 || next >= children.size() || !isAnonymous(children[next])) {
                break;
            }
            position.index += position.forward ? 1 : -1;
        }
        return position;
    }

    // Replays one change on the changed adjacency map. A change that cannot apply is skipped with
    // a warning, and the others still apply, so that the menus are always valid and the worst
    // outcome is that one customization of the user is lost.
    void ActionRegistryPrivate::replayLayoutChange(const ActionLayoutChange &change) const {
        const auto skip = [&change](const char *reason) {
            qCWarning(qActionKitLog).noquote().nospace()
                << "Layout change "
                << QJsonDocument(change.toJsonObject()).toJson(QJsonDocument::Compact)
                << " is skipped, because " << reason;
        };

        const auto container = actionItems.find(change.container);
        if (container == actionItems.end()) {
            skip("the container is not declared");
            return;
        }
        const auto containerType = container->second.type();
        if (containerType != ActionItemInfo::Menu && containerType != ActionItemInfo::Group) {
            skip("the container is not a menu or a group");
            return;
        }

        // The declared type of the entry must allow its form, as in a manifest
        const auto &entry = change.entry;
        if (!isAnonymous(entry)) {
            const auto item = actionItems.find(entry.id());
            if (item == actionItems.end()) {
                skip("the entry is not declared");
                return;
            }
            bool allowed = false;
            switch (item->second.type()) {
                case ActionItemInfo::Action:
                    allowed = entry.type() == ActionLayoutEntry::Action;
                    break;
                case ActionItemInfo::Menu:
                case ActionItemInfo::Group:
                    allowed = entry.type() == ActionLayoutEntry::Menu ||
                              entry.type() == ActionLayoutEntry::Group;
                    break;
                case ActionItemInfo::Phony:
                    break;
            }
            if (!allowed) {
                skip("the declared type of the entry does not allow its form");
                return;
            }
        }

        auto &children = changedAdjacencyMap[change.container];
        if (change.kind == ActionLayoutChange::Remove) {
            int index = -1;
            if (isAnonymous(entry)) {
                if (const auto position = positionOf(change, children)) {
                    const int i = position->entryIndex();
                    if (i >= 0 && i < children.size() && children[i].type() == entry.type()) {
                        index = i;
                    }
                }
            } else {
                index = int(children.indexOf(entry));
            }
            if (index < 0) {
                skip("the entry is not in the container");
                return;
            }
            children.remove(index);
            if (change.moved && !isAnonymous(entry)) {
                movedIds[entry.id()]++;
            }
            return;
        }

        if (change.moved && !isAnonymous(entry)) {
            auto it = movedIds.find(entry.id());
            if (it == movedIds.end() || it.value() == 0) {
                skip("no removal of the move has applied before it");
                return;
            }
            --it.value();
        }
        // A missing anchor puts the entry at the end, as for an insertion
        const auto position = positionOf(change, children);
        children.insert(position ? position->index : children.size(), entry);
    }

    // Sets the position of the change to index within children: after the nearest entry with an id
    // before it, past the separators and stretches in between, or from the beginning.
    static void setPosition(ActionLayoutChange &change, const QVector<ActionLayoutEntry> &children,
                            int index) {
        int reference = index - 1;
        while (reference >= 0 && isAnonymous(children[reference])) {
            --reference;
        }
        change.offset = index - 1 - reference;
        if (reference < 0) {
            change.anchor = ActionInsertion::First;
        } else {
            change.anchor = ActionInsertion::After;
            change.relativeTo = children[reference].id();
        }
    }

    // Returns for each entry of a and of b whether it belongs to a heaviest common subsequence of
    // the two, which stays in place while the other entries are removed and added. An entry with
    // an id weighs more than a separator or stretch, so that the entries stay and the separators
    // move when either could.
    static std::pair<QVector<bool>, QVector<bool>>
        commonEntries(const QVector<ActionLayoutEntry> &a, const QVector<ActionLayoutEntry> &b) {
        const int n = int(a.size());
        const int m = int(b.size());
        // weights[i][j] is the weight of a heaviest common subsequence of a[i:] and b[j:]. Since
        // the weight of a match depends on the entry only, a match of a[i] and b[j] belongs to one.
        QVector<QVector<int>> weights(n + 1, QVector<int>(m + 1, 0));
        for (int i = n - 1; i >= 0; --i) {
            for (int j = m - 1; j >= 0; --j) {
                weights[i][j] = a[i] == b[j] ? weights[i + 1][j + 1] + (isAnonymous(a[i]) ? 1 : 2)
                                             : std::max(weights[i + 1][j], weights[i][j + 1]);
            }
        }
        QVector<bool> keptA(n, false);
        QVector<bool> keptB(m, false);
        for (int i = 0, j = 0; i < n && j < m;) {
            if (a[i] == b[j]) {
                keptA[i++] = true;
                keptB[j++] = true;
            } else if (weights[i + 1][j] >= weights[i][j + 1]) {
                ++i;
            } else {
                ++j;
            }
        }
        return {keptA, keptB};
    }

    // Returns the changes that turn the adjacency map from into to when replayed on it. Within each
    // container, the entries outside a longest common subsequence are removed and added, and a
    // removal and an addition of the same id become a move. The changes are computed on a copy of
    // from, all removals first, so that each position refers to the state it is replayed on. A
    // container missing from to is unchanged.
    static QVector<ActionLayoutChange>
        diffLayouts(const QMap<QString, QVector<ActionLayoutEntry>> &from,
                    const QMap<QString, QVector<ActionLayoutEntry>> &to) {
        QMap<QString, QVector<bool>> keptFrom;
        QMap<QString, QVector<bool>> keptTo;
        for (auto it = to.begin(); it != to.end(); ++it) {
            std::tie(keptFrom[it.key()], keptTo[it.key()]) =
                commonEntries(from.value(it.key()), it.value());
        }

        // A removed entry with an id is paired with the first later addition of the same id
        using Place = std::pair<QString, int>;
        QHash<QString, QVector<Place>> removedPlaces;
        for (auto it = to.begin(); it != to.end(); ++it) {
            const auto &children = from.value(it.key());
            const auto &kept = keptFrom[it.key()];
            for (int i = 0; i < children.size(); ++i) {
                if (!kept[i] && !isAnonymous(children[i])) {
                    removedPlaces[children[i].id()].append({it.key(), i});
                }
            }
        }
        QSet<Place> movedFrom;
        QSet<Place> movedTo;
        for (auto it = to.begin(); it != to.end(); ++it) {
            const auto &kept = keptTo[it.key()];
            for (int i = 0; i < it.value().size(); ++i) {
                const auto &entry = it.value()[i];
                if (kept[i] || isAnonymous(entry)) {
                    continue;
                }
                auto &places = removedPlaces[entry.id()];
                if (!places.isEmpty()) {
                    movedFrom.insert(places.takeFirst());
                    movedTo.insert({it.key(), i});
                }
            }
        }

        QVector<ActionLayoutChange> changes;
        auto work = from;
        for (auto it = to.begin(); it != to.end(); ++it) {
            auto &children = work[it.key()];
            const auto original = from.value(it.key());
            const auto &kept = keptFrom[it.key()];
            // index is the place of original[i] in children, from which the removed ones are gone
            for (int i = 0, index = 0; i < original.size(); ++i) {
                if (kept[i]) {
                    ++index;
                    continue;
                }
                ActionLayoutChange change;
                change.kind = ActionLayoutChange::Remove;
                change.container = it.key();
                change.entry = original[i];
                change.moved = movedFrom.contains({it.key(), i});
                if (isAnonymous(change.entry)) {
                    setPosition(change, children, index);
                }
                children.remove(index);
                changes.append(change);
            }
        }
        for (auto it = to.begin(); it != to.end(); ++it) {
            auto &children = work[it.key()];
            const auto &kept = keptTo[it.key()];
            for (int i = 0; i < it.value().size(); ++i) {
                if (kept[i]) {
                    continue;
                }
                ActionLayoutChange change;
                change.kind = ActionLayoutChange::Add;
                change.container = it.key();
                change.entry = it.value()[i];
                change.moved = movedTo.contains({it.key(), i});
                setPosition(change, children, i);
                children.insert(i, change.entry);
                changes.append(change);
            }
        }
        return changes;
    }

    void ActionRegistryPrivate::replayLayoutChanges() const {
        changedAdjacencyMap = defaultLayouts.adjacencyMap();
        movedIds.clear();
        for (const auto &change : layoutChanges) {
            replayLayoutChange(change);
        }
        buildLayouts();
    }

    // Builds the layouts in effect from the changed adjacency map, dropping cycles that the
    // changes have made
    void ActionRegistryPrivate::buildLayouts() const {
        QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap;
        for (auto it = changedAdjacencyMap.begin(); it != changedAdjacencyMap.end(); ++it) {
            std::set<QString> visiting;
            buildGraph<LayoutsTrait>(it.key(), it.value(), changedAdjacencyMap, adjacencyMap,
                                     visiting);
        }
        layouts = ActionLayouts(adjacencyMap);
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

    // Flushed at once, since the library of the extension may be unloaded before the next query
    void ActionRegistry::removeExtension(const ActionExtension *extension) {
        Q_D(ActionRegistry);
        const auto it = d->extensions.find(extension->id());
        if (it == d->extensions.end() || it->second != extension) {
            return;
        }
        d->extensions.erase(it);
        d->extensionsDirty = true;
        d->flushActionItems();
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

    ActionLayouts ActionRegistry::defaultLayouts() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->defaultLayouts;
    }

    ActionLayouts ActionRegistry::layouts() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->layouts;
    }

    QVector<ActionLayoutChange> ActionRegistry::layoutChanges() const {
        Q_D(const ActionRegistry);
        return d->layoutChanges;
    }

    QVector<ActionLayoutChange>
        ActionRegistry::computeLayoutChanges(const ActionLayouts &edited) const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return diffLayouts(d->defaultLayouts.adjacencyMap(), edited.adjacencyMap());
    }

    // Pending extensions are flushed after the changes are stored, since flushing replays them
    void ActionRegistry::setLayoutChanges(const QVector<ActionLayoutChange> &changes) {
        Q_D(ActionRegistry);
        d->layoutChanges = changes;
        if (d->extensionsDirty) {
            d->flushActionItems();
        } else {
            d->replayLayoutChanges();
        }
    }

    void ActionRegistry::addLayoutChange(const ActionLayoutChange &change) {
        Q_D(ActionRegistry);
        d->layoutChanges.append(change);
        if (d->extensionsDirty) {
            d->flushActionItems();
        } else {
            d->replayLayoutChange(change);
            d->buildLayouts();
        }
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
