#ifndef QAKGLOBAL_P_H
#define QAKGLOBAL_P_H

#include <QtCore/QLoggingCategory>
#include <QtCore/QHashFunctions>

#include <QAKCore/qakglobal.h>

QAK_CORE_EXPORT Q_DECLARE_LOGGING_CATEGORY(qActionKitLog)

QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QStringList)

#endif // QAKGLOBAL_P_H