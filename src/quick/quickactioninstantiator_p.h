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
        Q_PROPERTY(QQmlComponent *overrideMenuComponent READ overrideMenuComponent WRITE setOverrideMenuComponent NOTIFY overrideMenuComponentChanged)
        Q_PROPERTY(QQmlComponent *overrideSeparatorComponent READ overrideSeparatorComponent WRITE setOverrideSeparatorComponent NOTIFY overrideSeparatorComponentChanged)
        Q_PROPERTY(QQmlComponent *overrideStretchComponent READ overrideStretchComponent WRITE setOverrideStretchComponent NOTIFY overrideStretchComponentChanged)
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

        QQmlComponent *overrideMenuComponent() const;
        void setOverrideMenuComponent(QQmlComponent *component);

        QQmlComponent *overrideSeparatorComponent() const;
        void setOverrideSeparatorComponent(QQmlComponent *component);

        QQmlComponent *overrideStretchComponent() const;
        void setOverrideStretchComponent(QQmlComponent *component);

    signals:
        void idChanged();
        void objectAdded(int index, QObject *object);
        void objectAboutToRemove(int index, QObject *object);
        void contextChanged();
        void countChanged();
        void overrideMenuComponentChanged(QQmlComponent *);
        void overrideSeparatorComponentChanged(QQmlComponent *);
        void overrideStretchComponentChanged(QQmlComponent *);

    private:
        QScopedPointer<QuickActionInstantiatorPrivate> d_ptr;
    };

}

#endif //QUICKACTIONINSTANTIATOR_P_H
