#include "unionchatlistitemdelegate.h"
#include <QListView>

UnionChatListItemDelegate::UnionChatListItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{

}

UnionChatListItemDelegate::~UnionChatListItemDelegate()
{

}

QSize UnionChatListItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(48, 48);
}

void UnionChatListItemDelegate::paint(QPainter * /*painter*/, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
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
}
