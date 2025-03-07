#include "qakglobal_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/private/qcoreapplication_p.h>

namespace QAK {

    static void replacePercentN(QString *result, int n) {
        if (n >= 0) {
            int percentPos = 0;
            int len = 0;
            while ((percentPos = result->indexOf(QLatin1Char('%'), percentPos + len)) != -1) {
                len = 1;
                if (percentPos + len == result->length())
                    break;
                QString fmt;
                if (result->at(percentPos + len) == QLatin1Char('L')) {
                    ++len;
                    if (percentPos + len == result->length())
                        break;
                    fmt = QLatin1String("%L1");
                } else {
                    fmt = QLatin1String("%1");
                }
                if (result->at(percentPos + len) == QLatin1Char('n')) {
                    fmt = fmt.arg(n);
                    ++len;
                    result->replace(percentPos, len, fmt);
                    len = fmt.length();
                }
            }
        }
    }

    /*!
        Returns the translation text for \a sourceText, along with the success flag.
    */
    QString tryTranslate(const char *context, const char *sourceText, const char *disambiguation,
                         int n, bool *ok) {
        if (ok)
            *ok = false;

        QString result;
        if (!sourceText) {
            return result;
        }

        class HackedApplication : public QCoreApplication {
        public:
            inline QCoreApplicationPrivate *d_func() {
                return static_cast<QCoreApplicationPrivate *>(d_ptr.data());
            }
        };

        auto self = QCoreApplication::instance();
        if (self) {
            QCoreApplicationPrivate *d = static_cast<HackedApplication *>(self)->d_func();
            QReadLocker locker(&d->translateMutex);
            if (!d->translators.isEmpty()) {
                QList<QTranslator *>::ConstIterator it;
                QTranslator *translationFile;
                for (it = d->translators.constBegin(); it != d->translators.constEnd(); ++it) {
                    translationFile = *it;
                    result = translationFile->translate(context, sourceText, disambiguation, n);
                    if (!result.isNull())
                        break;
                }
            }
        }

        if (result.isNull()) {
            result = QString::fromUtf8(sourceText);
        } else if (ok) {
            *ok = true;
        }
        replacePercentN(&result, n);
        return result;
    }

}