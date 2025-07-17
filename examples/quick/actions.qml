import QtQml
import QtQuick
import QtQuick.Controls.Basic

Item {
    required property QtObject context
    readonly property Component openFileAction: Action {
        onTriggered: console.log("open file")
    }
    readonly property Component saveFileAction: Action {
        onTriggered: console.log("save file")
    }
    readonly property Component aboutAppAction: Action {
        onTriggered: console.log("about app")
    }
    readonly property Component aboutQtAction: Action {
        onTriggered: console.log("about Qt")
    }
    readonly property Component showHelloAction: Action {
        onTriggered: console.log("show hello")
    }
    readonly property Component menu: Menu {}
    readonly property Component separator: MenuSeparator {}
    readonly property Component stretch: MenuSeparator {}

    Component.onCompleted: () => {
        context.addAction("core.openFile", openFileAction)
        context.addAction("core.saveFile", saveFileAction)
        context.addAction("core.aboutApp", aboutAppAction)
        context.addAction("core.aboutQt", aboutQtAction)
        context.addAction("plugin.showHello", showHelloAction)
        context.menuComponent = menu
        context.separatorComponent = separator
        context.stretchComponent = stretch
    }

}