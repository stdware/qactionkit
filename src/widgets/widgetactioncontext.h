#ifndef WIDGETACTIONCONTEXT_H
#define WIDGETACTIONCONTEXT_H

#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidgetAction>

#include <QAKCore/actioncontext.h>
#include <QAKWidgets/qakwidgetsglobal.h>

namespace QAK {

    class WidgetActionContextPrivate;

    class QAK_WIDGETS_EXPORT WidgetActionContext : public ActionContext {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WidgetActionContext)
    public:
        explicit WidgetActionContext(QObject *parent = nullptr);
        ~WidgetActionContext();

        void addAction(const QString &id, QAction *action);
        void removeAction(const QString &id);

        void addWidgetFactory(const QString &id, const std::function<QWidget *()> &fac);
        void removeWidgetFactory(const QString &id);

        void addMenu(const QString &id, QMenu *menu);
        void removeMenu(QMenu *menu);

        void addMenuBar(const QString &id, QMenuBar *menuBar);
        void removeMenuBar(QMenuBar *menuBar);

        void addToolBar(const QString &id, QToolBar *toolBar);
        void removeToolBar(QMenuBar *toolBar);

    Q_SIGNALS:
        void actionTriggered(const QString &id);
        void actionHovered(const QString &id);
        void actionToggled(const QString &id, bool checked);

    protected:
        void updateLayouts() override;
        void updateTexts() override;
        void updateKeymap() override;
        void updateIcons() override;

    protected:
        virtual QAction *createAction(const QString &id, QObject *parent) const;
        virtual QMenu *createSubMenu(const QString &id, QWidget *parent) const;

        friend class ActionRegistry;
    };

}

#endif // WIDGETACTIONCONTEXT_H