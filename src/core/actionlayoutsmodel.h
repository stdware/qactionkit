#ifndef ACTIONLAYOUTSMODEL_H
#define ACTIONLAYOUTSMODEL_H

#include <QtCore/QAbstractItemModel>

#include <QAKCore/qakglobal.h>
#include <QAKCore/actionregistry.h>

namespace QAK {

    class ActionLayoutsModelPrivate;

    /// A tree of layouts for a settings page that edits them as a whole, after which
    /// \c ActionRegistry::computeLayoutChanges() turns the result into the changes of the user.
    ///
    /// The layouts are a directed acyclic graph, which the model shows as a tree from the
    /// top-level nodes: a menu that several containers hold appears once under each of them.
    /// Editing its children therefore changes every place where it appears, and the model then
    /// resets instead of reporting the rows that changed.
    ///
    /// The top-level nodes are read-only, and only a menu or a group holds children. With a
    /// registry, an entry must be declared and take a form that its declared type allows, as in
    /// a manifest. Without one, only the structure is checked: a separator or stretch has no id,
    /// any other entry has one, and no edit makes a cycle.
    class QAK_CORE_EXPORT ActionLayoutsModel : public QAbstractItemModel {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionLayoutsModel)

    public:
        explicit ActionLayoutsModel(QObject *parent = nullptr);
        ~ActionLayoutsModel();

        ActionLayouts actionLayouts() const;
        void setActionLayouts(const ActionLayouts &layouts);

        QVector<ActionLayoutEntry> topLevelNodes() const;
        void setTopLevelNodes(const QVector<ActionLayoutEntry> &nodes);

        /// Returns the registry that entries are checked against, or \c nullptr if none is set.
        ActionRegistry *registry() const;
        void setRegistry(ActionRegistry *registry);

        // QAbstractItemModel interface
        QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &child) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

        // Editing interface. The entry of a row is set with Qt::UserRole, and an inserted row
        // holds a separator until it is set.
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value,
                     int role = Qt::EditRole) override;
        bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                      const QModelIndex &destinationParent, int destinationChild) override;

        // Validation
        bool validateSetData(const QModelIndex &index, const QVariant &value,
                             int role = Qt::EditRole) const;

    private:
        QScopedPointer<ActionLayoutsModelPrivate> d_ptr;
    };

}

#endif // ACTIONLAYOUTSMODEL_H
