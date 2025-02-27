#include "actionextension.h"

#include "actionextension_p.h"

namespace QAK {

    static ActionItemInfoData sharedNullItemInfoData = {
        {}, {}, {}, {}, {}, {}, {}, {}, {},
    };

    static ActionLayoutInfoEntry sharedLayoutInfoEntry = {
        {},
        {},
        {0},
    };

    static int sharedNullLayoutEntryIndex = 0;

    static ActionInsertionData sharedNullInsertion = {
        {},
        {},
        {},
        {0},
    };

    static ActionExtensionData sharedNullExtensionData = {
        QAK_ACTION_EXTENSION_VERSION, QStringLiteral(""),
        QStringLiteral(""),           0,
        &sharedNullItemInfoData,      0,
        &sharedLayoutInfoEntry,       0,
        &sharedNullLayoutEntryIndex,  0,
        &sharedNullInsertion,
    };

    ActionItemInfo::ActionItemInfo() : e(&sharedNullExtensionData), i(0) {
    }
    bool ActionItemInfo::isNull() const {
        return e != &sharedNullExtensionData;
    }
    QString ActionItemInfo::id() const {
        return e->items[i].id;
    }
    ActionItemInfo::Type ActionItemInfo::type() const {
        return e->items[i].type;
    }
    QString ActionItemInfo::text() const {
        return e->items[i].text;
    }
    QString ActionItemInfo::actionClass() const {
        return e->items[i].actionClass;
    }
    QString ActionItemInfo::description() const {
        return e->items[i].description;
    }
    QStringList ActionItemInfo::shortcuts() const {
        return e->items[i].shortcuts;
    }
    QStringList ActionItemInfo::catalog() const {
        return e->items[i].catalog;
    }
    bool ActionItemInfo::topLevel() const {
        return e->items[i].topLevel;
    }
    QMap<QString, QString> ActionItemInfo::attributes() const {
        return e->items[i].attributes;
    }
    ActionLayoutInfo::ActionLayoutInfo() : e(&sharedNullExtensionData), i(0) {
    }
    bool ActionLayoutInfo::isNull() const {
        return e != &sharedNullExtensionData;
    }
    QString ActionLayoutInfo::id() const {
        return e->layoutEntries[i].id;
    }
    ActionLayoutInfo::Type ActionLayoutInfo::type() const {
        return e->layoutEntries[i].type;
    }
    int ActionLayoutInfo::childCount() const {
        return static_cast<int>(e->layoutEntries[i].childIndexes.size());
    }
    ActionLayoutInfo ActionLayoutInfo::child(int index) const {
        ActionLayoutInfo result;
        result.e = e;
        result.i = e->layoutEntries[i].childIndexes[index];
        return result;
    }
    ActionInsertion::ActionInsertion() : e(&sharedNullExtensionData), i(0) {
    }
    bool ActionInsertion::isNull() const {
        return e != &sharedNullExtensionData;
    }
    ActionInsertion::Anchor ActionInsertion::anchor() const {
        return e->insertions[i].anchor;
    }
    QString ActionInsertion::target() const {
        return e->insertions[i].target;
    }
    QString ActionInsertion::relativeTo() const {
        return e->insertions[i].relativeTo;
    }
    int ActionInsertion::itemCount() const {
        return static_cast<int>(e->insertions[i].entryIndexes.size());
    }
    ActionLayoutInfo ActionInsertion::item(int index) const {
        ActionLayoutInfo result;
        result.e = e;
        result.i = e->insertions[i].entryIndexes[index];
        return result;
    }
    QString ActionExtension::version() const {
        return d.data->version;
    }
    QString ActionExtension::id() const {
        return d.data->id;
    }
    QString ActionExtension::hash() const {
        return d.data->hash;
    }
    int ActionExtension::itemCount() const {
        return d.data->itemCount;
    }
    ActionItemInfo ActionExtension::item(int index) const {
        ActionItemInfo result;
        result.e = d.data;
        result.i = index;
        return result;
    }
    int ActionExtension::layoutCount() const {
        return d.data->layoutCount;
    }
    ActionLayoutInfo ActionExtension::layout(int index) const {
        ActionLayoutInfo result;
        result.e = d.data;
        result.i = index;
        return result;
    }
    int ActionExtension::insertionCount() const {
        return d.data->insertionCount;
    }
    ActionInsertion ActionExtension::insertion(int index) const {
        ActionInsertion result;
        result.e = d.data;
        result.i = index;
        return result;
    }

}