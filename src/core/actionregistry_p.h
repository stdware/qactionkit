#ifndef ACTIONREGISTRY_P_H
#define ACTIONREGISTRY_P_H

#include <QAKCore/actionregistry.h>
#include <QAKCore/private/actionfamily_p.h>

namespace QAK {

    class ActionCatalogData : public QSharedData {
    public:
        QString id;
        QList<ActionCatalog> children;
    };

    class ActionLayoutData : public QSharedData {
    public:
        QString id;
        ActionLayoutInfo::Type type = ActionLayoutInfo::Action;
        QList<ActionLayout> children;
    };

    class QAK_CORE_EXPORT ActionRegistryPrivate : public ActionFamilyPrivate {
        Q_DECLARE_PUBLIC(ActionRegistry)
    public:
        ActionRegistryPrivate();
        ~ActionRegistryPrivate();
    };

}

#endif // ACTIONREGISTRY_P_H