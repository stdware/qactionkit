#include "widgetactioncontext.h"

#include <QAKCore/private/actioncontext_p.h>

namespace QAK {

    class WidgetActionContextPrivate : public ActionContextPrivate {
        Q_DECLARE_PUBLIC(WidgetActionContext)
    public:
        WidgetActionContextPrivate() = default;
        ~WidgetActionContextPrivate() = default;

        WidgetActionContext::Attributes attrs;
    };

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
        return {};
    }

    void WidgetActionContext::addAction(const QString &id, QAction *action) {
    }

    void WidgetActionContext::removeAction(const QString &id) {
    }

    QList<QWidget *> WidgetActionContext::widgets(const QString &id) const {
        return {};
    }

    void WidgetActionContext::addWidgetFactory(const QString &id,
                                               const std::function<QWidget *()> &fac) {
    }

    void WidgetActionContext::removeWidgetFactory(const QString &id) {
    }

    QList<QMenu *> WidgetActionContext::menus(const QString &id) const {
        return {};
    }

    void WidgetActionContext::addMenu(const QString &id, QMenu *menu) {
    }

    void WidgetActionContext::removeMenu(QMenu *menu) {
    }

    QList<QMenuBar *> WidgetActionContext::menuBars(const QString &id) const {
        return {};
    }

    void WidgetActionContext::addMenuBar(const QString &id, QMenuBar *menuBar) {
    }

    void WidgetActionContext::removeMenuBar(QMenuBar *menuBar) {
    }

    QList<QToolBar *> WidgetActionContext::toolBars(const QString &id) const {
        return {};
    }

    void WidgetActionContext::addToolBar(const QString &id, QToolBar *toolBar) {
    }

    void WidgetActionContext::removeToolBar(QToolBar *toolBar) {
    }

    void WidgetActionContext::updateElement(ActionElement element) {
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