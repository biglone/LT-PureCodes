#include "commoncombobox.h"
#include <QPainter>
#include <QListView>

CommonComboBox::CommonComboBox(QWidget *parent /*= 0*/)
	: QComboBox(parent)
{
	QListView *listView = new QListView(this);
	listView->setSelectionMode(QAbstractItemView::SingleSelection);
	listView->setFocusPolicy(Qt::NoFocus);
	listView->setMouseTracking(true);
	listView->setAttribute(Qt::WA_Hover, true);
	listView->setItemDelegate(new ComboItemDelegate(listView));
	this->setView(listView);
}

CommonComboBox::~CommonComboBox()
{

}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS ComboItemDelegate
ComboItemDelegate::ComboItemDelegate(QObject *parent) : QItemDelegate(parent)
{

}

QSize ComboItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
	return QSize(26, 26);
}

void ComboItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();

	QColor textColor;

	// draw background
	QRect paintRect = option.rect;
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, QColor("#3991D0"));
		textColor = QColor("#FFFFFF");
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, QColor("#FCF0CA"));
		textColor = QColor("#000000");
	}
	else
	{
		painter->fillRect(paintRect, QColor("#FFFFFF"));
		textColor = QColor("#000000");
	}

	QFontMetrics fontMetrics = option.fontMetrics;
	painter->setPen(textColor);

	// draw text
	QString text = index.data(Qt::DisplayRole).toString();
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 5);
	paintRect.setRight(paintRect.right() - 12);
	text = fontMetrics.elidedText(text, Qt::ElideRight, paintRect.width());
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, text);

	painter->restore();
}
