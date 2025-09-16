#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolBar>

namespace QAK {
    class ActionRegistry;
    class WidgetActionContext;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initActions();

    QToolBar *m_toolBar;

    QAK::ActionRegistry *m_actionRegistry;
    QAK::WidgetActionContext *m_actionContext;
};

#endif // MAINWINDOW_H
