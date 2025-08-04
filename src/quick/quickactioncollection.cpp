#include "quickactioncollection_p.h"
#include "quickactioncollection_p_p.h"
#include "quickactionitem_p.h"
#include <QAKQuick/quickactioncontext.h>

namespace QAK {

    QuickActionCollection::QuickActionCollection(QObject *parent) : QObject(parent), d_ptr(new QuickActionCollectionPrivate) {
        Q_D(QuickActionCollection);
        d->q_ptr = this;
    }
    QuickActionCollection::~QuickActionCollection() = default;

    QQmlListProperty<QuickActionItem> QuickActionCollection::data() {
        Q_D(QuickActionCollection);
        return {this, d,
                QuickActionCollectionPrivate::appendItem,
                QuickActionCollectionPrivate::itemCount,
                QuickActionCollectionPrivate::itemAt,
                QuickActionCollectionPrivate::clearItems};
    }

    void QuickActionCollection::registerToContext(QuickActionContext *context) {
        Q_D(QuickActionCollection);
        if (!context)
            return;

        for (QuickActionItem *item : d->items) {
            if (item && item->actionComponent()) {
                context->addAction(item->actionId(), item->actionComponent());
            }
        }
    }

    void QuickActionCollectionPrivate::appendItem(QQmlListProperty<QuickActionItem> *property, QuickActionItem *item) {
        if (!item)
            return;
        auto *d = static_cast<QuickActionCollectionPrivate *>(property->data);
        d->items.append(item);
    }

    qsizetype QuickActionCollectionPrivate::itemCount(QQmlListProperty<QuickActionItem> *property) {
        auto *d = static_cast<QuickActionCollectionPrivate *>(property->data);
        return d->items.count();
    }

    QuickActionItem *QuickActionCollectionPrivate::itemAt(QQmlListProperty<QuickActionItem> *property, qsizetype index) {
        auto *d = static_cast<QuickActionCollectionPrivate *>(property->data);
        if (index < 0 || index >= d->items.count())
            return nullptr;
        return d->items.at(index);
    }

    void QuickActionCollectionPrivate::clearItems(QQmlListProperty<QuickActionItem> *property) {
        auto *d = static_cast<QuickActionCollectionPrivate *>(property->data);
        d->items.clear();
    }

}

#include "moc_quickactioncollection_p.cpp"
