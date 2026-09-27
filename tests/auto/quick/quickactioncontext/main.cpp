#include <QtQml/QQmlComponent>
#include <QtQml/QQmlEngine>
#include <QtTest/QtTest>

#include <QAKCore/actionregistry.h>
#include <QAKQuick/quickactioncontext.h>

#include "actions.qak.h"

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
    QQmlEngine *engine = nullptr;
    QAK::ActionRegistry *registry = nullptr;
    QAK::QuickActionContext *context = nullptr;
    QObject *root = nullptr;

    // Returns the object at the index among those that the instantiator of main.qml created.
    QObject *objectAt(int index) const {
        const auto instantiator = root->property("instantiator").value<QObject *>();
        QObject *object = nullptr;
        QMetaObject::invokeMethod(instantiator, "objectAt", Q_RETURN_ARG(QObject *, object),
                                  Q_ARG(int, index));
        return object;
    }

    // Returns the texts of the objects that the instantiator created, with separators shown as a
    // bar and menus as their titles.
    QStringList contents() const {
        const auto instantiator = root->property("instantiator").value<QObject *>();
        const int count = instantiator->property("count").toInt();
        QStringList result;
        for (int i = 0; i < count; ++i) {
            const auto object = objectAt(i);
            if (object->inherits("QQuickMenuSeparator")) {
                result.append(QStringLiteral("|"));
            } else if (object->inherits("QQuickMenu")) {
                result.append(object->property("title").toString());
            } else {
                result.append(object->property("text").toString());
            }
        }
        return result;
    }

private Q_SLOTS:
    void init() {
        engine = new QQmlEngine;
        engine->addImportPath(QStringLiteral(QAK_TEST_QML_IMPORT_PATH));

        registry = new QAK::ActionRegistry;
        registry->setExtensions({testActions()});
        context = new QAK::QuickActionContext;
        registry->addContext(context);

        QQmlComponent component(engine, QUrl::fromLocalFile(QFINDTESTDATA("main.qml")));
        root = component.createWithInitialProperties({
            {QStringLiteral("context"), QVariant::fromValue(context)}
        });
        QVERIFY2(root, qPrintable(component.errorString()));
    }

    void cleanup() {
        delete root;
        root = nullptr;
        delete context;
        context = nullptr;
        delete registry;
        registry = nullptr;
        delete engine;
        engine = nullptr;
    }

    void testLayout() {
        registry->updateContext(QAK::AE_Layouts);

        // The group adds a separator on each side. The leading one is removed at the beginning of
        // the menu, and the trailing one merges with the explicit separator.
        QCOMPARE(contents(), QStringList({"&Open File", "Save File", "|", "Recent Files", "Exit"}));
    }

    void testDescriptionIsAttached() {
        registry->updateContext(QAK::AE_Layouts);

        const auto openFile = objectAt(0);
        QVERIFY(openFile);
        QCOMPARE(openFile->property("actionDescription").toString(), QStringLiteral("Open a file"));
    }

    void testTranslatedTexts() {
        ContextTranslator translator;
        QCoreApplication::installTranslator(&translator);
        const auto guard = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });

        registry->updateContext(QAK::AE_Layouts);
        const auto texts = contents();
        QCOMPARE(texts.value(0), QStringLiteral("QActionKit::ActionText|&Open File"));
        QCOMPARE(texts.value(3), QStringLiteral("QActionKit::ActionText|Recent Files"));
    }

    void testTextsAreUpdated() {
        registry->updateContext(QAK::AE_Layouts);

        ContextTranslator translator;
        QCoreApplication::installTranslator(&translator);
        const auto guard = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });

        // Both the actions and the menus that were already created are updated
        registry->updateContext(QAK::AE_Texts);
        const auto texts = contents();
        QCOMPARE(texts.value(0), QStringLiteral("QActionKit::ActionText|&Open File"));
        QCOMPARE(texts.value(3), QStringLiteral("QActionKit::ActionText|Recent Files"));
    }

    void testKeymapOverrideIsApplied() {
        registry->updateContext(QAK::AE_Layouts);
        const auto openFile = objectAt(0);
        QVERIFY(openFile);
        QCOMPARE(openFile->property("shortcut").value<QKeySequence>(),
                 QKeySequence(QStringLiteral("Ctrl+O")));

        registry->setShortcuts(QStringLiteral("test.openFile"),
                               QList<QKeySequence>{QKeySequence(QStringLiteral("Ctrl+Shift+O"))});
        registry->updateContext(QAK::AE_Keymap);
        QCOMPARE(openFile->property("shortcut").value<QKeySequence>(),
                 QKeySequence(QStringLiteral("Ctrl+Shift+O")));
    }
};

QTEST_MAIN(Test)

#include "main.moc"
