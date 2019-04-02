#include "blacklistitemdelegate.h"
#include <QPainter>
#include <QListView>

BlackListItemDelegate::BlackListItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{

}

BlackListItemDelegate::~BlackListItemDelegate()
{

}

QSize BlackListItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(36, 36);
}

void BlackListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(painter);

	QListView *view = static_cast<QListView *>(parent());
	if (!view)
	{
		return;
	}

	QWidget *w = view->indexWidget(index);
	if (w)
	{
		w->setGeometry(QRect(option.rect));
	}
}