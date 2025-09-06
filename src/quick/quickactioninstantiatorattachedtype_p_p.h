#ifndef QUICKACTIONINSTANTIATORATTACHEDTYPE_P_P_H
#define QUICKACTIONINSTANTIATORATTACHEDTYPE_P_P_H

#include <QAKQuick/private/quickactioninstantiatorattachedtype_p.h>

#include <QVariant>
#include <QUrl>
#include <QtQuickTemplates2/private//qquickaction_p.h>

namespace QAK {
    class QuickActionInstantiatorAttachedTypePrivate {
    public:
        QString id;
        QString text;
        QString description;
        QUrl iconSource;
        QList<QKeySequence> shortcuts;
        QVariantList attributes;
        QuickActionInstantiator *instantiator;

        QQuickIcon icons[4];
    };
}

#endif //QUICKACTIONINSTANTIATORATTACHEDTYPE_P_P_H
