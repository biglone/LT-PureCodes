#ifndef UNREADMESSAGELISTVIEW_H
#define UNREADMESSAGELISTVIEW_H

#include <QListView>
#include <QItemDelegate>
#include <QModelIndex>

class QToolButton;

class UnreadMessageListView : public QListView
{
	Q_OBJECT

public:
	UnreadMessageListView(QWidget *parent);
	~UnreadMessageListView();
};

class UnreadMessageItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit UnreadMessageItemDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class UnreadMessageItemWidget : public QWidget
{
	Q_OBJECT

public:
	explicit UnreadMessageItemWidget(const QModelIndex &dataIndex, QWidget *parent = 0);

Q_SIGNALS:
	void ignoreItem(const QModelIndex &index);

protected:
	void paintEvent(QPaintEvent *e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void onIgnoreButtonClicked();

private:
	bool         m_isHover;
	QModelIndex  m_dataIndex;
	QToolButton *m_ignoreButton;
};

#endif // UNREADMESSAGELISTVIEW_H
