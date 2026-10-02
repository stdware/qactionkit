#include <QtTest/QtTest>

#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidgetAction>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionregistry.h>
#include <QAKWidgets/widgetactioncontext.h>

#include "actions.qak.h"

class Test : public QObject {
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr) : QObject(parent) {
    }

private:
    QAK::ActionRegistry *registry = nullptr;
    QAK::WidgetActionContext *context = nullptr;
    QMenuBar *menuBar = nullptr;
    QToolBar *toolBar = nullptr;

    // Returns the texts of the actions of the widget, with separators shown as a bar and widget
    // actions as an ellipsis.
    static QStringList contents(const QWidget *widget) {
        QStringList result;
        const auto actions = widget->actions();
        for (const auto &action : actions) {
            if (action->isSeparator()) {
                result.append(QStringLiteral("|"));
            } else if (qobject_cast<QWidgetAction *>(action)) {
                result.append(QStringLiteral("..."));
            } else {
                result.append(action->text());
            }
        }
        return result;
    }

    QMenu *subMenu(const QString &id) const {
        auto menu = context->menu(id);
        Q_ASSERT(menu);
        return menu;
    }

private Q_SLOTS:
    void init() {
        registry = new QAK::ActionRegistry;
        registry->setExtensions({testActions()});

        context = new QAK::WidgetActionContext;
        registry->addContext(context);

        menuBar = new QMenuBar;
        toolBar = new QToolBar;
        context->addMenuBar(QStringLiteral("test.mainMenu"), menuBar);
        context->addToolBar(QStringLiteral("test.mainToolBar"), toolBar);

        registry->updateContext(QAK::AE_Layouts);
    }

    void cleanup() {
        delete menuBar;
        menuBar = nullptr;
        delete toolBar;
        toolBar = nullptr;
        delete context;
        context = nullptr;
        delete registry;
        registry = nullptr;
    }

    void testMenuBar() {
        QCOMPARE(contents(menuBar), QStringList({"File", "Help"}));
    }

    void testSubMenusAreCreated() {
        QVERIFY(context->menu(QStringLiteral("test.file")));
        QVERIFY(context->menu(QStringLiteral("test.help")));
        QCOMPARE(menuBar->actions().at(0)->menu(), context->menu(QStringLiteral("test.file")));
    }

    void testGroupSeparatorsAreCollapsed() {
        // The group adds a separator on each side. The leading one is removed at the beginning of
        // the menu, and the trailing one merges with the explicit separator.
        QCOMPARE(contents(subMenu(QStringLiteral("test.file"))),
                 QStringList({"Open File", "Save File", "|", "Exit"}));
    }

    void testInsertionWithoutAnchorAppends() {
        QCOMPARE(contents(subMenu(QStringLiteral("test.help"))),
                 QStringList({"About", "Check Update"}));
    }

    // The menu shows the short text of an action, and the text remains for a command palette.
    void testShortTextInMenus() {
        QCOMPARE(context->action(QStringLiteral("test.checkUpdate"))->text(),
                 QStringLiteral("Check Update"));
        QCOMPARE(registry->actionInfo(QStringLiteral("test.checkUpdate"))->text().source,
                 QStringLiteral("Check for Updates"));
    }

    void testShortTextUsesFullTextAsToolTip() {
        auto action = context->action(QStringLiteral("test.checkUpdate"));
        QVERIFY(action);
        QCOMPARE(action->toolTip(), QStringLiteral("Check for Updates"));
    }

    void testToolBarStretch() {
        QCOMPARE(contents(toolBar), QStringList({"Open File", "...", "About"}));
    }

    void testToolTipFromDescription() {
        context->setAttribute(QAK::WidgetActionContext::UpdateToolTipWithDescription);
        registry->updateContext(QAK::AE_Texts);

        auto action = context->action(QStringLiteral("test.exit"));
        QVERIFY(action);
        QCOMPARE(action->toolTip(), QStringLiteral("Exit the application"));
    }

    void testShortcutsFromManifest() {
        auto action = context->action(QStringLiteral("test.openFile"));
        QVERIFY(action);
        QCOMPARE(action->shortcut(), QKeySequence(QStringLiteral("Ctrl+O")));
    }

    void testKeymapOverrideIsApplied() {
        registry->setShortcuts(QStringLiteral("test.openFile"),
                               QList<QKeySequence>{QKeySequence(QStringLiteral("Ctrl+Shift+O"))});
        registry->updateContext(QAK::AE_Keymap);

        auto action = context->action(QStringLiteral("test.openFile"));
        QVERIFY(action);
        QCOMPARE(action->shortcut(), QKeySequence(QStringLiteral("Ctrl+Shift+O")));
    }

    void testTriggeredSignalCarriesTheId() {
        QSignalSpy spy(context, &QAK::WidgetActionContext::actionTriggered);
        auto action = context->action(QStringLiteral("test.saveFile"));
        QVERIFY(action);
        action->trigger();

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("test.saveFile"));
    }

    void testRebuildDoesNotDuplicate() {
        const auto menuBarBefore = contents(menuBar);
        const auto fileBefore = contents(subMenu(QStringLiteral("test.file")));
        const auto toolBarBefore = contents(toolBar);

        registry->updateContext(QAK::AE_Layouts);
        registry->updateContext(QAK::AE_Layouts);

        QCOMPARE(contents(menuBar), menuBarBefore);
        QCOMPARE(contents(subMenu(QStringLiteral("test.file"))), fileBefore);
        QCOMPARE(contents(toolBar), toolBarBefore);
    }

    // Returns the change that removes the help menu from the menu bar
    static QAK::ActionLayoutChange removeHelpMenu() {
        QAK::ActionLayoutChange change;
        change.kind = QAK::ActionLayoutChange::Remove;
        change.container = QStringLiteral("test.mainMenu");
        change.entry =
            QAK::ActionLayoutEntry(QStringLiteral("test.help"), QAK::ActionLayoutEntry::Menu);
        return change;
    }

    void testLayoutChangeIsReflected() {
        registry->addLayoutChange(removeHelpMenu());
        registry->updateContext(QAK::AE_Layouts);

        QCOMPARE(contents(menuBar), QStringList({"File"}));
        // The menu is no longer referenced and must have been destroyed.
        QVERIFY(!context->menu(QStringLiteral("test.help")));

        // Without changes, the default layouts are restored
        registry->setLayoutChanges({});
        registry->updateContext(QAK::AE_Layouts);
        QCOMPARE(contents(menuBar), QStringList({"File", "Help"}));
    }

    void testLayoutChangesSurviveJsonRoundTrip() {
        const auto restored =
            QAK::ActionLayoutChange::fromJsonObject(removeHelpMenu().toJsonObject());
        QVERIFY(restored);
        registry->setLayoutChanges({*restored});
        registry->updateContext(QAK::AE_Layouts);

        QCOMPARE(contents(menuBar), QStringList({"File"}));
    }
};

QTEST_MAIN(Test)

#include "main.moc"
