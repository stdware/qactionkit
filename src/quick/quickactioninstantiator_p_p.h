#ifndef QUICKACTIONINSTANTIATOR_P_P_H
#define QUICKACTIONINSTANTIATOR_P_P_H

#include <QAKQuick/private/quickactioninstantiator_p.h>

#include <QPointer>

namespace QAK {
    class QuickActionInstantiatorPrivate {
        Q_DECLARE_PUBLIC(QuickActionInstantiator)
    public:
        QuickActionInstantiator *q_ptr;

        QPointer<QuickActionContext> context = nullptr;
        QObjectList objects;

        QQmlComponent *overrideMenuComponent = nullptr;
        QQmlComponent *overrideSeparatorComponent = nullptr;
        QQmlComponent *overrideStretchComponent = nullptr;

        void updateContext();
    };
}

#endif //QUICKACTIONINSTANTIATOR_P_P_H
