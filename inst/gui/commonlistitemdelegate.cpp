#include "commonlistitemdelegate.h"
#include <QPainter>

CommonListItemDelegate::CommonListItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{

}

CommonListItemDelegate::~CommonListItemDelegate()
{

}

QSize CommonListItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(32, 32);
}

void CommonListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();

	QColor textColor;

	// draw background
	QRect paintRect = option.rect;
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, QColor("#ffffff"));
		textColor = QColor(0, 120, 216);

		QRect selRect = paintRect;
		selRect.setWidth(4);
		painter->fillRect(selRect, QColor(0, 120, 216));
	}
	else if ((option.state & QStyle::State_MouseOver) && (option.state & QStyle::State_Enabled))
	{
		textColor = QColor(0, 120, 216);
	}
	else
	{
		textColor = QColor("#000000");
	}

	// draw text
	painter->setPen(textColor);
	QString text = index.data(Qt::DisplayRole).toString();
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 30);
	QFontMetrics fontMetrics = option.fontMetrics;
	text = fontMetrics.elidedText(text, Qt::ElideRight, paintRect.width());
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, text);

	painter->restore();
}
