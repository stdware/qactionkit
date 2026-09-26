#include "widgetactioncontext.h"

#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtWidgets/QWidgetAction>

#include <QAKCore/actionregistry.h>
#include <QAKCore/private/actioncontext_p.h>

namespace QAK {

    class WidgetAction : public QWidgetAction {
    public:
        explicit WidgetAction(std::function<QWidget *(QWidget *)> fac, QObject *parent = nullptr)
            : QWidgetAction(parent), fac(std::move(fac)) {
        }

        inline QList<QWidget *> createdWidgets() const {
            return QWidgetAction::createdWidgets();
        }

    protected:
        QWidget *createWidget(QWidget *parent) override {
            return fac ? fac(parent) : nullptr;
        }

        std::function<QWidget *(QWidget *)> fac;
    };

    // An item registered by the application through WidgetActionContext. The item is a plain
    // value. The object it refers to is owned by the application or parented to the context,
    // therefore copying an item neither transfers nor duplicates ownership.
    struct ActionItem {
        enum Type {
            Invalid,
            Action,
            Menu,
            MenuBar,
            ToolBar,
            Widget,
        };

        Type t = Invalid;
        QPointer<QObject> o;
        // Whether the object was created by the context and is destroyed when the item is
        // replaced or removed.
        bool owned = false;

        ActionItem() = default;
        ActionItem(Type t, QObject *o, bool owned = false) : t(t), o(o), owned(owned) {
        }

        QAction *action() const {
            return t == Action ? qobject_cast<QAction *>(o.data()) : nullptr;
        }
        QMenu *menu() const {
            return t == Menu ? qobject_cast<QMenu *>(o.data()) : nullptr;
        }
        QMenuBar *menuBar() const {
            return t == MenuBar ? qobject_cast<QMenuBar *>(o.data()) : nullptr;
        }
        QToolBar *toolBar() const {
            return t == ToolBar ? qobject_cast<QToolBar *>(o.data()) : nullptr;
        }
        WidgetAction *widgetAction() const {
            // WidgetAction has no Q_OBJECT, and the type tag guarantees the static cast.
            return t == Widget ? static_cast<WidgetAction *>(o.data()) : nullptr;
        }

        // Returns the action that represents the item in a parent container, or nullptr for a
        // menu bar or a tool bar, which cannot be nested.
        QAction *asAction() const {
            if (auto a = action()) {
                return a;
            }
            if (auto wa = widgetAction()) {
                return wa;
            }
            if (auto m = menu()) {
                return m->menuAction();
            }
            return nullptr;
        }

        // Returns the widget into which the layout entries of the item are built, or nullptr for
        // an item that is not a container.
        QWidget *asContainer() const {
            if (auto m = menu()) {
                return m;
            }
            if (auto mb = menuBar()) {
                return mb;
            }
            return toolBar();
        }
    };

    class WidgetActionContextPrivate : public ActionContextPrivate {
        Q_DECLARE_PUBLIC(WidgetActionContext)
    public:
        WidgetActionContextPrivate() = default;
        ~WidgetActionContextPrivate() = default;

        WidgetActionContext::Attributes attrs;

        // The items registered by the application.
        QMap<QString, ActionItem> items;
        // The actions and sub-menus created while building the layouts, keyed by item id. Each
        // is parented to the context or to its container, which may destroy it, therefore the
        // pointers are guarded.
        QMap<QString, QPointer<QObject>> autoItems;
        // The separators and stretches, which have no id and are created anew on every build
        // pass.
        QList<QPointer<QObject>> transientItems;

        void setItem(const QString &id, const ActionItem &item);
        void removeItem(const QString &id);

        void updateLayouts();
        void updateProperties(ActionElement element);

    private:
        // An entry resolved during a build pass, before redundant separators are collapsed.
        struct PendingEntry {
            enum Kind {
                Item,
                Separator,
                Stretch,
            };
            QAction *action = nullptr; // for Item only
            Kind kind = Item;
        };

        // The layouts of the pass currently in progress.
        QMap<QString, QVector<ActionLayoutEntry>> currentLayouts;

        QAction *actionForId(const QString &id, QSet<QString> &usedIds);
        QMenu *menuForId(const QString &id, QWidget *parent, QSet<QString> &usedIds);

        void applyInfo(QAction *action, const QString &id, ActionElement element) const;

        void collectEntries(QWidget *container, const QVector<ActionLayoutEntry> &entries,
                            QList<PendingEntry> &result, QSet<QString> &usedIds,
                            QSet<QString> &builtContainers);
        void buildContainer(const QString &id, QWidget *container, QSet<QString> &usedIds,
                            QSet<QString> &builtContainers);
    };

    void WidgetActionContextPrivate::setItem(const QString &id, const ActionItem &item) {
        removeItem(id);
        items.insert(id, item);
    }

    void WidgetActionContextPrivate::removeItem(const QString &id) {
        auto it = items.find(id);
        if (it == items.end()) {
            return;
        }
        if (it->owned) {
            delete it->o.data();
        }
        items.erase(it);
    }

    void WidgetActionContextPrivate::applyInfo(QAction *action, const QString &id,
                                               ActionElement element) const {
        auto reg = registry;
        if (!action || !reg) {
            return;
        }

        const auto info = reg->actionInfo(id);

        if (element == AE_Layouts || element == AE_Texts) {
            QString text = info.text(true);
            if (text.isEmpty()) {
                text = info.text();
            }
            if (text.isEmpty()) {
                text = id;
            }
            action->setText(text);

            if (attrs & WidgetActionContext::UpdateToolTipWithDescription) {
                QString description = info.description(true);
                if (description.isEmpty()) {
                    description = info.description();
                }
                if (!description.isEmpty()) {
                    action->setToolTip(description);
                }
            }
        }

        if (element == AE_Layouts || element == AE_Keymap) {
            action->setShortcuts(reg->actionShortcuts(id));
        }

        if ((element == AE_Layouts || element == AE_Icons) && !info.isNull()) {
            // TODO: theme
            action->setIcon(reg->actionIcon(QString(), info.icon()).icon());
        }
    }

    QAction *WidgetActionContextPrivate::actionForId(const QString &id, QSet<QString> &usedIds) {
        Q_Q(WidgetActionContext);

        if (auto it = items.find(id); it != items.end()) {
            return it->asAction();
        }

        usedIds.insert(id);
        if (auto it = autoItems.find(id); it != autoItems.end()) {
            if (auto action = qobject_cast<QAction *>(it.value().data())) {
                return action;
            }
            // The object was destroyed by its parent, and a new one is created below.
            autoItems.erase(it);
        }

        auto action = q->createAction(id, q);
        if (!action) {
            return nullptr;
        }
        QObject::connect(action, &QAction::triggered, q,
                         [q, id] { Q_EMIT q->actionTriggered(id); });
        QObject::connect(action, &QAction::hovered, q, [q, id] { Q_EMIT q->actionHovered(id); });
        QObject::connect(action, &QAction::toggled, q,
                         [q, id](bool checked) { Q_EMIT q->actionToggled(id, checked); });
        autoItems.insert(id, action);
        return action;
    }

    QMenu *WidgetActionContextPrivate::menuForId(const QString &id, QWidget *parent,
                                                 QSet<QString> &usedIds) {
        Q_Q(WidgetActionContext);

        if (auto it = items.find(id); it != items.end()) {
            return it->menu();
        }

        usedIds.insert(id);
        if (auto it = autoItems.find(id); it != autoItems.end()) {
            if (auto menu = qobject_cast<QMenu *>(it.value().data())) {
                // The parent is left unchanged, because re-parenting a QMenu clears its popup
                // window flags.
                return menu;
            }
            autoItems.erase(it);
        }

        auto menu = q->createSubMenu(id, parent);
        if (!menu) {
            return nullptr;
        }
        autoItems.insert(id, menu);
        return menu;
    }

    void WidgetActionContextPrivate::collectEntries(QWidget *container,
                                                    const QVector<ActionLayoutEntry> &entries,
                                                    QList<PendingEntry> &result,
                                                    QSet<QString> &usedIds,
                                                    QSet<QString> &builtContainers) {
        for (const auto &entry : entries) {
            switch (entry.type()) {
                case ActionLayoutEntry::Separator: {
                    result.append({nullptr, PendingEntry::Separator});
                    break;
                }
                case ActionLayoutEntry::Stretch: {
                    result.append({nullptr, PendingEntry::Stretch});
                    break;
                }
                case ActionLayoutEntry::Action: {
                    if (auto action = actionForId(entry.id(), usedIds)) {
                        applyInfo(action, entry.id(), AE_Layouts);
                        result.append({action, PendingEntry::Item});
                    }
                    break;
                }
                case ActionLayoutEntry::Menu: {
                    auto menu = menuForId(entry.id(), container, usedIds);
                    if (!menu) {
                        break;
                    }
                    buildContainer(entry.id(), menu, usedIds, builtContainers);
                    // The text of the menu action is the title of the menu.
                    applyInfo(menu->menuAction(), entry.id(), AE_Layouts);
                    result.append({menu->menuAction(), PendingEntry::Item});
                    break;
                }
                case ActionLayoutEntry::Group: {
                    // A group has no widget of its own. Its entries are placed in the container
                    // directly, between two separators.
                    result.append({nullptr, PendingEntry::Separator});
                    collectEntries(container, currentLayouts.value(entry.id()), result, usedIds,
                                   builtContainers);
                    result.append({nullptr, PendingEntry::Separator});
                    break;
                }
            }
        }
    }

    void WidgetActionContextPrivate::buildContainer(const QString &id, QWidget *container,
                                                    QSet<QString> &usedIds,
                                                    QSet<QString> &builtContainers) {
        if (builtContainers.contains(id)) {
            return;
        }
        builtContainers.insert(id);

        QList<PendingEntry> pending;
        collectEntries(container, currentLayouts.value(id), pending, usedIds, builtContainers);

        // Leading and consecutive separators are removed.
        QList<PendingEntry> entries;
        entries.reserve(pending.size());
        for (const auto &entry : std::as_const(pending)) {
            if (entry.kind == PendingEntry::Separator &&
                (entries.isEmpty() || entries.last().kind == PendingEntry::Separator)) {
                continue;
            }
            entries.append(entry);
        }
        // Trailing separators are removed.
        while (!entries.isEmpty() && entries.last().kind == PendingEntry::Separator) {
            entries.removeLast();
        }

        auto toolBar = qobject_cast<QToolBar *>(container);
        for (const auto &entry : std::as_const(entries)) {
            switch (entry.kind) {
                case PendingEntry::Item: {
                    container->addAction(entry.action);
                    break;
                }
                case PendingEntry::Separator: {
                    auto separator = new QAction(container);
                    separator->setSeparator(true);
                    transientItems.append(separator);
                    container->addAction(separator);
                    break;
                }
                case PendingEntry::Stretch: {
                    // A stretch applies to a tool bar only.
                    if (!toolBar) {
                        break;
                    }
                    auto spacer = new QWidget(toolBar);
                    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    // The returned action owns the spacer.
                    transientItems.append(toolBar->addWidget(spacer));
                    break;
                }
            }
        }
    }

    void WidgetActionContextPrivate::updateLayouts() {
        auto reg = registry;
        if (!reg) {
            return;
        }

        currentLayouts = reg->layouts().adjacencyMap();

        // The separators and stretches of the previous pass are destroyed.
        for (const auto &object : std::as_const(transientItems)) {
            delete object.data();
        }
        transientItems.clear();

        // Every action is removed from the containers before they are rebuilt.
        const auto detach = [](QWidget *container) {
            const auto actions = container->actions();
            for (const auto &action : actions) {
                container->removeAction(action);
            }
        };
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (auto container = it->asContainer()) {
                detach(container);
            }
        }
        for (const auto &object : std::as_const(autoItems)) {
            if (auto menu = qobject_cast<QMenu *>(object.data())) {
                detach(menu);
            }
        }

        QSet<QString> usedIds;
        QSet<QString> builtContainers;
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (auto container = it->asContainer()) {
                buildContainer(it.key(), container, usedIds, builtContainers);
            }
        }

        // The created items that the current layouts no longer reference are destroyed.
        for (auto it = autoItems.begin(); it != autoItems.end();) {
            if (it.value() && usedIds.contains(it.key())) {
                ++it;
                continue;
            }
            delete it.value().data();
            it = autoItems.erase(it);
        }

        currentLayouts.clear();
    }

    void WidgetActionContextPrivate::updateProperties(ActionElement element) {
        if (!registry) {
            return;
        }
        for (auto it = items.begin(); it != items.end(); ++it) {
            applyInfo(it->asAction(), it.key(), element);
        }
        for (auto it = autoItems.begin(); it != autoItems.end(); ++it) {
            auto action = qobject_cast<QAction *>(it.value().data());
            if (!action) {
                if (auto menu = qobject_cast<QMenu *>(it.value().data())) {
                    action = menu->menuAction();
                }
            }
            applyInfo(action, it.key(), element);
        }
    }

    WidgetActionContext::WidgetActionContext(QObject *parent)
        : ActionContext(*new WidgetActionContextPrivate(), parent) {
    }

    WidgetActionContext::~WidgetActionContext() = default;

    WidgetActionContext::Attributes WidgetActionContext::attributes() const {
        Q_D(const WidgetActionContext);
        return d->attrs;
    }

    void WidgetActionContext::setAttribute(Attribute attr, bool on) {
        Q_D(WidgetActionContext);
        if (on) {
            d->attrs |= attr;
        } else {
            d->attrs &= ~attr;
        }
    }

    void WidgetActionContext::setAttributes(Attributes attrs) {
        Q_D(WidgetActionContext);
        d->attrs = attrs;
    }

    QAction *WidgetActionContext::action(const QString &id) const {
        Q_D(const WidgetActionContext);
        if (auto it = d->items.find(id); it != d->items.end()) {
            return it->action();
        }
        // Otherwise the action created while building the layouts.
        return qobject_cast<QAction *>(d->autoItems.value(id).data());
    }

    void WidgetActionContext::addAction(const QString &id, QAction *action) {
        Q_D(WidgetActionContext);
        d->setItem(id, ActionItem(ActionItem::Action, action));
    }

    QList<QWidget *> WidgetActionContext::widgets(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        if (it == d->items.end()) {
            return {};
        }
        auto widgetAction = it->widgetAction();
        return widgetAction ? widgetAction->createdWidgets() : QList<QWidget *>{};
    }

    void WidgetActionContext::addWidgetFactory(const QString &id,
                                               std::function<QWidget *(QWidget *)> fac) {
        Q_D(WidgetActionContext);
        d->setItem(id,
                   ActionItem(ActionItem::Widget, new WidgetAction(std::move(fac), this), true));
    }

    QMenu *WidgetActionContext::menu(const QString &id) const {
        Q_D(const WidgetActionContext);
        if (auto it = d->items.find(id); it != d->items.end()) {
            return it->menu();
        }
        // Otherwise the sub-menu created while building the layouts.
        return qobject_cast<QMenu *>(d->autoItems.value(id).data());
    }

    void WidgetActionContext::addMenu(const QString &id, QMenu *menu) {
        Q_D(WidgetActionContext);
        d->setItem(id, ActionItem(ActionItem::Menu, menu));
    }

    QMenuBar *WidgetActionContext::menuBar(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        return it == d->items.end() ? nullptr : it->menuBar();
    }

    void WidgetActionContext::addMenuBar(const QString &id, QMenuBar *menuBar) {
        Q_D(WidgetActionContext);
        d->setItem(id, ActionItem(ActionItem::MenuBar, menuBar));
    }

    QToolBar *WidgetActionContext::toolBar(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        return it == d->items.end() ? nullptr : it->toolBar();
    }

    void WidgetActionContext::addToolBar(const QString &id, QToolBar *toolBar) {
        Q_D(WidgetActionContext);
        d->setItem(id, ActionItem(ActionItem::ToolBar, toolBar));
    }

    void WidgetActionContext::remove(const QString &id) {
        Q_D(WidgetActionContext);
        d->removeItem(id);
    }

    void WidgetActionContext::updateElement(ActionElement element) {
        Q_D(WidgetActionContext);
        switch (element) {
            case AE_Layouts:
                d->updateLayouts();
                break;
            case AE_Texts:
            case AE_Keymap:
            case AE_Icons:
                d->updateProperties(element);
                break;
        }
    }

    QAction *WidgetActionContext::createAction(const QString &id, QObject *parent) const {
        Q_UNUSED(id);
        return new QAction(parent);
    }

    QMenu *WidgetActionContext::createSubMenu(const QString &id, QWidget *parent) const {
        Q_UNUSED(id);
        return new QMenu(parent);
    }

}
