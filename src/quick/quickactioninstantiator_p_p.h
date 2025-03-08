#ifndef QUICKACTIONINSTANTIATOR_P_P_H
#define QUICKACTIONINSTANTIATOR_P_P_H

#include <QAKQuick/private/quickactioninstantiator_p.h>

#include <QPointer>

#include <QAKCore/actionregistry.h>

namespace QAK {
    class QuickActionInstantiatorPrivate {
        Q_DECLARE_PUBLIC(QuickActionInstantiator)
    public:
        QuickActionInstantiator *q_ptr;

        QString id;

        QPointer<QuickActionContext> context = nullptr;
        QObjectList objects;

        QPointer<QQmlComponent> overrideMenuComponent;
        QPointer<QQmlComponent> overrideSeparatorComponent;
        QPointer<QQmlComponent> overrideStretchComponent;

        QQmlComponent *menuComponent() const;
        QQmlComponent *separatorComponent() const;
        QQmlComponent *stretchComponent() const;

        void addObject(int index, QObject *object);
        void removeObject(int index);
        void modifyObject(int index, QObject *object);

        QObjectList createObject(const ActionLayoutEntry &entry) const;
        QObject *createAction(const QString &actionId, QQmlComponent *component) const;
        QObject *createMenu(const QString &menuId) const;
        QObject *createSeparator() const;
        QObject *createStretch() const;

        void updateContext();
        void updateLayouts();

        enum ActionProperty { Text = 1, Icon = 2, Keymap = 4, All = Text | Icon | Keymap };
        void updateActionProperty(ActionProperty property);
        QuickActionInstantiatorAttachedType *attachInfoObjectTo(const QString &id, QObject *object, ActionProperty property) const;

        enum Element { Menu = 0x1000, Separator, Stretch };
        static Element getElement(QObject *object);
        static void setElement(QObject *object, Element element);
        void updateElement(Element element);
    };
}

#endif //QUICKACTIONINSTANTIATOR_P_P_H
