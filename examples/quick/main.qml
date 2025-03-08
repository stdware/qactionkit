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
        id: menuBar
        ActionInstantiator {
            actionId: "core.mainMenu"
            context: main.context
            onObjectAdded: (index, menu) => {
                if (!(menu instanceof Menu))
                    return
                menuBar.insertMenu(index, menu)
            }
            onObjectAboutToRemove: (index, menu) => {
                if (!(menu instanceof Menu))
                    return
                menuBar.removeMenu(menu)
            }
        }
    }

}