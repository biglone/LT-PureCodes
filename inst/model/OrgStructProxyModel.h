#ifndef ORGSTRUCTPROXYMODEL_H
#define ORGSTRUCTPROXYMODEL_H

#include <QSortFilterProxyModel>

class OrgStructProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit OrgStructProxyModel(QObject *parent = 0);


protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
	
};

#endif // ORGSTRUCTPROXYMODEL_H
