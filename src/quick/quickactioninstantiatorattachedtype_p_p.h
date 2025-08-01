#ifndef QUICKACTIONINSTANTIATORATTACHEDTYPE_P_P_H
#define QUICKACTIONINSTANTIATORATTACHEDTYPE_P_P_H

#include <QAKQuick/private/quickactioninstantiatorattachedtype_p.h>

#include <QVariant>
#include <QUrl>

namespace QAK {
    class QuickActionInstantiatorAttachedTypePrivate {
    public:
        QString id;
        QString text;
        QString description;
        QUrl iconSource;
        QList<QKeySequence> shortcuts;
        QMap<QString, QString> attributes;
        QuickActionInstantiator *instantiator;
    };
}

#endif //QUICKACTIONINSTANTIATORATTACHEDTYPE_P_P_H
