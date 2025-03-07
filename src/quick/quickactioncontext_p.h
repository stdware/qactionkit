#ifndef QUICKACTIONCONTEXT_P_H
#define QUICKACTIONCONTEXT_P_H

#include <QAKQuick/quickactioncontext.h>

namespace QAK {

    class QuickActionContextPrivate {
        Q_DECLARE_PUBLIC(QuickActionContext)
    public:
        QuickActionContext *q_ptr;

        QHash<QString, QQmlComponent *> actions;
        QQmlComponent *menuComponent = nullptr;
        QQmlComponent *separatorComponent = nullptr;
        QQmlComponent *stretchComponent = nullptr;

    };
}

#endif //QUICKACTIONCONTEXT_P_H
