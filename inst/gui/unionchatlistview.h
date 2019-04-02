#ifndef UNIONCHATLISTVIEW_H
#define UNIONCHATLISTVIEW_H

#include <QListView>
#include <QTimer>

class UnionChatListView : public QListView
{
	Q_OBJECT

public:
	UnionChatListView(QWidget *parent);
	~UnionChatListView();

Q_SIGNALS:
	void dragMoveToIndex(const QModelIndex &index);
	void insertMimeData(const QModelIndex &index, const QMimeData *source);

protected:
	void dragEnterEvent(QDragEnterEvent *e);
	void dragMoveEvent(QDragMoveEvent *e);
	void dropEvent(QDropEvent *e);

	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

private slots:
	void onDragMoveTimeout();

private:
	void initDragMoveTimer();
	void startDragMoveTimer();
	void stopDragMoveTimer();

private:
	QModelIndex m_dragMoveIndex;
	QTimer      m_dragMoveTimer;
	QModelIndex m_clickedIndex;
};

#endif // UNIONCHATLISTVIEW_H
