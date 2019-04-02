#ifndef GROUPITEMDELEGATE_H
#define GROUPITEMDELEGATE_H

#include <QItemDelegate>

class GroupModel;
class FlickerHelper;

class GroupItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit GroupItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setGroupModel(GroupModel *groupModel) {m_groupModel = groupModel;}
	void setFlickerHelper(FlickerHelper *flickerHelper) {m_flickerHelper = flickerHelper;}

private:
	GroupModel *m_groupModel;
	FlickerHelper *m_flickerHelper;
};

#endif // GROUPITEMDELEGATE_H
