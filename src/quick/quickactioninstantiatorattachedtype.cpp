#include "quickactioninstantiatorattachedtype_p.h"
#include "quickactioninstantiatorattachedtype_p_p.h"

#include <QKeySequence>

#include <QAKQuick/private/quickactioninstantiator_p.h>

namespace QAK {
    QuickActionInstantiatorAttachedType::QuickActionInstantiatorAttachedType(QObject *parent) : QObject(parent), d_ptr(new QuickActionInstantiatorAttachedTypePrivate) {
    }
    QuickActionInstantiatorAttachedType::~QuickActionInstantiatorAttachedType() = default;
    QString QuickActionInstantiatorAttachedType::id() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->id;
    }
    void QuickActionInstantiatorAttachedType::setId(const QString &id) {
        Q_D(QuickActionInstantiatorAttachedType);
        d->id = id;
    }
    QString QuickActionInstantiatorAttachedType::text() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->text;
    }
    void QuickActionInstantiatorAttachedType::setText(const QString &text) {
        Q_D(QuickActionInstantiatorAttachedType);
        if (d->text != text) {
            d->text = text;
            emit textChanged();
        }
    }
    QString QuickActionInstantiatorAttachedType::description() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->description;
    }
    void QuickActionInstantiatorAttachedType::setDescription(const QString &description) {
        Q_D(QuickActionInstantiatorAttachedType);
        if (d->description != description) {
            d->description = description;
            emit descriptionChanged();
        }
    }
    QString QuickActionInstantiatorAttachedType::iconSource() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->iconSource;
    }
    void QuickActionInstantiatorAttachedType::setIconSource(const QString &iconSource) {
        Q_D(QuickActionInstantiatorAttachedType);
        if (d->iconSource != iconSource) {
            d->iconSource = iconSource;
            emit iconSourceChanged();
        }
    }
    QList<QKeySequence> QuickActionInstantiatorAttachedType::shortcuts() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->shortcuts;
    }
    void QuickActionInstantiatorAttachedType::setShortcuts(const QList<QKeySequence> &shortcuts) {
        Q_D(QuickActionInstantiatorAttachedType);
        if (d->shortcuts != shortcuts) {
            d->shortcuts = shortcuts;
            emit shortcutsChanged();
        }
    }
    QMap<QString, QString> QuickActionInstantiatorAttachedType::attributes() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->attributes;
    }
    void QuickActionInstantiatorAttachedType::setAttributes(const QMap<QString, QString> &attributes) {
        Q_D(QuickActionInstantiatorAttachedType);
        d->attributes = attributes;
    }
    QuickActionInstantiator *QuickActionInstantiatorAttachedType::instantiator() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->instantiator;
    }
    void QuickActionInstantiatorAttachedType::setInstantiator(QuickActionInstantiator *instantiator) {
        Q_D(QuickActionInstantiatorAttachedType);
        d->instantiator = instantiator;
    }
}

#include "moc_quickactioninstantiatorattachedtype_p.cpp"