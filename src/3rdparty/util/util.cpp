#include "util.h"

#include <QtCore/QStringView>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

namespace Util {

    QString parseExpression(const QStringView &s, const QHash<QString, QString> &vars) {
        using SIZE_TYPE = QString::size_type;

        QString result;
        for (SIZE_TYPE i = 0; i < s.size();) {
            if (s[i] == '$' && i + 1 < s.size() && s[i + 1] == '{') {
                SIZE_TYPE start = i + 2;
                SIZE_TYPE j = start;

                int braceCount = 1;
                while (j < s.size() && braceCount > 0) {
                    if (s[j] == '{')
                        braceCount++;
                    if (s[j] == '}')
                        braceCount--;
                    j++;
                }

                if (braceCount == 0) {
                    QStringView varName = s.mid(start, j - start - 1);
                    QString innerValue = parseExpression(varName, vars);
                    if (auto it = vars.find(innerValue); it != vars.end()) {
                        result += it.value();
                    }
                    i = j;
                } else {
                    // Invalid expression
                    return {};
                }
            } else {
                result += s[i];
                i++;
            }
        }

        // Replace "$$" with "$"
        QString finalResult;
        for (SIZE_TYPE i = 0; i < result.size(); ++i) {
            if (result[i] == '$' && i + 1 < result.size() && result[i + 1] == '$') {
                finalResult += '$';
                ++i;
            } else {
                finalResult += result[i];
            }
        }
        return finalResult;
    }

    QString absolutePath(const QString &path, const QString &baseDir) {
        if (baseDir.isEmpty()) {
            return path;
        }
        if (const QFileInfo info(path); info.isAbsolute()) {
            return QDir::cleanPath(path);
        }
        return QDir(baseDir).absoluteFilePath(path);
    }

}
