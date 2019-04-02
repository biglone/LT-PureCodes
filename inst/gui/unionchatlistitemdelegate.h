#ifndef UNIONCHATLISTITEMDELEGATE_H
#define UNIONCHATLISTITEMDELEGATE_H

#include <QItemDelegate>

class UnionChatListItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	UnionChatListItemDelegate(QObject *parent);
	~UnionChatListItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // UNIONCHATLISTITEMDELEGATE_H
