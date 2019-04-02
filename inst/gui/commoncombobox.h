#ifndef COMMONCOMBOBOX_H
#define COMMONCOMBOBOX_H

#include <QComboBox>
#include <QItemDelegate>

class CommonComboBox : public QComboBox
{
	Q_OBJECT

public:
	CommonComboBox(QWidget *parent = 0);
	~CommonComboBox();
};

class ComboItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	ComboItemDelegate(QObject *parent);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // COMMONCOMBOBOX_H
