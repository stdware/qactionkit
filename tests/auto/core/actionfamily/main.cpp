#include <QtTest/QtTest>

#include <QAKCore/actionfamily.h>

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
    }
};

QTEST_MAIN(Test)

#include "main.moc"