#ifndef QUICKMENUBARACTIONINSTANTIATOR_P_H
#define QUICKMENUBARACTIONINSTANTIATOR_P_H

#include "abstractquickmenuactioninstantiator_p.h"

namespace QAK {

    class QuickMenuBarActionInstantiator : public AbstractQuickMenuActionInstantiator {
        Q_OBJECT
        QML_NAMED_ELEMENT(MenuBarActionInstantiator)
    public:
        explicit QuickMenuBarActionInstantiator(QObject *parent = nullptr);
        ~QuickMenuBarActionInstantiator() override;

    protected:
        void addToMenu(int index, QObject *object) override;
        void removeFromMenu(int index, QObject *object) override;
    };

}

#endif //QUICKMENUBARACTIONINSTANTIATOR_P_H
