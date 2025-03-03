#ifndef ACTIONREGISTRY_P_H
#define ACTIONREGISTRY_P_H

#include <QAKCore/actionregistry.h>
#include <QAKCore/private/actionfamily_p.h>

namespace QAK {

    class QAK_CORE_EXPORT ActionRegistryPrivate : public ActionFamilyPrivate {
        Q_DECLARE_PUBLIC(ActionRegistry)
    public:
        ActionRegistryPrivate();
        ~ActionRegistryPrivate();
    };

}

#endif // ACTIONREGISTRY_P_H