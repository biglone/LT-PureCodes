#include "addfriendlistview.h"
#include <QPainter>
#include <QFont>
#include <QBrush>
#include <QStandardItemModel>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AddFriendListView
AddFriendListView::AddFriendListView(QWidget *parent)
	: QListView(parent)
{
	setContextMenuPolicy(Qt::NoContextMenu);
	setEditTriggers(NoEditTriggers);

	setDragEnabled(false);
	setAcceptDrops(false);
	setDropIndicatorShown(false);

	setStyleSheet("QListView{"
		"border: none;"
		"background-color: transparent;"
		"}"
	);

	setItemDelegate(new AddFriendListItemDelegate(this));

	m_itemModel = new QStandardItemModel(this);
	setModel(m_itemModel);
}

AddFriendListView::~AddFriendListView()
{

}

void AddFriendListView::removeRow(int row)
{
	if (row < 0 || row >= m_itemModel->rowCount())
		return;

	QStandardItem *item = m_itemModel->takeRow(row).first();
	if (!item)
		return;

	setIndexWidget(item->index(), 0);
	delete item;
	item = 0;
}

void AddFriendListView::clear()
{
	for (int i = 0; i < m_itemModel->rowCount(); i++)
	{
		QStandardItem *item = m_itemModel->item(i);
		setIndexWidget(item->index(), 0);
	}
	m_itemModel->clear();
}

void AddFriendListView::paintEvent(QPaintEvent *event)
{
	if (this->model()->rowCount() > 0)
		return QListView::paintEvent(event);

	QPainter painter(this->viewport());
	QRect rt = rect();
	rt.setHeight(rt.height()/2);
	QFont originalFont = painter.font();
	QFont smallFont = originalFont;
	smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/5);
	painter.setFont(smallFont);
	painter.setPen(QColor(160, 160, 160));
	painter.setBrush(Qt::NoBrush);
	painter.drawText(rt, Qt::AlignHCenter|Qt::AlignVCenter, tr("No messages"));
	painter.setFont(originalFont);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AddFriendListItemDelegate
AddFriendListItemDelegate::AddFriendListItemDelegate(QObject *parent /*= 0*/) : QItemDelegate(parent)
{

}

QSize AddFriendListItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(30, 126);
}

void AddFriendListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
