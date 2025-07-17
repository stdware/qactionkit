#ifndef ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_P_H
#define ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_P_H

#include "abstractquickmenuactioninstantiator_p.h"

#include <QPointer>

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
