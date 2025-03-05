#ifndef ACTIONCONTEXT_P_H
#define ACTIONCONTEXT_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the QActionKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

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
