#ifndef UTIL_H
#define UTIL_H

#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>

namespace Util {

    QString parseExpression(const QStringView &s, const QHash<QString, QString> &vars);

    QString absolutePath(const QString &path, const QString &baseDir);

}

#endif // UTIL_H
