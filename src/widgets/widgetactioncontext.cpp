#include "widgetactioncontext.h"

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
            auto w = fac(parent);
            return w;
        }

        std::function<QWidget *(QWidget *)> fac;

        friend class ActionItem;
    };

    struct ActionItem {
        enum Type {
            Action,
            Menu,
            MenuBar,
            ToolBar,
            Widget,
        };
        Type t;
        QObject *o;
        bool owned;

        QAction *action() const {
            if (t == Action)
                return static_cast<QAction *>(o);
            return nullptr;
        }

        QMenu *menu() const {
            if (t == Menu)
                return static_cast<QMenu *>(o);
            return nullptr;
        }

        QMenuBar *menuBar() const {
            if (t == MenuBar)
                return static_cast<QMenuBar *>(o);
            return nullptr;
        }

        QToolBar *toolBar() const {
            if (t == ToolBar)
                return static_cast<QToolBar *>(o);
            return nullptr;
        }

        WidgetAction *widgetAction() const {
            if (t == Widget)
                return static_cast<WidgetAction *>(o);
            return nullptr;
        }

        ActionItem() : t(Type(0)), o(nullptr), owned(false) {
        }
        ActionItem(QAction *a) : t(Action), o(a), owned(false) {
        }
        ActionItem(QMenu *m, bool own = true) : t(Menu), o(m), owned(own) {
        }
        ActionItem(QMenuBar *mb) : t(MenuBar), o(mb), owned(false) {
        }
        ActionItem(QToolBar *tb) : t(ToolBar), o(tb), owned(false) {
        }
        ActionItem(std::function<QWidget *(QWidget *)> fac, const QString &id)
            : t(Widget), o(new WidgetAction(std::move(fac))), owned(true) {
        }

        ~ActionItem() {
            if (owned) {
                o->deleteLater();
            }
        }
    };

    class WidgetActionContextPrivate : public ActionContextPrivate {
        Q_DECLARE_PUBLIC(WidgetActionContext)
    public:
        WidgetActionContextPrivate() = default;
        ~WidgetActionContextPrivate() = default;

        WidgetActionContext::Attributes attrs;
        QMap<QString, ActionItem> items;

        void updateLayouts();
    };

    void WidgetActionContextPrivate::updateLayouts() {
        Q_Q(WidgetActionContext);

        auto reg = registry;
        if (!reg) {
            return;
        }

        const auto &layouts = reg->layouts();
        
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
        auto it = d->items.find(id);
        if (it == d->items.end()) {
            return nullptr;
        }
        return it->action();
    }

    void WidgetActionContext::addAction(const QString &id, QAction *action) {
        Q_D(WidgetActionContext);
        d->items[id] = ActionItem(action);
    }

    QList<QWidget *> WidgetActionContext::widgets(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        if (it == d->items.end()) {
            return {};
        }
        return it->widgetAction()->createdWidgets();
    }

    void WidgetActionContext::addWidgetFactory(const QString &id,
                                               std::function<QWidget *(QWidget *)> fac) {
        Q_D(WidgetActionContext);
        d->items[id] = ActionItem(std::move(fac), id);
    }

    QMenu *WidgetActionContext::menu(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        if (it == d->items.end()) {
            return nullptr;
        }
        return it->menu();
    }

    void WidgetActionContext::addMenu(const QString &id, QMenu *menu) {
        Q_D(WidgetActionContext);
        d->items[id] = ActionItem(menu, false);
    }

    QMenuBar *WidgetActionContext::menuBar(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        if (it == d->items.end()) {
            return nullptr;
        }
        return it->menuBar();
    }

    void WidgetActionContext::addMenuBar(const QString &id, QMenuBar *menuBar) {
        Q_D(WidgetActionContext);
        d->items[id] = ActionItem(menuBar);
    }

    QToolBar *WidgetActionContext::toolBar(const QString &id) const {
        Q_D(const WidgetActionContext);
        auto it = d->items.find(id);
        if (it == d->items.end()) {
            return nullptr;
        }
        return it->toolBar();
    }

    void WidgetActionContext::addToolBar(const QString &id, QToolBar *toolBar) {
        Q_D(WidgetActionContext);
        d->items[id] = ActionItem(toolBar);
    }

    void WidgetActionContext::remove(const QString &id) {
        Q_D(WidgetActionContext);
        d->items.remove(id);
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