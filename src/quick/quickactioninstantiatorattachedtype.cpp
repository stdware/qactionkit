#include "quickactioninstantiatorattachedtype_p.h"
#include "quickactioninstantiatorattachedtype_p_p.h"

#include <QKeySequence>

#include <QAKQuick/private/quickactioninstantiator_p.h>
#include <QAKQuick/private/quickactioninstantiator_p_p.h>
#include <QAKQuick/quickactioncontext.h>
#include <QAKCore/actionextension.h>

namespace QAK {
    QuickActionInstantiatorAttachedType::QuickActionInstantiatorAttachedType(QObject *parent) : QObject(parent), d_ptr(new QuickActionInstantiatorAttachedTypePrivate) {
    }
    QuickActionInstantiatorAttachedType::~QuickActionInstantiatorAttachedType() = default;

    void QuickActionInstantiatorAttachedType::init(const ActionItemInfo &info, QuickActionContext *context, int property) {
        setId(info.id());
        if (property & QuickActionInstantiatorPrivate::Text) {
            auto text = info.text(true);
            if (text.isEmpty()) {
                text = info.text();
            }
            if (text.isEmpty()) {
                text = info.id();
            }
            setText(text);
            auto description = info.description(true);
            if (description.isEmpty()) {
                description = info.description();
            }
            setDescription(description);
        }
        if (property & QuickActionInstantiatorPrivate::Icon) {
            // TODO: theme
            setIconSource(QUrl::fromLocalFile(context->registry()->actionIcon("", info.id()).filePath()));
        }
        if (property & QuickActionInstantiatorPrivate::Keymap) {
            setShortcuts(context->registry()->actionShortcuts(info.id()));
        }
        
        // Convert ActionAttributeKey map to QVariantList format
        QVariantList attributesList;
        const auto infoAttributes = info.attributes();
        for (auto it = infoAttributes.begin(); it != infoAttributes.end(); ++it) {
            QVariantMap attributeMap;
            attributeMap.insert("name", it.key().name);
            attributeMap.insert("namespace", it.key().namespaceUri);
            attributeMap.insert("value", it.value());
            attributesList.append(attributeMap);
        }
        setAttributes(attributesList);
    }

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
    QUrl QuickActionInstantiatorAttachedType::iconSource() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->iconSource;
    }
    void QuickActionInstantiatorAttachedType::setIconSource(const QUrl &iconSource) {
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
    QVariantList QuickActionInstantiatorAttachedType::attributes() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->attributes;
    }
    void QuickActionInstantiatorAttachedType::setAttributes(const QVariantList &attributes) {
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