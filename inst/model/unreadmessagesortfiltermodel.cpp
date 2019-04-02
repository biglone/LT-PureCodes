#include "unreadmessagesortfiltermodel.h"
#include "unreadmsgitem.h"

UnreadMessageSortFilterModel::UnreadMessageSortFilterModel(QObject *parent /* = 0 */)
 : QSortFilterProxyModel(parent)
{

}

bool UnreadMessageSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
	if (!index.isValid())
	{
		return false;	
	}

	if (index.data(UnreadMsgItem::IgnoreBeforeRole).toBool())
	{
		return false;
	}

	if (index.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_Chat ||
		index.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_GroupChat ||
		index.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_DiscussChat)
	{
		return true;
	}

	return false;
}
