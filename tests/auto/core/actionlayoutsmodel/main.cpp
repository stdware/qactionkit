#include <QtTest/QAbstractItemModelTester>
#include <QtTest/QtTest>

#include <QAKCore/actionlayoutsmodel.h>
#include <QAKCore/actionregistry.h>

#include "actions.qak.h"

using namespace QAK;

// The tree of the model, from actions.xml with t.main and t.tools as top-level nodes:
//
//     t.main:  t.file (t.open, t.save), t.edit (t.undo, t.recent (t.clear)), |, t.about
//     t.tools: t.open, t.file (t.open, t.save)
class Test : public QObject {
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr) : QObject(parent) {
    }

private:
    ActionRegistry *registry = nullptr;
    ActionLayoutsModel *model = nullptr;
    QAbstractItemModelTester *tester = nullptr;

    static ActionLayoutEntry action(const char *id) {
        return ActionLayoutEntry(QString::fromUtf8(id), ActionLayoutEntry::Action);
    }

    static ActionLayoutEntry menu(const char *id) {
        return ActionLayoutEntry(QString::fromUtf8(id), ActionLayoutEntry::Menu);
    }

    // Returns the index at the rows from the root
    QModelIndex at(std::initializer_list<int> rows) const {
        QModelIndex index;
        for (int row : rows) {
            index = model->index(row, 0, index);
        }
        return index;
    }

    // Returns the ids of the children of the container in the layouts of the model, with
    // separators shown as a bar
    QStringList childrenOf(const char *container) const {
        QStringList result;
        for (const auto &entry :
             model->actionLayouts().adjacencyMap().value(QString::fromUtf8(container))) {
            result.append(entry.type() == ActionLayoutEntry::Separator ? QStringLiteral("|")
                                                                       : entry.id());
        }
        return result;
    }

    // Returns the ids of the rows under index, as the model shows them
    QStringList rowsOf(const QModelIndex &index) const {
        QStringList result;
        for (int row = 0; row < model->rowCount(index); ++row) {
            const auto id = model->index(row, 0, index).data().toString();
            result.append(id.isEmpty() ? QStringLiteral("|") : id);
        }
        return result;
    }

private Q_SLOTS:
    void init() {
        registry = new ActionRegistry;
        registry->addExtension(testActions());
        model = new ActionLayoutsModel;
        model->setTopLevelNodes({menu("t.main"), menu("t.tools")});
        model->setActionLayouts(registry->defaultLayouts());
        tester = new QAbstractItemModelTester(
            model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void cleanup() {
        delete tester;
        delete model;
        delete registry;
    }

    void testStructure() {
        QCOMPARE(rowsOf({}), QStringList({"t.main", "t.tools"}));
        QCOMPARE(rowsOf(at({0})), QStringList({"t.file", "t.edit", "|", "t.about"}));
        QCOMPARE(rowsOf(at({0, 1, 1})), QStringList({"t.clear"}));
        // A menu held by two containers appears under both
        QCOMPARE(rowsOf(at({1, 1})), QStringList({"t.open", "t.save"}));
        QCOMPARE(model->parent(at({0, 1, 1})), at({0, 1}));
        QCOMPARE(at({0, 3}).data(Qt::UserRole).value<ActionLayoutEntry>(), action("t.about"));
    }

    void testTopLevelNodesAreReadOnly() {
        QVERIFY(!(model->flags(at({0})) & Qt::ItemIsEditable));
        QVERIFY(!(model->flags(at({0})) & Qt::ItemIsDragEnabled));
        QVERIFY(model->flags(at({0, 3})) & Qt::ItemIsEditable);
        QVERIFY(!model->setData(at({0}), QVariant::fromValue(menu("t.edit")), Qt::UserRole));
        QVERIFY(!model->removeRows(0, 1));
        QVERIFY(!model->insertRows(0, 1));
    }

    void testInsertRowsAddsSeparators() {
        QSignalSpy spy(model, &QAbstractItemModel::rowsInserted);
        QVERIFY(model->insertRows(1, 2, at({0, 1})));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(childrenOf("t.edit"), QStringList({"t.undo", "|", "|", "t.recent"}));
        QCOMPARE(rowsOf(at({0, 1})), childrenOf("t.edit"));

        // Only a menu or a group holds rows
        QVERIFY(!model->insertRows(0, 1, at({0, 3})));
    }

    void testRemoveRows() {
        QSignalSpy spy(model, &QAbstractItemModel::rowsRemoved);
        QVERIFY(model->removeRows(2, 2, at({0})));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(childrenOf("t.main"), QStringList({"t.file", "t.edit"}));
    }

    void testSetDataReplacesTheRowAndItsChildren() {
        QSignalSpy spy(model, &QAbstractItemModel::dataChanged);
        QVERIFY(model->setData(at({0, 1, 0}), QVariant::fromValue(action("t.save")), Qt::UserRole));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(childrenOf("t.edit"), QStringList({"t.save", "t.recent"}));
        QCOMPARE(at({0, 1, 0}).data().toString(), QStringLiteral("t.save"));

        // A menu brings its children
        QVERIFY(model->setData(at({0, 1, 0}), QVariant::fromValue(menu("t.recent")), Qt::UserRole));
        QCOMPARE(rowsOf(at({0, 1, 0})), QStringList({"t.clear"}));
    }

    void testSetDataOnARowNeverExpanded() {
        // The tester expands every row, which a view does not, so the tree is built again without
        // it
        delete tester;
        tester = nullptr;
        model->setActionLayouts(registry->defaultLayouts());

        const auto edit = at({0, 1});
        QVERIFY(model->setData(edit, QVariant::fromValue(menu("t.recent")), Qt::UserRole));
        QCOMPARE(rowsOf(edit), QStringList({"t.clear"}));
    }

    void testMoveWithinContainer() {
        QSignalSpy spy(model, &QAbstractItemModel::rowsMoved);
        QVERIFY(model->moveRows(at({0, 1}), 1, 1, at({0, 1}), 0));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(childrenOf("t.edit"), QStringList({"t.recent", "t.undo"}));
        QCOMPARE(rowsOf(at({0, 1})), childrenOf("t.edit"));
        QCOMPARE(rowsOf(at({0, 1, 0})), QStringList({"t.clear"}));
    }

    void testMoveAcrossContainers() {
        QSignalSpy spy(model, &QAbstractItemModel::rowsMoved);
        QVERIFY(model->moveRows(at({0}), 3, 1, at({0, 1}), 2));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(childrenOf("t.main"), QStringList({"t.file", "t.edit", "|"}));
        QCOMPARE(childrenOf("t.edit"), QStringList({"t.undo", "t.recent", "t.about"}));
        QCOMPARE(rowsOf(at({0, 1})), childrenOf("t.edit"));
        QCOMPARE(model->parent(at({0, 1, 2})), at({0, 1}));
    }

    void testEditingASharedMenuResets() {
        // Both places of the file menu are shown
        QCOMPARE(rowsOf(at({0, 0})), QStringList({"t.open", "t.save"}));
        QCOMPARE(rowsOf(at({1, 1})), QStringList({"t.open", "t.save"}));

        QSignalSpy spy(model, &QAbstractItemModel::modelReset);
        QVERIFY(model->insertRows(2, 1, at({0, 0})));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(rowsOf(at({0, 0})), QStringList({"t.open", "t.save", "|"}));
        QCOMPARE(rowsOf(at({1, 1})), QStringList({"t.open", "t.save", "|"}));
    }

    void testCyclesAreRejected() {
        // The recent menu is under the edit menu, which cannot go under it
        QVERIFY(
            !model->setData(at({0, 1, 1, 0}), QVariant::fromValue(menu("t.edit")), Qt::UserRole));
        QVERIFY(!model->moveRows(at({0}), 1, 1, at({0, 1, 1}), 0));
        QCOMPARE(childrenOf("t.recent"), QStringList({"t.clear"}));
    }

    void testCycleInTheLayouts() {
        // A menu that already encloses a row is shown there without its children
        QMap<QString, QVector<ActionLayoutEntry>> map;
        map[QStringLiteral("a")] = {menu("b")};
        map[QStringLiteral("b")] = {menu("a")};
        model->setTopLevelNodes({menu("a")});
        model->setActionLayouts(ActionLayouts(map));
        QCOMPARE(rowsOf(at({0, 0})), QStringList({"a"}));
        QCOMPARE(model->rowCount(at({0, 0, 0})), 0);
    }

    void testRegistryChecksTheForm() {
        const auto openAsMenu = QVariant::fromValue(menu("t.open"));
        const auto undeclared = QVariant::fromValue(action("t.missing"));

        // Without a registry, only the structure is checked
        QVERIFY(model->validateSetData(at({0, 3}), openAsMenu, Qt::UserRole));
        QVERIFY(model->validateSetData(at({0, 3}), undeclared, Qt::UserRole));

        model->setRegistry(registry);
        QVERIFY(!model->validateSetData(at({0, 3}), openAsMenu, Qt::UserRole));
        QVERIFY(!model->validateSetData(at({0, 3}), undeclared, Qt::UserRole));
        QVERIFY(model->validateSetData(at({0, 3}), QVariant::fromValue(action("t.open")),
                                       Qt::UserRole));
    }

    void testEditsBecomeLayoutChanges() {
        model->setRegistry(registry);
        QVERIFY(model->moveRows(at({0}), 3, 1, at({0, 1}), 0));
        QVERIFY(model->insertRows(0, 1, at({0, 0})));
        QVERIFY(model->removeRows(1, 1, at({1})));

        registry->setLayoutChanges(registry->computeLayoutChanges(model->actionLayouts()));
        QVERIFY(registry->layouts().adjacencyMap() == model->actionLayouts().adjacencyMap());
    }
};

QTEST_MAIN(Test)

#include "main.moc"
