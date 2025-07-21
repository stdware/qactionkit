import QtQml
import QtQuick
import QtQuick.Controls.Basic

import QActionKit

ApplicationWindow {
    id: main
    width: 800
    height: 600
    visible: true

    required property QtObject context

    menuBar: MenuBar {
        MenuBarActionInstantiator {
            actionId: "core.mainMenu"
            context: main.context
        }
    }

}