#include <QDebug>
#include "model/rosteritemdef.h"
#include "OrgStructProxyModel.h"

OrgStructProxyModel::OrgStructProxyModel( QObject *parent /*= 0*/ )
: QSortFilterProxyModel(parent)
{

}

bool OrgStructProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

	return (sourceModel()->data(index, RosterBaseItem::RosterIdRole).toString().contains(filterRegExp())
		|| sourceModel()->data(index, RosterBaseItem::RosterNameRole).toString().contains(filterRegExp()));
}

bool OrgStructProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
	bool bRet = false;

    int nLeftIndex = left.data(RosterBaseItem::RosterIndexRole).toInt();
	int nRightIndex = right.data(RosterBaseItem::RosterIndexRole).toInt();

	QString sLeft = left.data(RosterBaseItem::RosterIdRole).toString();
	QString sRight = right.data(RosterBaseItem::RosterIdRole).toString();

	bool bIsLeftGroup = (left.data(RosterBaseItem::RosterTypeRole) == RosterBaseItem::RosterTypeGroup ||
		left.data(RosterBaseItem::RosterTypeRole) == RosterBaseItem::RosterTypeGroupMuc);
	bool bIsRightGroup = (right.data(RosterBaseItem::RosterTypeRole) == RosterBaseItem::RosterTypeGroup ||
		right.data(RosterBaseItem::RosterTypeRole) == RosterBaseItem::RosterTypeGroupMuc);
	
	// all groups
	if (bIsLeftGroup && bIsRightGroup)
	{
		if (nLeftIndex < nRightIndex)
		{
			bRet = true;
		}
		else if (nLeftIndex == nRightIndex)
		{
			if (sLeft.compare(sRight) < 0)
				bRet = true;
		}
	}

	// all contact
	if (!bIsLeftGroup && !bIsRightGroup )
	{
		if (nLeftIndex < nRightIndex)
			bRet = true;
		else if (nLeftIndex == nRightIndex)
		{
			if (sLeft.compare(sRight) < 0)
				bRet = true;
		}
	}



	if (bIsLeftGroup && !bIsRightGroup)
	{
		bRet = false;
	}

	if (!bIsLeftGroup && bIsRightGroup)
	{
		bRet = true;
	}
	
    return bRet;
}
