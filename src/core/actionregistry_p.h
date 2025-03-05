#ifndef ACTIONREGISTRY_P_H
#define ACTIONREGISTRY_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the QActionKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QtCore/QPointer>

#include <stdcorelib/linked_map.h>

#include <QAKCore/actionregistry.h>
#include <QAKCore/actioncontext.h>
#include <QAKCore/private/actionfamily_p.h>

namespace QAK {

    class QAK_CORE_EXPORT ActionRegistryPrivate : public ActionFamilyPrivate {
        Q_DECLARE_PUBLIC(ActionRegistry)
    public:
        ActionRegistryPrivate();
        ~ActionRegistryPrivate();

        stdc::linked_map<QString, const ActionExtension *> extensions;

        mutable stdc::linked_map<QString, ActionItemInfo> actionItems; // id(lowercase) -> info
        mutable bool actionItemsDirty;

        mutable ActionRegistry::Catalog catalog;
        mutable ActionRegistry::Layouts layouts;

        QVector<QPointer<ActionContext>> contexts;

        void flushActionItems() const;

        ActionRegistry::Catalog defaultCatalog() const;
        ActionRegistry::Layouts defaultLayouts() const;
    };

}

#endif // ACTIONREGISTRY_P_H