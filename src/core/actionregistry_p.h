#ifndef ACTIONREGISTRY_P_H
#define ACTIONREGISTRY_P_H

#include <stdcorelib/linked_map.h>

#include <QAKCore/actionregistry.h>
#include <QAKCore/private/actionfamily_p.h>

namespace QAK {

    class QAK_CORE_EXPORT ActionRegistryPrivate : public ActionFamilyPrivate {
        Q_DECLARE_PUBLIC(ActionRegistry)
    public:
        ActionRegistryPrivate();
        ~ActionRegistryPrivate();

        stdc::linked_map<QString, const ActionExtension *> extensions;

        mutable stdc::linked_map<QString, ActionItemInfo> actionItems;
        mutable bool actionItemsDirty;

        mutable ActionRegistry::Catalog catalog;
        mutable ActionRegistry::Layouts layouts;

        void flushActionItems() const;
        
        ActionRegistry::Catalog defaultCatalog() const;
        ActionRegistry::Layouts defaultLayouts() const;
    };

}

#endif // ACTIONREGISTRY_P_H