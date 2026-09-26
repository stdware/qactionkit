#ifndef ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_P_H
#define ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_P_H

#include <QtCore/QPointer>

#include <QAKQuick/private/abstractquickmenuactioninstantiator_p.h>

namespace QAK {
    class AbstractQuickMenuActionInstantiatorPrivate {
        Q_DECLARE_PUBLIC(AbstractQuickMenuActionInstantiator)
    public:
        AbstractQuickMenuActionInstantiator *q_ptr;

        QPointer<QObject> target;
        bool isTargetExplicitlySet{};

        void handleTargetChanged();
    };
}

#endif //ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_P_H
