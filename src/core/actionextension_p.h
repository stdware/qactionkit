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
        QString category;
        QString description;

        // Translation contexts written on the item, each empty if that of the extension applies
        QString textContext;
        QString categoryContext;
        QString descriptionContext;

        QString icon;
        QList<QKeySequence> shortcuts;
        QString catalog;
        bool topLevel;
        bool external;
        QMap<ActionAttributeKey, QString> attributes;

        QVector<ActionLayoutEntry> children;
    };

    struct ActionInsertionData {
        ActionInsertion::Anchor anchor;
        QString target;
        QString relativeTo;
        int priority;
        QVector<ActionLayoutEntry> items;

        // The priority of an insertion that does not specify one
        static constexpr int defaultPriority = 1000;
    };

    struct ActionExtensionData {
        QString version;

        QString id;
        QString hash;

        // Translation contexts of the configuration, each empty if the built-in default applies
        QString textContext;
        QString categoryContext;
        QString descriptionContext;

        int itemCount;
        ActionItemInfoData *items;

        int insertionCount;
        ActionInsertionData *insertions;

        // Translation contexts used if neither the item nor the configuration specifies one
        static constexpr char defaultTextContext[] = "QActionKit::ActionText";
        static constexpr char defaultCategoryContext[] = "QActionKit::ActionCategory";
        static constexpr char defaultDescriptionContext[] = "QActionKit::ActionDescription";

        static inline const ActionExtensionData *get(const ActionExtension *q) {
            Q_ASSERT(q->d.data);
            return static_cast<const ActionExtensionData *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
