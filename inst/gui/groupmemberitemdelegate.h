#ifndef GROUPMEMBERITEMDELEGATE_H
#define GROUPMEMBERITEMDELEGATE_H

#include <QItemDelegate>

class GroupItemListModel;

class GroupMemberItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit GroupMemberItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setGroupMemberModel(GroupItemListModel *groupMemberModel) {m_groupMemberModel = groupMemberModel;}

private:
	GroupItemListModel *m_groupMemberModel;
};

#endif // GROUPMEMBERITEMDELEGATE_H
