#include "quickactioncontext.h"
#include "quickactioncontext_p.h"

#include <QQmlComponent>

namespace QAK {

    QuickActionContext::QuickActionContext(QObject *parent) : ActionContext(parent), d_ptr(new QuickActionContextPrivate) {
        Q_D(QuickActionContext);
        d->q_ptr = this;
    }
    QuickActionContext::~QuickActionContext() = default;

    void QuickActionContext::addAction(const QString &id, QQmlComponent *component) {
        Q_D(QuickActionContext);
        if (!d->actions.contains(id) || d->actions.value(id) != component) {
            d->actions.insert(id, component);
            emit actionChanged(id);
        }
    }
    QQmlComponent *QuickActionContext::action(const QString &id) const {
        Q_D(const QuickActionContext);
        return d->actions.value(id);
    }
    QQmlComponent *QuickActionContext::menuComponent() const {
        Q_D(const QuickActionContext);
        return d->menuComponent;
    }
    void QuickActionContext::setMenuComponent(QQmlComponent *component) {
        Q_D(QuickActionContext);
        if (component != d->menuComponent) {
            d->menuComponent = component;
            emit menuComponentChanged();
        }
    }
    QQmlComponent *QuickActionContext::separatorComponent() const {
        Q_D(const QuickActionContext);
        return d->separatorComponent;
    }
    void QuickActionContext::setSeparatorComponent(QQmlComponent *component) {
        Q_D(QuickActionContext);
        if (component != d->separatorComponent) {
            d->separatorComponent = component;
            emit separatorComponentChanged();
        }
    }
    QQmlComponent *QuickActionContext::stretchComponent() const {
        Q_D(const QuickActionContext);
        return d->stretchComponent;
    }
    void QuickActionContext::setStretchComponent(QQmlComponent *component) {
        Q_D(QuickActionContext);
        if (component != d->stretchComponent) {
            d->stretchComponent = component;
            emit stretchComponentChanged();
        }
    }
    void QuickActionContext::updateLayouts() {
        emit layoutsAboutToUpdate();
    }
    void QuickActionContext::updateTexts() {
        emit textsAboutToUpdate();
    }
    void QuickActionContext::updateIcons() {
        emit iconsAboutToUpdate();
    }
    void QuickActionContext::updateKeymap() {
        emit keymapAboutToUpdate();
    }

}

#include "moc_quickactioncontext.cpp"