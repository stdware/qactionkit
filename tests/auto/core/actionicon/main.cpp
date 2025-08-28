#include <QtTest/QtTest>

#include <QAKCore/actionicon.h>

static QJsonObject sizeToJson(const QSize &size) {
    QJsonObject obj;
    obj.insert("width", size.width());
    obj.insert("height", size.height());
    return obj;
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

    void test() {
        static QPair<QJsonValue, QAK::ActionIcon> testData[] = {
            // 1: string
            {
             "1",    []() {
                    QAK::ActionIcon icon;
                    icon.addFile("1");
                    return icon;
                }(),
             },
            // 2: path-size object
            {
             []() {
                    QJsonObject obj;
                    obj.insert("path", "2");
                    obj.insert("size", sizeToJson(QSize(2, 2)));
                    obj.insert("currentColor", "c2");
                    return obj;
                }(),
             []() {
                    QAK::ActionIcon icon;
                    icon.addFile("2", QSize(2, 2));
                    icon.setCurrentColor("c2");
                    return icon;
                }(),
             },
            // 3: unchecked object > string
            {
             []() {
                    QJsonObject obj;
                    obj.insert("unchecked", "2.5");
                    obj.insert("currentColor", "c2.5");
                    return obj;
                }(),
             []() {
                    QAK::ActionIcon icon;
                    icon.addFile("2.5");
                    icon.setCurrentColor("c2.5");
                    return icon;
                }(),
             },
            // 4. unchecked object > path-size object
            {
             []() {
                    QJsonObject obj;
                    {
                        QJsonObject uncheckedObj;
                        uncheckedObj.insert("path", "3");
                        uncheckedObj.insert("size", sizeToJson(QSize(3, 3)));
                        obj.insert("unchecked", uncheckedObj);
                    }
                    obj.insert("currentColor", "c3");
                    return obj;
                }(),
             []() {
                    QAK::ActionIcon icon;
                    icon.addFile("3", QSize(3, 3), true, false);
                    icon.setCurrentColor("c3");
                    return icon;
                }(),
             },
            // 5. unchecked object > enabled object + disabled object
            {
             []() {
                    QJsonObject obj;
                    {
                        QJsonObject uncheckedObj;
                        {
                            QJsonObject enabledObj;
                            enabledObj.insert("path", "4");
                            enabledObj.insert("size", sizeToJson(QSize(4, 4)));
                            uncheckedObj.insert("enabled", enabledObj);
                        }
                        {
                            QJsonObject disabledObj;
                            disabledObj.insert("path", "5");
                            disabledObj.insert("size", sizeToJson(QSize(5, 5)));
                            uncheckedObj.insert("disabled", disabledObj);
                        }
                        obj.insert("unchecked", uncheckedObj);
                    }
                    obj.insert("currentColor", "c4");
                    return obj;
                }(),
             []() {
                    QAK::ActionIcon icon;
                    icon.addFile("4", QSize(4, 4), true, false);
                    icon.addFile("5", QSize(5, 5), false, false);
                    icon.setCurrentColor("c4");
                    return icon;
                }(),
             },
            // 6. unchecked object + checked object > enabled object + disabled object
            {
             []() {
                    QJsonObject obj;
                    {
                        QJsonObject uncheckedObj;
                        {
                            QJsonObject enabledObj;
                            enabledObj.insert("path", "6");
                            enabledObj.insert("size", sizeToJson(QSize(6, 6)));
                            uncheckedObj.insert("enabled", enabledObj);
                        }
                        {
                            QJsonObject disabledObj;
                            disabledObj.insert("path", "7");
                            disabledObj.insert("size", sizeToJson(QSize(7, 7)));
                            uncheckedObj.insert("disabled", disabledObj);
                        }
                        obj.insert("unchecked", uncheckedObj);
                    }
                    {
                        QJsonObject checkedObj;
                        {
                            QJsonObject enabledObj;
                            enabledObj.insert("path", "8");
                            enabledObj.insert("size", sizeToJson(QSize(8, 8)));
                            checkedObj.insert("enabled", enabledObj);
                        }
                        {
                            QJsonObject disabledObj;
                            disabledObj.insert("path", "9");
                            disabledObj.insert("size", sizeToJson(QSize(9, 9)));
                            checkedObj.insert("disabled", disabledObj);
                        }
                        obj.insert("checked", checkedObj);
                    }
                    obj.insert("currentColor", "c5");
                    return obj;
                }(),
             []() {
                    QAK::ActionIcon icon;
                    icon.addFile("6", QSize(6, 6), true, false);
                    icon.addFile("7", QSize(7, 7), false, false);
                    icon.addFile("8", QSize(8, 8), true, true);
                    icon.addFile("9", QSize(9, 9), false, true);
                    icon.setCurrentColor("c5");
                    return icon;
                }(),
             },
        };

        for (auto &data : testData) {
            QJsonValue json = data.first;
            QAK::ActionIcon expected = data.second;
            QAK::ActionIcon actual = QAK::ActionIcon::fromJson(json);
            QCOMPARE(actual.filePath(true, false), expected.filePath(true, false));
            QCOMPARE(actual.filePath(false, false), expected.filePath(false, false));
            QCOMPARE(actual.filePath(true, true), expected.filePath(true, true));
            QCOMPARE(actual.filePath(false, true), expected.filePath(false, true));

            QCOMPARE(actual.size(true, false), expected.size(true, false));
            QCOMPARE(actual.size(false, false), expected.size(false, false));
            QCOMPARE(actual.size(true, true), expected.size(true, true));
            QCOMPARE(actual.size(false, true), expected.size(false, true));

            QCOMPARE(actual.currentColor(), expected.currentColor());

            QCOMPARE(actual.toJson(), expected.toJson());
        }
    }
};

QTEST_MAIN(Test)

#include "main.moc"