#ifndef QUICKACTIONCONTEXT_P_H
#define QUICKACTIONCONTEXT_P_H

#include <QAKQuick/quickactioncontext.h>

#include <QPointer>

namespace QAK {

    class QuickActionContextPrivate {
        Q_DECLARE_PUBLIC(QuickActionContext)
    public:
        QuickActionContext *q_ptr;

        QHash<QString, QQmlComponent *> actions;
        QPointer<QQmlComponent> menuComponent;
        QPointer<QQmlComponent> separatorComponent;
        QPointer<QQmlComponent> stretchComponent;

    };
}

#endif //QUICKACTIONCONTEXT_P_H
