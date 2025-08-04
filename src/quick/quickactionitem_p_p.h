#ifndef QUICKACTIONITEM_P_P_H
#define QUICKACTIONITEM_P_P_H

#include "quickactionitem_p.h"

class QQmlComponent;

namespace QAK {
    class QuickActionItemPrivate {
        Q_DECLARE_PUBLIC(QuickActionItem)
    public:
        QuickActionItem *q_ptr;
        QString actionId;
        QQmlComponent *actionComponent = nullptr;
    };
}

#endif // QUICKACTIONITEM_P_P_H
