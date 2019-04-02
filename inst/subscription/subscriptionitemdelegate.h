#ifndef SUBSCRIPTIONITEMDELEGATE_H
#define SUBSCRIPTIONITEMDELEGATE_H

#include <QItemDelegate>

class SubscriptionModel;

class SubscriptionItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	SubscriptionItemDelegate(SubscriptionModel *model, QObject *parent = 0);
	~SubscriptionItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	SubscriptionModel     *m_model;
};

#endif // SUBSCRIPTIONITEMDELEGATE_H
