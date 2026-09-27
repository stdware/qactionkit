#include <QtTest/QtTest>

#include <QAKCore/actioncontext.h>
#include <QAKCore/actionregistry.h>

#include "actions.qak.h"

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

class Test : public QObject {
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr) : QObject(parent) {
    }

private:
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
        QCOMPARE(openFile->category(), QStringLiteral("File"));

        // The category is empty if the manifest does not specify one, whatever the id
        const auto saveFile = registry.actionInfo(QStringLiteral("test.file.saveFile"));
        QVERIFY(saveFile);
        QVERIFY(saveFile->category().isEmpty());
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
