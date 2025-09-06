#ifndef QUICKACTIONINSTANTIATORATTACHEDTYPE_P_H
#define QUICKACTIONINSTANTIATORATTACHEDTYPE_P_H

#include <QObject>
#include <qqmlintegration.h>

class QQuickIcon;

namespace QAK {

    class ActionIcon;

    class ActionItemInfo;

    class QuickActionContext;

    class QuickActionInstantiator;

    class QuickActionInstantiatorAttachedTypePrivate;

    class QuickActionInstantiatorAttachedType : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(QuickActionInstantiatorAttachedType)
        QML_ANONYMOUS

        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(QString text READ text NOTIFY textChanged)
        Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
        Q_PROPERTY(QQuickIcon icon READ icon NOTIFY iconChanged)
        Q_PROPERTY(QList<QKeySequence> shortcuts READ shortcuts NOTIFY shortcutsChanged)
        Q_PROPERTY(QVariantList attributes READ attributes CONSTANT)
        Q_PROPERTY(QuickActionInstantiator *instantiator READ instantiator CONSTANT)

    public:
        explicit QuickActionInstantiatorAttachedType(QObject *parent = nullptr);
        ~QuickActionInstantiatorAttachedType() override;

        void init(const ActionItemInfo &info, QuickActionContext *context, int);

        QString id() const;
        void setId(const QString &id);

        QString text() const;
        void setText(const QString &text);

        QString description() const;
        void setDescription(const QString &description);

        QQuickIcon icon() const;

        QList<QKeySequence> shortcuts() const;
        void setShortcuts(const QList<QKeySequence> &shortcuts);

        QVariantList attributes() const;
        void setAttributes(const QVariantList &attributes);

        QuickActionInstantiator *instantiator() const;
        void setInstantiator(QuickActionInstantiator *instantiator);

        void setActionIcon(const ActionIcon &actionIcon);

        Q_INVOKABLE QQuickIcon selectIconByStatus(bool enabled, bool checked) const;

    signals:
        void textChanged();
        void descriptionChanged();
        void iconChanged();
        void shortcutsChanged();

    private:
        QScopedPointer<QuickActionInstantiatorAttachedTypePrivate> d_ptr;
    };

}

#endif //QUICKACTIONINSTANTIATORATTACHEDTYPE_P_H
