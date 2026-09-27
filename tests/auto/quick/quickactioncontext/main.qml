import QtQuick
import QtQuick.Controls.Basic

import QActionKit

// Registers the components of the test with the context, and instantiates the layout of the file
// menu.
Item {
    id: root

    required property QtObject context
    readonly property QtObject instantiator: fileMenu

    readonly property Component action: Action {
        readonly property string actionDescription: ActionInstantiator.description
    }
    readonly property Component menu: Menu {}
    readonly property Component separator: MenuSeparator {}

    ActionInstantiator {
        id: fileMenu
        actionId: "test.file"
        context: root.context
    }

    Component.onCompleted: {
        for (const id of ["test.openFile", "test.saveFile", "test.exit"])
            context.addAction(id, action)
        context.menuComponent = menu
        context.separatorComponent = separator
    }
}
