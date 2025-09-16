#include "quickactioncontext.h"
#include "quickactioncontext_p.h"

#include <QQmlComponent>

#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>

#include <QAKCore/actionregistry.h>
#include <QAKQuick/private/quickactioninstantiatorattachedtype_p_p.h>
#include <QAKQuick/private/quickactioninstantiator_p_p.h>

namespace QAK {

    QuickActionContext::QuickActionContext(QObject *parent)
        : ActionContext(parent), d_ptr(new QuickActionContextPrivate) {
        Q_D(QuickActionContext);
        d->q_ptr = this;
    }
    QuickActionContext::~QuickActionContext() = default;

    void QuickActionContext::addAction(const QString &id, QQmlComponent *component) {
        Q_D(QuickActionContext);
        if (!d->actions.contains(id) || d->actions.value(id) != component) {
            d->actions.insert(id, component);
            emit actionChanged(id);
            emit actionsChanged();
        }
    }
    QQmlComponent *QuickActionContext::action(const QString &id) const {
        Q_D(const QuickActionContext);
        return d->actions.value(id);
    }
    void QuickActionContext::attachActionInfo(const QString &id, QObject *object) {
        auto attachedInfoObject = qobject_cast<QuickActionInstantiatorAttachedType *>(
            qmlAttachedPropertiesObject<QuickActionInstantiator>(object));
        Q_ASSERT(attachedInfoObject);
        attachedInfoObject->init(registry()->actionInfo(id), this,
                                 QuickActionInstantiatorPrivate::All);
        if (auto action = qobject_cast<QQuickAction *>(object)) {
            action->setText(attachedInfoObject->text());
            auto icon = action->icon();
            icon.setSource(attachedInfoObject->icon().source());
            QObject::connect(attachedInfoObject, &QuickActionInstantiatorAttachedType::iconChanged, action, [=] {
                auto icon = action->icon();
                icon.setSource(attachedInfoObject->icon().source());
                action->setIcon(icon);
            });
            if (attachedInfoObject->icon().color().isValid()) {
                icon.setColor(attachedInfoObject->icon().color());
            }
            action->setIcon(icon);
            if (!attachedInfoObject->shortcuts().isEmpty()) {
                action->setShortcut(attachedInfoObject->shortcuts().first());
            }
        } else if (auto menu = qobject_cast<QQuickMenu *>(object)) {
            menu->setTitle(attachedInfoObject->text());
            auto icon = menu->icon();
            icon.setSource(attachedInfoObject->icon().source());
            QObject::connect(attachedInfoObject, &QuickActionInstantiatorAttachedType::iconChanged, menu, [=] {
                auto icon = menu->icon();
                icon.setSource(attachedInfoObject->icon().source());
                menu->setIcon(icon);
            });
            if (attachedInfoObject->icon().color().isValid()) {
                icon.setColor(attachedInfoObject->icon().color());
            }
            menu->setIcon(icon);
        }
    }
    QStringList QuickActionContext::actions() const {
        Q_D(const QuickActionContext);
        return d->actions.keys();
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

    void QuickActionContext::updateElement(ActionElement element) {
        switch (element) {
            case AE_Layouts:
                emit layoutsAboutToUpdate();
                break;
            case AE_Texts:
                emit textsAboutToUpdate();
                break;
            case AE_Keymap:
                emit keymapAboutToUpdate();
                break;
            case AE_Icons:
                emit iconsAboutToUpdate();
                break;
        }
    }

}

#include "moc_quickactioncontext.cpp"
