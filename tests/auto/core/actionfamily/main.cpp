#include <set>

#include <QtTest/QtTest>

#include <QAKCore/actionfamily.h>

static std::set<QString> stringListToSet(const QStringList &list) {
    std::set<QString> set;
    for (const auto &str : list) {
        set.insert(str);
    }
    return set;
}

class Test : public QObject {
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr) : QObject(parent) {
    }

private:
private Q_SLOTS:
    void initTestCase() {
    }

    void init() {
    }

    void cleanup() {
    }

    void testShortcutsOverride() {
        QAK::ActionFamily family;

        QAK::ActionFamily::ShortcutsFamily shortcutsFamily{
            {"key1", QList<QKeySequence>({QKeySequence("Ctrl+A"), QKeySequence("Ctrl+B")})},
            {"key2", QList<QKeySequence>({QKeySequence("Ctrl+C"), QKeySequence("Ctrl+D")})},
            {"key3", std::nullopt                                                         },
            {"key4", QList<QKeySequence>()                                                },
        };
        family.setShortcutsFamily(shortcutsFamily);

        QCOMPARE(family.shortcuts("key1"),
                 QList<QKeySequence>({QKeySequence("Ctrl+A"), QKeySequence("Ctrl+B")}));
        QCOMPARE(family.shortcuts("key2"),
                 QList<QKeySequence>({QKeySequence("Ctrl+C"), QKeySequence("Ctrl+D")}));
        QCOMPARE(family.shortcuts("key3"), std::nullopt);
        QCOMPARE(family.shortcuts("key4"), QList<QKeySequence>());

        auto actualToJson = QAK::ActionFamily::shortcutsFamilyToJson(shortcutsFamily);
        auto expectedToJson = QJsonArray({
            QJsonObject({
                         {"id", "key1"},
                         {"keys", QJsonArray({"Ctrl+A", "Ctrl+B"})},
                         }
            ),
            QJsonObject({
                         {"id", "key2"},
                         {"keys", QJsonArray({"Ctrl+C", "Ctrl+D"})},
                         }
            ),
            QJsonObject({{"id", "key3"}, {"keys", QJsonValue::Null}}
            ),
            QJsonObject({{"id", "key4"}, {"keys", QJsonArray()}}
            ),
        });
        QCOMPARE(actualToJson, expectedToJson);

        auto actualFromJson = QAK::ActionFamily::shortcutsFamilyFromJson(actualToJson);
        QCOMPARE(actualFromJson, shortcutsFamily);
    }

    void testIconOverride() {
        QAK::ActionFamily family;

        QAK::ActionIcon icon1(QUrl("file:///icon1"));
        QAK::ActionIcon icon2(QUrl("file:///icon2"));

        QAK::ActionFamily::IconFamily iconFamily{
            {"icon1", icon1            },
            {"icon2", icon2            },
            {"icon3", std::nullopt     },
            {"icon4", QAK::ActionIcon()},
        };
        family.setIconFamily(iconFamily);

        QVERIFY(family.icon("icon1"));
        QCOMPARE(family.icon("icon1")->url(), QUrl("file:///icon1"));
        QVERIFY(family.icon("icon2"));
        QCOMPARE(family.icon("icon2")->url(), QUrl("file:///icon2"));
        QVERIFY(!family.icon("icon3"));
        QVERIFY(family.icon("icon4"));
        QCOMPARE(family.icon("icon4")->url(), QUrl(""));

        auto actualToJson = QAK::ActionFamily::iconFamilyToJson(iconFamily);
        auto expectedToJson = QJsonArray({
            QJsonObject({
                         {"id", "icon1"},
                         {"icon", icon1.toJson()},
                         }
            ),
            QJsonObject({
                         {"id", "icon2"},
                         {"icon", icon2.toJson()},
                         }
            ),
            QJsonObject({
                         {"id", "icon3"},
                         {"icon", QJsonValue::Null},
                         }
            ),
            QJsonObject({
                         {"id", "icon4"},
                         {"icon", QAK::ActionIcon().toJson()},
                         }
            ),
        });

        QCOMPARE(actualToJson, expectedToJson);

        auto actualFromJson = QAK::ActionFamily::iconFamilyFromJson(actualToJson);
        family.setIconFamily(actualFromJson);
        QVERIFY(family.icon("icon1"));
        QCOMPARE(family.icon("icon1")->url(), QUrl("file:///icon1"));
        QVERIFY(family.icon("icon2"));
        QCOMPARE(family.icon("icon2")->url(), QUrl("file:///icon2"));
        QVERIFY(!family.icon("icon3"));
        QVERIFY(family.icon("icon4"));
        QCOMPARE(family.icon("icon4")->url(), QUrl(""));
    }

    void testAddRemoveIcon() {
        QAK::ActionFamily family;
        family.addIcon("theme1", "theme1.icon2",
                       QAK::ActionIcon(QUrl("file:///path/to/theme1.icon2.first")));
        family.addIcon("theme1", "theme1.icon4",
                       QAK::ActionIcon(QUrl("file:///path/to/theme1.icon4.first")));
        family.addIconManifest(":/config.json");
        family.addIcon("theme2", "theme2.icon1",
                       QAK::ActionIcon(QUrl("file:///path/to/theme2.icon1.second")));
        family.addIcon("theme2", "theme2.icon2",
                       QAK::ActionIcon(QUrl("file:///path/to/theme2.icon2.second")));

        // check themes
        QCOMPARE(stringListToSet(family.iconThemes()),
                 stringListToSet(QStringList({"theme1", "theme2", "theme3"})));

        // check icon ids
        QCOMPARE(stringListToSet(family.iconIds("theme1")),
                 stringListToSet(QStringList(
                     {"theme1.icon1", "theme1.icon2", "theme1.icon3", "theme1.icon4"})));
        QCOMPARE(stringListToSet(family.iconIds("theme2")),
                 stringListToSet(QStringList({"theme2.icon1", "theme2.icon2"})));
        QCOMPARE(stringListToSet(family.iconIds("theme3")),
                 stringListToSet(QStringList({"theme3.icon1"})));

        // check icon data
        QCOMPARE(family.icon("theme1", "theme1.icon1").url(),
                 QUrl("file:///path/to/theme1.icon1.unchecked.enabled"));
        QCOMPARE(family.icon("theme1", "theme1.icon2").url(),
                 QUrl("file:///path/to/theme1.icon2.icon"));
        QCOMPARE(family.icon("theme1", "theme1.icon3").url(),
                 QUrl("file:///path/to/theme1.icon3.icon"));
        QCOMPARE(family.icon("theme1", "theme1.icon4").url(),
                 QUrl("file:///path/to/theme1.icon4.first"));
        QCOMPARE(family.icon("theme2", "theme2.icon1").url(),
                 QUrl("file:///path/to/theme2.icon1.second"));
        QCOMPARE(family.icon("theme2", "theme2.icon2").url(),
                 QUrl("file:///path/to/theme2.icon2.second"));
        QCOMPARE(family.icon("theme3", "theme3.icon1").url(),
                 QUrl("file:///path/to/theme3.icon1.icon"));

        // remove icon
        family.removeIcon("theme2", "theme2.icon1");
        family.removeIcon("theme2", "theme2.icon2");

        // check icon ids
        QCOMPARE(stringListToSet(family.iconIds("theme2")),
                 stringListToSet(QStringList({"theme2.icon1"})));

        // check icon data
        QCOMPARE(family.icon("theme2", "theme2.icon1").url(),
                 QUrl("file:///path/to/theme2.icon1.icon"));

        // remove icon config
        family.removeIconManifest(":/config.json");

        // check themes
        QCOMPARE(stringListToSet(family.iconThemes()), stringListToSet(QStringList({"theme1"})));

        // check icon ids
        QCOMPARE(stringListToSet(family.iconIds("theme1")),
                 stringListToSet(QStringList({"theme1.icon2", "theme1.icon4"})));
        QCOMPARE(family.icon("theme1", "theme1.icon2").url(),
                 QUrl("file:///path/to/theme1.icon2.first"));
        QCOMPARE(family.icon("theme1", "theme1.icon4").url(),
                 QUrl("file:///path/to/theme1.icon4.first"));
    }
};

QTEST_MAIN(Test)

#include "main.moc"
