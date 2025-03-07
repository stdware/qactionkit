#ifndef QAKGLOBAL_P_H
#define QAKGLOBAL_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the QActionKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#include <QtCore/QLoggingCategory>
#include <QtCore/QHashFunctions>

#include <QAKCore/qakglobal.h>

QAK_CORE_EXPORT Q_DECLARE_LOGGING_CATEGORY(qActionKitLog);

QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QStringList);

namespace QAK {

    QAK_CORE_EXPORT QString tryTranslate(const char *context, const char *sourceText,
                                         const char *disambiguation, int n, bool *ok);

}

#endif // QAKGLOBAL_P_H