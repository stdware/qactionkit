#include "mainwindow.h"

#include <QAKCore/actionextension.h>

// Get the action extensions, must from the global namespace
static const QAK::ActionExtension *getPluginActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(plugin_actions);
}

static const QAK::ActionExtension *getMainWindowActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(mainwindow_actions);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto pluginActionExtension = getPluginActionExtension();
    auto mainWindowActionExtension = getMainWindowActionExtension();
    qDebug() << "plugin: " << pluginActionExtension->id() << pluginActionExtension->hash();
    qDebug() << "mainWindow: " << mainWindowActionExtension->id()
             << mainWindowActionExtension->hash();
}

MainWindow::~MainWindow() = default;