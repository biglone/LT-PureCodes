#include "unionchatlistview.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

UnionChatListView::UnionChatListView(QWidget *parent)
	: QListView(parent)
{
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	initDragMoveTimer();
}

UnionChatListView::~UnionChatListView()
{

}

void UnionChatListView::dragEnterEvent(QDragEnterEvent *e)
{
	const QMimeData *source = e->mimeData();
	if (source->hasImage() || source->hasText() || source->hasUrls())
	{
		m_dragMoveIndex = QModelIndex();
		e->acceptProposedAction();
		return;
	}

	e->ignore();
}

void UnionChatListView::dragMoveEvent(QDragMoveEvent *e)
{
	const QMimeData *source = e->mimeData();
	if (source->hasImage() || source->hasText() || source->hasUrls())
	{
		QModelIndex index = this->indexAt(e->pos());
		if (!index.isValid())
			index = this->currentIndex();

		if (index.isValid())
		{
			if (m_dragMoveIndex != index)
			{
				m_dragMoveIndex = index;
				startDragMoveTimer();
			}

			e->acceptProposedAction();
			return;
		}
	}

	e->ignore();
}

void UnionChatListView::dropEvent(QDropEvent *e)
{
	const QMimeData *source = e->mimeData();
	if (source->hasImage() || source->hasText() || source->hasUrls())
	{
		stopDragMoveTimer();

		QModelIndex index = this->indexAt(e->pos());
		if (!index.isValid())
			index = this->currentIndex();

		if (index.isValid())
		{
			emit dragMoveToIndex(index);
			emit insertMimeData(index, source);
			e->acceptProposedAction();
			return;
		}
	}

	e->ignore();
}

void UnionChatListView::mousePressEvent(QMouseEvent *e)
{
	QModelIndex index = this->indexAt(e->pos());
	if (!index.isValid())
	{
		QListView::mousePressEvent(e);
		return;
	}

	m_clickedIndex = index;
	QListView::mousePressEvent(e);
}

void UnionChatListView::mouseMoveEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
}

void UnionChatListView::mouseReleaseEvent(QMouseEvent *e)
{
	do {
		QModelIndex index = this->indexAt(e->pos());
		if (!index.isValid())
		{
			QListView::mouseReleaseEvent(e);
			break;
		}

		if (index == m_clickedIndex)
		{
			QListView::mouseReleaseEvent(e);
			break;
		}
	} while (0);

	m_clickedIndex = QModelIndex();
}

void UnionChatListView::onDragMoveTimeout()
{
	QPoint cursorPos = this->cursor().pos();
	cursorPos = this->mapFromGlobal(cursorPos);
	QModelIndex index = this->indexAt(cursorPos);
	if (index.isValid() && index == m_dragMoveIndex)
	{
		emit dragMoveToIndex(index);
	}
}

void UnionChatListView::initDragMoveTimer()
{
	m_dragMoveTimer.setSingleShot(true);
	m_dragMoveTimer.setInterval(500);
	connect(&m_dragMoveTimer, SIGNAL(timeout()), this, SLOT(onDragMoveTimeout()));
}

void UnionChatListView::startDragMoveTimer()
{
	m_dragMoveTimer.stop();
	m_dragMoveTimer.start();
}

void UnionChatListView::stopDragMoveTimer()
{
	m_dragMoveTimer.stop();
	m_dragMoveIndex = QModelIndex();
}