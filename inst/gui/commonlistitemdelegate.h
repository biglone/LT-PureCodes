#ifndef COMMONLISTITEMDELEGATE_H
#define COMMONLISTITEMDELEGATE_H

#include <QItemDelegate>

class CommonListItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	CommonListItemDelegate(QObject *parent);
	~CommonListItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // COMMONLISTITEMDELEGATE_H
