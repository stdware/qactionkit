#ifndef ACTIONICONIMAGEPROVIDER_H
#define ACTIONICONIMAGEPROVIDER_H

#include <QQuickImageProvider>

#include <QAKQuick/qakquickglobal.h>

namespace QAK {

    class ActionFamily;

    class ActionIconImageProviderPrivate;

    class QAK_QUICK_EXPORT ActionIconImageProvider : public QQuickImageProvider {
        Q_DECLARE_PRIVATE(ActionIconImageProvider)
    public:
        explicit ActionIconImageProvider();
        ~ActionIconImageProvider() override;

        ActionFamily *actionFamily() const;
        void setActionFamily(ActionFamily *actionFamily);

        QString theme() const;
        void setTheme(const QString &theme);

        QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    private:
        QScopedPointer<ActionIconImageProviderPrivate> d_ptr;
    };

}

#endif //ACTIONICONIMAGEPROVIDER_H
