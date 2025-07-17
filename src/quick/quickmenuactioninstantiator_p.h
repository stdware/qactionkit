#ifndef QUICKMENUACTIONINSTANTIATOR_P_H
#define QUICKMENUACTIONINSTANTIATOR_P_H

#include "abstractquickmenuactioninstantiator_p.h"

namespace QAK {

    class QuickMenuActionInstantiator : public AbstractQuickMenuActionInstantiator {
        Q_OBJECT
        QML_NAMED_ELEMENT(MenuActionInstantiator)
    public:
        explicit QuickMenuActionInstantiator(QObject *parent = nullptr);
        ~QuickMenuActionInstantiator() override;

    protected:
        void addToMenu(int index, QObject *object) override;
        void removeFromMenu(int index, QObject *object) override;
    };

}

#endif //QUICKMENUACTIONINSTANTIATOR_P_H
