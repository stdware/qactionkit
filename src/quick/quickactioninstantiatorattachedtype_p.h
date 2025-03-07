#ifndef QUICKACTIONINSTANTIATORATTACHEDTYPE_P_H
#define QUICKACTIONINSTANTIATORATTACHEDTYPE_P_H

#include <QObject>
#include <qqmlintegration.h>

namespace QAK {

    class QuickActionInstantiator;

    class QuickActionInstantiatorAttachedTypePrivate;

    class QuickActionInstantiatorAttachedType : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(QuickActionInstantiatorAttachedType)
        QML_ANONYMOUS

        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(QString text READ text NOTIFY textChanged)
        Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
        Q_PROPERTY(QString iconSource READ iconSource NOTIFY iconSourceChanged)
        Q_PROPERTY(QList<QKeySequence> shortcuts READ shortcuts NOTIFY shortcutsChanged)
        Q_PROPERTY(QMap<QString, QString> attributes READ attributes CONSTANT)
        Q_PROPERTY(QuickActionInstantiator *instantiator READ instantiator CONSTANT)

    public:
        explicit QuickActionInstantiatorAttachedType(QObject *parent = nullptr);
        ~QuickActionInstantiatorAttachedType() override;

        QString id() const;
        void setId(const QString &id);

        QString text() const;
        void setText(const QString &text);

        QString description() const;
        void setDescription(const QString &description);

        QString iconSource() const;
        void setIconSource(const QString &iconSource);

        QList<QKeySequence> shortcuts() const;
        void setShortcuts(const QList<QKeySequence> &shortcuts);

        QMap<QString, QString> attributes() const;
        void setAttributes(const QMap<QString, QString> &attributes);

        QuickActionInstantiator *instantiator() const;
        void setInstantiator(QuickActionInstantiator *instantiator);

    signals:
        void textChanged();
        void descriptionChanged();
        void iconSourceChanged();
        void shortcutsChanged();

    private:
        QScopedPointer<QuickActionInstantiatorAttachedTypePrivate> d_ptr;
    };

}

#endif //QUICKACTIONINSTANTIATORATTACHEDTYPE_P_H
