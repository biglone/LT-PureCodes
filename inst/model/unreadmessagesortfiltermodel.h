#ifndef UNREADMESSAGESORTFILTERMODEL_H
#define UNREADMESSAGESORTFILTERMODEL_H

#include <QSortFilterProxyModel>

class UnreadMessageSortFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit UnreadMessageSortFilterModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

#endif // UNREADMESSAGESORTFILTERMODEL_H
