#include "mainwindow.h"

#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionregistry.h>
#include <QAKWidgets/widgetactioncontext.h>

// Get the action extensions, must from the global namespace
static auto getCoreActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(core_actions);
}

static auto getPluginActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(plugin_actions);
}

class ClockWidget : public QLabel {
public:
    explicit ClockWidget(QWidget *parent = nullptr) : QLabel(parent) {
        startTimer(100);
        setAlignment(Qt::AlignCenter);
    }

    ~ClockWidget() override = default;

protected:
    void timerEvent(QTimerEvent *event) override {
        QLabel::timerEvent(event);
        setText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    }
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_toolBar = new QToolBar();

    auto centralWidget = new QWidget(this);
    auto layout = new QVBoxLayout(centralWidget);
    layout->addWidget(m_toolBar);
    layout->addWidget(new ClockWidget());
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    initActions();

    resize(800, 600);
}

MainWindow::~MainWindow() = default;

void MainWindow::initActions() {
    // Create action registry (Application scope)
    m_actionRegistry = new QAK::ActionRegistry(this);

    // Get core actions
    auto coreActionExtension = getCoreActionExtension();
    printf("core:\n    id=%s\n    hash=%s\n", qPrintable(coreActionExtension->id()),
           qPrintable(coreActionExtension->hash()));

    // Get plugin actions
    auto pluginActionExtension = getPluginActionExtension();
    printf("plugin:\n    id=%s\n    hash=%s\n", qPrintable(pluginActionExtension->id()),
           qPrintable(pluginActionExtension->hash()));

    // Register extensions
    m_actionRegistry->addExtension(coreActionExtension);
    m_actionRegistry->addExtension(pluginActionExtension);

    // Create action context (Window scope)
    m_actionContext = new QAK::WidgetActionContext(this);

    // Add action context to registry
    m_actionRegistry->addContext(m_actionContext);

    // Add important action/menu instances
    m_actionContext->addMenuBar(QStringLiteral("core.mainMenu"), menuBar());
    m_actionContext->addToolBar(QStringLiteral("core.mainToolBar"), m_toolBar);

    // Build actions
    m_actionRegistry->updateContextLayouts();
    m_actionRegistry->updateContextTexts();
    m_actionRegistry->updateContextKeymap();
    m_actionRegistry->updateContextIcons();
}