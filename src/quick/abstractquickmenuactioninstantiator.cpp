#include "abstractquickmenuactioninstantiator_p.h"
#include "abstractquickmenuactioninstantiator_p_p.h"

#include <QPointer>

namespace QAK {

    void AbstractQuickMenuActionInstantiatorPrivate::handleTargetChanged() {
        Q_Q(AbstractQuickMenuActionInstantiator);
        auto menu = q->target();
        if (!menu) {
            return;
        }
        for (int i = 0; i < q->count(); i++) {
            q->addToMenu(i, q->objectAt(i));
        }
    }

    AbstractQuickMenuActionInstantiator::AbstractQuickMenuActionInstantiator(QObject *parent)
        : QuickActionInstantiator(parent), d_ptr(new AbstractQuickMenuActionInstantiatorPrivate) {
        Q_D(AbstractQuickMenuActionInstantiator);
        d->q_ptr = this;
        connect(this, &QuickActionInstantiator::objectAdded, this, &AbstractQuickMenuActionInstantiator::addToMenu);
        connect(this, &QuickActionInstantiator::objectRemoved, this, &AbstractQuickMenuActionInstantiator::removeFromMenu);
        connect(this, &AbstractQuickMenuActionInstantiator::targetChanged, [=] {
            d->handleTargetChanged();
        });
    }

    AbstractQuickMenuActionInstantiator::~AbstractQuickMenuActionInstantiator() = default;

    QObject *AbstractQuickMenuActionInstantiator::parent() const {
        return QObject::parent();
    }

    void AbstractQuickMenuActionInstantiator::setParent(QObject *parent) {
        Q_D(AbstractQuickMenuActionInstantiator);
        if (parent == QObject::parent()) {
            return;
        }
        QObject::setParent(parent);
        emit parentChanged();
        if (!d->isTargetExplicitlySet) {
            emit targetChanged();
        }
    }

    QObject *AbstractQuickMenuActionInstantiator::target() const {
        Q_D(const AbstractQuickMenuActionInstantiator);
        return d->isTargetExplicitlySet ? d->target.get() : QObject::parent();
    }

    void AbstractQuickMenuActionInstantiator::setTarget(QObject *target) {
        Q_D(AbstractQuickMenuActionInstantiator);
        if (d->isTargetExplicitlySet && d->target == target)
            return;
        d->target = target;
        d->isTargetExplicitlySet = true;
        emit targetChanged();
    }

    void AbstractQuickMenuActionInstantiator::resetTarget() {
        Q_D(AbstractQuickMenuActionInstantiator);
        if (!d->target)
            return;
        d->target = nullptr;
        d->isTargetExplicitlySet = false;
        emit targetChanged();
    }

}
