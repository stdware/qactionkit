#include "actioncatalogmodel.h"

#include <QHash>

namespace QAK {

    class ActionCatalogModelPrivate {
        Q_DECLARE_PUBLIC(ActionCatalogModel)
    public:
        ActionCatalogModel *q_ptr;
        ActionCatalog catalog;
        
        // Cache for node ID storage to ensure pointer validity
        mutable QHash<QString, quintptr> nodeIdCache;
        mutable quintptr nextCacheId = 1;

        QString nodeIdFromIndex(const QModelIndex &index) const;
        QModelIndex indexFromNodeId(const QString &nodeId) const;
        int findRowInParent(const QString &nodeId, const QString &parentId) const;
        quintptr cacheNodeId(const QString &nodeId) const;
        QString nodeIdFromCache(quintptr cacheId) const;
        void clearCache();
    };

    QString ActionCatalogModelPrivate::nodeIdFromIndex(const QModelIndex &index) const {
        if (!index.isValid()) {
            return QString(); // Root (forest)
        }
        return nodeIdFromCache(index.internalId());
    }

    QModelIndex ActionCatalogModelPrivate::indexFromNodeId(const QString &nodeId) const {
        Q_Q(const ActionCatalogModel);
        
        if (nodeId.isEmpty()) {
            return QModelIndex(); // Root index
        }

        QString parentId = catalog.parent(nodeId);
        int row = findRowInParent(nodeId, parentId);
        
        if (row == -1) {
            return QModelIndex();
        }

        quintptr cacheId = cacheNodeId(nodeId);
        return q->createIndex(row, 0, cacheId);
    }

    int ActionCatalogModelPrivate::findRowInParent(const QString &nodeId, const QString &parentId) const {
        QStringList siblings = catalog.children(parentId);
        return siblings.indexOf(nodeId);
    }

    quintptr ActionCatalogModelPrivate::cacheNodeId(const QString &nodeId) const {
        auto it = nodeIdCache.find(nodeId);
        if (it != nodeIdCache.end()) {
            return it.value();
        }
        
        quintptr cacheId = nextCacheId++;
        nodeIdCache.insert(nodeId, cacheId);
        return cacheId;
    }

    QString ActionCatalogModelPrivate::nodeIdFromCache(quintptr cacheId) const {
        for (auto it = nodeIdCache.begin(); it != nodeIdCache.end(); ++it) {
            if (it.value() == cacheId) {
                return it.key();
            }
        }
        return QString();
    }

    void ActionCatalogModelPrivate::clearCache() {
        nodeIdCache.clear();
        nextCacheId = 1;
    }

    ActionCatalogModel::ActionCatalogModel(QObject *parent)
        : QAbstractItemModel(parent), d_ptr(new ActionCatalogModelPrivate) {
        Q_D(ActionCatalogModel);
        d->q_ptr = this;
    }

    ActionCatalogModel::~ActionCatalogModel() = default;

    ActionCatalog ActionCatalogModel::catalog() const {
        Q_D(const ActionCatalogModel);
        return d->catalog;
    }

    void ActionCatalogModel::setCatalog(const ActionCatalog &catalog) {
        Q_D(ActionCatalogModel);
        
        beginResetModel();
        d->catalog = catalog;
        d->clearCache();
        endResetModel();
    }

    QModelIndex ActionCatalogModel::index(int row, int column, const QModelIndex &parent) const {
        Q_D(const ActionCatalogModel);
        
        if (!hasIndex(row, column, parent)) {
            return QModelIndex();
        }

        QString parentId = d->nodeIdFromIndex(parent);
        QStringList children = d->catalog.children(parentId);
        
        if (row >= children.size()) {
            return QModelIndex();
        }

        QString childId = children.at(row);
        quintptr cacheId = d->cacheNodeId(childId);
        return createIndex(row, column, cacheId);
    }

    QModelIndex ActionCatalogModel::parent(const QModelIndex &child) const {
        Q_D(const ActionCatalogModel);
        
        if (!child.isValid()) {
            return QModelIndex();
        }

        QString childId = d->nodeIdFromIndex(child);
        QString parentId = d->catalog.parent(childId);
        
        if (parentId.isEmpty()) {
            return QModelIndex(); // Parent is root
        }

        return d->indexFromNodeId(parentId);
    }

    int ActionCatalogModel::rowCount(const QModelIndex &parent) const {
        Q_D(const ActionCatalogModel);
        
        QString parentId = d->nodeIdFromIndex(parent);
        return d->catalog.children(parentId).size();
    }

    int ActionCatalogModel::columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent)
        return 1;
    }

    QVariant ActionCatalogModel::data(const QModelIndex &index, int role) const {
        Q_D(const ActionCatalogModel);
        
        if (!index.isValid()) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            QString nodeId = d->nodeIdFromIndex(index);
            return nodeId;
        }

        return QVariant();
    }

} // QAK
