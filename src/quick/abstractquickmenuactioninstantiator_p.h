#ifndef ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_H
#define ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_H

#include "quickactioninstantiator_p.h"

namespace QAK {

    class AbstractQuickMenuActionInstantiatorPrivate;

    class AbstractQuickMenuActionInstantiator : public QuickActionInstantiator {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AbstractQuickMenuActionInstantiator)
        Q_PROPERTY(QObject *parent READ parent WRITE setParent NOTIFY parentChanged)
        Q_PROPERTY(QObject *target READ target WRITE setTarget RESET resetTarget NOTIFY targetChanged)
    public:
        explicit AbstractQuickMenuActionInstantiator(QObject *parent = nullptr);
        ~AbstractQuickMenuActionInstantiator() override;

        QObject *parent() const;
        void setParent(QObject *parent);

        QObject *target() const;
        void setTarget(QObject *target);
        void resetTarget();

    protected:
        virtual void addToMenu(int index, QObject *object) = 0;
        virtual void removeFromMenu(int index, QObject *object) = 0;

    signals:
        void parentChanged();
        void targetChanged();

    private:
        QScopedPointer<AbstractQuickMenuActionInstantiatorPrivate> d_ptr;
    };

}

#endif //ABSTRACTQUICKMENUACTIONINSTANTIATOR_P_H
