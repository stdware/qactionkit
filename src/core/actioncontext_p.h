#ifndef ACTIONCONTEXT_P_H
#define ACTIONCONTEXT_P_H

#include <QAKCore/actioncontext.h>

namespace QAK {

    class QAK_CORE_EXPORT ActionContextPrivate {
        Q_DECLARE_PUBLIC(ActionContext)
    public:
        ActionContextPrivate();
        virtual ~ActionContextPrivate();

        void init();

        ActionContext *q_ptr;

        ActionRegistry *registry = nullptr;
    };

}


#endif // ACTIONCONTEXT_P_H
