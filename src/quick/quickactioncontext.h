#ifndef QUICKACTIONCONTEXT_H
#define QUICKACTIONCONTEXT_H

#include <QAKCore/actioncontext.h>

#include <QAKQuick/qakquickglobal.h>

namespace QAK {

    class QAK_QUICK_EXPORT QuickActionContext : public ActionContext {
    public:
        ~QuickActionContext();
    };

}

#endif // QUICKACTIONCONTEXT_H