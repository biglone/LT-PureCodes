#ifndef UNIONCHATLISTITEMWIDGET_H
#define UNIONCHATLISTITEMWIDGET_H

#include <QWidget>
namespace Ui {class UnionChatListItemWidget;};
class QStandardItem;
class QListView;

class UnionChatListItemWidget : public QWidget
{
	Q_OBJECT

public:
	UnionChatListItemWidget(QStandardItem *item, QListView *listView, QWidget *parent = 0);
	~UnionChatListItemWidget();

	void refresh();
	void setItem(QStandardItem *item);

Q_SIGNALS:
	void closeChat();

protected:
	void paintEvent(QPaintEvent *ev);
	void enterEvent(QEvent *ev);
	void leaveEvent(QEvent *ev);
	void resizeEvent(QResizeEvent *ev);

private:
	Ui::UnionChatListItemWidget *ui;
	QStandardItem               *m_item;
	QListView                   *m_listView;
	bool                         m_isHover;
};

#endif // UNIONCHATLISTITEMWIDGET_H
