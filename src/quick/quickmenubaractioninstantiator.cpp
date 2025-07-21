#include "quickmenubaractioninstantiator_p.h"

#include <QQmlInfo>

#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>
#include <QtQuick/QQuickItem>

namespace QAK {

    void QuickMenuBarActionInstantiator::addToMenu(int index, QObject *object) {
        auto menuBar = target();
        if (!menuBar) {
            return;
        }
        if (auto submenu = qobject_cast<QQuickMenu *>(object)) {
            QMetaObject::invokeMethod(menuBar, "insertMenu", index, submenu);
        } else {
            qmlWarning(this) << "QAK::QuickMenuBarActionInstantiator: Unknown object type"
                          << object->metaObject()->className();
            auto placeholderMenu = menuComponent()->create(menuComponent()->creationContext());
            placeholderMenu->setParent(object);
            QMetaObject::invokeMethod(menuBar, "insertMenu", index, placeholderMenu);
        }
    }

    void QuickMenuBarActionInstantiator::removeFromMenu(int index, QObject *object) {
        auto menuBar = target();
        if (!menuBar) {
            return;
        }
        if (auto submenu = qobject_cast<QQuickMenu *>(object)) {
            QMetaObject::invokeMethod(menuBar, "removeMenu", submenu);
        } else {
            qmlWarning(this) << "QAK::QuickMenuBarActionInstantiator: Unknown object type"
                          << object->metaObject()->className();
            QMetaObject::invokeMethod(menuBar, "takeMenu", index);
        }
    }
    
    QuickMenuBarActionInstantiator::QuickMenuBarActionInstantiator(QObject *parent)
        : AbstractQuickMenuActionInstantiator(parent) {
    }

    QuickMenuBarActionInstantiator::~QuickMenuBarActionInstantiator() = default;

}
