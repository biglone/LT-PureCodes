#ifndef EDITFILTERITEMDELEGATE_H
#define EDITFILTERITEMDELEGATE_H

#include <QItemDelegate>

class EditFilterProxyModel;

class EditFilterItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	EditFilterItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	void drawGroup(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void drawItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // EDITFILTERITEMDELEGATE_H
