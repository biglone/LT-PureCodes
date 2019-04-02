#ifndef COMBOUSERITEMDELEGATE_H
#define COMBOUSERITEMDELEGATE_H

#include <QItemDelegate>

class ComboUserItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	ComboUserItemDelegate(QObject *parent);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // COMBOUSERITEMDELEGATE_H
