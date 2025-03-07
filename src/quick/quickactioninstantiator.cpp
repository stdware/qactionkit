#include "quickactioninstantiator_p.h"
#include "quickactioninstantiator_p_p.h"

#include <QQmlComponent>

#include <QAKQuick/quickactioncontext.h>
#include <QAKQuick/private/quickactioninstantiatorattachedtype_p.h>

namespace QAK {

    void QuickActionInstantiatorPrivate::updateContext() {
        
    }

    QuickActionInstantiator::QuickActionInstantiator(QObject *parent) : QObject(parent), d_ptr(new QuickActionInstantiatorPrivate) {
        Q_D(QuickActionInstantiator);
        d->q_ptr = this;
    }
    QuickActionInstantiator::~QuickActionInstantiator() = default;
    QuickActionInstantiatorAttachedType *QuickActionInstantiator::qmlAttachedProperties(QObject *object) {
        return new QuickActionInstantiatorAttachedType(object);
    }
    QuickActionContext *QuickActionInstantiator::context() const {
        Q_D(const QuickActionInstantiator);
        return d->context;
    }
    void QuickActionInstantiator::setContext(QuickActionContext *context) {
        Q_D(QuickActionInstantiator);
        if (d->context == context)
            return;
        d->context = context;
        d->updateContext();
        emit contextChanged();
    }
    int QuickActionInstantiator::count() const {
        Q_D(const QuickActionInstantiator);
        return d->objects.count();
    }
    QObject *QuickActionInstantiator::objectAt(int index) const {
        Q_D(const QuickActionInstantiator);
        if (index < 0 || index >= d->objects.count())
            return nullptr;
        return d->objects.at(index);
    }
    QQmlComponent *QuickActionInstantiator::overrideMenuComponent() const {
        Q_D(const QuickActionInstantiator);
        return d->overrideMenuComponent;
    }
    void QuickActionInstantiator::setOverrideMenuComponent(QQmlComponent *component) {
        Q_D(QuickActionInstantiator);
        if (d->overrideMenuComponent != component) {
            d->overrideMenuComponent = component;
            emit overrideMenuComponentChanged();
        }
    }
    QQmlComponent *QuickActionInstantiator::overrideSeparatorComponent() const {
        Q_D(const QuickActionInstantiator);
        return d->overrideSeparatorComponent;
    }
    void QuickActionInstantiator::setOverrideSeparatorComponent(QQmlComponent *component) {
        Q_D(QuickActionInstantiator);
        if (d->overrideSeparatorComponent != component) {
            d->overrideSeparatorComponent = component;
            emit overrideSeparatorComponentChanged();
        }
    }
    QQmlComponent *QuickActionInstantiator::overrideStretchComponent() const {
        Q_D(const QuickActionInstantiator);
        return d->overrideStretchComponent;
    }
    void QuickActionInstantiator::setOverrideStretchComponent(QQmlComponent *component) {
        Q_D(QuickActionInstantiator);
        if (d->overrideStretchComponent != component) {
            d->overrideStretchComponent = component;
            emit overrideStretchComponentChanged();
        }
    }
}

#include "moc_quickactioninstantiator_p.cpp"