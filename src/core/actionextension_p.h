#ifndef ACTIONEXTENSION_P_H
#define ACTIONEXTENSION_P_H

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
        
        QString textTranslationContext;
        QString actionClassTranslationContext;
        QString descriptionTranslationContext;

        QMap<QString, QString> attributes;
    };

    struct ActionLayoutInfoEntry {
        QString id;
        ActionLayoutInfo::Type type;

        // references
        QVector<int> childIndexes;
    };

    struct ActionInsertionData {
        ActionInsertion::Anchor anchor;
        QString target;
        QString relativeTo;

        // references
        QVector<int> entryIndexes;
    };

    struct ActionExtensionData {
        QString version;

        QString id;
        QString hash;

        int itemCount;
        ActionItemInfoData *items;

        [[maybe_unused]] int layoutEntryCount; // Not used
        ActionLayoutInfoEntry *layoutEntries;

        int layoutCount;
        int *layouts;

        int insertionCount;
        ActionInsertionData *insertions;

        static inline const ActionExtensionData *get(const ActionExtension *q) {
            Q_ASSERT(q->d.data);
            return static_cast<const ActionExtensionData *>(q->d.data);
        }
    };

}

#endif // ACTIONEXTENSION_P_H
