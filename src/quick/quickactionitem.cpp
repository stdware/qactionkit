#include "quickactionitem_p.h"
#include "quickactionitem_p_p.h"

#include <QQmlComponent>

namespace QAK {

    QuickActionItem::QuickActionItem(QObject *parent) : QObject(parent), d_ptr(new QuickActionItemPrivate) {
        Q_D(QuickActionItem);
        d->q_ptr = this;
    }
    QuickActionItem::~QuickActionItem() = default;

    QString QuickActionItem::actionId() const {
        Q_D(const QuickActionItem);
        return d->actionId;
    }

    void QuickActionItem::setActionId(const QString &actionId) {
        Q_D(QuickActionItem);
        if (d->actionId != actionId) {
            d->actionId = actionId;
            emit actionIdChanged();
        }
    }

    QQmlComponent *QuickActionItem::actionComponent() const {
        Q_D(const QuickActionItem);
        return d->actionComponent;
    }

    void QuickActionItem::setActionComponent(QQmlComponent *component) {
        Q_D(QuickActionItem);
        if (d->actionComponent != component) {
            d->actionComponent = component;
            emit actionComponentChanged();
        }
    }

}

#include "moc_quickactionitem_p.cpp"
