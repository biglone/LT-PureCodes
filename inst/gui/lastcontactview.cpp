#include "lastcontactview.h"
#include "lastcontactitemdelegate.h"
#include "model/lastcontactmodeldef.h"
#include "model/lastcontactitemdef.h"
#include <QEvent>
#include <QStandardItem>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include <QSortFilterProxyModel>
#include <QToolTip>
#include "util/TextUtil.h"
#include "flickerhelper.h"
#include "PmApp.h"
#include <QActionGroup>
#include "Account.h"
#include "model/ModelManager.h"
#include "model/rostermodeldef.h"
#include <QPainter>
#include "interphonemanager.h"
#include "pmessagebox.h"
#include "Constants.h"
#include "configmanager.h"
#include "iospushmanager.h"

LastContactView::LastContactView(QWidget *parent) : QListView(parent), m_lastContactModel(0)
{
	m_itemDelegate = new LastContactItemDelegate(this);
	setItemDelegate(m_itemDelegate);

	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setEditTriggers(NoEditTriggers);

	setDragEnabled(false);
	setAcceptDrops(false);
	setDropIndicatorShown(false);

	setupActions();
	setSkin();

	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked(const QModelIndex&)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(const QPoint&)));
}

void LastContactView::setLastContactModel(LastContactModel *lastContactModel)
{
	if (m_lastContactModel)
	{
		disconnect(this, SIGNAL(removeAllChats()), m_lastContactModel, SLOT(onRemoveAllRecords()));
	}

	m_lastContactModel = lastContactModel;
	m_itemDelegate->setLastContactModel(lastContactModel);
	if (m_lastContactModel->proxyModel())
		setModel(m_lastContactModel->proxyModel());
	else
		setModel(m_lastContactModel);

	if (m_lastContactModel)
	{
		connect(this, SIGNAL(removeAllChats()), m_lastContactModel, SLOT(onRemoveAllRecords()));
	}
}

void LastContactView::setFlickerHelper(FlickerHelper *flickerHelper)
{
	m_itemDelegate->setFlickerHelper(flickerHelper);
}

bool LastContactView::containsFlickerItem(const QString &id, bean::MessageType msgType) const
{
	if (!m_lastContactModel)
		return false;

	if (msgType == bean::Message_Invalid)
		return false;

	if (msgType == bean::Message_Chat)
		return m_lastContactModel->containsContact(id);

	if (msgType == bean::Message_GroupChat)
		return m_lastContactModel->containsMucGroup(id);

	if (msgType == bean::Message_DiscussChat)
		return m_lastContactModel->containsDiscuss(id);

	return false;
}

void LastContactView::doUpdate()
{
	update();
}

void LastContactView::doubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		LastContactItem *lastContactItem = m_lastContactModel->nodeFromProxyIndex(index);
		if (lastContactItem->itemType() == LastContactItem::LastContactTypeGroupMuc)
		{
			emit groupChat(lastContactItem->itemId());
		}
		else if (lastContactItem->itemType() == LastContactItem::LastContactTypeDiscuss)
		{
			emit discussChat(lastContactItem->itemId());
		}
		else if (lastContactItem->itemType() == LastContactItem::LastContactTypeMultiSend)
		{
			emit msgMultiSend(lastContactItem->multiSendMemebers(), lastContactItem->itemId());
		}
		else
		{
			emit chat(lastContactItem->itemId());
		}
	}
}

void LastContactView::groupSetting(QAction *action)
{
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];
	int silence = 0;

	AccountSettings::GroupMsgSettingType setting = AccountSettings::Tip;
	if (action == m_groupTip)
	{
		setting = AccountSettings::Tip;
	}
	else
	{
		setting = AccountSettings::UnTip;
		silence = 1;
	}

	QString groupType;
	bool iOSPush = false;
	if (typeStr == LastContactModel::TYPE_GROUPCHAT)
	{
		groupType = "group";
		iOSPush = true;
		// set conf3
		if (Account::settings()->groupMsgSetting(id) != setting )
		{
			Account::settings()->setGroupMsgSetting(id, setting);

			QStringList silenceList = Account::settings()->silenceList();
			ConfigManager *configManager = qPmApp->getConfigManager();
			configManager->setConfig3(silenceList);
			Account::settings()->setSilenceList(silenceList);
		}
	}
	else if (typeStr == LastContactModel::TYPE_DISCUSS)
	{
		groupType = "discuss";
		iOSPush = true;
		// set conf3
		if (Account::settings()->discussMsgSetting(id) != setting)
		{
			Account::settings()->setDiscussMsgSetting(id, setting);

			QStringList silenceList = Account::settings()->silenceList();
			ConfigManager *configManager = qPmApp->getConfigManager();
			configManager->setConfig3(silenceList);
			Account::settings()->setSilenceList(silenceList);
		}
	}

	if (iOSPush)
	{
		// ios push
		IOSPushManager *pIOSPushManager = qPmApp->getIOSPushManager();
		pIOSPushManager->pushForIOS(groupType, id, silence);
	}
}

void LastContactView::onStartInterphone(const QString &interphoneId)
{
	m_itemDelegate->setInterphoneState(interphoneId, LastContactItemDelegate::InterphoneNotIn);
	update();
}

void LastContactView::onFinishInterphone(const QString &interphoneId)
{
	m_itemDelegate->setInterphoneState(interphoneId, LastContactItemDelegate::InterphoneNone);
	update();
}

void LastContactView::onAddInterphone(bool ok, const QString &interphoneId)
{
	if (ok)
	{
		if (m_itemDelegate->interphoneState(interphoneId) == LastContactItemDelegate::InterphoneNotIn)
		{
			m_itemDelegate->setInterphoneState(interphoneId, LastContactItemDelegate::InterphoneIn);
			update();
		}
	}
}

void LastContactView::onQuitInterphone(bool ok, const QString &interphoneId)
{
	if (ok)
	{
		if (m_itemDelegate->interphoneState(interphoneId) == LastContactItemDelegate::InterphoneIn)
		{
			m_itemDelegate->setInterphoneState(interphoneId, LastContactItemDelegate::InterphoneNotIn);
			update();
		}
	}
}

void LastContactView::onInterphoneCleared()
{
	m_itemDelegate->clearInterphoneStates();
}

void LastContactView::addBlack()
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
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Add blocked list"), 
		tr("Are you sure to add %1 to blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit addBlack(id);
}

void LastContactView::removeBlack()
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
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Remove out of blocked list"), 
		tr("Are you sure to remove %1 out of blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit removeBlack(id);
}

bool LastContactView::event(QEvent *event)
{
	if ((event->type() == QEvent::HoverLeave)) 
	{
		// if don't has focus, hide the card
		emit hideCard();
	}
	return QListView::event(event);
}

void LastContactView::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pt = event->pos();
	QModelIndex index = indexAt(pt);
	if (index.isValid())
	{
		QRect rect = visualRect(index);

		// show card
		LastContactItem::LastContactItemType itemType = static_cast<LastContactItem::LastContactItemType>(index.data(LastContactItem::LastContactTypeRole).toInt());
		if (itemType == LastContactItem::LastContactTypeContact)
		{
			LastContactItem *contactItem = m_lastContactModel->nodeFromProxyIndex(index);
			QString id = contactItem->itemId();
			if (id != Account::instance()->phoneFullId() && id != QString(SUBSCRIPTION_ROSTER_ID))
			{
				if (pt.x() > (rect.left() + 5) && pt.x() < rect.left() + 5 + 42 // check if cursor is in the avatar area
					&& pt.y() > (rect.top()+8) && pt.y() < (rect.top()+58-8))
				{
					emit showCard(id, rect.top());
					QListView::mouseMoveEvent(event);
					return;
				}
			}
		}

		// other area show last message
		if (pt.x() > (rect.left()+5+42+5) && pt.x() < (rect.right()-2) 
			&& pt.y() > (rect.top()+30) && pt.y() < (rect.top()+58-2))
		{
			QPoint ptToolTip(rect.left()+5+42+5, rect.top()+58);
			ptToolTip = mapToGlobal(ptToolTip);
			ptToolTip.setY(ptToolTip.y() - 22);
			LastContactItem *contactItem = m_lastContactModel->nodeFromProxyIndex(index);
			if (contactItem)
			{
				QString lastMessage = contactItem->lastBody();
				if (lastMessage.length() > 100)
				{
					lastMessage = lastMessage.left(100);
					lastMessage.append("...");
				}
				QString wrapLastMessage = TextUtil::wrapText(QToolTip::font(), lastMessage, 180);
				QToolTip::showText(ptToolTip, wrapLastMessage);
			}
			QListView::mouseMoveEvent(event);
			return;
		}
	}

	// if index is invalid, hide card
	emit hideCard();

	QListView::mouseMoveEvent(event);
}

void LastContactView::paintEvent(QPaintEvent *event)
{
	if (m_lastContactModel && m_lastContactModel->rowCount() > 0)
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

LastContactItem *LastContactView::selectedItem()
{
	if(selectedIndexes().size() > 0 && selectedIndexes().at(0).isValid())
	{
		return m_lastContactModel->nodeFromProxyIndex(selectedIndexes().at(0));
	}
	else
	{
		return 0;
	}
}


void LastContactView::chat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 3)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_GROUPCHAT)
	{
		emit groupChat(id);
	}
	else if (typeStr == LastContactModel::TYPE_DISCUSS)
	{
		emit discussChat(id);
	}
	else if (typeStr == LastContactModel::TYPE_MULTISEND)
	{
		QString memberPara = paras[2];
		QStringList members = memberPara.split(",");
		emit msgMultiSend(members, id);
	}
	else
	{
		emit chat(id);
	}
}

void LastContactView::sendMail()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_CHAT)
	{
		emit sendMail(id);
	}
}

void LastContactView::viewPastChat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_GROUPCHAT)
	{
		emit groupViewPastChat(id);
	}
	else if (typeStr == LastContactModel::TYPE_DISCUSS)
	{
		emit discussViewPastChat(id);
	}
	else 
	{
		emit viewPastChat(id);
	}
}

void LastContactView::sendFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_GROUPCHAT)
	{
		emit groupSendFile(id, QStringList());
	}
	else if (typeStr == LastContactModel::TYPE_DISCUSS)
	{
		emit discussSendFile(id, QStringList());
	}
	else
	{
		emit sendFile(id, QStringList());
	}
}

void LastContactView::viewMaterial()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_GROUPCHAT)
	{
		emit groupViewMaterial(id);
	}
	else if (typeStr == LastContactModel::TYPE_DISCUSS)
	{
		emit discussViewMaterial(id);
	}
	else
	{
		emit viewMaterial(id);
	}
}

void LastContactView::removeChat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_GROUPCHAT)
	{
		m_lastContactModel->onRemoveGroupChat(id);
	}
	else if (typeStr == LastContactModel::TYPE_DISCUSS)
	{
		m_lastContactModel->onRemoveDiscuss(id);
	}
	else if (typeStr == LastContactModel::TYPE_MULTISEND)
	{
		m_lastContactModel->onRemoveMultiSend(id);
	}
	else
	{
		m_lastContactModel->onRemoveChat(id);
	}
}

void LastContactView::addFriend()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QStringList paras = action->data().toStringList();
	if (paras.count() != 2)
		return;

	QString typeStr = paras[0];
	QString id = paras[1];

	if (typeStr == LastContactModel::TYPE_CHAT)
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QString name = modelManager->userName(id);
		emit addFriendRequest(id, name);
	}
}

void LastContactView::contextMenu(const QPoint &position)
{
	QModelIndex index = indexAt(position);
	if (index.isValid())
	{
		// hide card first
		emit hideCard();

		// set action data
		LastContactItem *lastContactItem = m_lastContactModel->nodeFromProxyIndex(index);
		if (!lastContactItem)
			return;

		QString id = lastContactItem->itemId();
		QString typeStr = LastContactModel::typeToString(lastContactItem->itemType());
		QStringList paras;
		paras << typeStr << id;
		QStringList chatParas = paras;
		QStringList members = lastContactItem->multiSendMemebers();
		chatParas.append(members.join(","));
		
		m_chatAction->setText(tr("Send Message"));
		m_viewPastChatsAction->setText(tr("View Chat History"));
		m_viewMaterialAction->setText(tr("View Contact Profile"));
		m_chatAction->setData(chatParas);
		m_sendMailAction->setData(paras);
		// m_sendFileAction->setData(paras);
		m_viewPastChatsAction->setData(paras);
		m_viewMaterialAction->setData(paras);
		m_addFriendAction->setData(paras);
		m_removeChatAction->setData(paras);
		m_groupTip->setData(paras);
		m_groupUntip->setData(paras);

		// show menu
		QMenu menu(this);

		if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact &&
			id == Account::instance()->phoneFullId()) // my device phone
		{
			menu.addAction(m_chatAction);
		}
		else if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact &&
			id == QString(SUBSCRIPTION_ROSTER_ID)) // subscription
		{
			menu.addAction(m_chatAction);
		}
		else
		{
			menu.addAction(m_chatAction);
			menu.setDefaultAction(m_chatAction);
			if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact)
				menu.addAction(m_sendMailAction);

			if (lastContactItem->itemType() != LastContactItem::LastContactTypeMultiSend)
			{
				// menu.addAction(m_multiMailAction);
				// menu.addAction(m_sendFileAction);
				menu.addSeparator();
				menu.addAction(m_viewPastChatsAction);
			}

			if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact)
			{
				menu.addAction(m_viewMaterialAction);

				ModelManager *modelManager = qPmApp->getModelManager();
				RosterModel *rosterModel = modelManager->rosterModel();
				if (rosterModel && (!rosterModel->isFriend(id)))
				{
					menu.addSeparator();
					menu.addAction(m_addFriendAction);
				}
			}

			if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact)
			{
				m_addBlackAction->setData(id);
				m_removeBlackAction->setData(id);

				menu.addSeparator();
				if (!qPmApp->getModelManager()->isInBlackList(id))
					menu.addAction(m_addBlackAction);
				else
					menu.addAction(m_removeBlackAction);
				menu.addAction(m_manageBlackAction);
			}

			if (lastContactItem->itemType() == LastContactItem::LastContactTypeGroupMuc)
			{
				menu.addSeparator();

				QMenu *msgSettingMenu = menu.addMenu(tr("Group message setting"));
				msgSettingMenu->addAction(m_groupTip);
				msgSettingMenu->addAction(m_groupUntip);

				AccountSettings::GroupMsgSettingType setting = Account::settings()->groupMsgSetting(id);
				if (setting == AccountSettings::Tip)
					m_groupTip->setChecked(true);
				else
					m_groupUntip->setChecked(true);
			}
			else if (lastContactItem->itemType() == LastContactItem::LastContactTypeDiscuss)
			{
				menu.addSeparator();

				QMenu *msgSettingMenu = menu.addMenu(tr("Discuss message setting"));
				msgSettingMenu->addAction(m_groupTip);
				msgSettingMenu->addAction(m_groupUntip);

				AccountSettings::GroupMsgSettingType setting = Account::settings()->discussMsgSetting(id);
				if (setting == AccountSettings::Tip)
					m_groupTip->setChecked(true);
				else
					m_groupUntip->setChecked(true);
			}
		}

		menu.addSeparator();
		menu.addAction(m_removeChatAction);
		menu.addAction(m_removeAllChatsAction);

		menu.exec(QCursor::pos());
	}
	else
	{
		// show menu
		if (m_lastContactModel->rowCount() > 0)
		{
			QMenu menu(this);
			menu.addAction(m_removeAllChatsAction);
			menu.exec(QCursor::pos());
		}
	}
}

void LastContactView::setupActions()
{
	m_chatAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chatAction, SIGNAL(triggered()), SLOT(chat()));

	m_sendMailAction = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Send Mail"), this);
	connect(m_sendMailAction, SIGNAL(triggered()), SLOT(sendMail()));

	m_multiMailAction = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Mail to Others"), this);
	connect(m_multiMailAction, SIGNAL(triggered()), SIGNAL(multiMail()));

	m_sendFileAction = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Send File"), this);
	connect(m_sendFileAction, SIGNAL(triggered()), SLOT(sendFile()));

	m_viewPastChatsAction = new QAction(QIcon(":/images/Icon_102.png"), tr("View Chat History"), this);
	connect(m_viewPastChatsAction, SIGNAL(triggered()), SLOT(viewPastChat()));

	m_viewMaterialAction = new QAction(/*QIcon(":/images/icon.png"),*/ tr("View Contact Profile"), this);
	connect(m_viewMaterialAction, SIGNAL(triggered()), SLOT(viewMaterial()));

	m_removeChatAction = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Remove Chat"), this);
	connect(m_removeChatAction, SIGNAL(triggered()), SLOT(removeChat()));

	m_removeAllChatsAction = new QAction(tr("Clear Chats"), this);
	connect(m_removeAllChatsAction, SIGNAL(triggered()), SIGNAL(removeAllChats()));

	m_addFriendAction = new QAction(tr("Add Friends"), this);
	connect(m_addFriendAction, SIGNAL(triggered()), SLOT(addFriend()));

	m_groupActionGroup = new QActionGroup(this);
	m_groupTip = new QAction(tr("Tip normal"), m_groupActionGroup);
	m_groupTip->setCheckable(true);
	m_groupUntip = new QAction(tr("No tip"), m_groupActionGroup);
	m_groupUntip->setCheckable(true);
	m_groupActionGroup->addAction(m_groupTip);
	m_groupActionGroup->addAction(m_groupUntip);
	m_groupActionGroup->setExclusive(true);
	m_groupTip->setChecked(true);
	connect(m_groupActionGroup, SIGNAL(triggered(QAction *)), SLOT(groupSetting(QAction *)));

	m_addBlackAction = new QAction(tr("Add Blocked List"), this);
	connect(m_addBlackAction, SIGNAL(triggered()), SLOT(addBlack()));

	m_removeBlackAction = new QAction(tr("Remove out of Blocked List"), this);
	connect(m_removeBlackAction, SIGNAL(triggered()), SLOT(removeBlack()));

	m_manageBlackAction = new QAction(tr("View Blocked List"), this);
	connect(m_manageBlackAction, SIGNAL(triggered()), SIGNAL(manageBlack()));
}

void LastContactView::setSkin()
{
	setStyleSheet("QListView{"
		"border: none;"
		"background-color: transparent;"
		"font-size: 10.5pt;"
		"}"
		);
}
