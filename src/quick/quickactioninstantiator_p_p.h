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

        QQmlComponent *overrideMenuComponent = nullptr;
        QQmlComponent *overrideSeparatorComponent = nullptr;
        QQmlComponent *overrideStretchComponent = nullptr;

        QQmlComponent *menuComponent() const;
        QQmlComponent *separatorComponent() const;
        QQmlComponent *stretchComponent() const;

        void addObject(int index, QObject *object);
        void removeObject(int index);
        void modifyObject(int index, QObject *object);

        QObjectList createObject(const ActionLayoutEntry &entry) const;
        QObject *createAction(const ActionLayoutEntry &entry, QQmlComponent *component) const;
        QObject *createMenu(const ActionLayoutEntry &entry) const;
        QObject *createSeparator() const;
        QObject *createStretch() const;

        void updateContext();
        void updateLayouts();
    };
}

#endif //QUICKACTIONINSTANTIATOR_P_P_H
