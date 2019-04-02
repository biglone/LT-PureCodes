#include "combouseritemdelegate.h"
#include <QPainter>
#include <QListView>

ComboUserItemDelegate::ComboUserItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

QSize ComboUserItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(33, 33);
}

void ComboUserItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(painter);

	QListView *view = static_cast<QListView *>(parent());
	if (!view)
	{
		return;
	}

	QModelIndex sourceIndex = index;
	QWidget *w = view->indexWidget(sourceIndex);
	if (w)
	{
		w->setGeometry(QRect(option.rect));
	}

	/*
	// draw background
	QRect paintRect = option.rect;
	paintRect.adjust(1, 1, -1, -1);
	if (option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, QColor("#3991D0"));
	}
	else
	{
		painter->fillRect(paintRect, QColor("#FFFFFF"));
	}
	*/
}
