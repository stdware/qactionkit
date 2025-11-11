#ifndef UTIL_H
#define UTIL_H

#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>

namespace Util {

    QString parseExpression(const QStringView &s, const QHash<QString, QString> &vars);

    QString absolutePath(const QString &path, const QString &baseDir);

    QUrl absoluteUrl(const QUrl &url, const QUrl &baseUrl);

}

#endif // UTIL_H
