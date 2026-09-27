#include "actionextension.h"
#include "actionextension_p.h"

#include <QtCore/QLoggingCategory>

#include "qakglobal_p.h"

Q_LOGGING_CATEGORY(qActionKitLog, "qactionkit")

namespace QAK {

    static ActionItemInfoData sharedNullItemInfoData = {
        {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    };

    static int sharedNullLayoutEntryIndex = 0;

    static ActionInsertionData sharedNullInsertion = {
        {},
        {},
        {},
        {},
    };

    static ActionExtensionData sharedNullExtensionData = {
        ACTION_EXTENSION_VERSION, {}, {}, 0, &sharedNullItemInfoData, 0, &sharedNullInsertion,
    };

    static inline QString translateString(const QString &s, const QMap<ActionAttributeKey, QString> &attrs,
                                          const QString &key, const QString &defaultCtx) {
        // Look for the translation context in attributes (without namespace)
        QString contextKey;
        for (auto it = attrs.begin(); it != attrs.end(); ++it) {
            if (it.key().name == key && it.key().namespaceUri.isEmpty()) {
                contextKey = it.value();
                break;
            }
        }
        if (contextKey.isEmpty()) {
            contextKey = defaultCtx;
        }
        
        bool ok;
        QString res = tryTranslate(contextKey.toUtf8().constData(),
                                   s.toUtf8().constData(), nullptr, -1, &ok);
        if (!ok) {
            return {};
        }
        return res;
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
    QString ActionItemInfo::text(bool translated) const {
        auto &d = e->items[i];
        if (!translated)
            return d.text;
        return translateString(d.text, d.attributes, QStringLiteral("textTr"),
                               QStringLiteral("QActionKit::ActionText"));
    }
    QString ActionItemInfo::category(bool translated) const {
        auto &d = e->items[i];
        if (!translated)
            return d.category;
        return translateString(d.category, d.attributes, QStringLiteral("categoryTr"),
                               QStringLiteral("QActionKit::ActionCategory"));
    }
    QString ActionItemInfo::description(bool translated) const {
        auto &d = e->items[i];
        if (!translated)
            return d.description;
        return translateString(d.description, d.attributes, QStringLiteral("descriptionTr"),
                               QStringLiteral("QActionKit::ActionDescription"));
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
