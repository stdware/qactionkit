#include "mainwindow.h"

#include <QAKCore/actionextension.h>

// Get the action extensions, must from the global namespace
static auto getCoreActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(core_actions);
}

static auto getPluginActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(plugin_actions);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto coreActionExtension = getCoreActionExtension();
    auto pluginActionExtension = getPluginActionExtension();
    qDebug() << "core: " << coreActionExtension->id() << coreActionExtension->hash();
    qDebug() << "plugin: " << pluginActionExtension->id() << pluginActionExtension->hash();
}

MainWindow::~MainWindow() = default;