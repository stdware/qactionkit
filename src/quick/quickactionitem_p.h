#ifndef QUICKACTIONITEM_P_H
#define QUICKACTIONITEM_P_H

#include <QObject>
#include <qqmlintegration.h>

class QQmlComponent;

namespace QAK {

    class QuickActionItemPrivate;

    class QuickActionItem : public QObject {
        Q_OBJECT
        QML_NAMED_ELEMENT(ActionItem)
        Q_PROPERTY(QString actionId READ actionId WRITE setActionId NOTIFY actionIdChanged)
        Q_PROPERTY(QQmlComponent *actionComponent READ actionComponent WRITE setActionComponent NOTIFY actionComponentChanged)
        Q_DECLARE_PRIVATE(QuickActionItem)
        Q_CLASSINFO("DefaultProperty", "actionComponent")
    public:
        explicit QuickActionItem(QObject *parent = nullptr);
        ~QuickActionItem() override;

        QString actionId() const;
        void setActionId(const QString &actionId);

        QQmlComponent *actionComponent() const;
        void setActionComponent(QQmlComponent *component);

    signals:
        void actionIdChanged();
        void actionComponentChanged();

    private:
        QScopedPointer<QuickActionItemPrivate> d_ptr;
    };

}

#endif // QUICKACTIONITEM_P_H
