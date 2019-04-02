#ifndef GLOBALNOTIFICATIONLASTMSGITEMDELEGATE_H
#define GLOBALNOTIFICATIONLASTMSGITEMDELEGATE_H

#include <QItemDelegate>

class GlobalNotificationLastMsgModel;

class GlobalNotificationLastMsgItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	GlobalNotificationLastMsgItemDelegate(GlobalNotificationLastMsgModel *model, QObject *parent = 0);
	~GlobalNotificationLastMsgItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	GlobalNotificationLastMsgModel     *m_model;
};

#endif // GLOBALNOTIFICATIONLASTMSGITEMDELEGATE_H
