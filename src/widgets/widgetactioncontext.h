#ifndef WIDGETACTIONCONTEXT_H
#define WIDGETACTIONCONTEXT_H

#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>

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

        enum Attribute {
            AttributeDisabled = 0x0,
            UpdateToolTipWithDescription = 0x1,
        };
        Q_DECLARE_FLAGS(Attributes, Attribute)

        Attributes attributes() const;
        void setAttribute(Attribute attr, bool on = true);
        void setAttributes(Attributes attrs);

    public:
        QAction *action(const QString &id) const;
        void addAction(const QString &id, QAction *action);
        void removeAction(const QString &id);

        QList<QWidget *> widgets(const QString &id) const;
        void addWidgetFactory(const QString &id, const std::function<QWidget *()> &fac);
        void removeWidgetFactory(const QString &id);

        QList<QMenu *> menus(const QString &id) const;
        void addMenu(const QString &id, QMenu *menu);
        void removeMenu(QMenu *menu);

        QList<QMenuBar *> menuBars(const QString &id) const;
        void addMenuBar(const QString &id, QMenuBar *menuBar);
        void removeMenuBar(QMenuBar *menuBar);

        QList<QToolBar *> toolBars(const QString &id) const;
        void addToolBar(const QString &id, QToolBar *toolBar);
        void removeToolBar(QToolBar *toolBar);

    Q_SIGNALS:
        void actionTriggered(const QString &id);
        void actionHovered(const QString &id);
        void actionToggled(const QString &id, bool checked);

    protected:
        void updateElement(ActionElement element) override;

    protected:
        virtual QAction *createAction(const QString &id, QObject *parent) const;
        virtual QMenu *createSubMenu(const QString &id, QWidget *parent) const;

        friend class ActionRegistry;
    };

}

#endif // WIDGETACTIONCONTEXT_H