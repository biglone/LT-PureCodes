#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMenu>
#include <QToolTip>
#include <QPainter>

#include "model/orgstructmodeldef.h"
#include "model/orgstructitemdef.h"
#include "model/rostermodeldef.h"

#include "login/Account.h"
#include "model/ModelManager.h"
#include "PmApp.h"
#include "sortfilterproxymodel.h"
#include "util/TextUtil.h"
#include "flickerhelper.h"

#include "OrganizationDelegate.h"

#include "pmessagebox.h"

#include "OrganizationTreeView.h"

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS OrganizationHeaderView
OrganizationHeaderView::OrganizationHeaderView(QWidget *parent)
: QHeaderView(Qt::Horizontal, parent), m_orgStructModel(0), m_groupItem(0)
{
	setStyleSheet(QString("QHeaderView::section {"
		"font-size: 10.5pt;"
		"color: black;"
		"background-color: white;"
		"padding-left: 0px;"
		"border: none;"
		"border-bottom: 3px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(231, 231, 231), stop: 1.0 rgb(249, 249, 249));"
		"}"
		));

	setSectionResizeMode(QHeaderView::Stretch);
	setSortIndicatorShown(false);
	setSectionsClickable(true);
	setFixedHeight(30);

	connect(this, SIGNAL(sectionClicked(int)), this, SLOT(onSectionClicked(int)));
}

void OrganizationHeaderView::setGroupItem(OrgStructGroupItem *item)
{
	if (m_groupItem != item)
	{
		m_groupItem = item;
		if (m_groupItem)
		{
			QStandardItem *headerItem = m_orgStructModel->horizontalHeaderItem(0);
			if (headerItem)
			{
				// QString headerText = QString("%1 [%2]").arg(m_groupItem->itemName()).arg(m_groupItem->contactCount());
				QString headerText = m_groupItem->itemName();
				OrgStructGroupItem *itemParent = static_cast<OrgStructGroupItem *>(m_groupItem->parent());
				while (itemParent)
				{
					headerText.insert(0, itemParent->itemName() + " | ");
					itemParent = static_cast<OrgStructGroupItem *>(itemParent->parent());
				}

				int elideWidth = width() - 16 - 6 - 6;
				OrganizationTreeView *treeView = static_cast<OrganizationTreeView *>(parent());
				if (treeView)
				{
					if (treeView->verticalScrollBar() && treeView->verticalScrollBar()->isVisible())
					{
						elideWidth -= treeView->verticalScrollBar()->width();
					}
				}

				QFontMetrics fm = fontMetrics();
				headerText = fm.elidedText(headerText, Qt::ElideLeft, elideWidth);
				headerItem->setText(headerText);
		
				setSortIndicatorShown(false);
				setSectionsClickable(true);
				update();
			}
		}
	}
}

void OrganizationHeaderView::setOrgStructModel(OrgStructModel *model)
{
	m_orgStructModel = model;
}

void OrganizationHeaderView::setVisible(bool visible)
{
	QHeaderView::setVisible(visible);
}

void OrganizationHeaderView::onSectionClicked(int index)
{
	Q_UNUSED(index);

	if (!m_groupItem)
		return;

	if (!isVisible())
		return;

	OrganizationTreeView *treeView = static_cast<OrganizationTreeView *>(parent());
	if (!treeView)
		return;

	// get group index
	QModelIndex sourceIndex = m_groupItem->index();

	// make header view invisible
	m_groupItem->stopLoading();
	setGroupItem(0);
	treeView->setHeaderVisible(false);

	// collapse group item and scroll to it
	treeView->collapse(sourceIndex);
	treeView->scrollTo(sourceIndex);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS OrganizationTreeView
OrganizationTreeView::OrganizationTreeView( QWidget *parent /*= 0*/ )
: QTreeView(parent), m_orgStructModel(0)
{
	setRootIsDecorated(false);
	setAnimated(false);
	setEditTriggers(NoEditTriggers);
	setItemsExpandable(true);
	setExpandsOnDoubleClick(false);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setAcceptDrops(false/*true*/);
	setDropIndicatorShown(false/*true*/);
	setAutoExpandDelay(100);
	setIndentation(16);
	setSortingEnabled(false);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	m_headerView = new OrganizationHeaderView(this);
	setHeader(m_headerView);
	setHeaderHidden(true);
	
	m_clickedIndex = QModelIndex();

	m_itemDelegate = new OrganizationDelegate(this);
	setItemDelegate(m_itemDelegate);
	setupActions();
	setSkin();

	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked(const QModelIndex&)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));

	QScrollBar *vScrollBar = verticalScrollBar();
	if (vScrollBar)
	{
		connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(treeViewScrolled(int)));
	}
}

OrganizationTreeView::~OrganizationTreeView()
{

}

RosterBaseItem* OrganizationTreeView::selectedItem()
{
	QModelIndexList indexes = selectedIndexes();
	if (indexes.count() > 0)
	{
		QModelIndex selIndex = indexes.at(0);
		indexes.clear();
		if (selIndex.isValid())
		{
			RosterBaseItem *selItem = m_orgStructModel->orgStructItemFromIndex(selIndex);
			return selItem;
		}
	}
	return 0;
}

void OrganizationTreeView::setOrgStructModel( OrgStructModel *orgModel )
{
	m_orgStructModel = orgModel;

	m_itemDelegate->setOrgModel(m_orgStructModel);
	m_headerView->setOrgStructModel(m_orgStructModel);

	setModel(m_orgStructModel);
	onOrgStructSetFinished();

	bool connected = connect(m_orgStructModel, SIGNAL(orgStructSetFinished()), 
		this, SLOT(onOrgStructSetFinished()), Qt::UniqueConnection);
	Q_ASSERT(connected);
	connected = connect(m_orgStructModel, SIGNAL(orgStructChildrenAdded(QString, QStringList)), 
		this, SLOT(onOrgStructChildrenAdded(QString, QStringList)), Qt::UniqueConnection);
	Q_ASSERT(connected);
}

OrgStructModel * OrganizationTreeView::orgStructModel() const
{
	return m_orgStructModel;
}

void OrganizationTreeView::setFlickerHelper( FlickerHelper *flickerHelper )
{
	m_itemDelegate->setFlickerHelper(flickerHelper);
}

void OrganizationTreeView::setHeaderVisible(bool visible)
{
	QScrollBar *vScrollBar = verticalScrollBar();
	if (!vScrollBar)
	{
		setHeaderHidden(!visible);
		return;
	}

	if (visible)
	{
		if (isHeaderHidden())
		{
			disconnect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(treeViewScrolled(int)));
			setHeaderHidden(false);
			connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(treeViewScrolled(int)));
		}
	}
	else
	{
		if (!isHeaderHidden())
		{
			disconnect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(treeViewScrolled(int)));
			setHeaderHidden(true);
			connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(treeViewScrolled(int)));
		}
	}
}

bool OrganizationTreeView::containsFlickerItem( const QString &id, bean::MessageType msgType ) const
{
	if (!m_orgStructModel)
		return false;

	if (msgType != bean::Message_Chat)
		return false;

	return m_orgStructModel->containsContactByUid(id);
}

void OrganizationTreeView::doUpdate()
{
	update();
}

void OrganizationTreeView::doubleClicked( const QModelIndex &index )
{
	if (index.isValid())
	{
		RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
		if (itemType == RosterBaseItem::RosterTypeContact)
		{
			RosterBaseItem *rosterItem = m_orgStructModel->orgStructItemFromIndex(index);
			if (rosterItem)
			{
				QString id = rosterItem->itemId();
				if (id == Account::instance()->id()) // check if is self
				{
					emit viewMaterial(id);
				}
				else
				{
					emit chat(id);
				}
			}
		}
	}
}

void OrganizationTreeView::clicked( const QModelIndex &index )
{
	if (index == m_clickedIndex && m_clickedIndex.isValid())
	{
		RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
		if (itemType == RosterBaseItem::RosterTypeGroup)
		{
			OrgStructGroupItem *groupItem = static_cast<OrgStructGroupItem *>(m_orgStructModel->orgStructItemFromIndex(index));
			if (isExpanded(index))
			{
				collapse(index);
				groupItem->stopLoading();
			}
			else
			{
				expand(index);
				groupItem->startLoading();

				QString gid = index.data(RosterBaseItem::RosterIdRole).toString();
				m_orgStructModel->checkToAddChildren(gid);
			}
		}
	}
}

void OrganizationTreeView::contextMenu( const QPoint &position )
{
	QModelIndex index = indexAt(position);
	if (index.isValid())
	{
		// hide card first
		emit hideCard();

		RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
		if (itemType == RosterBaseItem::RosterTypeContact)
		{
			RosterBaseItem *rosterItem = m_orgStructModel->orgStructItemFromIndex(index);
			if (rosterItem)
			{
				QString id = rosterItem->itemId();

				// set action data here
				m_chat->setData(id);
				QStringList multiSendParam;
				multiSendParam << "contact" << id;
				m_msgMultiSend->setData(multiSendParam);
				m_sendMail->setData(id);
				// m_sendFile->setData(id);
				m_viewPastChats->setData(id);
				m_viewMaterial->setData(id);
				m_addFriend->setData(id);
				m_addBlack->setData(id);
				m_removeBlack->setData(id);

				QMenu menu(this);

				// add all menu actions
				if (id != Account::instance()->id())
				{
					menu.addAction(m_chat);
					menu.setDefaultAction(m_chat);
					menu.addAction(m_msgMultiSend);
					// menu.addAction(m_sendMail);
					// menu.addAction(m_multiMail);
					// menu.addAction(m_sendFile);
					menu.addSeparator();
					menu.addAction(m_viewPastChats);
				}
				else
				{
					// menu.addAction(m_sendMail);
					// menu.addAction(m_multiMail);
				}
				
				menu.addAction(m_viewMaterial);
				
				bool addSeperator = false;
				if (id != Account::instance()->id())
				{
					ModelManager *modelManager = qPmApp->getModelManager();
					RosterModel *rosterModel = modelManager->rosterModel();
					if (rosterModel && (!rosterModel->isFriend(id)))
					{
						addSeperator = true;
						menu.addSeparator();
						menu.addAction(m_addFriend);
					}
				}

				/*
				if (!addSeperator)
					menu.addSeparator();
				menu.addAction(m_multiAddFriend);
				*/

				addSeperator = false;
				if (id != Account::instance()->id())
				{
					menu.addSeparator();
					addSeperator = true;
					if (!qPmApp->getModelManager()->isInBlackList(id))
						menu.addAction(m_addBlack);
					else
						menu.addAction(m_removeBlack);
				}
				if (!addSeperator)
					menu.addSeparator();
				menu.addAction(m_manageBlack);

				// show menu
				menu.exec(QCursor::pos());
			}
		}
		else if (itemType == RosterBaseItem::RosterTypeGroup)
		{
			RosterBaseItem *groupItem = m_orgStructModel->orgStructItemFromIndex(index);
			if (groupItem)
			{
				QMenu menu(this);
				QStringList multiSendParam;
				multiSendParam << "group" << groupItem->itemId();
				m_msgMultiSend->setData(multiSendParam);
				menu.addAction(m_msgMultiSend);
				menu.exec(QCursor::pos());
			}
		}
	}
}

void OrganizationTreeView::chat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (id == Account::instance()->id()) // check if is self
		return;

	emit chat(id);
}

void OrganizationTreeView::sendMail()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit sendMail(id);
}

void OrganizationTreeView::viewMaterial()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewMaterial(id);
}

void OrganizationTreeView::viewPastChat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (id == Account::instance()->id()) // check if is self
		return;

	emit viewPastChat(id);
}

void OrganizationTreeView::sendFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (id == Account::instance()->id()) // check if is self
		return;


	emit sendFile(id, QStringList());
}

void OrganizationTreeView::addFriend()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (id == Account::instance()->id()) // check if is self
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(id);
	emit addFriendRequest(id, name);
}

void OrganizationTreeView::msgMultiSend()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList params = action->data().toStringList();
	if (params.count() != 2)
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't multi-send message"));
		return;
	}

	QStringList members;
	if (params[0] == QString("contact"))
	{
		members.append(params[1]);
	}
	else
	{
		QString id = params[1];
		if (!m_orgStructModel->containsGroup(id))
			return;

		OrgStructGroupItem *groupItem = m_orgStructModel->orgStructGroup(id);
		members = groupItem->allContactUids();
	}
	members.removeDuplicates();
	members.removeAll(Account::instance()->id()); // remove self

	if (members.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("No member selected"));
		return;
	}

	emit msgMultiSend(members);
}

void OrganizationTreeView::rosterUpdated()
{
	this->update();
}

void OrganizationTreeView::addBlack()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't add blocked list"));
		return;
	}

	QString name = qPmApp->getModelManager()->userName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Add Blocked List"), 
		tr("Are you sure to add %1 to blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit addBlack(id);
}

void OrganizationTreeView::removeBlack()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't remove out of blocked list"));
		return;
	}

	QString name = qPmApp->getModelManager()->userName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Remove out of Blocked List"), 
		tr("Are you sure to remove %1 out of blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit removeBlack(id);
}

void OrganizationTreeView::mouseReleaseEvent( QMouseEvent *event )
{
	m_clickedIndex = QModelIndex();
	if (event->button() == Qt::LeftButton)
	{
		m_clickedIndex = indexAt(event->pos());
	}
	QTreeView::mouseReleaseEvent(event);
}

bool OrganizationTreeView::event(QEvent *event)
{
	if ((event->type() == QEvent::HoverLeave)) 
	{
		// if don't has focus, hide the card
		emit hideCard();
	}
	return QTreeView::event(event);
}

void OrganizationTreeView::mouseMoveEvent( QMouseEvent *event )
{
	QPoint pt = event->pos();
	QModelIndex index = indexAt(pt);
	if (index.isValid())
	{
		RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
		if (itemType == RosterBaseItem::RosterTypeContact)
		{
			RosterBaseItem *rosterItem = m_orgStructModel->orgStructItemFromIndex(index);
			int depth = 0/*rosterItem->depth()*/;
			QString id = rosterItem->itemId();

			QRect rect = visualRect(index);
			if (pt.x() > (rect.left() + depth*OrganizationDelegate::kmargin + 5) && pt.x() < rect.left() + depth*OrganizationDelegate::kmargin + 6 + 20 + 1 // check if cursor is in the avatar area
				&& pt.y() > (rect.top()+3) && pt.y() < (rect.top()+30-5))
			{
				emit showCard(id, rect.top());
				QTreeView::mouseMoveEvent(event);
				return;
			}
		}
	}

	// if index is invalid, hide card
	emit hideCard();

	QTreeView::mouseMoveEvent(event);
}

void OrganizationTreeView::treeViewScrolled(int value)
{
	Q_UNUSED(value);

	QPoint topPt(30, 15);
	QModelIndex topIndex = indexAt(topPt);
	if (!topIndex.isValid())
		return;

	RosterBaseItem *item = m_orgStructModel->orgStructItemFromIndex(topIndex);
	if (!item)
		return;

	if (item->itemType() == RosterBaseItem::RosterTypeContact)
	{
		RosterContactItem *contactItem = static_cast<RosterContactItem *>(item);
		OrgStructGroupItem *groupItem = static_cast<OrgStructGroupItem *>(contactItem->parent());
		if (groupItem)
		{
			m_headerView->setGroupItem(groupItem);
			m_headerView->update();
			if (isHeaderHidden())
			{
				setHeaderVisible(true);
			}
		}
	}
	else if (item->itemType() == RosterBaseItem::RosterTypeGroup)
	{
		m_headerView->setGroupItem(0);
		if (!isHeaderHidden())
		{
			setHeaderVisible(false);
		}
	}
}

void OrganizationTreeView::onOrgStructSetFinished()
{
	setHeaderVisible(false);
	update();
}

void OrganizationTreeView::onOrgStructChildrenAdded(const QString &gid, const QStringList &childrenGid)
{
	Q_UNUSED(childrenGid);

	OrgStructGroupItem *groupItem = m_orgStructModel->orgStructGroup(gid);
	if (!groupItem)
		return;

	groupItem->stopLoading();
	update();
}

void OrganizationTreeView::setupActions()
{
	m_chat = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chat, SIGNAL(triggered()), SLOT(chat()));

	m_sendMail = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Send Mail"), this);
	connect(m_sendMail, SIGNAL(triggered()), SLOT(sendMail()));

	m_multiMail = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Mail to Others"), this);
	connect(m_multiMail, SIGNAL(triggered()), SIGNAL(multiMail()));

	m_sendFile = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Send File"), this);
	connect(m_sendFile, SIGNAL(triggered()), SLOT(sendFile()));

	m_viewPastChats = new QAction(QIcon(":/images/Icon_102.png"), tr("View Chat History"), this);
	connect(m_viewPastChats, SIGNAL(triggered()), SLOT(viewPastChat()));

	m_viewMaterial = new QAction(/*QIcon(":/images/icon.png"),*/ tr("View Contact Profile"), this);
	connect(m_viewMaterial, SIGNAL(triggered()), SLOT(viewMaterial()));

	m_addFriend = new QAction(tr("Add Friends"), this);
	connect(m_addFriend, SIGNAL(triggered()), SLOT(addFriend()));

	m_multiAddFriend = new QAction(tr("Add Friends in One Time"), this);
	connect(m_multiAddFriend, SIGNAL(triggered()), SIGNAL(multiAddFriend()));

	m_msgMultiSend = new QAction(tr("Multi-Send Message"), this);
	connect(m_msgMultiSend, SIGNAL(triggered()), SLOT(msgMultiSend()));

	m_addBlack = new QAction(tr("Add Blocked List"), this);
	connect(m_addBlack, SIGNAL(triggered()), SLOT(addBlack()));

	m_removeBlack = new QAction(tr("Remove out of Blocked List"), this);
	connect(m_removeBlack, SIGNAL(triggered()), SLOT(removeBlack()));

	m_manageBlack = new QAction(tr("View Blocked List"), this);
	connect(m_manageBlack, SIGNAL(triggered()), SIGNAL(manageBlack()));
}

void OrganizationTreeView::setSkin()
{
	setStyleSheet(QString("QTreeView{"
		"font-size: 10.5pt;"
		"border: none;"
		"background-color: transparent;"
		"}"

		"QTreeView::branch:has-siblings:!adjoins-item {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:has-siblings:adjoins-item {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:has-children:!has-siblings:closed,"
		"QTreeView::branch:closed:has-children:has-siblings {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:open:has-children:!has-siblings,"
		"QTreeView::branch:open:has-children:has-siblings  {"
		"border-image: none;"
		"image: none"
		"}"
		));
}
