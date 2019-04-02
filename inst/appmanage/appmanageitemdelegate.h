#ifndef APPMANAGEITEMDELEGATE_H
#define APPMANAGEITEMDELEGATE_H

#include <QItemDelegate>

class AppManageItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	AppManageItemDelegate(QObject *parent = 0);
	~AppManageItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


#endif // APPMANAGEITEMDELEGATE_H
