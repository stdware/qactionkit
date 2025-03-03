/****************************************************************************
** Action extension structure code from reading XML file 'sample-actions.xml'
**
** Created by: QActionKit Action Extension Compiler version 1.0.0.0 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/QString>
#include <QtCore/QCoreApplication>

#include <QAKCore/private/actionextension_p.h>

namespace qakStaticActionExtension_sample_actions {

using namespace QAK;

static ActionExtensionData *get_data() {
    static ActionExtensionData data;
    data.version = QStringLiteral("1.0");
    data.id = QStringLiteral("com.example.myEditor");
    data.hash = QStringLiteral("5a2e4c37d63f39e32178fd7ab6b34a962a22b6ca951b9f6b37329504b74a3e5c");

    static ActionItemInfoData staticItems[] = {
        {
            // index 0
            // id
            QStringLiteral("core.openFile"),
            // type
            ActionItemInfo::Action,
            // text
            QStringLiteral("Open File"),
            // actionClass
            QStringLiteral("File"),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral("core.openFile"),
            // shortcuts
            {
                QKeySequence(QStringLiteral("Ctrl+N")),
            },
            // catalog
            QStringLiteral("core.fileOpenActions"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 1
            // id
            QStringLiteral("core.saveFile"),
            // type
            ActionItemInfo::Action,
            // text
            QStringLiteral("Save File"),
            // actionClass
            QStringLiteral("File"),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral("core.saveFile"),
            // shortcuts
            {
                QKeySequence(QStringLiteral("Ctrl+S")),
            },
            // catalog
            QStringLiteral("core.file"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 2
            // id
            QStringLiteral("core.mainMenu"),
            // type
            ActionItemInfo::Menu,
            // text
            QStringLiteral("Main Menu"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral("core.mainMenu"),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("Others"),
            // topLevel
            true,
            // attributes
            {
            },
            // children
            {
                { QStringLiteral("core.file"), ActionLayoutEntry::Menu },
                { QStringLiteral("core.help"), ActionLayoutEntry::Menu },
            },
        },
        {
            // index 3
            // id
            QStringLiteral("core.plugins"),
            // type
            ActionItemInfo::Phony,
            // text
            QStringLiteral("Plugins"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral("core.plugins"),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral(""),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 4
            // id
            QStringLiteral("plugin.showHello"),
            // type
            ActionItemInfo::Action,
            // text
            QStringLiteral("Show Hello"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral("Show Hello Text"),
            // icon
            QStringLiteral("plugin.showHello"),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.plugins"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 5
            // id
            QStringLiteral("core.file"),
            // type
            ActionItemInfo::Menu,
            // text
            QStringLiteral("File"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.mainMenu"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
                { QStringLiteral("core.fileOpenActions"), ActionLayoutEntry::Group },
            },
        },
        {
            // index 6
            // id
            QStringLiteral("core.fileOpenActions"),
            // type
            ActionItemInfo::Group,
            // text
            QStringLiteral("File Open Actions"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.file"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
                { QStringLiteral("core.openFile"), ActionLayoutEntry::Action },
                { QStringLiteral("core.saveFile"), ActionLayoutEntry::Action },
            },
        },
        {
            // index 7
            // id
            QStringLiteral("core.help"),
            // type
            ActionItemInfo::Menu,
            // text
            QStringLiteral("Help"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.mainMenu"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
                { QStringLiteral("core.documentations"), ActionLayoutEntry::Action },
                { QStringLiteral(""), ActionLayoutEntry::Separator },
                { QStringLiteral("core.aboutApp"), ActionLayoutEntry::Action },
                { QStringLiteral("core.aboutQt"), ActionLayoutEntry::Action },
            },
        },
        {
            // index 8
            // id
            QStringLiteral("core.documentations"),
            // type
            ActionItemInfo::Action,
            // text
            QStringLiteral("Documentations"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.help"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 9
            // id
            QStringLiteral("core.aboutApp"),
            // type
            ActionItemInfo::Action,
            // text
            QStringLiteral("About App"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.help"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 10
            // id
            QStringLiteral("core.aboutQt"),
            // type
            ActionItemInfo::Action,
            // text
            QStringLiteral("About Qt"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("core.help"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
            },
        },
        {
            // index 11
            // id
            QStringLiteral("core.mainToolbar"),
            // type
            ActionItemInfo::Menu,
            // text
            QStringLiteral("Main Toolbar"),
            // actionClass
            QStringLiteral(""),
            // description
            QStringLiteral(""),
            // icon
            QStringLiteral(""),
            // shortcuts
            {
            },
            // catalog
            QStringLiteral("Others"),
            // topLevel
            false,
            // attributes
            {
            },
            // children
            {
                { QStringLiteral("core.file"), ActionLayoutEntry::Menu },
            },
        },
    };
    data.items = staticItems;
    data.itemCount = sizeof(staticItems) / sizeof(staticItems[0]);

    static ActionInsertionData staticInsertions[] = {
        {
            // index 0
            // type
            ActionInsertion::After,
            // target
            QStringLiteral("core.help"),
            // relativeTo
            QStringLiteral("core.documentations"),
            // items
            {
                { QStringLiteral("plugin.showHello"), ActionLayoutEntry::Action },
                { QStringLiteral(""), ActionLayoutEntry::Separator },
            },
        },
    };
    data.insertions = staticInsertions;
    data.insertionCount = sizeof(staticInsertions) / sizeof(staticInsertions[0]);

    return &data;
}

}

const QAK::ActionExtension *QT_MANGLE_NAMESPACE(qakGetStaticActionExtension_sample_actions)() {
    static QAK::ActionExtension extension{
        {
            qakStaticActionExtension_sample_actions::get_data(),
        },
    };
    return &extension;
}

#if 0
// This field is only used to generate translation files for the Qt linguist tool
static void qakStaticActionTranslations_sample_actions() {
    // Action Text
    QCoreApplication::translate("WHAT", "Open File");
    QCoreApplication::translate("WHAT", "Save File");
    QCoreApplication::translate("WHAT", "Main Menu");
    QCoreApplication::translate("WHAT", "Plugins");
    QCoreApplication::translate("WHAT", "Show Hello");
    QCoreApplication::translate("WHAT", "File");
    QCoreApplication::translate("WHAT", "File Open Actions");
    QCoreApplication::translate("WHAT", "Help");
    QCoreApplication::translate("WHAT", "Documentations");
    QCoreApplication::translate("WHAT", "About App");
    QCoreApplication::translate("WHAT", "About Qt");
    QCoreApplication::translate("WHAT", "Main Toolbar");

    // Action Class
    QCoreApplication::translate("Application::ActionClass", "File");
    QCoreApplication::translate("Application::ActionClass", "File");

    // Action Description
    QCoreApplication::translate("Application::ActionDescription", "Show Hello Text");

}
#endif

/*
    [Action]
    core.openFile
    core.saveFile
    plugin.showHello
    core.documentations
    core.aboutApp
    core.aboutQt

    [Group]
    core.fileOpenActions

    [Menu]
    core.mainMenu
    core.file
    core.help
    core.mainToolbar

    [Phony]
    core.plugins

*/
