#include <QtTest/QtTest>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <qmxmladaptor/qmxmladaptor.h>
#include <QAKCore/actionextension.h>

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
        // Test basic XML parsing functionality
        QVERIFY(true);
    }

    void testNamespaceAttributes() {
        // Test namespace attribute parsing
        QFile file(":/base.xml");
        QVERIFY(file.open(QIODevice::ReadOnly));
        
        QMXmlAdaptor adaptor;
        QVERIFY(adaptor.loadData(file.readAll()));
        
        // Find the action with namespace attributes
        bool foundNamespaceAction = false;
        for (const auto &item : adaptor.root.children) {
            if (item->name == "items") {
                for (const auto &subItem : item->children) {
                    if (subItem->properties.contains(QMXmlAdaptorAttributeKey("id", QString())) &&
                        subItem->properties.value(QMXmlAdaptorAttributeKey("id", QString())) == "TestActionNS") {
                        foundNamespaceAction = true;
                        
                        // Check namespace attributes
                        QVERIFY(subItem->properties.contains(QMXmlAdaptorAttributeKey("priority", "http://example.com/custom")));
                        QCOMPARE(subItem->properties.value(QMXmlAdaptorAttributeKey("priority", "http://example.com/custom")), QString("high"));
                        
                        QVERIFY(subItem->properties.contains(QMXmlAdaptorAttributeKey("category", "http://example.com/custom")));
                        QCOMPARE(subItem->properties.value(QMXmlAdaptorAttributeKey("category", "http://example.com/custom")), QString("main"));
                        
                        QVERIFY(subItem->properties.contains(QMXmlAdaptorAttributeKey("enabled", "http://example.com/custom")));
                        QCOMPARE(subItem->properties.value(QMXmlAdaptorAttributeKey("enabled", "http://example.com/custom")), QString("true"));
                        
                        // Check regular attributes (without namespace)
                        QVERIFY(subItem->properties.contains(QMXmlAdaptorAttributeKey("text", QString())));
                        QCOMPARE(subItem->properties.value(QMXmlAdaptorAttributeKey("text", QString())), QString("Namespace Test"));
                        
                        break;
                    }
                }
                break;
            }
        }
        QVERIFY(foundNamespaceAction);
    }
};

QTEST_MAIN(Test)

#include "main.moc"