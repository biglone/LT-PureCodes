#ifndef BLACKLISTITEMDELEGATE_H
#define BLACKLISTITEMDELEGATE_H

#include <QItemDelegate>

class BlackListItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	BlackListItemDelegate(QObject *parent);
	~BlackListItemDelegate();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // BLACKLISTITEMDELEGATE_H
