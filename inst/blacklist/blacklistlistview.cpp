#include "blacklistlistview.h"
#include "blacklistmodel.h"
#include "blacklistitemdelegate.h"
#include "blacklistitemwidget.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QPainter>

BlackListListView::BlackListListView(QWidget *parent)
	: QListView(parent)
{
	BlackListItemDelegate *itemDelegate = new BlackListItemDelegate(this);
	setItemDelegate(itemDelegate);

	setContextMenuPolicy(Qt::NoContextMenu);
	setEditTriggers(NoEditTriggers);

	setDragEnabled(false);
	setAcceptDrops(false);
	setDropIndicatorShown(false);

	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));
}

BlackListListView::~BlackListListView()
{

}

void BlackListListView::setBlackListModel(BlackListModel *model)
{
	if (!model)
		return;

	m_blackListModel = model;
	setModel(m_blackListModel);
	onBlackListChanged();

	connect(m_blackListModel, SIGNAL(blackListChanged()), this, SLOT(onBlackListChanged()), Qt::UniqueConnection);
}

BlackListModel *BlackListListView::blackListModel() const
{
	return m_blackListModel;
}

void BlackListListView::paintEvent(QPaintEvent *event)
{
	if (m_blackListModel && m_blackListModel->rowCount() > 0)
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
	painter.drawText(rt, Qt::AlignHCenter|Qt::AlignVCenter, tr("No one blocked"));
	painter.setFont(originalFont);
}

void BlackListListView::onBlackListChanged()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	for (int i = 0; i < m_blackListModel->rowCount(); i++)
	{
		QStandardItem *item = m_blackListModel->item(i);
		QString id = item->data().toString();
		BlackListItemWidget *w = new BlackListItemWidget(this);
		w->setId(id);
		QPixmap avatar = modelManager->getUserAvatar(id);
		w->setAvatar(avatar);
		QString name = modelManager->userName(id);
		w->setNameText(name);
		setIndexWidget(item->index(), w);
		connect(w, SIGNAL(removeBlack(QString)), this, SIGNAL(removeBlack(QString)));
	}
}

void BlackListListView::onDoubleClicked(const QModelIndex &index)
{
	QStandardItem *item = m_blackListModel->itemFromIndex(index);
	if (!item)
		return;

	QString id = item->data().toString();
	emit viewMaterial(id);
}



