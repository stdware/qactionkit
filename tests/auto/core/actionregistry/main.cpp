#include <QtTest/QtTest>

#include <QAKCore/actioncontext.h>
#include <QAKCore/actionregistry.h>

#include "actions.qak.h"
#include "insertions.qak.h"
#include "plugin.qak.h"
#include "priority-a.qak.h"
#include "priority-b.qak.h"

using namespace QAK;

class TestContext : public ActionContext {
public:
    explicit TestContext(QObject *parent = nullptr) : ActionContext(parent) {
    }

    void updateElement(ActionElement element) override {
        updates.append(element);
    }

    QList<ActionElement> updates;
};

// Translates every string to its context and source text joined by a vertical bar, which shows the
// context in which a string was looked up.
class ContextTranslator : public QTranslator {
public:
    QString translate(const char *context, const char *sourceText, const char *disambiguation,
                      int n) const override {
        Q_UNUSED(disambiguation);
        Q_UNUSED(n);
        return QString::fromUtf8(context) + QLatin1Char('|') + QString::fromUtf8(sourceText);
    }

    bool isEmpty() const override {
        return false;
    }
};

class Test : public QObject {
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr) : QObject(parent) {
    }

private:
    static ActionLayoutChange layoutChange(ActionLayoutChange::Kind kind, const char *container,
                                           const ActionLayoutEntry &entry,
                                           ActionInsertion::Anchor anchor = ActionInsertion::Last,
                                           const char *relativeTo = "", int offset = 0,
                                           bool moved = false) {
        ActionLayoutChange change;
        change.kind = kind;
        change.container = QString::fromUtf8(container);
        change.entry = entry;
        change.anchor = anchor;
        change.relativeTo = QString::fromUtf8(relativeTo);
        change.offset = offset;
        change.moved = moved;
        return change;
    }

    static ActionLayoutEntry action(const char *id) {
        return ActionLayoutEntry(QString::fromUtf8(id), ActionLayoutEntry::Action);
    }

    static ActionLayoutEntry separator() {
        return ActionLayoutEntry({}, ActionLayoutEntry::Separator);
    }

    // Returns the ids of the children of the container, with separators shown as a bar
    static QStringList childrenOf(const ActionLayouts &layouts, const char *container) {
        QStringList result;
        for (const auto &entry : layouts.adjacencyMap().value(QString::fromUtf8(container))) {
            result.append(entry.type() == ActionLayoutEntry::Separator ? QStringLiteral("|")
                                                                       : entry.id());
        }
        return result;
    }

    static ActionLayouts sampleLayouts() {
        QMap<QString, QVector<ActionLayoutEntry>> map;
        map[QStringLiteral("root")] = {
            ActionLayoutEntry(QStringLiteral("action1"), ActionLayoutEntry::Action),
            ActionLayoutEntry({}, ActionLayoutEntry::Separator),
            ActionLayoutEntry(QStringLiteral("menu1"), ActionLayoutEntry::Menu),
            ActionLayoutEntry({}, ActionLayoutEntry::Stretch),
            ActionLayoutEntry(QStringLiteral("group1"), ActionLayoutEntry::Group),
        };
        map[QStringLiteral("menu1")] = {
            ActionLayoutEntry(QStringLiteral("action2"), ActionLayoutEntry::Action),
        };
        return ActionLayouts(map, {QStringLiteral("hash1"), QStringLiteral("hash2")});
    }

private Q_SLOTS:
    void initTestCase() {
    }

    void init() {
    }

    void cleanup() {
    }

    void testCatalogParent() {
        const ActionCatalog catalog(QVector<QPair<QString, QString>>({
            {QStringLiteral("menu1"),   QString()              },
            {QStringLiteral("action1"), QStringLiteral("menu1")},
        }));

        QVERIFY(catalog.parent(QStringLiteral("action1")));
        QCOMPARE(*catalog.parent(QStringLiteral("action1")), QStringLiteral("menu1"));

        // A top-level node has the empty id as its parent, which differs from an unknown id
        QVERIFY(catalog.parent(QStringLiteral("menu1")));
        QVERIFY(catalog.parent(QStringLiteral("menu1"))->isEmpty());
        QVERIFY(!catalog.parent(QStringLiteral("action2")));
    }

    void testUnknownActionInfo() {
        ActionRegistry registry;
        QVERIFY(!registry.actionInfo(QStringLiteral("action1")));
        QVERIFY(registry.actionShortcuts(QStringLiteral("action1")).isEmpty());
    }

    void testCategory() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        const auto openFile = registry.actionInfo(QStringLiteral("test.file.openFile"));
        QVERIFY(openFile);
        QCOMPARE(openFile->category().source, QStringLiteral("File"));

        // The category is empty if the manifest does not specify one, whatever the id
        const auto saveFile = registry.actionInfo(QStringLiteral("test.file.saveFile"));
        QVERIFY(saveFile);
        QVERIFY(saveFile->category().source.isEmpty());
    }

    void testExternal() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        const auto recentFiles = registry.actionInfo(QStringLiteral("test.file.recentFiles"));
        QVERIFY(recentFiles);
        QVERIFY(recentFiles->isExternal());

        const auto openFile = registry.actionInfo(QStringLiteral("test.file.openFile"));
        QVERIFY(openFile);
        QVERIFY(!openFile->isExternal());
    }

    void testCommand() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        // Only an action that is not external is a command
        const auto openFile = registry.actionInfo(QStringLiteral("test.file.openFile"));
        QVERIFY(openFile);
        QVERIFY(openFile->isCommand());

        const auto recentFiles = registry.actionInfo(QStringLiteral("test.file.recentFiles"));
        QVERIFY(recentFiles);
        QVERIFY(!recentFiles->isCommand());

        const auto file = registry.actionInfo(QStringLiteral("test.file"));
        QVERIFY(file);
        QVERIFY(!file->isCommand());
    }

    void testInsertionPriorities() {
        ActionRegistry registry;
        registry.setExtensions(
            {qak::test::testActions(), qak::test::testPriorityA(), qak::test::testPriorityB()});

        QStringList ids;
        for (const auto &entry :
             registry.layouts().adjacencyMap().value(QStringLiteral("test.file"))) {
            ids.append(entry.id());
        }

        // At each position, a smaller priority comes first, and the same priority follows the
        // order of registration and of the manifest, at the beginning and after a sibling as well
        QCOMPARE(ids, QStringList({
                          "b.first",
                          "a.first",
                          "test.file.openFile",
                          "a.after",
                          "b.after",
                          "test.file.saveFile",
                          "a.before",
                          "b.before",
                          "test.file.revert",
                          "b.early",
                          "a.last1",
                          "a.last2",
                          "b.last",
                          "a.late",
                      }));
    }

    void testSkippedInsertionsAreReported() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testInsertions());

        QTest::ignoreMessage(QtWarningMsg,
                             "Action extension \"org.qactionkit.test.insertions\" inserts into "
                             "\"test.missing\", which does not exist");
        QTest::ignoreMessage(QtWarningMsg,
                             "Action extension \"org.qactionkit.test.insertions\" inserts relative "
                             "to \"test.missing\", which \"test.plugin.menu\" does not contain");
        const auto layouts = registry.layouts();

        // Both insertions are skipped
        QVERIFY(layouts.adjacencyMap().value(QStringLiteral("test.plugin.menu")).isEmpty());
    }

    void testOwnerTakesPrecedenceOverPhony() {
        ActionRegistry registry;
        // The plugin is registered first, and the menu of the other extension still takes
        // precedence over its phony
        registry.setExtensions({qak::test::testPlugin(), qak::test::testActions()});

        QTest::ignoreMessage(QtWarningMsg,
                             "Action item \"test.file.openFile\" is declared by both "
                             "\"org.qactionkit.test.plugin\" and \"org.qactionkit.test.registry\", "
                             "and the first declaration is kept");
        const auto file = registry.actionInfo(QStringLiteral("test.file"));
        QVERIFY(file);
        QCOMPARE(file->type(), ActionItemInfo::Menu);
        QCOMPARE(file->text().source, QStringLiteral("File"));

        // Two declarations of other types conflict, and the first one is kept
        const auto openFile = registry.actionInfo(QStringLiteral("test.file.openFile"));
        QVERIFY(openFile);
        QCOMPARE(openFile->text().source, QStringLiteral("Plugin Open"));

        QCOMPARE(
            registry.catalog().parent(QStringLiteral("test.plugin.export")).value_or(QString()),
            QStringLiteral("test.file"));
    }

    void testPhonyAfterOwner() {
        ActionRegistry registry;
        registry.setExtensions({qak::test::testActions(), qak::test::testPlugin()});

        // The phony registered later neither replaces the menu nor conflicts with it
        QTest::failOnWarning(QRegularExpression(QStringLiteral("\"test\\.file\" is declared")));
        QTest::ignoreMessage(QtWarningMsg,
                             "Action item \"test.file.openFile\" is declared by both "
                             "\"org.qactionkit.test.registry\" and \"org.qactionkit.test.plugin\", "
                             "and the first declaration is kept");
        const auto file = registry.actionInfo(QStringLiteral("test.file"));
        QVERIFY(file);
        QCOMPARE(file->type(), ActionItemInfo::Menu);
        QCOMPARE(file->text().source, QStringLiteral("File"));
    }

    void testPhonyWithoutOwner() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testPlugin());

        // Without the extension that owns the node, the phony of the plugin supplies it
        const auto file = registry.actionInfo(QStringLiteral("test.file"));
        QVERIFY(file);
        QCOMPARE(file->type(), ActionItemInfo::Phony);
        QCOMPARE(file->text().source, QStringLiteral("Plugin File"));
    }

    void testAttributes() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        // A custom attribute is kept with its namespace, and the condition is not an attribute
        const auto close = registry.actionInfo(QStringLiteral("test.file.close"));
        QVERIFY(close);
        const auto attributes = close->attributes();
        QCOMPARE(attributes.size(), 1);
        QCOMPARE(attributes.value(ActionAttributeKey(QStringLiteral("tag"),
                                                     QStringLiteral("urn:qactionkit:test"))),
                 QStringLiteral("close"));
    }

    void testUntranslatedText() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        // Without a translation, the text is displayed as written
        const auto openFile = registry.actionInfo(QStringLiteral("test.file.openFile"));
        QVERIFY(openFile);
        const auto text = openFile->text();
        QCOMPARE(text.source, QStringLiteral("Open File"));
        QVERIFY(!text.translation);
        QCOMPARE(text.toString(), QStringLiteral("Open File"));

        // An empty string is never translated, even by a translator that translates every string
        ContextTranslator translator;
        QCoreApplication::installTranslator(&translator);
        const auto guard = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
        const auto saveFile = registry.actionInfo(QStringLiteral("test.file.saveFile"));
        QVERIFY(saveFile);
        QVERIFY(!saveFile->category().translation);
        QVERIFY(saveFile->category().toString().isEmpty());
    }

    void testWithoutMnemonic() {
        const auto strip = [](const QString &text) {
            return ActionText{text, std::nullopt}.withoutMnemonic();
        };
        QCOMPARE(strip(QStringLiteral("&Open File...")), QStringLiteral("Open File..."));
        QCOMPARE(strip(QStringLiteral("Save && Close")), QStringLiteral("Save & Close"));
        QCOMPARE(strip(QStringLiteral("Open")), QStringLiteral("Open"));

        // The translation is used if one exists, and a marker in parentheses is removed with the
        // spaces before it
        const ActionText text{QStringLiteral("&Open File..."),
                              QString::fromUtf8("\xE6\x89\x93\xE5\xBC\x80\xE6\x96\x87\xE4\xBB\xB6 "
                                                "(&O)...")};
        QCOMPARE(text.withoutMnemonic(),
                 QString::fromUtf8("\xE6\x89\x93\xE5\xBC\x80\xE6\x96\x87\xE4\xBB\xB6..."));
    }

    void testTranslationContexts() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        ContextTranslator translator;
        QCoreApplication::installTranslator(&translator);
        const auto guard = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });

        // The contexts of the configuration apply, and the built-in default where the
        // configuration specifies none
        const auto openFile = registry.actionInfo(QStringLiteral("test.file.openFile"));
        QVERIFY(openFile);
        QCOMPARE(openFile->text().toString(), QStringLiteral("Test::Text|Open File"));
        QCOMPARE(openFile->category().toString(),
                 QStringLiteral("QActionKit::ActionCategory|File"));

        // The contexts of the item take precedence
        const auto revert = registry.actionInfo(QStringLiteral("test.file.revert"));
        QVERIFY(revert);
        QCOMPARE(revert->text().toString(), QStringLiteral("Test::RevertText|Revert"));
        QCOMPARE(revert->category().toString(), QStringLiteral("Test::RevertCategory|File"));
        QCOMPARE(revert->description().toString(),
                 QStringLiteral("Test::Description|Revert the file"));

        // The source text is kept beside the translation
        QVERIFY(revert->text().translation);
        QCOMPARE(revert->text().source, QStringLiteral("Revert"));

        // The contexts are not attributes
        QVERIFY(openFile->attributes().isEmpty());
        QVERIFY(revert->attributes().isEmpty());
    }

    void testLayoutEntryIsNull() {
        // An entry that refers to an item is only usable with an id
        QVERIFY(ActionLayoutEntry().isNull());
        QVERIFY(ActionLayoutEntry({}, ActionLayoutEntry::Menu).isNull());
        QVERIFY(!ActionLayoutEntry(QStringLiteral("id"), ActionLayoutEntry::Action).isNull());
        QVERIFY(!ActionLayoutEntry(QStringLiteral("id"), ActionLayoutEntry::Menu).isNull());
        QVERIFY(!ActionLayoutEntry(QStringLiteral("id"), ActionLayoutEntry::Group).isNull());

        // Separators and stretches never carry an id
        QVERIFY(!ActionLayoutEntry({}, ActionLayoutEntry::Separator).isNull());
        QVERIFY(!ActionLayoutEntry({}, ActionLayoutEntry::Stretch).isNull());
    }

    void testLayoutEntryEquality() {
        const ActionLayoutEntry menu(QStringLiteral("id"), ActionLayoutEntry::Menu);
        QVERIFY(menu == ActionLayoutEntry(QStringLiteral("id"), ActionLayoutEntry::Menu));

        // Entries with the same id differ in their type
        QVERIFY(menu != ActionLayoutEntry(QStringLiteral("id"), ActionLayoutEntry::Group));
        QVERIFY(menu != ActionLayoutEntry(QStringLiteral("other"), ActionLayoutEntry::Menu));

        QVERIFY(ActionLayoutEntry({}, ActionLayoutEntry::Separator) ==
                ActionLayoutEntry({}, ActionLayoutEntry::Separator));
        QVERIFY(ActionLayoutEntry({}, ActionLayoutEntry::Separator) !=
                ActionLayoutEntry({}, ActionLayoutEntry::Stretch));
    }

    void testLayoutEntryTypeIsRegistered() {
        // The type property of the gadget is readable through the meta-object system
        const ActionLayoutEntry entry(QStringLiteral("id"), ActionLayoutEntry::Menu);
        const auto &metaObject = ActionLayoutEntry::staticMetaObject;
        const auto property = metaObject.property(metaObject.indexOfProperty("type"));
        QVERIFY(property.isEnumType());
        QCOMPARE(property.readOnGadget(&entry).toString(), QStringLiteral("Menu"));
    }

    void testLayoutsJsonRoundTrip() {
        const auto layouts = sampleLayouts();
        const auto restored = ActionLayouts::fromJsonObject(layouts.toJsonObject());

        QCOMPARE(restored.hashList(), layouts.hashList());
        QVERIFY(restored.adjacencyMap() == layouts.adjacencyMap());
    }

    void testReplayLayoutChanges() {
        using Change = ActionLayoutChange;
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());

        // Each step shows the file menu after it
        registry.setLayoutChanges({
            // openFile, revert
            layoutChange(Change::Remove, "test.file", action("test.file.saveFile")),
            // openFile, |, revert
            layoutChange(Change::Add, "test.file", separator(), ActionInsertion::After,
                         "test.file.openFile"),
            // openFile, |, close, revert
            layoutChange(Change::Add, "test.file", action("test.file.close"),
                         ActionInsertion::After, "test.file.openFile", 1),
            // openFile, |, close; and revert moves to the main menu
            layoutChange(Change::Remove, "test.file", action("test.file.revert"),
                         ActionInsertion::Last, "", 0, true),
            layoutChange(Change::Add, "test.mainMenu", action("test.file.revert"),
                         ActionInsertion::First, "", 0, true),
            // openFile, |, close, |
            layoutChange(Change::Add, "test.file", separator()),
            // openFile, |, close
            layoutChange(Change::Remove, "test.file", separator()),
            // openFile, |, |, close
            layoutChange(Change::Add, "test.file", separator(), ActionInsertion::Before,
                         "test.file.close"),
            // openFile, |, close
            layoutChange(Change::Remove, "test.file", separator(), ActionInsertion::Before,
                         "test.file.close", 1),
            // openFile, close
            layoutChange(Change::Remove, "test.file", separator(), ActionInsertion::After,
                         "test.file.openFile"),
        });

        const auto layouts = registry.layouts();
        QCOMPARE(childrenOf(layouts, "test.file"),
                 QStringList({"test.file.openFile", "test.file.close"}));
        QCOMPARE(childrenOf(layouts, "test.mainMenu"),
                 QStringList({"test.file.revert", "test.file"}));

        // The default layouts are unaffected
        QCOMPARE(childrenOf(registry.defaultLayouts(), "test.file"),
                 QStringList({"test.file.openFile", "test.file.saveFile", "test.file.revert"}));
    }

    void testComputedLayoutChanges_data() {
        using Edits = QMap<QString, QVector<ActionLayoutEntry>>;
        const auto openFile = action("test.file.openFile");
        const auto saveFile = action("test.file.saveFile");
        const auto revert = action("test.file.revert");
        const auto close = action("test.file.close");
        const auto undo = action("test.edit.undo");
        const auto redo = action("test.edit.redo");
        const auto file = ActionLayoutEntry(QStringLiteral("test.file"), ActionLayoutEntry::Menu);
        const auto sep = separator();

        // The containers that the user has edited, and the number of changes that belong to moves
        QTest::addColumn<Edits>("edits");
        QTest::addColumn<int>("moved");

        QTest::newRow("unchanged") << Edits() << 0;
        QTest::newRow("removal") << Edits({
                                        {"test.file", {openFile, revert}}
        })
                                 << 0;
        QTest::newRow("addition") << Edits({
                                         {"test.file", {openFile, close, saveFile, revert}}
        })
                                  << 0;
        QTest::newRow("reorder") << Edits({
                                        {"test.file", {revert, openFile, saveFile}}
        })
                                 << 2;
        QTest::newRow("move to another container") << Edits({
                                                          {"test.file",     {openFile, saveFile}},
                                                          {"test.mainMenu", {revert, file}      },
        })
                                                   << 2;
        QTest::newRow("moves across containers") << Edits({
                                                        {"test.file", {openFile, undo, saveFile}},
                                                        {"test.edit", {revert, sep, redo}       },
        })
                                                 << 4;
        QTest::newRow("form change")
            << Edits({
                   {"test.mainMenu",
                    {ActionLayoutEntry(QStringLiteral("test.file"), ActionLayoutEntry::Group)}},
        })
            << 2;
        QTest::newRow("separators added")
            << Edits({
                   {"test.file", {sep, openFile, sep, sep, saveFile, revert, sep}}
        })
            << 0;
        QTest::newRow("entry after a separator")
            << Edits({
                   {"test.file", {openFile, sep, close, saveFile, revert}}
        })
            << 0;
        QTest::newRow("separator removed") << Edits({
                                                  {"test.edit", {undo, redo}}
        })
                                           << 0;
        QTest::newRow("separator moved") << Edits({
                                                {"test.edit", {sep, undo, redo, sep}}
        })
                                         << 0;
    }

    void testComputedLayoutChanges() {
        using Edits = QMap<QString, QVector<ActionLayoutEntry>>;
        QFETCH(Edits, edits);
        QFETCH(int, moved);

        QTest::failOnWarning(QRegularExpression(QStringLiteral(".*")));
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());
        auto edited = registry.defaultLayouts().adjacencyMap();
        for (auto it = edits.begin(); it != edits.end(); ++it) {
            edited[it.key()] = it.value();
        }

        const auto changes = registry.computeLayoutChanges(ActionLayouts(edited, {}));
        QCOMPARE(changes.isEmpty(), edits.isEmpty());
        QCOMPARE(std::count_if(changes.begin(), changes.end(),
                               [](const ActionLayoutChange &change) { return change.moved; }),
                 moved);

        // Stored as JSON and replayed, the changes reproduce the edited layouts
        QVector<ActionLayoutChange> restored;
        for (const auto &change : changes) {
            const auto restoredChange = ActionLayoutChange::fromJsonObject(change.toJsonObject());
            QVERIFY(restoredChange);
            restored.append(*restoredChange);
        }
        registry.setLayoutChanges(restored);
        QVERIFY(registry.layouts().adjacencyMap() == edited);
    }

    void testMissingAnchorAppends() {
        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());
        registry.addLayoutChange(layoutChange(ActionLayoutChange::Add, "test.file",
                                              action("test.file.close"), ActionInsertion::After,
                                              "test.missing"));
        QCOMPARE(childrenOf(registry.layouts(), "test.file"),
                 QStringList({"test.file.openFile", "test.file.saveFile", "test.file.revert",
                              "test.file.close"}));
    }

    void testSkippedLayoutChanges_data() {
        using Change = ActionLayoutChange;
        QTest::addColumn<ActionLayoutChange>("change");
        QTest::addColumn<QString>("reason");

        QTest::newRow("undeclared container")
            << layoutChange(Change::Add, "test.missing", action("test.file.close"))
            << "the container is not declared";
        QTest::newRow("action as container")
            << layoutChange(Change::Add, "test.file.openFile", action("test.file.close"))
            << "the container is not a menu or a group";
        QTest::newRow("undeclared entry")
            << layoutChange(Change::Add, "test.file", action("test.missing"))
            << "the entry is not declared";
        QTest::newRow("action as menu")
            << layoutChange(Change::Add, "test.file",
                            ActionLayoutEntry("test.file.close", ActionLayoutEntry::Menu))
            << "the declared type of the entry does not allow its form";
        QTest::newRow("menu as action")
            << layoutChange(Change::Add, "test.mainMenu", action("test.file"))
            << "the declared type of the entry does not allow its form";
        QTest::newRow("removed entry not in the container")
            << layoutChange(Change::Remove, "test.file", action("test.file.close"))
            << "the entry is not in the container";
        QTest::newRow("removed separator not at the position")
            << layoutChange(Change::Remove, "test.file", separator(), ActionInsertion::After,
                            "test.file.openFile")
            << "the entry is not in the container";
        QTest::newRow("moved entry never removed")
            << layoutChange(Change::Add, "test.file", action("test.file.close"),
                            ActionInsertion::Last, "", 0, true)
            << "no removal of the move has applied before it";
    }

    void testSkippedLayoutChanges() {
        QFETCH(ActionLayoutChange, change);
        QFETCH(QString, reason);

        ActionRegistry registry;
        registry.addExtension(qak::test::testActions());
        const auto defaults = registry.defaultLayouts();

        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral("is skipped, because ") +
                                                QRegularExpression::escape(reason) + "$"));
        registry.addLayoutChange(change);
        QVERIFY(registry.layouts().adjacencyMap() == defaults.adjacencyMap());
    }

    void testLayoutChangeJsonRoundTrip() {
        ActionLayoutChange add;
        add.kind = ActionLayoutChange::Add;
        add.container = QStringLiteral("menu1");
        add.entry = ActionLayoutEntry(QStringLiteral("action1"), ActionLayoutEntry::Action);
        add.anchor = ActionInsertion::After;
        add.relativeTo = QStringLiteral("action2");
        add.offset = 2;
        add.moved = true;

        ActionLayoutChange remove;
        remove.kind = ActionLayoutChange::Remove;
        remove.container = QStringLiteral("menu1");
        remove.entry = ActionLayoutEntry({}, ActionLayoutEntry::Separator);
        remove.anchor = ActionInsertion::First;

        for (const auto &change : {add, remove}) {
            const auto restored = ActionLayoutChange::fromJsonObject(change.toJsonObject());
            QVERIFY(restored);
            QVERIFY(*restored == change);
        }
    }

    void testInvalidLayoutChangeJson_data() {
        QTest::addColumn<QString>("key");
        QTest::addColumn<QJsonValue>("value");

        QTest::newRow("unknown kind") << "kind" << QJsonValue("move");
        QTest::newRow("no container") << "container" << QJsonValue("");
        QTest::newRow("unknown type") << "entry"
                                      << QJsonValue(QJsonObject{
                                             {"id",   "action1"},
                                             {"type", "Button" }
        });
        QTest::newRow("item without id") << "entry"
                                         << QJsonValue(QJsonObject{
                                                {"id",   ""      },
                                                {"type", "Action"}
        });
        QTest::newRow("separator with id") << "entry"
                                           << QJsonValue(QJsonObject{
                                                  {"id",   "action1"  },
                                                  {"type", "Separator"}
        });
        QTest::newRow("unknown anchor") << "anchor" << QJsonValue("middle");
        QTest::newRow("after without sibling") << "relativeTo" << QJsonValue("");
        QTest::newRow("negative offset") << "offset" << QJsonValue(-1);
        QTest::newRow("fractional offset") << "offset" << QJsonValue(1.5);
    }

    void testInvalidLayoutChangeJson() {
        QFETCH(QString, key);
        QFETCH(QJsonValue, value);

        // A valid change with one field replaced
        ActionLayoutChange change;
        change.container = QStringLiteral("menu1");
        change.entry = ActionLayoutEntry(QStringLiteral("action1"), ActionLayoutEntry::Action);
        change.anchor = ActionInsertion::After;
        change.relativeTo = QStringLiteral("action2");
        auto obj = change.toJsonObject();
        QVERIFY(ActionLayoutChange::fromJsonObject(obj));

        obj.insert(key, value);
        QVERIFY(!ActionLayoutChange::fromJsonObject(obj));
    }

    void testContextRegistration() {
        ActionRegistry registry;
        auto context = new TestContext;

        registry.addContext(context);
        QCOMPARE(context->registry(), &registry);

        registry.updateContext(AE_Layouts);
        QCOMPARE(context->updates.size(), 1);
        QCOMPARE(int(context->updates.first()), int(AE_Layouts));

        // Adding the same context again must keep it registered
        registry.addContext(context);
        QCOMPARE(context->registry(), &registry);
        registry.updateContext(AE_Texts);
        QCOMPARE(context->updates.size(), 2);

        registry.removeContext(context);
        QCOMPARE(context->registry(), nullptr);
        registry.updateContext(AE_Icons);
        QCOMPARE(context->updates.size(), 2);

        delete context;
    }

    void testContextMovedBetweenRegistries() {
        ActionRegistry registry1;
        ActionRegistry registry2;
        auto context = new TestContext;

        registry1.addContext(context);
        registry2.addContext(context);
        QCOMPARE(context->registry(), &registry2);

        registry1.updateContext(AE_Layouts);
        QCOMPARE(context->updates.size(), 0);

        registry2.updateContext(AE_Layouts);
        QCOMPARE(context->updates.size(), 1);

        delete context;
    }

    void testContextDestroyedBeforeRegistry() {
        ActionRegistry registry;
        auto context = new TestContext;
        registry.addContext(context);

        delete context;
        // The registry must not touch the destroyed context
        registry.updateContext(AE_Layouts);
    }

    void testRegistryDestroyedBeforeContext() {
        TestContext context;
        {
            ActionRegistry registry;
            registry.addContext(&context);
            QCOMPARE(context.registry(), &registry);
        }
        // The context must not keep a dangling registry pointer
        QCOMPARE(context.registry(), nullptr);
    }
};

QTEST_MAIN(Test)

#include "main.moc"
