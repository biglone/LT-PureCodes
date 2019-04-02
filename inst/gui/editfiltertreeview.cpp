#include "editfiltertreeview.h"
#include "editfilteritemdelegate.h"
#include "login/Account.h"
#include <QStandardItemModel>
#include "model/rosteritemdef.h"
#include "model/orgstructitemdef.h"
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QToolTip>
#include <QKeyEvent>
#include "editfiltergroupitemwidget.h"
#include "ModelManager.h"

EditFilterTreeView::EditFilterTreeView(QWidget *parent)
	: QTreeView(parent)
	, m_sourceModel(0)
	, m_rosterGroup(0)
	, m_osContactGroup(0)
	, m_groupGroup(0)
	, m_discussGroup(0)
	, m_subscriptionGroup(0)
	, m_selectMode(EditFilterWidget::DoubleClick)
{
	setRootIsDecorated(false);
	setAnimated(false);
	setEditTriggers(NoEditTriggers);
	setItemsExpandable(true);
	setExpandsOnDoubleClick(false);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setAutoExpandDelay(100);
	setIndentation(0);
	setSortingEnabled(false);
	setHeaderHidden(true);

	this->setStyleSheet(QString("QTreeView {"
		"border: none;"
		"}"));

	m_itemDelegate = new EditFilterItemDelegate(this);
	setItemDelegate(m_itemDelegate);

	setupActions();

	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
}

EditFilterTreeView::~EditFilterTreeView()
{

}

void EditFilterTreeView::setSelectMode(EditFilterWidget::SelectMode mode)
{
	m_selectMode = mode;
}

EditFilterWidget::SelectMode EditFilterTreeView::selectMode() const
{
	return m_selectMode;
}

void EditFilterTreeView::setEditFilterModel(QStandardItemModel *sourceModel)
{
	if (m_sourceModel)
	{
		delete m_sourceModel;
		m_sourceModel = 0;
	}

	m_sourceModel = sourceModel;
	setModel(m_sourceModel);
}

QStandardItemModel *EditFilterTreeView::editFilterModel() const
{
	return m_sourceModel;
}

void EditFilterTreeView::selectFirstItem()
{
	for (int row = 0; row < m_sourceModel->rowCount(); row++)
	{
		QStandardItem *item = m_sourceModel->item(row);
		QModelIndex sourceIndex = item->index();
		int childCount = m_sourceModel->rowCount(sourceIndex);
		if (childCount > 0)
		{
			QModelIndex selIndex = sourceIndex.child(0, 0);
			if (selIndex.isValid())
			{
				this->setCurrentIndex(selIndex);
				return;
			}
		}
	}
}

void EditFilterTreeView::setRosterGroup(RosterBaseItem *rosterGroup)
{
	Q_ASSERT(m_sourceModel);
	m_sourceModel->appendRow(rosterGroup);
	m_rosterGroup = rosterGroup;
	
	EditFilterGroupItemWidget *w = new EditFilterGroupItemWidget(this);
	w->setType(EditFilterGroupItemWidget::Roster);
	w->setGroupText(tr("Friends"));
	connect(w, SIGNAL(viewAll(int)), this, SIGNAL(viewAll(int)), Qt::UniqueConnection);
	setIndexWidget(m_rosterGroup->index(), w);
}

void EditFilterTreeView::setOsGroup(RosterBaseItem *osGroup)
{
	Q_ASSERT(m_sourceModel);
	m_sourceModel->appendRow(osGroup);
	m_osContactGroup = osGroup;

	EditFilterGroupItemWidget *w = new EditFilterGroupItemWidget(this);
	w->setType(EditFilterGroupItemWidget::Os);
	w->setGroupText(tr("Corporation"));
	connect(w, SIGNAL(viewAll(int)), this, SIGNAL(viewAll(int)), Qt::UniqueConnection);
	setIndexWidget(m_osContactGroup->index(), w);
}

void EditFilterTreeView::setGroupGroup(RosterBaseItem *groupGroup)
{
	Q_ASSERT(m_sourceModel);
	m_sourceModel->appendRow(groupGroup);
	m_groupGroup = groupGroup;

	EditFilterGroupItemWidget *w = new EditFilterGroupItemWidget(this);
	w->setType(EditFilterGroupItemWidget::Group);
	w->setGroupText(tr("Groups"));
	connect(w, SIGNAL(viewAll(int)), this, SIGNAL(viewAll(int)), Qt::UniqueConnection);
	setIndexWidget(m_groupGroup->index(), w);
}

void EditFilterTreeView::setDiscussGroup(RosterBaseItem *discussGroup)
{
	Q_ASSERT(m_sourceModel);
	m_sourceModel->appendRow(discussGroup);
	m_discussGroup = discussGroup;

	EditFilterGroupItemWidget *w = new EditFilterGroupItemWidget(this);
	w->setType(EditFilterGroupItemWidget::Discuss);
	w->setGroupText(tr("Discusses"));
	connect(w, SIGNAL(viewAll(int)), this, SIGNAL(viewAll(int)), Qt::UniqueConnection);
	setIndexWidget(m_discussGroup->index(), w);
}

void EditFilterTreeView::setSubscriptionGroup(RosterBaseItem *subscriptionGroup)
{
	Q_ASSERT(m_sourceModel);
	m_sourceModel->appendRow(subscriptionGroup);
	m_subscriptionGroup = subscriptionGroup;

	EditFilterGroupItemWidget *w = new EditFilterGroupItemWidget(this);
	w->setType(EditFilterGroupItemWidget::Subscription);
	w->setGroupText(ModelManager::subscriptionShowName());
	connect(w, SIGNAL(viewAll(int)), this, SIGNAL(viewAll(int)), Qt::UniqueConnection);
	setIndexWidget(m_subscriptionGroup->index(), w);
}

void EditFilterTreeView::setRosterGroupInfo(const QString &groupText, bool viewAll)
{
	if (m_rosterGroup)
	{
		EditFilterGroupItemWidget *w = static_cast<EditFilterGroupItemWidget *>(this->indexWidget(m_rosterGroup->index()));
		w->setGroupText(groupText);
		w->setViewAllVisible(viewAll);
	}
}

void EditFilterTreeView::setOsGroupInfo(const QString &groupText, bool viewAll)
{
	if (m_osContactGroup)
	{
		EditFilterGroupItemWidget *w = static_cast<EditFilterGroupItemWidget *>(this->indexWidget(m_osContactGroup->index()));
		w->setGroupText(groupText);
		w->setViewAllVisible(viewAll);
	}
}

void EditFilterTreeView::setGroupGroupInfo(const QString &groupText, bool viewAll)
{
	if (m_groupGroup)
	{
		EditFilterGroupItemWidget *w = static_cast<EditFilterGroupItemWidget *>(this->indexWidget(m_groupGroup->index()));
		w->setGroupText(groupText);
		w->setViewAllVisible(viewAll);
	}
}

void EditFilterTreeView::setDiscussGroupInfo(const QString &groupText, bool viewAll)
{
	if (m_discussGroup)
	{
		EditFilterGroupItemWidget *w = static_cast<EditFilterGroupItemWidget *>(this->indexWidget(m_discussGroup->index()));
		w->setGroupText(groupText);
		w->setViewAllVisible(viewAll);
	}
}

void EditFilterTreeView::setSubscriptionGroupInfo(const QString &groupText, bool viewAll)
{
	if (m_subscriptionGroup)
	{
		EditFilterGroupItemWidget *w = static_cast<EditFilterGroupItemWidget *>(this->indexWidget(m_subscriptionGroup->index()));
		w->setGroupText(groupText);
		w->setViewAllVisible(viewAll);
	}
}

RosterBaseItem *EditFilterTreeView::rosterGroup() const
{
	return m_rosterGroup;
}

RosterBaseItem *EditFilterTreeView::osContactGroup() const
{
	return m_osContactGroup;
}

RosterBaseItem *EditFilterTreeView::groupGroup() const
{
	return m_groupGroup;
}

RosterBaseItem *EditFilterTreeView::discussGroup() const
{
	return m_discussGroup;
}

RosterBaseItem *EditFilterTreeView::subscriptionGroup() const
{
	return m_subscriptionGroup;
}

void EditFilterTreeView::addRosterItem(RosterBaseItem *rosterItem)
{
	if (m_rosterGroup)
	{
		m_rosterGroup->appendRow(rosterItem);
	}
}

void EditFilterTreeView::addOsContactItem(RosterBaseItem *osContactItem)
{
	if (m_osContactGroup)
	{
		m_osContactGroup->appendRow(osContactItem);
	}
}

void EditFilterTreeView::addGroupItem(RosterBaseItem *groupItem)
{
	if (m_groupGroup)
	{
		m_groupGroup->appendRow(groupItem);
	}
}

void EditFilterTreeView::addDiscussItem(RosterBaseItem *discussItem)
{
	if (m_discussGroup)
	{
		m_discussGroup->appendRow(discussItem);
	}
}

void EditFilterTreeView::addSubscriptionItem(RosterBaseItem *subscriptionItem)
{
	if (m_subscriptionGroup)
	{
		m_subscriptionGroup->appendRow(subscriptionItem);
	}
}

void EditFilterTreeView::clearAll()
{
	clearRoster();
	clearOsContact();
	clearGroup();
	clearDiscuss();
	clearSubscription();
}

void EditFilterTreeView::clearRoster()
{
	if (m_rosterGroup)
	{
		int count = m_rosterGroup->rowCount();
		if (count > 0)
		{
			m_rosterGroup->removeRows(0, count);
		}
	}
}

void EditFilterTreeView::clearOsContact()
{
	if (m_osContactGroup)
	{
		int count = m_osContactGroup->rowCount();
		if (count > 0)
		{
			m_osContactGroup->removeRows(0, count);
		}
	}
}

void EditFilterTreeView::clearGroup()
{
	if (m_groupGroup)
	{
		int count = m_groupGroup->rowCount();
		if (count)
		{
			m_groupGroup->removeRows(0, count);
		}
	}
}

void EditFilterTreeView::clearDiscuss()
{
	if (m_discussGroup)
	{
		int count = m_discussGroup->rowCount();
		if (count > 0)
		{
			m_discussGroup->removeRows(0, count);
		}
	}
}

void EditFilterTreeView::clearSubscription()
{
	if (m_subscriptionGroup)
	{
		int count = m_subscriptionGroup->rowCount();
		if (count > 0)
		{
			m_subscriptionGroup->removeRows(0, count);
		}
	}
}

void EditFilterTreeView::setExpandAll()
{
	expandAll();
}

void EditFilterTreeView::upKeyPressed()
{
	QModelIndex curIndex = this->currentIndex();
	if (!curIndex.isValid())
		return;

	QModelIndex parIndex = curIndex.parent();
	if (!parIndex.isValid())
		return;

	// find the up sibling
	QModelIndex upIndex = curIndex.sibling(curIndex.row()-1, 0);
	if (upIndex.isValid())
	{
		this->setCurrentIndex(upIndex);
		return;
	}

	// the up sibling is invalid, find the next up one with different parent
	int upRow = parIndex.row() - 1;
	if (upRow < 0)
		upRow = m_sourceModel->rowCount() - 1; 
	parIndex = parIndex.sibling(upRow, 0);

	while (parIndex.isValid())
	{
		int rowCount = m_sourceModel->rowCount(parIndex);
		if (rowCount > 0)
		{
			upIndex = parIndex.child(rowCount-1, 0);
			if (upIndex.isValid())
			{
				if (upIndex != curIndex)
					this->setCurrentIndex(upIndex);
				break;
			}
		}

		upRow = parIndex.row() - 1;
		if (upRow < 0)
			upRow = m_sourceModel->rowCount() - 1; 
		parIndex = parIndex.sibling(upRow, 0);
	}
}

void EditFilterTreeView::downKeyPressed()
{
	QModelIndex curIndex = this->currentIndex();
	if (!curIndex.isValid())
		return;

	QModelIndex parIndex = curIndex.parent();
	if (!parIndex.isValid())
		return;

	// find the down sibling
	QModelIndex downIndex = curIndex.sibling(curIndex.row()+1, 0);
	if (downIndex.isValid())
	{
		this->setCurrentIndex(downIndex);
		return;
	}

	// the down sibling is invalid, find the next down one with different parent
	int downRow = parIndex.row() + 1;
	if (downRow >= m_sourceModel->rowCount())
		downRow = 0; 
	parIndex = parIndex.sibling(downRow, 0);

	while (parIndex.isValid())
	{
		int rowCount = m_sourceModel->rowCount(parIndex);
		if (rowCount > 0)
		{
			downIndex = parIndex.child(0, 0);
			if (downIndex.isValid())
			{
				if (downIndex != curIndex)
					this->setCurrentIndex(downIndex);
				break;
			}
		}

		int downRow = parIndex.row() + 1;
		if (downRow >= m_sourceModel->rowCount())
			downRow = 0; 
		parIndex = parIndex.sibling(downRow, 0);
	}
}

void EditFilterTreeView::returnKeyPressed()
{
	QModelIndex curIndex = this->currentIndex();
	if (!curIndex.isValid())
		return;

	clickAction(curIndex);
}

void EditFilterTreeView::doubleClicked(const QModelIndex &index)
{
	if (index == m_clickedIndex && m_clickedIndex.isValid())
	{
		if (m_selectMode == EditFilterWidget::DoubleClick)
		{
			clickAction(index);
		}
	}
}

void EditFilterTreeView::clicked(const QModelIndex &index)
{
	if (index == m_clickedIndex && m_clickedIndex.isValid())
	{
		if (m_selectMode == EditFilterWidget::SingleClick)
		{
			clickAction(index);
		}
	}
}

void EditFilterTreeView::contextMenu(const QPoint &position)
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
		RosterBaseItem *parent = static_cast<RosterBaseItem *>(item->parent());
		if (parent)
		{
			QString parentName = parent->itemName();
			if (parentName == tr("Friends"))
			{
				selectData << id << QString::number(SELECT_SOURCE_ROSTER) << QString();
			}
			else if (parentName == tr("Corporation"))
			{
				OrgStructContactItem *osContactItem = static_cast<OrgStructContactItem *>(item);
				selectData << id << QString::number(SELECT_SOURCE_OS) << osContactItem->itemWid();
			}
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

void EditFilterTreeView::mouseReleaseEvent(QMouseEvent *event)
{
	m_clickedIndex = QModelIndex();
	if (event->button() == Qt::LeftButton)
	{
		m_clickedIndex = indexAt(event->pos());
	}
	QTreeView::mouseReleaseEvent(event);
}

void EditFilterTreeView::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();  
	if (Qt::Key_Down == key) 
	{
		downKeyPressed();
	} 
	else if (Qt::Key_Up == key) 
	{  
		upKeyPressed();
	}
	else if (Qt::Key_Enter == key || Qt::Key_Return == key) 
	{  
		returnKeyPressed();
	} 
	else 
	{  
		QTreeView::keyPressEvent(event);  
	}  
}

void EditFilterTreeView::chat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit chat(id);
}

void EditFilterTreeView::groupChat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit groupChat(id);
}

void EditFilterTreeView::discussChat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit discussChat(id);
}

void EditFilterTreeView::subscriptionChat()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit subscriptionChat(id);
}

void EditFilterTreeView::viewMaterial()
{
	QAction *action = static_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewMaterial(id);
}

void EditFilterTreeView::selectSearchItem()
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

void EditFilterTreeView::clickAction(const QModelIndex &index)
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
			RosterBaseItem *parent = static_cast<RosterBaseItem *>(item->parent());
			if (parent)
			{
				QString parentName = parent->itemName();
				if (parentName == tr("Friends"))
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

void EditFilterTreeView::setupActions()
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
