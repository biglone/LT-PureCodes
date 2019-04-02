#include "editfilterlistview.h"
#include "editfilteritemdelegate.h"
#include "login/Account.h"
#include <QStandardItemModel>
#include "model/rosteritemdef.h"
#include "model/orgstructitemdef.h"
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QToolTip>
#include <QHeaderView>
#include <QKeyEvent>

EditFilterListView::EditFilterListView(QWidget *parent)
: QListView(parent), m_sourceModel(0), m_listType(Roster), m_selectMode(EditFilterWidget::DoubleClick)
{
	setEditTriggers(NoEditTriggers);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);

	this->setStyleSheet(QString("QListView {"
		"border: none;"
		"}"));

	m_itemDelegate = new EditFilterItemDelegate(this);
	setItemDelegate(m_itemDelegate);

	setupActions();

	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
}

EditFilterListView::~EditFilterListView()
{

}

void EditFilterListView::setSelectMode(EditFilterWidget::SelectMode mode)
{
	m_selectMode = mode;
}

EditFilterWidget::SelectMode EditFilterListView::selectMode() const
{
	return m_selectMode;
}

void EditFilterListView::setListType(ListType listType)
{
	m_listType = listType;
}

EditFilterListView::ListType EditFilterListView::listType() const
{
	return m_listType;
}

void EditFilterListView::setEditFilterModel(QStandardItemModel *sourceModel)
{
	if (m_sourceModel)
	{
		delete m_sourceModel;
		m_sourceModel = 0;
	}

	m_sourceModel = sourceModel;
	setModel(m_sourceModel);
}

QStandardItemModel *EditFilterListView::editFilterModel() const
{
	return m_sourceModel;
}

void EditFilterListView::addItem(RosterBaseItem *item)
{
	Q_ASSERT(m_sourceModel);
	m_sourceModel->appendRow(item);
}

void EditFilterListView::clearAll()
{
	m_sourceModel->clear();
}

void EditFilterListView::doubleClicked(const QModelIndex &index)
{
	if (index == m_clickedIndex && m_clickedIndex.isValid())
	{
		if (m_selectMode == EditFilterWidget::DoubleClick)
		{
			clickAction(index);
		}
	}
}

void EditFilterListView::clicked(const QModelIndex &index)
{
	if (index == m_clickedIndex && m_clickedIndex.isValid())
	{
		if (m_selectMode == EditFilterWidget::SingleClick)
		{
			clickAction(index);
		}
	}
}

void EditFilterListView::contextMenu(const QPoint &position)
{
	QModelIndex index = indexAt(position);
	if (!index.isValid())
		return;

	QModelIndex sourceIndex = index;
	RosterBaseItem *item = static_cast<RosterBaseItem *>(m_sourceModel->itemFromIndex(sourceIndex));
	if (!item)
		return;

	RosterBaseItem::RosterItemType itemType = item->itemType();
	QString id = item->itemId();
	if (itemType == RosterBaseItem::RosterTypeContact)
	{
		m_chatAction->setData(id);
		m_viewMaterialAction->setData(id);
		QStringList selectData;
		if (m_listType == Roster)
		{
			selectData << id << QString::number(SELECT_SOURCE_ROSTER) << QString();
		}
		else
		{
			OrgStructContactItem *osContactItem = static_cast<OrgStructContactItem *>(item);
			selectData << id << QString::number(SELECT_SOURCE_OS) << osContactItem->itemWid();
		}

		m_selectAction->setData(selectData);

		QMenu menu(this);
		if (id != Account::instance()->id())
		{
			menu.addAction(m_chatAction);
			menu.addAction(m_viewMaterialAction);
			menu.addSeparator();
			menu.addAction(m_selectAction);
		}
		else
		{
			menu.addAction(m_viewMaterialAction);
			menu.addSeparator();
			menu.addAction(m_selectAction);
		}

		// show menu
		menu.exec(QCursor::pos());
	}
	else if (itemType == RosterBaseItem::RosterTypeGroupMuc)
	{
		m_groupAction->setData(id);
		QStringList selectData;
		selectData << id << QString::number(SELECT_SOURCE_GROUP) << QString();
		m_selectAction->setData(selectData);

		QMenu menu(this);
		menu.addAction(m_groupAction);
		menu.addSeparator();
		menu.addAction(m_selectAction);

		// show menu
		menu.exec(QCursor::pos());
	}
	else if (itemType == RosterBaseItem::RosterTypeDiscuss)
	{
		m_discussAction->setData(id);
		QStringList selectData;
		selectData << id << QString::number(SELECT_SOURCE_DISCUSS) << QString();
		m_selectAction->setData(selectData);

		QMenu menu(this);
		menu.addAction(m_discussAction);
		menu.addSeparator();
		menu.addAction(m_selectAction);

		// show menu
		menu.exec(QCursor::pos());
	}
	else if (itemType == RosterBaseItem::RosterTypeSubscription)
	{
		m_subscriptionAction->setData(id);
		QStringList selectData;
		selectData << id << QString::number(SELECT_SOURCE_SUBSCRIPTION) << QString();
		m_selectAction->setData(selectData);

		QMenu menu(this);
		menu.addAction(m_subscriptionAction);
		menu.addSeparator();
		menu.addAction(m_selectAction);

		// show menu
		menu.exec(QCursor::pos());
	}
}

void EditFilterListView::mouseReleaseEvent(QMouseEvent *event)
{
	m_clickedIndex = QModelIndex();
	if (event->button() == Qt::LeftButton)
	{
		m_clickedIndex = indexAt(event->pos());
	}
	QListView::mouseReleaseEvent(event);
}

void EditFilterListView::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();  
	if (Qt::Key_Enter == key || Qt::Key_Return == key) 
	{  
		returnKeyPressed();
	} 
	else 
	{  
		QListView::keyPressEvent(event);  
	}  
}

void EditFilterListView::chat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit chat(id);
}

void EditFilterListView::groupChat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit groupChat(id);
}

void EditFilterListView::discussChat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit discussChat(id);
}

void EditFilterListView::subscriptionChat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit subscriptionChat(id);
}

void EditFilterListView::viewMaterial()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewMaterial(id);
}

void EditFilterListView::selectSearchItem()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList datas = action->data().toStringList();
	if (datas.count() != 3)
		return;

	QString id = datas[0];
	QString sourceStr = datas[1];
	QString wid = datas[2];
	int source = sourceStr.toInt();
	emit selectSearchItem(id, source, wid);
}

void EditFilterListView::clickAction(const QModelIndex &index)
{
	if (index.isValid())
	{
		QModelIndex sourceIndex = index;
		RosterBaseItem *item = static_cast<RosterBaseItem *>(m_sourceModel->itemFromIndex(sourceIndex));
		RosterBaseItem::RosterItemType itemType = item->itemType();
		QString id = item->itemId();
		if (itemType == RosterBaseItem::RosterTypeContact)
		{
			if (id == Account::instance()->id()) // check if is self
			{
				emit viewMaterial(id);
			}
			else
			{
				emit chat(id);
			}

			// select item from source panel
			if (m_listType == Roster)
			{
				emit selectSearchItem(id, SELECT_SOURCE_ROSTER, QString());
			}
			else
			{
				OrgStructContactItem *osContactItem = static_cast<OrgStructContactItem *>(item);
				QString wid = osContactItem->itemWid();
				emit selectSearchItem(id, SELECT_SOURCE_OS, wid);
			}
		}
		else if (itemType == RosterBaseItem::RosterTypeGroupMuc)
		{
			emit groupChat(id);

			// select item from source panel
			emit selectSearchItem(id, SELECT_SOURCE_GROUP, QString());
		}
		else if (itemType == RosterBaseItem::RosterTypeDiscuss)
		{
			emit discussChat(id);

			// select item from source panel
			emit selectSearchItem(id, SELECT_SOURCE_DISCUSS, QString());
		}
		else if (itemType == RosterBaseItem::RosterTypeSubscription)
		{
			emit subscriptionChat(id);

			// select item from source panel
			// emit selectSearchItem(id, SELECT_SOURCE_SUBSCRIPTION, QString());
		}
	}
}

void EditFilterListView::setupActions()
{
	m_chatAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chatAction, SIGNAL(triggered()), SLOT(chat()));

	m_groupAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_groupAction, SIGNAL(triggered()), SLOT(groupChat()));

	m_discussAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_discussAction, SIGNAL(triggered()), SLOT(discussChat()));

	m_subscriptionAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Enter Subscription"), this);
	connect(m_subscriptionAction, SIGNAL(triggered()), SLOT(subscriptionChat()));

	m_selectAction = new QAction(tr("Check in Panel"), this);
	connect(m_selectAction, SIGNAL(triggered()), SLOT(selectSearchItem()));

	m_viewMaterialAction = new QAction(tr("View Contact Profile"), this);
	connect(m_viewMaterialAction, SIGNAL(triggered()), SLOT(viewMaterial()));
}

void EditFilterListView::returnKeyPressed()
{
	QModelIndex curIndex = this->currentIndex();
	if (!curIndex.isValid())
		return;

	doubleClicked(curIndex);
}

