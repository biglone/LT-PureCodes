#include "rostertreeview.h"
#include "model/rostermodeldef.h"
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMenu>
#include <QDropEvent>
#include <QFileInfo>
#include <QToolTip>
#include "login/Account.h"
#include "model/ModelManager.h"
#include "PmApp.h"
#include "loginmgr.h"
#include "sortfilterproxymodel.h"
#include "util/TextUtil.h"
#include "flickerhelper.h"
#include "manager/rostermanager.h"
#include "pmessagebox.h"
#include "plaintextlineinput.h"
#include <QDebug>
#include "settings/GlobalSettings.h"
#include "addfriendmanager.h"
#include <QMimeData>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterTreeView
RosterTreeView::RosterTreeView(QWidget *parent) : QTreeView(parent), m_rosterModel(0), m_avatarType(RosterDelegate::BigAvatar)
{
	setRootIsDecorated(false);
	setAnimated(false);
	setEditTriggers(NoEditTriggers);
	setItemsExpandable(true);
	setExpandsOnDoubleClick(false);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setDragEnabled(false);
	setAcceptDrops(false);
	setAutoExpandDelay(100);
	setIndentation(0);
	setSortingEnabled(false);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setHeaderHidden(true);

	m_clickedIndex = QModelIndex();

	AccountSettings::RosterAvatarType rosterAvatarType = Account::settings()->rosterAvatarType();
	if (rosterAvatarType == AccountSettings::UnknownAvatar)
	{
		if (GlobalSettings::isRosterSmallAvatar())
			m_avatarType = RosterDelegate::SmallAvatar;
		else
			m_avatarType = RosterDelegate::BigAvatar;
	}
	else
	{
		m_avatarType = (RosterDelegate::AvatarType)rosterAvatarType;
	}
	m_itemDelegate = new RosterDelegate(this);
	m_itemDelegate->setAvatarType(m_avatarType);
	setItemDelegate(m_itemDelegate);
	setupActions();
	setSkin();

	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked(const QModelIndex&)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));
}

RosterTreeView::~RosterTreeView()
{

}

QStandardItem *RosterTreeView::selectedItem()
{
	QModelIndexList indexes = selectedIndexes();
	if (indexes.count() > 0)
	{
		QModelIndex selIndex = indexes.at(0);
		indexes.clear();
		if (selIndex.isValid())
		{
			QStandardItem *selItem = m_rosterModel->nodeFromProxyIndex(selIndex);
			return selItem;
		}
	}
	return 0;
}

void RosterTreeView::setRosterModel(RosterModel *rosterModel)
{
	m_rosterModel = rosterModel;
	m_itemDelegate->setRosterModel(m_rosterModel);
	if (m_rosterModel->proxyModel())
		setModel(m_rosterModel->proxyModel());
	else
		setModel(m_rosterModel);

	expandAll();
	connect(m_rosterModel, SIGNAL(rosterSetFinished()), this, SLOT(onRosterSetFinished()));
}

RosterModel *RosterTreeView::rosterModel() const
{
	return m_rosterModel;
}

void RosterTreeView::setFlickerHelper(FlickerHelper *flickerHelper)
{
	m_itemDelegate->setFlickerHelper(flickerHelper);
}

bool RosterTreeView::containsFlickerItem(const QString &id, bean::MessageType msgType) const
{
	if (!m_rosterModel)
		return false;

	if (msgType != bean::Message_Chat)
		return false;

	return m_rosterModel->containsRoster(id);
}

void RosterTreeView::doUpdate()
{
	update();
}

void RosterTreeView::setupActions()
{
	m_chat = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chat, SIGNAL(triggered()), SLOT(chat()));

	m_sendMail = new QAction(tr("Send Mail"), this);
	connect(m_sendMail, SIGNAL(triggered()), SLOT(sendMail()));

	m_viewPastChats = new QAction(QIcon(":/images/Icon_102.png"), tr("View Chat History"), this);
	connect(m_viewPastChats, SIGNAL(triggered()), SLOT(viewPastChat()));

	m_viewMaterial = new QAction(tr("View Contact Profile"), this);
	connect(m_viewMaterial, SIGNAL(triggered()), SLOT(viewMaterial()));

	m_removeRoster = new QAction(tr("Delete"), this);
	connect(m_removeRoster, SIGNAL(triggered()), SLOT(removeRoster()));

	m_msgMultSend = new QAction(tr("Multi-Send Message"), this);
	connect(m_msgMultSend, SIGNAL(triggered()), SLOT(msgMultiSend()));

	m_addBlack = new QAction(tr("Add Blocked List"), this);
	connect(m_addBlack, SIGNAL(triggered()), SLOT(addBlack()));

	m_removeBlack = new QAction(tr("Remove out of Blocked List"), this);
	connect(m_removeBlack, SIGNAL(triggered()), SLOT(removeBlack()));

	m_manageBlack = new QAction(tr("View Blocked List"), this);
	connect(m_manageBlack, SIGNAL(triggered()), SIGNAL(manageBlack()));

	m_avatarTypeSwitchAction = new QAction(tr("Switch to Small Avatar"), this);
	connect(m_avatarTypeSwitchAction, SIGNAL(triggered()), SLOT(switchAvatarType()));
}

void RosterTreeView::chat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit chat(id);
}

void RosterTreeView::sendMail()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit sendMail(id);
}

void RosterTreeView::viewPastChat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewPastChat(id);
}

void RosterTreeView::viewMaterial()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewMaterial(id);
}

void RosterTreeView::removeRoster()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (!checkLogined(tr("delete friend")))
		return;

	QString groupName = m_rosterModel->rosterGroupName(id);
	if (groupName.isEmpty())
		return;

	QString name = m_rosterModel->rosterName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Delete"), 
		tr("Are you sure to delete %1").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	// send remove roster request
	QString selfId = Account::instance()->id();
	qPmApp->getAddFriendManager()->deleteFriend(selfId, id);
}

void RosterTreeView::msgMultiSend()
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

	QStringList ids;
	if (params[0] == QString("contact"))
	{
		ids.append(params[1]);
	}
	else
	{
		QString groupName = params[1];
		if (!m_rosterModel->containsGroup(groupName))
			return;

		ids = m_rosterModel->groupRosters(groupName);
	}
	ids.removeDuplicates();
	ids.removeAll(Account::instance()->id());

	if (ids.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("No members selected"));
		return;
	}

	emit msgMultiSend(ids);
}

void RosterTreeView::addBlack()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (!checkLogined(tr("add blocked list")))
		return;

	QString name = m_rosterModel->rosterName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Add Blocked List"), 
		tr("Are you sure to add %1 to blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit addBlack(id);
}

void RosterTreeView::removeBlack()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (!checkLogined(tr("remove out of black list")))
		return;

	QString name = m_rosterModel->rosterName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Remove out of Blocked List"), 
		tr("Are you sure to remove %1 out of blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit removeBlack(id);
}

void RosterTreeView::onPresenceChanged(const QString &id)
{
	if (m_rosterModel)
	{
		QString fromId = id;
		if (fromId == Account::instance()->id())
			fromId = Account::instance()->phoneFullId();
		if (m_rosterModel->containsRoster(fromId))
		{
			update();
		}
	}
}

void RosterTreeView::onPresenceCleared()
{
	if (m_rosterModel)
	{
		update();
	}
}

void RosterTreeView::doubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		QStandardItem *rosterItem = m_rosterModel->nodeFromProxyIndex(index);
		if (rosterItem)
		{
			RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(rosterItem->data(RosterModel::TypeRole).toInt());
			if (itemType == RosterModel::Roster)
			{
				QString id = rosterItem->data(RosterModel::IdRole).toString();
				if (id == QString(SUBSCRIPTION_ROSTER_ID))
					emit openSubscription();
				else
					emit chat(id);
			}
		}
	}
}

void RosterTreeView::contextMenu(const QPoint &position)
{
	QModelIndex index = indexAt(position);
	if (!index.isValid())
		return;
	
	QStandardItem *item = m_rosterModel->nodeFromProxyIndex(index);
	if (!item)
		return;

	// hide card first
	emit hideCard();

	RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(item->data(RosterModel::TypeRole).toInt());
	if (itemType == RosterModel::Roster)
	{
		// roster menu
		QStandardItem *rosterItem = item;
		QString id = rosterItem->data(RosterModel::IdRole).toString();

		// set action data here
		m_chat->setData(id);
		QStringList multiSendParam;
		multiSendParam << "contact" << id;
		m_msgMultSend->setData(multiSendParam);
		m_sendMail->setData(id);
		m_viewPastChats->setData(id);
		m_viewMaterial->setData(id);
		m_removeRoster->setData(id);
		bool enabled = isLogined();
		m_removeRoster->setEnabled(enabled);
		m_addBlack->setData(id);
		m_removeBlack->setData(id);
		if (m_avatarTypeSwitchAction)
		{
			if (m_avatarType == RosterDelegate::BigAvatar)
				m_avatarTypeSwitchAction->setText(tr("Switch to Small Avatar"));
			else
				m_avatarTypeSwitchAction->setText(tr("Switch to Big Avatar"));
		}

		if (id != QString(SUBSCRIPTION_ROSTER_ID) && id != Account::instance()->phoneFullId())
		{
			QMenu menu(this);

			// add all menu actions
			menu.addAction(m_chat);
			menu.setDefaultAction(m_chat);
			menu.addAction(m_msgMultSend);
			menu.addAction(m_sendMail);
			menu.addSeparator();
			menu.addAction(m_viewPastChats);
			menu.addAction(m_viewMaterial);
			menu.addSeparator();
			menu.addAction(m_removeRoster);
			menu.addSeparator();
			if (!qPmApp->getModelManager()->isInBlackList(id))
				menu.addAction(m_addBlack);
			else
				menu.addAction(m_removeBlack);
			menu.addAction(m_manageBlack);
			menu.addSeparator();
			menu.addAction(m_avatarTypeSwitchAction);

			// show menu
			menu.exec(QCursor::pos());
		}
	}
}

bool RosterTreeView::event(QEvent *event)
{
	if ((event->type() == QEvent::HoverLeave)) 
	{
		// if don't has focus, hide the card
		emit hideCard();
	}
	return QTreeView::event(event);
}

void RosterTreeView::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pt = event->pos();
	QModelIndex index = indexAt(pt);
	if (index.isValid())
	{
		RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(index.data(RosterModel::TypeRole).toInt());
		if (itemType == RosterModel::Roster)
		{
			QStandardItem *rosterItem = m_rosterModel->nodeFromProxyIndex(index);
			QString id = rosterItem->data(RosterModel::IdRole).toString();
			if (id != Account::instance()->phoneFullId() && id != QString(SUBSCRIPTION_ROSTER_ID))
			{
				QRect rect = visualRect(index);
				if (m_avatarType == RosterDelegate::BigAvatar)
				{
					if (pt.x() > (rect.left() + 5) && pt.x() < rect.left() + 5 + 42 // check if cursor is in the avatar area
						&& pt.y() > (rect.top()+8) && pt.y() < (rect.top()+58-8))
					{
						emit showCard(id, rect.top());
						QTreeView::mouseMoveEvent(event);
						return;
					}
					else if(pt.x() > (rect.left()+5+42+5) && pt.x() < (rect.right()-2) // other area show the signature
						&& pt.y() > (rect.top()+8) && pt.y() < (rect.top()+58-8))
					{
						QPoint pt(rect.left()+5+42+5, rect.top()+58);
						pt = mapToGlobal(pt);
						if (isHeaderHidden())
						{
							pt.setY(pt.y() - 22);
						}
						bean::DetailItem *rosterDetail = qPmApp->getModelManager()->detailItem(id);
						if (rosterDetail)
						{
							QString signature = rosterDetail->message();
							QString wrapSignature = TextUtil::wrapText(QToolTip::font(), signature, 180);
							QToolTip::showText(pt, wrapSignature);
						}

						QTreeView::mouseMoveEvent(event);
						return;
					}
				}
				else // RosterDelegate::SmallAvatar
				{
					if (pt.x() > (rect.left() + 16) && pt.x() < rect.left() + 16 + 24 // check if cursor is in the avatar area
						&& pt.y() > (rect.top()+5) && pt.y() < (rect.top()+36-5))
					{
						emit showCard(id, rect.top());
						QTreeView::mouseMoveEvent(event);
						return;
					}
				}
			}
		}
	}

	// if index is invalid, hide card
	emit hideCard();

	QTreeView::mouseMoveEvent(event);
}

void RosterTreeView::mouseReleaseEvent(QMouseEvent *event)
{
	m_clickedIndex = QModelIndex();
	if (event->button() == Qt::LeftButton)
	{
		m_clickedIndex = indexAt(event->pos());
	}
	QTreeView::mouseReleaseEvent(event);
}

void RosterTreeView::onRosterSetFinished()
{
	expandAll();
}

void RosterTreeView::switchAvatarType()
{
	if (m_avatarType == RosterDelegate::BigAvatar)
		m_avatarType = RosterDelegate::SmallAvatar;
	else
		m_avatarType = RosterDelegate::BigAvatar;
	m_itemDelegate->setAvatarType(m_avatarType);
	Account::settings()->setRosterAvatarType((AccountSettings::RosterAvatarType)m_avatarType);
	
	// check if need to expand
	QStandardItem *selItem = selectedItem();
	QModelIndex itemIndex;
	if (selItem)
	{
		RosterProxyModel *rosterProxyModel = m_rosterModel->proxyModel();
		if (selItem->data(RosterModel::TypeRole) == RosterModel::Group)
		{
			QModelIndex groupIndex = selItem->index();
			itemIndex = selItem->index();
			if (rosterProxyModel)
			{
				groupIndex = rosterProxyModel->mapFromSource(groupIndex);
				itemIndex = rosterProxyModel->mapFromSource(itemIndex);
			}
		}
		else // roster
		{
			QModelIndex groupIndex = selItem->parent()->index();
			itemIndex = selItem->index();
			if (rosterProxyModel)
			{
				groupIndex = rosterProxyModel->mapFromSource(groupIndex);
				itemIndex = rosterProxyModel->mapFromSource(itemIndex);
			}
		}
	}

	// first collapse
	collapseAll();
	
	// then expand to that group
	expandAll();

	// make original index visible
	if (itemIndex.isValid())
		scrollTo(itemIndex, QAbstractItemView::PositionAtCenter);

	update();
}

void RosterTreeView::clicked(const QModelIndex &index)
{
	Q_UNUSED(index);
}

void RosterTreeView::setSkin()
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

bool RosterTreeView::isLogined()
{
	return qPmApp->GetLoginMgr()->isLogined();
}

bool RosterTreeView::checkLogined(const QString &operation)
{
	if (!isLogined())
	{
		QString prompt = tr("You are offline, can't %1").arg(operation);
		PMessageBox::information(this, tr("Tip"), prompt);
		return false;
	}
	return true;
}
