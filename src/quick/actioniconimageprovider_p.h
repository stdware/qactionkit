#ifndef ACTIONICONIMAGEPROVIDER_P_H
#define ACTIONICONIMAGEPROVIDER_P_H

#include <QAKQuick/actioniconimageprovider.h>

namespace QAK {

    class ActionIconImageProviderPrivate {
        Q_DECLARE_PUBLIC(ActionIconImageProvider)
    public:
        ActionIconImageProvider *q_ptr;

        ActionFamily *actionFamily{};
        QString theme;
    };

}

#endif //ACTIONICONIMAGEPROVIDER_P_H
