#ifndef SUBSCRIPTIONLASTMSGITEMDELEGATE_H
#define SUBSCRIPTIONLASTMSGITEMDELEGATE_H

#include <QItemDelegate>

class SubscriptionLastMsgModel;

class SubscriptionLastMsgItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	SubscriptionLastMsgItemDelegate(SubscriptionLastMsgModel *model, QObject *parent = 0);
	~SubscriptionLastMsgItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	SubscriptionLastMsgModel     *m_model;
};

#endif // SUBSCRIPTIONLASTMSGITEMDELEGATE_H
