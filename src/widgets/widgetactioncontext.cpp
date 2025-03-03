#include "widgetactioncontext.h"

#include <variant>

#include <QAKCore/private/actioncontext_p.h>

namespace QAK {

    class WidgetActionContextPrivate : public ActionContextPrivate {
        Q_DECLARE_PUBLIC(WidgetActionContext)
    public:
        WidgetActionContextPrivate() = default;
        ~WidgetActionContextPrivate() = default;
    };

    WidgetActionContext::WidgetActionContext(QObject *parent)
        : ActionContext(*new WidgetActionContextPrivate(), parent) {
    }

    WidgetActionContext::~WidgetActionContext() = default;

    void WidgetActionContext::addAction(const QString &id, QAction *action) {
    }

    void WidgetActionContext::removeAction(const QString &id) {
    }

    void WidgetActionContext::addWidgetFactory(const QString &id,
                                               const std::function<QWidget *()> &fac) {
    }

    void WidgetActionContext::removeWidgetFactory(const QString &id) {
    }

    void WidgetActionContext::addMenu(const QString &id, QMenu *menu) {
    }

    void WidgetActionContext::removeMenu(QMenu *menu) {
    }

    void WidgetActionContext::addMenuBar(const QString &id, QMenuBar *menuBar) {
    }

    void WidgetActionContext::removeMenuBar(QMenuBar *menuBar) {
    }

    void WidgetActionContext::addToolBar(const QString &id, QToolBar *toolBar) {
    }

    void WidgetActionContext::removeToolBar(QMenuBar *toolBar) {
    }

    void WidgetActionContext::updateLayouts() {
        Q_D(WidgetActionContext);

        Q_ASSERT(d->registry);
    }

    void WidgetActionContext::updateTexts() {
    }

    void WidgetActionContext::updateKeymap() {
    }

    void WidgetActionContext::updateIcons() {
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