#include "quickactioninstantiator_p.h"
#include "quickactioninstantiator_p_p.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QtQuickTemplates2/private/qquickaction_p.h>

#include <QAKQuick/quickactioncontext.h>
#include <QAKQuick/private/quickactioninstantiatorattachedtype_p.h>

namespace QAK {

    QQmlComponent *QuickActionInstantiatorPrivate::menuComponent() const {
        return overrideMenuComponent ? overrideMenuComponent : context ? context->menuComponent() : nullptr;
    }
    QQmlComponent *QuickActionInstantiatorPrivate::separatorComponent() const {
        return overrideSeparatorComponent ? overrideSeparatorComponent : context ? context->separatorComponent() : nullptr;
    }
    QQmlComponent *QuickActionInstantiatorPrivate::stretchComponent() const {
        return overrideStretchComponent ? overrideStretchComponent : context ? context->stretchComponent() : nullptr;
    }
    void QuickActionInstantiatorPrivate::addObject(int index, QObject *object) {
        Q_Q(QuickActionInstantiator);
        objects.insert(index, object);
        emit q->objectAdded(index, object);
        emit q->countChanged();
    }
    void QuickActionInstantiatorPrivate::removeObject(int index) {
        Q_Q(QuickActionInstantiator);
        emit q->objectAboutToRemove(index, objects.at(index));
        objects.at(index)->deleteLater();
        objects.remove(index);
        emit q->countChanged();
    }
    void QuickActionInstantiatorPrivate::modifyObject(int index, QObject *object) {
        Q_Q(QuickActionInstantiator);
        emit q->objectAboutToRemove(index, objects.at(index));
        objects.at(index)->deleteLater();
        objects[index] = object;
        emit q->objectAdded(index, object);
    }
    QObjectList QuickActionInstantiatorPrivate::createObject(const ActionLayoutEntry &entry) const {
        switch (entry.type()) {
            QObject *o;
            QQmlComponent *component;
            case ActionLayoutEntry::Separator:
                o = createSeparator();
                if (o)
                    return {o};
                break;
            case ActionLayoutEntry::Stretch:
                o = createStretch();
                if (o)
                    return {o};
                break;
            case ActionLayoutEntry::Action:
                component = context->action(entry.id());
                if (!component)
                    break;
                return {createAction(entry, component)};
            case ActionLayoutEntry::Menu:
                if (menuComponent())
                    return {createMenu(entry)};
                break;
            case ActionLayoutEntry::Group:
                QObjectList list;
                o = createSeparator();
                if (o)
                    list.append(o);
                for (const auto &child : context->registry()->actionInfo(id).children()) {
                    list += createObject(child);
                }
                o = createSeparator();
                if (o)
                    list.append(o);
                return list;
        }
        return {};
    }
    QObject *QuickActionInstantiatorPrivate::createAction(const ActionLayoutEntry &entry, QQmlComponent *component) const {
        auto object = component->create(component->creationContext());
        auto info = context->registry()->actionInfo(entry.id());
        auto attachedInfoObject = qobject_cast<QuickActionInstantiatorAttachedType *>(qmlAttachedPropertiesObject<QuickActionInstantiator>(object));
        attachedInfoObject->setId(info.id());
        attachedInfoObject->setText(info.text(true));
        attachedInfoObject->setDescription(info.description(true));
        // TODO icon
        attachedInfoObject->setShortcuts(info.shortcuts());
        attachedInfoObject->setAttributes(info.attributes());
        if (auto action = qobject_cast<QQuickAction *>(object)) {
            action->setText(attachedInfoObject->text());
            // TODO icon
            if (!attachedInfoObject->shortcuts().isEmpty()) {
                action->setShortcut(attachedInfoObject->shortcuts().first());
            }
        }

        return object;
    }
    QObject *QuickActionInstantiatorPrivate::createMenu(const ActionLayoutEntry &entry) const {
        auto menu = context->menuComponent()->create(context->menuComponent()->creationContext());
        auto instantiator = new QuickActionInstantiator(menu);
        instantiator->setId(entry.id());

        return menu;
    }
    QObject *QuickActionInstantiatorPrivate::createSeparator() const {
        if (separatorComponent())
            return separatorComponent()->create(separatorComponent()->creationContext());
        return nullptr;
    }
    QObject *QuickActionInstantiatorPrivate::createStretch() const {
        if (stretchComponent())
            return stretchComponent()->create(stretchComponent()->creationContext());
        return nullptr;
    }
    void QuickActionInstantiatorPrivate::updateContext() {
        Q_Q(QuickActionInstantiator);
        QObject::connect(context, &QuickActionContext::layoutsAboutToUpdate, q, [=] {
            updateLayouts();
        });
    }
    void QuickActionInstantiatorPrivate::updateLayouts() {
        Q_Q(QuickActionInstantiator);
        while (!objects.isEmpty()) {
            removeObject(objects.size() - 1);
        }
        if (!context || !context->registry())
            return;
        auto info = context->registry()->actionInfo(id);
        if (info.isNull())
            return;
        for (const auto &child : info.children()) {
            for (auto object : createObject(child)) {
                object->setParent(q);
                addObject(objects.size(), object);
            }
        }
    }

    QuickActionInstantiator::QuickActionInstantiator(QObject *parent) : QObject(parent), d_ptr(new QuickActionInstantiatorPrivate) {
        Q_D(QuickActionInstantiator);
        d->q_ptr = this;
    }
    QuickActionInstantiator::~QuickActionInstantiator() = default;
    QuickActionInstantiatorAttachedType *
        QuickActionInstantiator::qmlAttachedProperties(QObject *object) {
        return new QuickActionInstantiatorAttachedType(object);
    }
    QString QuickActionInstantiator::id() const {
        Q_D(const QuickActionInstantiator);
        return d->id;
    }
    void QuickActionInstantiator::setId(const QString &id) {
        Q_D(QuickActionInstantiator);
        if (d->id != id) {
            d->id = id;
            emit idChanged();
        }
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