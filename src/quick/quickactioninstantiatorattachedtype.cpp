#include "quickactioninstantiatorattachedtype_p.h"
#include "quickactioninstantiatorattachedtype_p_p.h"

#include <QKeySequence>

#include <QAKQuick/private/quickactioninstantiator_p.h>
#include <QAKQuick/private/quickactioninstantiator_p_p.h>
#include <QAKQuick/quickactioncontext.h>
#include <QAKCore/actionextension.h>

namespace QAK {
    QuickActionInstantiatorAttachedType::QuickActionInstantiatorAttachedType(QObject *parent) : QObject(parent), d_ptr(new QuickActionInstantiatorAttachedTypePrivate) {
        if (auto action = qobject_cast<QQuickAction *>(parent)) {
            connect(action, &QQuickAction::enabledChanged, this, &QuickActionInstantiatorAttachedType::iconChanged);
            connect(action, &QQuickAction::checkedChanged, this, &QuickActionInstantiatorAttachedType::iconChanged);
        }
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
            setActionIcon(context->registry()->actionIcon("", info.id()));
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
    QQuickIcon QuickActionInstantiatorAttachedType::icon() const {
        if (auto action = qobject_cast<QQuickAction *>(parent())) {
            return selectIconByStatus(action->isEnabled(), action->isChecked());
        }
        return selectIconByStatus(true, false);
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
    void QuickActionInstantiatorAttachedType::setActionIcon(const ActionIcon &actionIcon) {
        Q_D(QuickActionInstantiatorAttachedType);
        for (int i = 0; i < 4; i++) {
            QQuickIcon icon;
            bool enabledFlag = i & 1;
            bool checkedFlag = i & 2;
            icon.setSource(QUrl::fromLocalFile(actionIcon.filePath(enabledFlag, checkedFlag)));
            auto color = QColor::fromString(actionIcon.currentColor());
            icon.setColor(color);
            d->icons[i] = icon;
            emit iconChanged();
        }
    }
    QQuickIcon QuickActionInstantiatorAttachedType::selectIconByStatus(bool enabled, bool checked) const {
        Q_D(const QuickActionInstantiatorAttachedType);
        int flag = 0;
        if (enabled) {
            flag |= 1;
        }
        if (checked) {
            flag |= 2;
        }
        return d->icons[flag];
    }
}

#include "moc_quickactioninstantiatorattachedtype_p.cpp"