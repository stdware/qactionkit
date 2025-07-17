#ifndef QUICKACTIONINSTANTIATOR_P_H
#define QUICKACTIONINSTANTIATOR_P_H

#include <QObject>
#include <qqmlintegration.h>

class QQmlComponent;

namespace QAK {

    class QuickActionContext;
    class QuickActionInstantiatorAttachedType;

    class QuickActionInstantiatorPrivate;

    class QuickActionInstantiator : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(QuickActionInstantiator)
        QML_NAMED_ELEMENT(ActionInstantiator)
        QML_ATTACHED(QuickActionInstantiatorAttachedType)
        Q_PROPERTY(QString actionId READ id WRITE setId NOTIFY idChanged)
        Q_PROPERTY(QuickActionContext *context READ context WRITE setContext NOTIFY contextChanged)
        Q_PROPERTY(int count READ count NOTIFY countChanged)
        Q_PROPERTY(QQmlComponent *menuComponent READ menuComponent WRITE setMenuComponent RESET resetMenuComponent NOTIFY menuComponentChanged)
        Q_PROPERTY(QQmlComponent *separatorComponent READ separatorComponent WRITE setSeparatorComponent RESET resetSeparatorComponent NOTIFY separatorComponentChanged)
        Q_PROPERTY(QQmlComponent *stretchComponent READ stretchComponent WRITE setStretchComponent RESET resetStretchComponent NOTIFY stretchComponentChanged)
    public:
        explicit QuickActionInstantiator(QObject *parent = nullptr);
        ~QuickActionInstantiator() override;

        static QuickActionInstantiatorAttachedType *qmlAttachedProperties(QObject *object);

        QString id() const;
        void setId(const QString &id);

        QuickActionContext *context() const;
        void setContext(QuickActionContext *context);

        int count() const;

        Q_INVOKABLE QObject *objectAt(int index) const;

        QQmlComponent *menuComponent() const;
        void setMenuComponent(QQmlComponent *component);
        void resetMenuComponent();

        QQmlComponent *separatorComponent() const;
        void setSeparatorComponent(QQmlComponent *component);
        void resetSeparatorComponent();

        QQmlComponent *stretchComponent() const;
        void setStretchComponent(QQmlComponent *component);
        void resetStretchComponent();

    signals:
        void idChanged();
        void objectAdded(int index, QObject *object);
        void objectRemoved(int index, QObject *object);
        void contextChanged();
        void countChanged();
        void menuComponentChanged();
        void separatorComponentChanged();
        void stretchComponentChanged();

    private:
        QScopedPointer<QuickActionInstantiatorPrivate> d_ptr;
    };

}

#endif //QUICKACTIONINSTANTIATOR_P_H
