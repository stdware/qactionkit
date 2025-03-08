#include "quickactioninstantiator_p.h"
#include "quickactioninstantiator_p_p.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlInfo>
#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>

#include <QAKQuick/quickactioncontext.h>
#include <QAKQuick/private/quickactioninstantiatorattachedtype_p.h>

namespace QAK {

    static const char *ELEMENT_PROPERTY = "_qak_element";

    QQmlComponent *QuickActionInstantiatorPrivate::menuComponent() const {
        return overrideMenuComponent ? overrideMenuComponent.get() : context ? context->menuComponent() : nullptr;
    }
    QQmlComponent *QuickActionInstantiatorPrivate::separatorComponent() const {
        return overrideSeparatorComponent ? overrideSeparatorComponent.get() : context ? context->separatorComponent() : nullptr;
    }
    QQmlComponent *QuickActionInstantiatorPrivate::stretchComponent() const {
        return overrideStretchComponent ? overrideStretchComponent.get() : context ? context->stretchComponent() : nullptr;
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
                return {createAction(entry.id(), component)};
            case ActionLayoutEntry::Menu:
                if (menuComponent())
                    return {createMenu(entry.id())};
                break;
            case ActionLayoutEntry::Group:
                QObjectList list;
                o = createSeparator();
                if (o)
                    list.append(o);
                for (const auto &child : context->registry()->actionInfo(entry.id()).children()) {
                    list += createObject(child);
                }
                o = createSeparator();
                if (o)
                    list.append(o);
                return list;
        }
        return {};
    }
    QObject *QuickActionInstantiatorPrivate::createAction(const QString &actionId, QQmlComponent *component) const {
        auto object = component->create(component->creationContext());
        auto attachedInfoObject = attachInfoObjectTo(actionId, object, All);
        if (auto action = qobject_cast<QQuickAction *>(object)) {
            action->setText(attachedInfoObject->text());
            auto icon = action->icon();
            icon.setSource(attachedInfoObject->iconSource());
            action->setIcon(icon);
            if (!attachedInfoObject->shortcuts().isEmpty()) {
                action->setShortcut(attachedInfoObject->shortcuts().first());
            }
        }
        return object;
    }
    QObject *QuickActionInstantiatorPrivate::createMenu(const QString &menuId) const {
        Q_Q(const QuickActionInstantiator);
        auto menu = context->menuComponent()->create(context->menuComponent()->creationContext());
        auto attachedInfoObject = attachInfoObjectTo(menuId, menu, All);
        if (auto menuMenu = qobject_cast<QQuickMenu *>(menu)) {
            menuMenu->setTitle(attachedInfoObject->text());
            auto icon = menuMenu->icon();
            icon.setSource(attachedInfoObject->iconSource());
            menuMenu->setIcon(icon);
        }
        auto instantiator = new QuickActionInstantiator(menu);
        attachedInfoObject->setInstantiator(instantiator);
        instantiator->setId(menuId);
        instantiator->setContext(context);
        instantiator->setOverrideMenuComponent(overrideMenuComponent);
        QObject::connect(q, &QuickActionInstantiator::overrideMenuComponentChanged, instantiator, &QuickActionInstantiator::setOverrideMenuComponent);
        instantiator->setOverrideSeparatorComponent(overrideSeparatorComponent);
        QObject::connect(q, &QuickActionInstantiator::overrideSeparatorComponentChanged, instantiator, &QuickActionInstantiator::setOverrideSeparatorComponent);
        instantiator->setOverrideStretchComponent(overrideStretchComponent);
        QObject::connect(q, &QuickActionInstantiator::overrideStretchComponentChanged, instantiator, &QuickActionInstantiator::setOverrideStretchComponent);
        QObject::connect(instantiator, &QuickActionInstantiator::objectAdded, menu, [=](int index, QObject *object) {
            if (auto action = qobject_cast<QQuickAction *>(object)) {
                QMetaObject::invokeMethod(menu, "insertAction", index, action);
            } else if (auto submenu = qobject_cast<QQuickMenu *>(object)) {
                QMetaObject::invokeMethod(menu, "insertMenu", index, submenu);
            } else if (auto item = qobject_cast<QQuickItem *>(object)) {
                QMetaObject::invokeMethod(menu, "insertItem", index, item);
            } else {
                qmlWarning(q) << "QAK::QuickActionInstantiator: Unknown object type" << object->metaObject()->className();
            }
        });
        QObject::connect(instantiator, &QuickActionInstantiator::objectAboutToRemove, menu, [=](int index, QObject *object) {
            if (auto action = qobject_cast<QQuickAction *>(object)) {
                QMetaObject::invokeMethod(menu, "removeAction", action);
            } else if (auto submenu = qobject_cast<QQuickMenu *>(object)) {
                QMetaObject::invokeMethod(menu, "removeMenu", submenu);
            } else if (auto item = qobject_cast<QQuickItem *>(object)) {
                QMetaObject::invokeMethod(menu, "removeItem", item);
            } else {
                qmlWarning(q) << "QAK::QuickActionInstantiator: Unknown object type" << object->metaObject()->className();
            }
        });
        instantiator->d_func()->updateLayouts();
        setElement(menu, Menu);
        return menu;
    }
    QObject *QuickActionInstantiatorPrivate::createSeparator() const {
        if (separatorComponent()) {
            auto separator = separatorComponent()->create(separatorComponent()->creationContext());
            setElement(separator, Separator);
            return separator;
        }
        return nullptr;
    }
    QObject *QuickActionInstantiatorPrivate::createStretch() const {
        if (stretchComponent()) {
            auto stretch = stretchComponent()->create(stretchComponent()->creationContext());
            setElement(stretch, Stretch);
            return stretch;
        }
        return nullptr;
    }
    QuickActionInstantiatorAttachedType *QuickActionInstantiatorPrivate::attachInfoObjectTo(const QString &id, QObject *object, ActionProperty property) const {
        auto info = context->registry()->actionInfo(id);
        auto attachedInfoObject = qobject_cast<QuickActionInstantiatorAttachedType *>(qmlAttachedPropertiesObject<QuickActionInstantiator>(object));
        attachedInfoObject->setId(info.id());
        if (property & Text) {
            auto text = info.text(true);
            if (text.isEmpty()) {
                text = info.text();
            }
            if (text.isEmpty()) {
                text = info.id();
            }
            attachedInfoObject->setText(text);
            auto description = info.description(true);
            if (description.isEmpty()) {
                description = info.description();
            }
            attachedInfoObject->setDescription(description);
        }
        if (property & Icon) {
            // TODO icon
        }
        if (property & Keymap) {
            attachedInfoObject->setShortcuts(context->registry()->actionShortcuts(info.id()));
        }
        attachedInfoObject->setAttributes(info.attributes());
        return attachedInfoObject;
    }
    void QuickActionInstantiatorPrivate::updateContext() {
        Q_Q(QuickActionInstantiator);
        QObject::connect(context, &QuickActionContext::layoutsAboutToUpdate, q, [=] {
            updateLayouts();
        });
        QObject::connect(context, &QuickActionContext::textsAboutToUpdate, q, [=] {
            updateActionProperty(Text);
        });
        QObject::connect(context, &QuickActionContext::iconsAboutToUpdate, q, [=] {
            updateActionProperty(Icon);
        });
        QObject::connect(context, &QuickActionContext::keymapAboutToUpdate, q, [=] {
            updateActionProperty(Keymap);
        });
        QObject::connect(context, &QuickActionContext::menuComponentChanged, q, [=] {
            if (!overrideMenuComponent)
                updateElement(Menu);
        });
        QObject::connect(context, &QuickActionContext::separatorComponentChanged, q, [=] {
            if (!overrideSeparatorComponent)
                updateElement(Separator);
        });
        QObject::connect(context, &QuickActionContext::stretchComponentChanged, q, [=] {
            if (!overrideStretchComponent)
                updateElement(Stretch);
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
            auto list = createObject(child);
            QObjectList filteredList;
            filteredList.reserve(list.size());
            for (int i = 0; i < list.size(); i++) {
                auto object = list[i];
                if (getElement(object) == Separator && (i == 0 || i == list.size() - 1 || !list[i - 1] || getElement(list[i - 1]) == Separator)) {
                    object->deleteLater();
                    list[i] = nullptr;
                    continue;
                }
                filteredList.append(object);

            }
            for (auto object : filteredList) {
                object->setParent(q);
                addObject(objects.size(), object);
            }
        }
    }
    void QuickActionInstantiatorPrivate::updateActionProperty(ActionProperty property) {
        for (auto object : objects) {
            auto action = qobject_cast<QQuickAction *>(object);
            if (!action)
                continue;
            auto attachedInfoObject = qobject_cast<QuickActionInstantiatorAttachedType *>(
                qmlAttachedPropertiesObject<QuickActionInstantiator>(object));
            attachInfoObjectTo(attachedInfoObject->id(), object, property);
        }
    }
    QuickActionInstantiatorPrivate::Element
        QuickActionInstantiatorPrivate::getElement(QObject *object) {
        return static_cast<Element>(object->property(ELEMENT_PROPERTY).toInt());
    }
    void QuickActionInstantiatorPrivate::setElement(QObject *object, Element element) {
        object->setProperty(ELEMENT_PROPERTY, element);
    }
    void QuickActionInstantiatorPrivate::updateElement(Element element) {
        QList<QPair<int, QObject *>> objectsToUpdate;
        for (int i = 0; i < objects.size(); i++) {
            auto object = objects[i];
            auto elementType = getElement(object);
            if (elementType != element)
                continue;
            objectsToUpdate.emplace_back(i, object);
        }
        for (const auto &[i, object] : objectsToUpdate) {
            if (element == Menu) {
                auto attachedInfoObject = qobject_cast<QuickActionInstantiatorAttachedType *>(qmlAttachedPropertiesObject<QuickActionInstantiator>(object));
                modifyObject(i, createMenu(attachedInfoObject->id()));
            } else if (element == Separator) {
                modifyObject(i, createSeparator());
            } else if (element == Stretch) {
                modifyObject(i, createStretch());
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
        if (d->context)
            disconnect(d->context, nullptr, this, nullptr);
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
            d->updateElement(QuickActionInstantiatorPrivate::Menu);
            emit overrideMenuComponentChanged(component);
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
            d->updateElement(QuickActionInstantiatorPrivate::Separator);
            emit overrideSeparatorComponentChanged(component);
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
            d->updateElement(QuickActionInstantiatorPrivate::Stretch);
            emit overrideStretchComponentChanged(component);
        }
    }
}

#include "moc_quickactioninstantiator_p.cpp"