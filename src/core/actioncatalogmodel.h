#ifndef ACTIONCATALOGMODEL_H
#define ACTIONCATALOGMODEL_H

#include <QAbstractItemModel>

#include <QAKCore/qakglobal.h>
#include <QAKCore/actionregistry.h>

namespace QAK {

    class ActionCatalogModelPrivate;

    class QAK_CORE_EXPORT ActionCatalogModel : public QAbstractItemModel {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionCatalogModel)

    public:
        explicit ActionCatalogModel(QObject *parent = nullptr);
        ~ActionCatalogModel() override;

        ActionCatalog catalog() const;
        void setCatalog(const ActionCatalog &catalog);

        // QAbstractItemModel interface
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &child) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    private:
        QScopedPointer<ActionCatalogModelPrivate> d_ptr;
    };

}

#endif // ACTIONCATALOGMODEL_H
