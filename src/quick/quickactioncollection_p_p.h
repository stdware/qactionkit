#ifndef QUICKACTIONCOLLECTION_P_P_H
#define QUICKACTIONCOLLECTION_P_P_H

#include <QtCore/QList>

#include <QAKQuick/private/quickactioncollection_p.h>

namespace QAK {

class QuickActionItem;

    class QuickActionCollectionPrivate {
        Q_DECLARE_PUBLIC(QuickActionCollection)
    public:
        QuickActionCollection *q_ptr;
        QList<QuickActionItem *> items;

        static void appendItem(QQmlListProperty<QuickActionItem> *property, QuickActionItem *item);
        static qsizetype itemCount(QQmlListProperty<QuickActionItem> *property);
        static QuickActionItem *itemAt(QQmlListProperty<QuickActionItem> *property, qsizetype index);
        static void clearItems(QQmlListProperty<QuickActionItem> *property);
    };

}

#endif // QUICKACTIONCOLLECTION_P_P_H
