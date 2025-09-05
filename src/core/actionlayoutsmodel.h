#ifndef ACTIONLAYOUTSMODEL_H
#define ACTIONLAYOUTSMODEL_H

#include <QAbstractItemModel>

#include <QAKCore/qakglobal.h>
#include <QAKCore/actionregistry.h>

namespace QAK {

    class ActionLayoutsModelPrivate;

    class QAK_CORE_EXPORT ActionLayoutsModel : public QAbstractItemModel {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionLayoutsModel)

    public:
        explicit ActionLayoutsModel(QObject *parent = nullptr);
        ~ActionLayoutsModel() override;

        ActionLayouts actionLayouts() const;
        void setActionLayouts(const ActionLayouts &layouts);

        QVector<ActionLayoutEntry> topLevelNodes() const;
        void setTopLevelNodes(const QVector<ActionLayoutEntry> &nodes);

        // QAbstractItemModel interface
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &child) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

        // Editing interface
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                      const QModelIndex &destinationParent, int destinationChild) override;

        // Validation
        bool validateSetData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) const;

    private:
        QScopedPointer<ActionLayoutsModelPrivate> d_ptr;
    };

}

#endif // ACTIONLAYOUTSMODEL_H