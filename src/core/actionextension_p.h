#ifndef ACTIONEXTENSION_P_H
#define ACTIONEXTENSION_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the QActionKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QAKCore/actionextension.h>

namespace QAK {

    struct ActionItemInfoData {
        QString id;
        ActionItemInfo::Type type;

        QString text;
        QString actionClass;
        QString description;
        QString icon;
        QList<QKeySequence> shortcuts;
        QString catalog;
        bool topLevel;
        QMap<QString, QString> attributes;

        QVector<ActionLayoutEntry> children;
    };

    struct ActionInsertionData {
        ActionInsertion::Anchor anchor;
        QString target;
        QString relativeTo;
        QVector<ActionLayoutEntry> items;
    };

    struct ActionExtensionData {
        QString version;

        QString id;
        QString hash;

        int itemCount;
        ActionItemInfoData *items;

        int insertionCount;
        ActionInsertionData *insertions;

        static inline const ActionExtensionData *get(const ActionExtension *q) {
            Q_ASSERT(q->d.data);
            return static_cast<const ActionExtensionData *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
