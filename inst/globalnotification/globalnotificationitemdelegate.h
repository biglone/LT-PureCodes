#ifndef GLOBALNOTIFICATIONITEMDELEGATE_H
#define GLOBALNOTIFICATIONITEMDELEGATE_H

#include <QItemDelegate>

class SubscriptionModel;
class GlobalNotificationModel;

class GlobalNotificationItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	GlobalNotificationItemDelegate(GlobalNotificationModel *model, QObject *parent = 0);
	~GlobalNotificationItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	GlobalNotificationModel     *m_model;
};

#endif // GLOBALNOTIFICATIONITEMDELEGATE_H
