#include "quickmenuactioninstantiator_p.h"

#include <QQmlInfo>

#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>
#include <QtQuick/QQuickItem>

namespace QAK {

    void QuickMenuActionInstantiator::addToMenu(int index, QObject *object) {
        auto menu = target();
        if (!menu) {
            return;
        }
        if (auto action = qobject_cast<QQuickAction *>(object)) {
            QMetaObject::invokeMethod(menu, "insertAction", index, action);
        } else if (auto submenu = qobject_cast<QQuickMenu *>(object)) {
            QMetaObject::invokeMethod(menu, "insertMenu", index, submenu);
        } else if (auto item = qobject_cast<QQuickItem *>(object)) {
            QMetaObject::invokeMethod(menu, "insertItem", index, item);
        } else {
            qmlWarning(this) << "QAK::QuickMenuActionInstantiator: Unknown object type"
                          << object->metaObject()->className();
            auto placeholderItem = new QQuickItem;
            placeholderItem->setParent(object);
            placeholderItem->setVisible(false);
            QMetaObject::invokeMethod(menu, "insertItem", index, placeholderItem);
        }
    }

    void QuickMenuActionInstantiator::removeFromMenu(int index, QObject *object) {
        auto menu = target();
        if (!menu) {
            return;
        }
        if (auto action = qobject_cast<QQuickAction *>(object)) {
            QMetaObject::invokeMethod(menu, "removeAction", action);
        } else if (auto submenu = qobject_cast<QQuickMenu *>(object)) {
            QMetaObject::invokeMethod(menu, "removeMenu", submenu);
        } else if (auto item = qobject_cast<QQuickItem *>(object)) {
            QMetaObject::invokeMethod(menu, "removeItem", item);
        } else {
            qmlWarning(this) << "QAK::QuickActionInstantiator: Unknown object type"
                          << object->metaObject()->className();
            QMetaObject::invokeMethod(menu, "takeItem", index);
        }
    }
    
    QuickMenuActionInstantiator::QuickMenuActionInstantiator(QObject *parent)
        : AbstractQuickMenuActionInstantiator(parent) {
    }

    QuickMenuActionInstantiator::~QuickMenuActionInstantiator() = default;

}
