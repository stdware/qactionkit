#include "quickactioninstantiatorattachedtype_p.h"
#include "quickactioninstantiatorattachedtype_p_p.h"

#include <QtGui/QKeySequence>
#include <QtQuickTemplates2/private/qquickmenu_p.h>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionregistry.h>

#include <QAKQuick/private/quickactioninstantiator_p.h>
#include <QAKQuick/private/quickactioninstantiator_p_p.h>
#include <QAKQuick/quickactioncontext.h>

namespace QAK {
    QuickActionInstantiatorAttachedType::QuickActionInstantiatorAttachedType(QObject *parent) : QObject(parent), d_ptr(new QuickActionInstantiatorAttachedTypePrivate) {
        connect(this, &QuickActionInstantiatorAttachedType::textChanged, this, &QuickActionInstantiatorAttachedType::updateText);
        connect(this, &QuickActionInstantiatorAttachedType::iconChanged, this, &QuickActionInstantiatorAttachedType::updateIcon);
        if (auto action = qobject_cast<QQuickAction *>(parent)) {
            connect(action, &QQuickAction::enabledChanged, this, &QuickActionInstantiatorAttachedType::iconChanged);
            connect(action, &QQuickAction::checkedChanged, this, &QuickActionInstantiatorAttachedType::iconChanged);
            connect(this, &QuickActionInstantiatorAttachedType::shortcutsChanged, this, &QuickActionInstantiatorAttachedType::updateShortcuts);
        }
    }
    QuickActionInstantiatorAttachedType::~QuickActionInstantiatorAttachedType() = default;

    void QuickActionInstantiatorAttachedType::init(const ActionItemInfo &info, QuickActionContext *context, int property) {
        Q_D(QuickActionInstantiatorAttachedType);
        if (d->context != context) {
            QObject::disconnect(d->keymapConnection);
            QObject::disconnect(d->textConnection);
            d->context = context;
            d->keymapConnection = connect(context, &QuickActionContext::keymapAboutToUpdate, this, [this] {
                Q_D(QuickActionInstantiatorAttachedType);
                if (d->context && d->context->registry()) {
                    auto registry = d->context->registry();
                    const auto info = registry->actionInfo(d->id);
                    setShortcuts(info && info->isCommand() ? registry->actionShortcuts(d->id) : QList<QKeySequence>());
                }
            });
            d->textConnection = connect(context, &QuickActionContext::textsAboutToUpdate, this, [this] {
                Q_D(QuickActionInstantiatorAttachedType);
                if (d->context && d->context->registry()) {
                    init(d->context->registry()->actionInfo(d->id).value_or(ActionItemInfo()), d->context, QuickActionInstantiatorPrivate::Text);
                }
            });
        }
        setId(info.id());
        if (property & QuickActionInstantiatorPrivate::Text) {
            // The item shows the short text, because it appears in menus and tool bars.
            auto text = info.shortText().toString();
            if (text.isEmpty()) {
                text = info.id();
            }
            setText(text);
            setDescription(info.description().toString());
            // The full text, stripped as QAction strips the text of an action without a tool
            // tip, which is the tool tip of the Widgets backend
            auto toolTip = info.text().strippedText();
            if (toolTip.isEmpty()) {
                toolTip = info.id();
            }
            setToolTip(toolTip);
        }
        if (property & QuickActionInstantiatorPrivate::Icon) {
            // TODO: theme
            setActionIcon(context->registry()
                              ->actionIcon(QString(), info.id(), info.icon())
                              .value_or(ActionIcon()));
        }
        if (property & QuickActionInstantiatorPrivate::Keymap) {
            const auto shortcuts = info.isCommand() ? context->registry()->actionShortcuts(info.id()) : QList<QKeySequence>();
            if (d->shortcuts == shortcuts) {
                updateShortcuts();
            } else {
                setShortcuts(shortcuts);
            }
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
        if (property & QuickActionInstantiatorPrivate::Text) {
            updateText();
        }
        if (property & QuickActionInstantiatorPrivate::Icon) {
            updateIcon();
        }
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
    QString QuickActionInstantiatorAttachedType::toolTip() const {
        Q_D(const QuickActionInstantiatorAttachedType);
        return d->toolTip;
    }
    void QuickActionInstantiatorAttachedType::setToolTip(const QString &toolTip) {
        Q_D(QuickActionInstantiatorAttachedType);
        if (d->toolTip != toolTip) {
            d->toolTip = toolTip;
            emit toolTipChanged();
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

    void QuickActionInstantiatorAttachedType::updateShortcuts() {
        Q_D(QuickActionInstantiatorAttachedType);
        auto action = qobject_cast<QQuickAction *>(parent());
        if (!action) {
            return;
        }
        action->setShortcut(d->shortcuts.isEmpty() ? QKeySequence() : d->shortcuts.first());
    }

    void QuickActionInstantiatorAttachedType::updateText() {
        if (auto action = qobject_cast<QQuickAction *>(parent())) {
            action->setText(text());
        } else if (auto menu = qobject_cast<QQuickMenu *>(parent())) {
            menu->setTitle(text());
        }
    }

    void QuickActionInstantiatorAttachedType::updateIcon() {
        if (auto action = qobject_cast<QQuickAction *>(parent())) {
            auto value = action->icon();
            value.setSource(icon().source());
            if (icon().color().isValid()) {
                value.setColor(icon().color());
            }
            action->setIcon(value);
        } else if (auto menu = qobject_cast<QQuickMenu *>(parent())) {
            auto value = menu->icon();
            value.setSource(icon().source());
            if (icon().color().isValid()) {
                value.setColor(icon().color());
            }
            menu->setIcon(value);
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
            icon.setSource(actionIcon.url(enabledFlag, checkedFlag));
            auto color = QColor::fromString(actionIcon.currentColor());
            icon.setColor(color);
            d->icons[i] = icon;
            emit iconChanged();
        }
        for (auto &icon : d->icons) {
            if (icon.source().isEmpty()) {
                icon = d->icons[1];
            }
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
