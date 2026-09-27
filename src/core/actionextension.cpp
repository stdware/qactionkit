#include "actionextension.h"
#include "actionextension_p.h"

#include <QtCore/QLoggingCategory>
#include <QtGui/qpa/qplatformtheme.h>

#include "qakglobal_p.h"

Q_LOGGING_CATEGORY(qActionKitLog, "qactionkit")

namespace QAK {

    static ActionItemInfoData sharedNullItemInfoData = {
        {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    };

    static int sharedNullLayoutEntryIndex = 0;

    static ActionInsertionData sharedNullInsertion = {
        {}, {}, {}, ActionInsertionData::defaultPriority, {},
    };

    static ActionExtensionData sharedNullExtensionData = {
        ACTION_EXTENSION_VERSION, {}, {}, {}, {}, {}, 0, &sharedNullItemInfoData, 0,
        &sharedNullInsertion,
    };

    // Translates the string in the first nonempty context among that of the item, that of the
    // extension and the built-in default.
    static inline ActionText translateString(const QString &s, const QString &itemContext,
                                             const QString &extensionContext,
                                             const char *defaultContext) {
        if (s.isEmpty()) {
            return {s, std::nullopt};
        }
        const QByteArray context = !itemContext.isEmpty()        ? itemContext.toUtf8()
                                   : !extensionContext.isEmpty() ? extensionContext.toUtf8()
                                                                 : QByteArray(defaultContext);
        bool ok;
        QString res = tryTranslate(context.constData(), s.toUtf8().constData(), nullptr, -1, &ok);
        if (!ok) {
            return {s, std::nullopt};
        }
        return {s, res};
    }

    QString ActionText::withoutMnemonic() const {
        return QPlatformTheme::removeMnemonics(toString());
    }

    ActionItemInfo::ActionItemInfo() : e(&sharedNullExtensionData), i(0) {
    }
    bool ActionItemInfo::isNull() const {
        return e == &sharedNullExtensionData;
    }
    QString ActionItemInfo::id() const {
        return e->items[i].id;
    }
    ActionItemInfo::Type ActionItemInfo::type() const {
        return e->items[i].type;
    }
    ActionText ActionItemInfo::text() const {
        auto &d = e->items[i];
        return translateString(d.text, d.textContext, e->textContext,
                               ActionExtensionData::defaultTextContext);
    }
    ActionText ActionItemInfo::category() const {
        auto &d = e->items[i];
        return translateString(d.category, d.categoryContext, e->categoryContext,
                               ActionExtensionData::defaultCategoryContext);
    }
    ActionText ActionItemInfo::description() const {
        auto &d = e->items[i];
        return translateString(d.description, d.descriptionContext, e->descriptionContext,
                               ActionExtensionData::defaultDescriptionContext);
    }
    QString ActionItemInfo::icon() const {
        return e->items[i].icon;
    }
    QList<QKeySequence> ActionItemInfo::shortcuts() const {
        return e->items[i].shortcuts;
    }
    QString ActionItemInfo::catalog() const {
        return e->items[i].catalog;
    }
    bool ActionItemInfo::topLevel() const {
        return e->items[i].topLevel;
    }
    bool ActionItemInfo::isExternal() const {
        return e->items[i].external;
    }
    bool ActionItemInfo::isCommand() const {
        const auto &d = e->items[i];
        return d.type == Action && !d.external;
    }
    QMap<ActionAttributeKey, QString> ActionItemInfo::attributes() const {
        return e->items[i].attributes;
    }
    QVector<ActionLayoutEntry> ActionItemInfo::children() const {
        return e->items[i].children;
    }
    ActionInsertion::ActionInsertion() : e(&sharedNullExtensionData), i(0) {
    }
    bool ActionInsertion::isNull() const {
        return e == &sharedNullExtensionData;
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
    int ActionInsertion::priority() const {
        return e->insertions[i].priority;
    }
    QVector<ActionLayoutEntry> ActionInsertion::items() const {
        return e->insertions[i].items;
    }
    // An extension is only obtained from generated code, which always sets the data, therefore a
    // null pointer and an index out of range are errors of the caller, checked as QList::at()
    // checks them.
    QString ActionExtension::version() const {
        Q_ASSERT(d.data);
        return d.data->version;
    }
    QString ActionExtension::id() const {
        Q_ASSERT(d.data);
        return d.data->id;
    }
    QString ActionExtension::hash() const {
        Q_ASSERT(d.data);
        return d.data->hash;
    }
    int ActionExtension::itemCount() const {
        Q_ASSERT(d.data);
        return d.data->itemCount;
    }
    ActionItemInfo ActionExtension::item(int index) const {
        Q_ASSERT(d.data);
        Q_ASSERT(index >= 0 && index < d.data->itemCount);
        ActionItemInfo result;
        result.e = d.data;
        result.i = index;
        return result;
    }
    int ActionExtension::insertionCount() const {
        Q_ASSERT(d.data);
        return d.data->insertionCount;
    }
    ActionInsertion ActionExtension::insertion(int index) const {
        Q_ASSERT(d.data);
        Q_ASSERT(index >= 0 && index < d.data->insertionCount);
        ActionInsertion result;
        result.e = d.data;
        result.i = index;
        return result;
    }

}
