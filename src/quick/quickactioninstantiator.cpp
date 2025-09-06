#include "quickactioninstantiator_p.h"
#include "quickactioninstantiator_p_p.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlInfo>
#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>

#include <QAKQuick/quickactioncontext.h>
#include <QAKQuick/private/quickactioninstantiatorattachedtype_p.h>
#include <QAKQuick/private/quickmenuactioninstantiator_p.h>

namespace QAK {

    static const char *ELEMENT_PROPERTY = "_qak_element";

    QQmlComponent *QuickActionInstantiatorPrivate::menuComponent() const {
        return isMenuComponentExplicitlySet ? menuComponent_override.get() : context ? context->menuComponent() : nullptr;
    }
    QQmlComponent *QuickActionInstantiatorPrivate::separatorComponent() const {
        return isSeparatorComponentExplicitlySet ? separatorComponent_override.get() : context ? context->separatorComponent() : nullptr;
    }
    QQmlComponent *QuickActionInstantiatorPrivate::stretchComponent() const {
        return isStretchComponentExplicitlySet ? stretchComponent_override.get() : context ? context->stretchComponent() : nullptr;
    }
    void QuickActionInstantiatorPrivate::addObject(int index, QObject *object) {
        Q_Q(QuickActionInstantiator);
        objects.insert(index, object);
        emit q->objectAdded(index, object);
        emit q->countChanged();
    }
    void QuickActionInstantiatorPrivate::removeObject(int index) {
        Q_Q(QuickActionInstantiator);
        emit q->objectRemoved(index, objects.at(index));
        objects.at(index)->deleteLater();
        objects.remove(index);
        emit q->countChanged();
    }
    void QuickActionInstantiatorPrivate::modifyObject(int index, QObject *object) {
        Q_Q(QuickActionInstantiator);
        emit q->objectRemoved(index, objects.at(index));
        objects.at(index)->deleteLater();
        objects[index] = object;
        emit q->objectAdded(index, object);
    }
    QObjectList QuickActionInstantiatorPrivate::createObject(const ActionLayoutEntry &entry) const {
        QObject *o;
        QQmlComponent *component;
        switch (entry.type()) {
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
        return object;
    }
    QObject *QuickActionInstantiatorPrivate::createMenu(const QString &menuId) const {
        Q_Q(const QuickActionInstantiator);
        auto menu = menuComponent()->create(context->menuComponent()->creationContext());
        auto attachedInfoObject = attachInfoObjectTo(menuId, menu, All);
        if (auto menuMenu = qobject_cast<QQuickMenu *>(menu)) {
            menuMenu->setTitle(attachedInfoObject->text());
            auto icon = menuMenu->icon();
            icon.setSource(attachedInfoObject->icon().source());
            if (attachedInfoObject->icon().color().isValid()) {
                icon.setColor(attachedInfoObject->icon().color());
            }
            menuMenu->setIcon(icon);
        }
        auto instantiator = new QuickMenuActionInstantiator(menu);
        attachedInfoObject->setInstantiator(instantiator);
        instantiator->setId(menuId);
        // No need to connect contextChanged, because all menus will be reloaded when context is changed
        instantiator->setContext(context);

        if (isMenuComponentExplicitlySet) {
            instantiator->setMenuComponent(menuComponent_override);
        } else {
            instantiator->resetMenuComponent();
        }
        QObject::connect(q, &QuickActionInstantiator::menuComponentChanged, instantiator, [=] {
            if (isMenuComponentExplicitlySet) {
                instantiator->setMenuComponent(menuComponent_override);
            } else {
                instantiator->resetMenuComponent();
            }
        });

        if (isSeparatorComponentExplicitlySet) {
            instantiator->setSeparatorComponent(separatorComponent_override);
        } else {
            instantiator->resetSeparatorComponent();
        }
        QObject::connect(q, &QuickActionInstantiator::separatorComponentChanged, instantiator, [=] {
            if (isSeparatorComponentExplicitlySet) {
                instantiator->setSeparatorComponent(separatorComponent_override);
            } else {
                instantiator->resetSeparatorComponent();
            }
        });

        if (isStretchComponentExplicitlySet) {
            instantiator->setStretchComponent(stretchComponent_override);
        } else {
            instantiator->resetStretchComponent();
        }
        QObject::connect(q, &QuickActionInstantiator::stretchComponentChanged, instantiator, [=] {
            if (isStretchComponentExplicitlySet) {
                instantiator->setStretchComponent(stretchComponent_override);
            } else {
                instantiator->resetStretchComponent();
            }
        });
        
        instantiator->QuickActionInstantiator::d_func()->updateLayouts();
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
        Q_Q(const QuickActionInstantiator);
        auto info = context->registry()->actionInfo(id);
        auto attachedInfoObject = qobject_cast<QuickActionInstantiatorAttachedType *>(qmlAttachedPropertiesObject<QuickActionInstantiator>(object));
        attachedInfoObject->init(info, context, property);
        attachedInfoObject->setInstantiator(const_cast<QuickActionInstantiator *>(q));
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
            if (!isMenuComponentExplicitlySet) {
                updateElement(Menu);
                emit q->menuComponentChanged();
            }

        });
        QObject::connect(context, &QuickActionContext::separatorComponentChanged, q, [=] {
            if (!isSeparatorComponentExplicitlySet) {
                updateElement(Separator);
                emit q->separatorComponentChanged();
            }
        });
        QObject::connect(context, &QuickActionContext::stretchComponentChanged, q, [=] {
            if (!isStretchComponentExplicitlySet) {
                updateElement(Stretch);
                emit q->stretchComponentChanged();
            }
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
        auto children = context->registry()->layouts().adjacencyMap().value(info.id());
        for (int childIndex = 0; childIndex < children.size(); childIndex++) {
            const auto &child = children[childIndex];
            auto list = createObject(child);
            QObjectList filteredList;
            filteredList.reserve(list.size());
            for (int i = 0; i < list.size(); i++) {
                auto object = list[i];
                if (getElement(object) == Separator) {
                    bool shouldDeleteSeparator =
                        !filteredList.isEmpty() && getElement(filteredList.last()) == Separator ||
                        filteredList.isEmpty() && (objects.isEmpty() || getElement(objects.last()) == Separator) ||
                        i == list.size() - 1 && childIndex == children.size() - 1;
                    if (shouldDeleteSeparator) {
                       delete object;
                        list[i] = nullptr;
                        continue;
                    }
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
        if (d->context)
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
    QQmlComponent *QuickActionInstantiator::menuComponent() const {
        Q_D(const QuickActionInstantiator);
        return d->menuComponent();
    }
    void QuickActionInstantiator::setMenuComponent(QQmlComponent *component) {
        Q_D(QuickActionInstantiator);
        if (d->isMenuComponentExplicitlySet && d->menuComponent_override == component)
            return;
        d->menuComponent_override = component;
        d->isMenuComponentExplicitlySet = true;
        d->updateElement(QuickActionInstantiatorPrivate::Menu);
        emit menuComponentChanged();
    }
    void QuickActionInstantiator::resetMenuComponent() {
        Q_D(QuickActionInstantiator);
        if (!d->isMenuComponentExplicitlySet)
            return;
        d->menuComponent_override = nullptr;
        d->isMenuComponentExplicitlySet = false;
        d->updateElement(QuickActionInstantiatorPrivate::Menu);
        emit menuComponentChanged();
    }
    QQmlComponent *QuickActionInstantiator::separatorComponent() const {
        Q_D(const QuickActionInstantiator);
        return d->separatorComponent();
    }
    void QuickActionInstantiator::setSeparatorComponent(QQmlComponent *component) {
        Q_D(QuickActionInstantiator);
        if (d->isSeparatorComponentExplicitlySet && d->separatorComponent_override == component)
            return;
        d->separatorComponent_override = component;
        d->isSeparatorComponentExplicitlySet = true;
        d->updateElement(QuickActionInstantiatorPrivate::Separator);
        emit separatorComponentChanged();
    }
    void QuickActionInstantiator::resetSeparatorComponent() {
        Q_D(QuickActionInstantiator);
        if (!d->isSeparatorComponentExplicitlySet)
            return;
        d->separatorComponent_override = nullptr;
        d->isSeparatorComponentExplicitlySet = false;
        d->updateElement(QuickActionInstantiatorPrivate::Separator);
        emit separatorComponentChanged();
    }
    QQmlComponent *QuickActionInstantiator::stretchComponent() const {
        Q_D(const QuickActionInstantiator);
        return d->stretchComponent();
    }
    void QuickActionInstantiator::setStretchComponent(QQmlComponent *component) {
        Q_D(QuickActionInstantiator);
        if (d->isStretchComponentExplicitlySet && d->stretchComponent_override == component)
            return;
        d->stretchComponent_override = component;
        d->isStretchComponentExplicitlySet = true;
        d->updateElement(QuickActionInstantiatorPrivate::Stretch);
        emit stretchComponentChanged();
    }
    void QuickActionInstantiator::resetStretchComponent() {
        Q_D(QuickActionInstantiator);
        if (!d->isStretchComponentExplicitlySet)
            return;
        d->stretchComponent_override = nullptr;
        d->isStretchComponentExplicitlySet = false;
        d->updateElement(QuickActionInstantiatorPrivate::Stretch);
        emit stretchComponentChanged();
    }
    void QuickActionInstantiator::forceUpdateLayouts() {
        Q_D(QuickActionInstantiator);
        d->updateLayouts();
    }
}

#include "moc_quickactioninstantiator_p.cpp"