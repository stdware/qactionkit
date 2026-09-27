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
#include <QtCore/QHash>
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

        mutable stdc::linked_map<QString, ActionItemInfo> actionItems; // id(lowercase) -> info
        mutable bool extensionsDirty = false;

        mutable ActionCatalog catalog;
        mutable ActionLayouts defaultLayouts;
        mutable ActionLayouts layouts;

        QVector<ActionLayoutChange> layoutChanges;
        // The default layouts with the changes replayed so far, before the graph is built, and the
        // ids that removals of moves have taken out of it, for the additions of those moves
        mutable QMap<QString, QVector<ActionLayoutEntry>> changedAdjacencyMap;
        mutable QHash<QString, int> movedIds;

        QVector<QPointer<ActionContext>> contexts;

        void flushActionItems() const;

        ActionCatalog computeDefaultCatalog() const;
        ActionLayouts computeDefaultLayouts() const;

        void replayLayoutChanges() const;
        void replayLayoutChange(const ActionLayoutChange &change) const;
        void buildLayouts() const;
    };

}

#endif // ACTIONREGISTRY_P_H
