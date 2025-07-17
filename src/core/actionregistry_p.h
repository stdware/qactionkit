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
#include <QtCore/QVarLengthArray>

#include <stdcorelib/linked_map.h>

#include <QAKCore/actionregistry.h>
#include <QAKCore/actioncontext.h>
#include <QAKCore/private/actionfamily_p.h>

namespace QAK {

    class ActionRegistryPrivate : public ActionFamilyPrivate {
        Q_DECLARE_PUBLIC(ActionRegistry)
    public:
        ActionRegistryPrivate() = default;
        ~ActionRegistryPrivate() = default;

        stdc::linked_map<QString, const ActionExtension *> extensions;
        stdc::linked_map<QString, ActionItemInfo> actionItems; // id(lowercase) -> info

        ActionCatalog catalog;
        ActionLayouts layouts;

        QVector<QPointer<ActionContext>> contexts;

        void flushActionItems();

        ActionCatalog defaultCatalog() const;
        ActionLayouts defaultLayouts() const;

        ActionLayouts correctLayouts(const ActionLayouts &layouts) const;
    };

}

#endif // ACTIONREGISTRY_P_H