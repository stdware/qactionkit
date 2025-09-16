#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionregistry.h>
#include <QAKQuick/quickactioncontext.h>

using namespace QAK;

// Get the action extensions, must from the global namespace
static auto getCoreActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(core_actions);
}

static auto getPluginActionExtension() {
    return QAK_STATIC_ACTION_EXTENSION(plugin_actions);
}

int main(int argc, char *argv[]) {
    QGuiApplication a(argc, argv);

    ActionRegistry registry;
    auto coreActionExtension = getCoreActionExtension();
    auto pluginActionExtension = getPluginActionExtension();
    registry.setExtensions({coreActionExtension, pluginActionExtension});

    QuickActionContext context;
    registry.addContext(&context);

    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"context", QVariant::fromValue(&context)}});

    QQmlComponent actionsComponent(&engine, ":/qt/qml/QActionKit/Examples/Quick/actions.qml");
    auto o = actionsComponent.createWithInitialProperties({{"context", QVariant::fromValue(&context)}});

    engine.load(":/qt/qml/QActionKit/Examples/Quick/main.qml");

    registry.updateContext(QAK::AE_Layouts);

    return a.exec();

}
