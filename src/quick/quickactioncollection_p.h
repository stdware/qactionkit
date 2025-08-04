#ifndef QUICKACTIONCOLLECTION_P_H
#define QUICKACTIONCOLLECTION_P_H

#include <QObject>
#include <qqmlintegration.h>
#include <qqmllist.h>

namespace QAK {

    class QuickActionItem;
    class QuickActionContext;
    class QuickActionCollectionPrivate;

    class QuickActionCollection : public QObject {
        Q_OBJECT
        QML_NAMED_ELEMENT(ActionCollection)
        Q_PROPERTY(QQmlListProperty<QuickActionItem> data READ data)
        Q_DECLARE_PRIVATE(QuickActionCollection)
        Q_CLASSINFO("DefaultProperty", "data")
    public:
        explicit QuickActionCollection(QObject *parent = nullptr);
        ~QuickActionCollection() override;

        QQmlListProperty<QuickActionItem> data();

        Q_INVOKABLE void registerToContext(QuickActionContext *context);

    private:
        QScopedPointer<QuickActionCollectionPrivate> d_ptr;
    };

}

#endif // QUICKACTIONCOLLECTION_P_H
