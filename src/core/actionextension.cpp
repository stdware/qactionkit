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
        QAK_ACTION_EXTENSION_VERSION, {}, {}, 0, &sharedNullItemInfoData, 0, &sharedNullInsertion,
    };

    static inline QString translateString(const QString &s, const QMap<QString, QString> &attrs,
                                          const QString &key, const QString &defaultCtx) {
        bool ok;
        QString res = tryTranslate(attrs.value(key, defaultCtx).toUtf8().constData(),
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
    QString ActionItemInfo::actionClass(bool translated) const {
        auto &d = e->items[i];
        if (!translated)
            return d.actionClass;
        return translateString(d.actionClass, d.attributes, QStringLiteral("classTr"),
                               QStringLiteral("QActionKit::ActionClass"));
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
    QMap<QString, QString> ActionItemInfo::attributes() const {
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