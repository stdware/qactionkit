#include <QtTest/QtTest>

#include <QAKCore/actionregistry.h>

using namespace QAK;

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

    void testLayoutsJsonRoundTrip() {
        const auto layouts = sampleLayouts();
        const auto restored = ActionLayouts::fromJsonObject(layouts.toJsonObject());

        QCOMPARE(restored.hashList(), layouts.hashList());
        QCOMPARE(restored.adjacencyMap().keys(), layouts.adjacencyMap().keys());

        const auto expected = layouts.adjacencyMap().value(QStringLiteral("root"));
        const auto actual = restored.adjacencyMap().value(QStringLiteral("root"));
        QCOMPARE(actual.size(), expected.size());
        for (int i = 0; i < expected.size(); ++i) {
            QCOMPARE(actual[i].id(), expected[i].id());
            QCOMPARE(int(actual[i].type()), int(expected[i].type()));
        }

        const auto children = restored.adjacencyMap().value(QStringLiteral("menu1"));
        QCOMPARE(children.size(), 1);
        QCOMPARE(children.first().id(), QStringLiteral("action2"));
    }
};

QTEST_MAIN(Test)

#include "main.moc"
