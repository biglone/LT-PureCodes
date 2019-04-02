#ifndef ADDFRIENDLISTVIEW_H
#define ADDFRIENDLISTVIEW_H

#include <QListView>
#include <QItemDelegate>
#include "addfriendmanager.h"

class QStandardItemModel;

class AddFriendListView : public QListView
{
	Q_OBJECT

public:
	AddFriendListView(QWidget *parent);
	~AddFriendListView();

	void removeRow(int row);
	void clear();
	QStandardItemModel *itemModel() const {return m_itemModel;}

protected:
	void paintEvent(QPaintEvent *event);

private:
	QStandardItemModel *m_itemModel;
};

class AddFriendListItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit AddFriendListItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ADDFRIENDLISTVIEW_H
