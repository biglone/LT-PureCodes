#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class CSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CSortFilterProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // SORTFILTERPROXYMODEL_H
