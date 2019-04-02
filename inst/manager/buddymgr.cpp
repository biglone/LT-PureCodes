#include <QtCore>
#include <QDebug>

#include <QPair>

#include "bean/ChatMessageExt.h"

#include "protocol/MessageNotification.h"

#include "pmclient/PmClient.h"

#include "cttk/Base.h"

#include "chatdialog.h"
#include "groupdialog.h"
#include "DiscussDialog.h"

#include "Constants.h"

#include "common/datetime.h"
#include "util/PlayBeep.h"
#include "util/FileUtil.h"
#include "util/FileDialog.h"
#include "util/ImageUtil.h"

#include "model/ModelManager.h"
#include "model/lastcontactmodeldef.h"

#include "filetransfer/attachtransfermgr.h"
#include "login/Account.h"

#include "db/MessageDB.h"

#include "MessageProcessor.h"

#include "PmApp.h"

#include "buddymgr.h"

#include "model/unreadmsgmodel.h"
#include "model/unreadmsgitem.h"
#include "manager/presencemanager.h"
#include "MessageDBStore.h"
#include "widgetmanager.h"
#include "pmessagebox.h"
#include "logger/logger.h"
#include "model/DiscussModeldef.h"
#include "manager/sendmessagemanager.h"
#include "db/SendMessageDB.h"
#include "interphonemanager.h"
#include "subscriptionmsgmanager.h"
#include <QDesktopWidget>
#include "guiconstants.h"
#include "unionchatdialog.h"
#include "secretmanager.h"
#include "maxtsmanager.h"
#include "shakedialog.h"
#include "manager/messagewithdrawmanager.h"
#include "selectchatdialog.h"
#include "pmessagelinetip.h"
#include "rtc/rtcsessionmanager.h"
#include "rtc/rtcsession.h"

static const QString kUpload =   "upload";
static const QString kDownload = "download";

CBuddyMgr::CBuddyMgr(UnreadMsgModel *unreadMsgModel, QObject *parent)
: QObject(parent), m_unreadMsgModel(unreadMsgModel), m_offlineRecved(false)
{
}

CBuddyMgr::~CBuddyMgr()
{
	Release();
}

void CBuddyMgr::Init()
{
	m_pSendMessageDB.reset(new DB::SendMessageDB("CBuddyMgr"));

	// BuddyMgr
	connect(qPmApp->getMessageProcessor(), SIGNAL(receiveMessage(bean::MessageBody)), this, SLOT(slot_receiveMessage(bean::MessageBody)), Qt::UniqueConnection);
	connect(qPmApp->getRtcSessionManager(), SIGNAL(recvInvite(QString, QString)), this, SLOT(onSessionRecvInvite(QString, QString)), Qt::UniqueConnection);

	m_unionChatDialog = new UnionChatDialog();

	m_readRecvSecretMessages.clear();

	m_unreadSendSecretMessages.clear();
	m_unreadRecvSecretMessages.clear();
}

void CBuddyMgr::Release()
{
	m_withdrawMessages.clear();
	m_withdrawedMessages.clear();

	m_readRecvSecretMessages.clear();

	m_unreadSendSecretMessages.clear();
	m_unreadRecvSecretMessages.clear();

	// close union chat dialog
	if (!m_unionChatDialog.isNull())
	{
		m_unionChatDialog.data()->close();
	}

	// clear all chat dialogs
	QString sKey;
	foreach(sKey, m_mapDialogs.keys())
	{
		QPointer<ChatBaseDialog> pDialog = m_mapDialogs[sKey];
		if (pDialog)
			pDialog.data()->closeChat();
	}
	m_mapDialogs.clear();

	// clear all message send dialog
	foreach(sKey, m_mapMsgMultiSendDlgs.keys())
	{
		QPointer<MsgMultiSendDlg> pDialog = m_mapMsgMultiSendDlgs[sKey];
		if (pDialog)
			pDialog.data()->close();
	}
	m_mapMsgMultiSendDlgs.clear();

	m_mutexSendMsgCache.lock();
	m_listSendMsgCache.clear();
	m_mutexSendMsgCache.unlock();

	m_pSendMessageDB.reset();
}

bool CBuddyMgr::saveImage(const QString &imagePath, QWidget *imageViewerWindow)
{
	QString sPath = imagePath;
	if (sPath.isEmpty())
		return false;

	QWidget *window = imageViewerWindow;
	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(window, tr("Tip"), tr("This file does not exist, may be deleted or moved to other place"));
		return false;
	}

	QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
		saveDir = accountSettings->getCurDir();

	QFileInfo fi(sPath);
	QDateTime dt = QDateTime::currentDateTime();
	QString currentTime = dt.toString("yyyy_MM_dd_hh_mm_ss_zzz");
	QString saveName = QString("%1/%2.%3").arg(saveDir).arg(currentTime).arg(fi.suffix());
	QString newFileName = FileDialog::getImageSaveFileName(window, tr("Save Image"), saveName);
	if (newFileName.isEmpty())
		return false;

	QFileInfo newFi(newFileName);
	if (fi == newFi)
	{
		qWarning() << Q_FUNC_INFO << "save image to same name: " << sPath << newFileName;
		return false;
	}

	if (accountSettings)
	{
		accountSettings->setCurDir(newFi.absoluteDir().absolutePath());
	}

	if (!ImageUtil::saveImage(sPath, newFileName))
	{
		qWarning() << Q_FUNC_INFO << "save image failed: " << sPath << newFileName;
		return false;
	}

	return true;
}

CChatDialog* CBuddyMgr::openChat(const QString& rsUid)
{
	if (rsUid == Account::instance()->id())
		return 0;

	/*
	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasUserItem(rsUid))
		return 0;
	*/

	CChatDialog *chatDialog = 0;
	QPointer<ChatBaseDialog> pDialog = slot_open_dialog(bean::Message_Chat, rsUid);
	if (pDialog.data())
		chatDialog = (CChatDialog *)(pDialog.data());
	return chatDialog;
}

CGroupDialog* CBuddyMgr::openGroupChat(const QString& rsUid)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasGroupItem(rsUid))
		return 0;

	CGroupDialog *groupDialog = 0;
	QPointer<ChatBaseDialog> pDialog = slot_open_dialog(bean::Message_GroupChat, rsUid);
	if (pDialog.data())
		groupDialog = (CGroupDialog *)(pDialog.data());
	return groupDialog;
}

DiscussDialog* CBuddyMgr::openDiscussChat( const QString& rsId )
{
	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasDiscussItem(rsId))
		return 0;

	DiscussDialog *discusDialog = 0;
	QPointer<ChatBaseDialog> pDialog = slot_open_dialog(bean::Message_DiscussChat, rsId);
	if (pDialog.data())
		discusDialog = (DiscussDialog *)(pDialog.data());
	return discusDialog;
}

void CBuddyMgr::openAllUnreadChats(const QList<int> &msgTypes, const QStringList &ids)
{
	QPointer<ChatBaseDialog> firstDialog;
	for (int i = msgTypes.count()-1; i >= 0; --i)
	{
		int msgType = msgTypes[i];
		QString id = ids[i];
		QPointer<ChatBaseDialog> dialog = slot_open_dialog(msgType, id, false);
		if (!dialog.isNull())
		{
			firstDialog = dialog;
		}
	}

	if (!firstDialog.isNull())
	{
		// set current chat
		m_unionChatDialog.data()->setCurrentChat(firstDialog.data());
	}
}

MsgMultiSendDlg* CBuddyMgr::openMsgMultiSend(const QString &id, const QStringList &members /*= QStringList()*/, bool checkMember /*= false*/)
{
	QPointer<MsgMultiSendDlg> msgMultiSendDlg;

	// find same id first
	if (!id.isEmpty() && m_mapMsgMultiSendDlgs.contains(id))
	{
		msgMultiSendDlg = m_mapMsgMultiSendDlgs[id];
	}

	// find same members
	bool newCreate = false;
	if (msgMultiSendDlg.isNull())
	{
		QPointer<MsgMultiSendDlg> curDlg;
		foreach (QString key, m_mapMsgMultiSendDlgs.keys())
		{
			curDlg = m_mapMsgMultiSendDlgs[key];
			if (!curDlg.isNull())
			{
				if (curDlg.data()->hasSameMembers(members))
				{
					msgMultiSendDlg = curDlg;
					break;
				}
			}
		}

		if (msgMultiSendDlg.isNull())
		{
			msgMultiSendDlg = new MsgMultiSendDlg(members, id);
			m_mapMsgMultiSendDlgs[msgMultiSendDlg.data()->id()] = msgMultiSendDlg;
			newCreate = true;
		}
	}

	if (!msgMultiSendDlg.isNull() && checkMember)
	{
		msgMultiSendDlg.data()->checkToAddMember();
	}

	if (!msgMultiSendDlg.isNull())
	{
		if (m_unionChatDialog.isNull())
		{
			m_unionChatDialog = new UnionChatDialog();
		}

		if (newCreate)
		{
			m_unionChatDialog.data()->insertMultiChatAtTop(id, members, msgMultiSendDlg.data());
		}
		else
		{
			m_unionChatDialog.data()->setCurrentChat(msgMultiSendDlg.data());
		}

		WidgetManager::showActivateRaiseWindow(m_unionChatDialog.data());
	}

	return msgMultiSendDlg.data();
}

QPointer<ChatBaseDialog> CBuddyMgr::slot_open_dialog(int nMsgType, const QString& rsId, bool makeCurrent /*= true*/)
{
	QPointer<ChatBaseDialog> pDialog;
	if (rsId == QString(ROSTER_ADD_MESSAGE_ID) && nMsgType == bean::Message_Chat)
	{
		// remove this message
		if (m_unreadMsgModel->containsMsg(rsId, (bean::MessageType)nMsgType))
		{
			m_unreadMsgModel->takeMsg(rsId, (bean::MessageType)nMsgType);
		}

		// roster add message
		emit rosterAddMsgActivated();

		return pDialog;
	}

	if (rsId == QString(SUBSCRIPTION_ROSTER_ID) && nMsgType == bean::Message_Chat)
	{
		// remove this message
		if (m_unreadMsgModel->containsMsg(rsId, (bean::MessageType)nMsgType))
		{
			m_unreadMsgModel->takeMsg(rsId, (bean::MessageType)nMsgType);
		}

		emit openSubscriptionLastMsg();

		return pDialog;
	}

	do 
	{
		bool newCreate = false;
		pDialog = GetDialogOrCreate(rsId, bean::MessageType(nMsgType), newCreate);
		if (!pDialog)
		{
			qWarning("create dialog (%d, %s) failed.", nMsgType, qPrintable(rsId));
			break;
		}

		// take sync messages first
		QList<bean::MessageBody> msgs = takeCacheFirstSyncMessages((bean::MessageType)nMsgType, rsId);

		// add unread messages
		if (m_unreadMsgModel->containsMsg(rsId, (bean::MessageType)nMsgType))
		{
			msgs << m_unreadMsgModel->takeMsg(rsId, (bean::MessageType)nMsgType);
		}

		OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
		OfflineMsgManager::FromType fromType = OfflineMsgManager::messageType2OfflineFromType((bean::MessageType)nMsgType);

		if (newCreate)
		{
			AccountSettings *accountSettings = Account::settings();
			bool loadHistory = accountSettings->chatLoadHistory();
			if (nMsgType == bean::Message_Chat && rsId == Account::instance()->phoneFullId())
				loadHistory = true;

			int msgCount = msgs.count();
			int moreCount = msgCount;
			bool hasOffline = false;
			if (offlineMsgManager->containOfflineItem(fromType, rsId))
			{
				moreCount += offlineMsgManager->syncMsgCount(fromType, rsId);
				hasOffline = true;
			}
			
			if (msgCount > 0)
			{
				// append message first
				if (hasOffline)
				{
					foreach (bean::MessageBody msg, msgs)
					{
						pDialog.data()->onMessage(msg);
					}
				}
				else
				{
					// max load one page
					if (msgCount > kPageHistoryMessageCount)
						msgCount = kPageHistoryMessageCount;

					for (int i = 0; i < msgCount; ++i)
					{
						bean::MessageBody msg = msgs[msgs.count() - msgCount + i];
						pDialog.data()->onMessage(msg);
					}

					QString lastMsgTime = msgs[0].time();

					bean::MessageBodyList leftMessages = msgs.mid(0, msgs.count() - msgCount);
					pDialog.data()->setMoreMessages(leftMessages);

					// if more message is empty, set history time
					if (leftMessages.isEmpty())
					{
						pDialog.data()->setHistoryMsgEndTime(lastMsgTime);
						pDialog.data()->setHistoryMsgPageIndex(INT_MAX);
					}
				}
		
				// update more count
				moreCount -= msgCount;
				pDialog.data()->setMoreCount(moreCount);
				if (moreCount <= 0)
				{
					if (loadHistory)
					{
						// 处理第一次打开新被拉入的讨论组，会显示以上是历史消息的问题
						if (nMsgType == bean::Message_DiscussChat && m_newAddedDiscussIds.contains(rsId))
						{
							pDialog.data()->closeMoreMsgTip();
						}
						else
						{
							pDialog.data()->appendHistorySeparator();
							pDialog.data()->showMoreMsgTip();
						}
					}
					else
					{
						pDialog.data()->closeMoreMsgTip();
					}
				}
				else
				{
					pDialog.data()->showMoreMsgTip();
				}
			}
			else // msgCount <= 0
			{
				if (loadHistory)
				{
					/*
					pDialog.data()->appendHistorySeparator();
					*/

					// load history messages
					pDialog.data()->setHistoryMsgEndTime(CDateTime::currentDateTimeUtcString());
					pDialog.data()->setHistoryMsgPageIndex(INT_MAX);
					pDialog.data()->loadHistoryMessages(kPageHistoryMessageCount);
				}
			}

			// 处理第一次打开新被拉入的讨论组，会显示以上是历史消息的问题
			if (nMsgType == bean::Message_DiscussChat && m_newAddedDiscussIds.contains(rsId))
			{
				m_newAddedDiscussIds.removeAll(rsId);
			}
		}
		else
		{
			// append message directly
			foreach (bean::MessageBody msg, msgs)
			{
				pDialog.data()->onMessage(msg);
			}
		}

		if (m_unionChatDialog.isNull())
		{
			m_unionChatDialog = new UnionChatDialog();
		}

		if (newCreate)
		{
			m_unionChatDialog.data()->insertChatAtTop((bean::MessageType)nMsgType, rsId, pDialog.data(), makeCurrent);
		}
		else
		{
			if (makeCurrent)
				m_unionChatDialog.data()->setCurrentChat(pDialog.data());
		}

		WidgetManager::showActivateRaiseWindow(m_unionChatDialog.data());

	} while (0);

	return pDialog;
}

void CBuddyMgr::slot_sendMessage(const bean::MessageBody &body)
{
	bean::MessageExt ext = body.ext();
	bool storeMessage = ext.data(bean::EXT_DATA_HISTORY_NAME, true).toBool();
	bool checkFailed = ext.data(bean::EXT_DATA_CHECKFAILED_NAME, true).toBool();

	// 保存数据库，如果需要检测发送成功，则在发送成功之后再保存消息
	if (storeMessage && !checkFailed)
	{
		this->storeMessage(body);
	}

	// 修改最近联系人
	checkToAddLastContact(body);

	// 保存到发送表
	setSendMessage(body);

	qDebug() << QString("(%1 %2)CBuddyMgr::slot_sendMessage: %3").arg(body.to()).arg(body.sequence()).arg(body.logBody());

	// 发送
	if (body.attachs().isEmpty())
	{
		// 没有附件
		deliver(body);
	}
	else
	{
		// 有附件
		m_mutexSendMsgCache.lock();
		m_listSendMsgCache.append(body);
		m_mutexSendMsgCache.unlock();
	}
}

void CBuddyMgr::slot_receiveMessage(const bean::MessageBody& rBody, 
									bool history /*= false*/, 
									bool firstHistory /*= false*/, 
									bool historySyncMsg /*= false*/)
{
	qDebug() << Q_FUNC_INFO << rBody.messageType() << rBody.to() << rBody.stamp() << history << firstHistory << historySyncMsg << rBody.logBody();

	if (!qPmApp->GetLoginMgr()->isLogined() && !history)
	{
		qDebug() << Q_FUNC_INFO << "does not login, will get in offline messages";
		return;
	}

	bean::MessageBody copyBody;
	if (!preprocessReceivedMessage(rBody, history, copyBody))
		return;

	bean::MessageType msgType = copyBody.messageType();
	QString sId = copyBody.to();
	bool isSyncMessage = copyBody.sync();         // chat sync message from phone

	// add to last contact
	checkToAddLastContact(copyBody);

	// save this message to database
	checkToStoreMessage(copyBody);

	// check secret message state
	if (msgType == bean::Message_Chat && copyBody.ext().type() == bean::MessageExt_Secret)
	{
		if (isSyncMessage)
		{
			checkSendSecretMessageDestroy(copyBody.stamp(), sId); // sync message
			m_unreadSendSecretMessages.insert(copyBody.stamp(), sId);
		}
		else
		{
			checkRecvSecretMessageDestory(copyBody.stamp(), sId); // received message
			m_unreadRecvSecretMessages.insert(copyBody.stamp(), sId);
		}
	}

	QPointer<ChatBaseDialog> pDialog = GetDialogFromMap(sId, msgType);
	if (pDialog.isNull() && historySyncMsg) // sync message do not prompt unread messages
	{
		if (firstHistory)
		{
			cacheFirstSyncMessage(msgType, sId, copyBody);
		}
		return;
	}

	// get group setting and discuss setting
	AccountSettings::GroupMsgSettingType groupSetting = Account::settings()->groupMsgSetting(sId);
	AccountSettings::GroupMsgSettingType discussSetting = Account::settings()->discussMsgSetting(sId);
	bool hasDialogBefore = true;
	bool hasUnreadBefore = m_unreadMsgModel->containsMsg(sId, msgType);

	if (pDialog.isNull())
	{
		hasDialogBefore = false;

		switch (msgType)
		{
		case bean::Message_Chat:
			if (!(isSyncMessage && !history))
			{
				// not on-line chat sync message
				// add this message to unread message model
				if (!history || (history && firstHistory))
					m_unreadMsgModel->insertMsg(sId, msgType, copyBody, false);
				/*
				else if (history && firstHistory)
					m_unreadMsgModel->insertMsg(sId, msgType, copyBody, true);
				*/
				else 
					qDebug() << Q_FUNC_INFO << " history " << history << " firstHistory " << firstHistory;
			}
			break;
		case bean::Message_GroupChat:
		case bean::Message_DiscussChat:
			if (!(isSyncMessage && !history))
			{
				// not on-line chat sync message
				// check if this message is at me
				bool atSelf = false;
				if (copyBody.ext().type() == bean::MessageExt_At)
				{
					QString selfId = Account::instance()->id();
					QString atText = copyBody.ext().data("at").toString();
					QStringList atUids = atText.split(",");
					foreach (QString atUid, atUids)
					{
						atUid = atUid.trimmed();
						if (atUid.startsWith(selfId) && (atUid.indexOf(":") == selfId.length()))
						{
							atSelf = true;
							break;
						}
					}
				}

				int msgSetting = groupSetting;
				if (msgType == bean::Message_DiscussChat)
					msgSetting = discussSetting;

				// group at message always need add to unread message
				bool ignore = false;
				if (!atSelf && (msgSetting == AccountSettings::UnTip))
				{
					// Untip mean do not show unread message number in unread message box but on last contact page
					ignore = true;
					UnreadMsgItem *unreadMsgItem = m_unreadMsgModel->peekUnreadMsg(sId, msgType);
					if (unreadMsgItem)
					{
						ignore = unreadMsgItem->isIgnoreBefore();
					}
				}

				if (!history || (history && firstHistory))
					m_unreadMsgModel->insertMsg(sId, msgType, copyBody, false, ignore);
				/*
				else if (history && firstHistory)
					m_unreadMsgModel->insertMsg(sId, msgType, copyBody, true, ignore);
				*/
				else 
					qDebug() << Q_FUNC_INFO << " history " << history << " firstHistory " << firstHistory;
			}
			break;
		default:
			qDebug("%d message type invalid.", msgType);
			break;
		}

		if (isSyncMessage && !history)
		{
			// on-line chat sync message will make all messages read
			OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
			offlineMsgManager->clearOfflineMsgCount(OfflineMsgManager::messageType2OfflineFromType(msgType), sId);

			// change unread messages to sync messages
			QList<bean::MessageBody> unreadMsgs = m_unreadMsgModel->takeMsg(sId, msgType);
			cacheFirstSyncMessages(msgType, sId, unreadMsgs);
			cacheFirstSyncMessage(msgType, sId, copyBody);
		}
	}
	
	if (!pDialog.isNull())
	{
		// add message to dialog
		pDialog.data()->onMessage(copyBody, history, firstHistory);

		if (isSyncMessage && !history)
		{
			if (!m_unionChatDialog.isNull())
				m_unionChatDialog.data()->clearChatUnread(pDialog.data());
		}

		// flash task bar
		if (!m_unionChatDialog.isNull() && !isSyncMessage)
		{
			if (m_unionChatDialog.data()->hasChatWidget(pDialog.data()) &&
				(!history || (history && firstHistory)))
			{
				m_unionChatDialog.data()->flashChat(pDialog.data());
			}
		}
	}

	// deal with shake message
	if (msgType == bean::Message_Chat
		&& copyBody.ext().type() == bean::MessageExt_Shake
		&& (!isSyncMessage))
	{
		QDateTime msgDT = CDateTime::QDateTimeFromString(copyBody.time());
		msgDT.setTimeSpec(Qt::UTC);
		QDateTime curDT = CDateTime::currentDateTimeUtc();
		if (qAbs(msgDT.secsTo(curDT)) < 20) // within 20 seconds, need to shake
		{
			ShakeDialog::shake(sId);
		}
	}

	if (!isSyncMessage)
	{
		// play received beep
		AccountSettings::GroupMsgSettingType msgTipSetting = (msgType == bean::Message_GroupChat) ? groupSetting : discussSetting;
		checkToPlayBeep(msgType, sId, copyBody.ext().type(), hasDialogBefore, hasUnreadBefore, (int)msgTipSetting, history, firstHistory);
	}
}

void CBuddyMgr::slot_talkCmdStart(const QString& rsUid)
{
	QPointer<ChatBaseDialog> pDialog = slot_open_dialog(bean::Message_Chat, rsUid);
	if (!pDialog.isNull())
	{
		pDialog.data()->startVideo();
	}
}

void CBuddyMgr::slot_transfer_finish( const QString& uuid, const QString& type, int result, int msgType )
{
	Q_UNUSED(uuid);
	Q_UNUSED(type);
	Q_UNUSED(result);
	Q_UNUSED(msgType);

	if (type == kUpload)
	{
		// check message send
		QList<int> listRemoveMsg;
		int nIndex = 0;

		m_mutexSendMsgCache.lock();
		for (nIndex = 0; nIndex < m_listSendMsgCache.length(); ++nIndex)
		{
			bean::MessageBody& rMsgBody = m_listSendMsgCache[nIndex];

			rMsgBody.setTransferFinish(uuid, result);

			if (rMsgBody.canSend())
			{
				deliver(rMsgBody);
				listRemoveMsg.append(nIndex);
			}
		}

		// remove message
		qSort(listRemoveMsg.begin(), listRemoveMsg.end(), qGreater<int>());
		foreach (nIndex, listRemoveMsg)
		{
			m_listSendMsgCache.removeAt(nIndex);
		}
		m_mutexSendMsgCache.unlock();
	}
	else if (type == kDownload)
	{
		// update db
		storeAttachResult((bean::MessageType)msgType, uuid, result);
	}
}

void CBuddyMgr::slot_transfer_error( const QString& uuid, const QString& type, int result, const QString& error, int msgType )
{
	Q_UNUSED(error);
	Q_UNUSED(type);
	Q_UNUSED(result);
	Q_UNUSED(msgType);

	if (type == kUpload)
	{
		// check message send
		QList<int> listRemoveMsg;
		int nIndex = 0;
		bean::AttachItem::TransferResult eResult = bean::AttachItem::TransferResult(result);

		m_mutexSendMsgCache.lock();
		for (nIndex = 0; nIndex < m_listSendMsgCache.length(); ++nIndex)
		{
			bean::MessageBody& rMsgBody = m_listSendMsgCache[nIndex];
			if (rMsgBody.containAttach(uuid))
			{
				if (eResult == bean::AttachItem::Transfer_Cancel)
				{
					onSendMessageCanceled(rMsgBody.sequence()); // notify message canceled
				}
				else
				{
					onSendMessageFailed(rMsgBody.sequence()); // notify message failed
				}
				listRemoveMsg.append(nIndex);
			}
		}

		// remove message
		qSort(listRemoveMsg.begin(), listRemoveMsg.end(), qGreater<int>());
		foreach (nIndex, listRemoveMsg)
		{
			m_listSendMsgCache.removeAt(nIndex);
		}
		m_mutexSendMsgCache.unlock();
	}
	else if (type == kDownload)
	{
		m_mutexSendMsgCache.lock();
		for (int nIndex = 0; nIndex < m_listSendMsgCache.length(); ++nIndex)
		{
			bean::MessageBody& rMsgBody = m_listSendMsgCache[nIndex];
			rMsgBody.setTransferFinish(uuid, result);
		}
		m_mutexSendMsgCache.unlock();

		// update db
		storeAttachResult((bean::MessageType)msgType, uuid, result);
	}
}

QPointer<ChatBaseDialog> CBuddyMgr::GetDialogOrCreate(const QString& rsUid, bean::MessageType type, bool &create)
{
	create = false;
	QPointer<ChatBaseDialog> pDialog;
	do 
	{
		if (rsUid.isEmpty())
			break;

		QString sKey = GetMapKey(rsUid, type);
		pDialog = m_mapDialogs.value(sKey);
		if (!pDialog.isNull())
			break;

		// create dialog
		switch (type)
		{
		case bean::Message_Chat:
			{
				CChatDialog *chatDialog = new CChatDialog(rsUid);
				pDialog = chatDialog;
				emit chatDialogCreated(chatDialog);
			}
			break;
		case bean::Message_GroupChat:
			{
				CGroupDialog *groupDialog = new CGroupDialog(rsUid);
				pDialog = groupDialog;
				emit groupDialogCreated(groupDialog);
			}              
			break;
		case bean::Message_DiscussChat:
			{
				DiscussDialog *discussDialog = new DiscussDialog(rsUid);
				pDialog = discussDialog;
				emit discussDialogCreated(discussDialog);
			}
			break;
		default:
			qWarning() << Q_FUNC_INFO << "unknown message type:" << ((int)type) << rsUid;
			break;
		}

		if (!pDialog)
			break;

		// save to map
		m_mapDialogs[sKey] = pDialog;

		create = true;

	} while (0);

	return pDialog;
}

QPointer<ChatBaseDialog> CBuddyMgr::GetDialogFromMap(const QString& rsUid, bean::MessageType type/* = bean::Message_Chat*/)
{
	QString sKey = GetMapKey(rsUid, type);
	QPointer<ChatBaseDialog> pDialog;
	do 
	{
		if (!m_mapDialogs.contains(sKey))
			break;
		pDialog = m_mapDialogs[sKey];
	} while (0);

	return pDialog;
}

QString CBuddyMgr::GetMapKey(const QString& rsUid, bean::MessageType type)
{
	return QString (MAP_DIALOG_KEY_FORMAT).arg(int(type)).arg(MAP_KEY_SEPARATION).arg(rsUid);
}

bool CBuddyMgr::FromMapKey(const QString &mapKey, QString &rsUid, bean::MessageType &type)
{
	QStringList parts = mapKey.split(MAP_KEY_SEPARATION);
	if (parts.count() != 2)
		return false;

	type = (bean::MessageType)(parts[0].toInt());
	rsUid = parts[1];
	return true;
}

void CBuddyMgr::setSendMessage(const bean::MessageBody &body)
{
	// get & set sequence
	SendMessageManager *sendMessageManager = qPmApp->getSendMessageManager();
	QString seq = sendMessageManager->setMessage(body);
	bean::MessageBody &rBody = const_cast<bean::MessageBody &>(body);
	rBody.setSequence(seq);

	// to store this seq and message
	bean::MessageExt ext = body.ext();
	bool checkFailed = ext.data(bean::EXT_DATA_CHECKFAILED_NAME, true).toBool();
	if (checkFailed)
	{
		m_pSendMessageDB->storeMessage(seq, body);
	}
}

void CBuddyMgr::deliver(const bean::MessageBody &body)
{
	QString seq = body.sequence();
	if (seq.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "sequence is empty";
		return;
 	}

	foreach (bean::AttachItem attach, body.attachs())
	{
		if (!attach.source().isEmpty())
		{
			m_pSendMessageDB->storeAttachSource(attach.uuid(), attach.source());
		}
	}

	SendMessageManager *sendMessageManager = qPmApp->getSendMessageManager();
	bean::MessageExt ext = body.ext();
	bool checkFailed = ext.data(bean::EXT_DATA_CHECKFAILED_NAME, true).toBool();
	if (!sendMessageManager->deliver(seq, body) && checkFailed)
	{
		m_pSendMessageDB->removeMsgBySequence(seq);
	}
}

void CBuddyMgr::storeMessage(const bean::MessageBody &rBody)
{
	if (rBody.to() != QString(SUBSCRIPTION_ROSTER_ID))
	{
		qPmApp->setMaxMsgTs(rBody.stamp());
	}
	qPmApp->getMessageDBStore()->append(rBody);
}

void CBuddyMgr::storeAttachResult(bean::MessageType msgType, const QString &rsUuid, int nResult)
{
	qPmApp->getMessageDBStore()->append(msgType, rsUuid, nResult);
}

void CBuddyMgr::storeAttachName(bean::MessageType msgType, const QString &rsUuid, const QString &filePath)
{
	qPmApp->getMessageDBStore()->append(msgType, rsUuid, filePath);
}

bool CBuddyMgr::hasOffline(bean::MessageType msgType, const QString &id) const
{
	switch (msgType)
	{
	case bean::Message_Chat:
		return m_offlineUserIds.contains(id);
	case bean::Message_GroupChat:
		return m_offlineGroupIds.contains(id);
	case bean::Message_DiscussChat:
		return m_offlineDiscussIds.contains(id);
	}

	return false;
}

bool CBuddyMgr::hasFirstOffline() const
{
	if (m_offlineUserIds.values().contains(true))
		return true;

	if (m_offlineGroupIds.values().contains(true))
		return true;

	if (m_offlineDiscussIds.values().contains(true))
		return true;

	return false;
}

bool CBuddyMgr::hasFirstOffline(bean::MessageType msgType, const QString &id) const
{
	if (!m_offlineRecved)
		return true;

	switch (msgType)
	{
	case bean::Message_Chat:
		return m_offlineUserIds.value(id, false);
	case bean::Message_GroupChat:
		return m_offlineGroupIds.value(id, false);
	case bean::Message_DiscussChat:
		return m_offlineDiscussIds.value(id, false);
	}

	return false;
}

bool CBuddyMgr::clearOffline(bean::MessageType msgType, const QString &id)
{
	switch (msgType)
	{
	case bean::Message_Chat:
		return (m_offlineUserIds.remove(id) > 0);
	case bean::Message_GroupChat:
		return (m_offlineGroupIds.remove(id) > 0);
	case bean::Message_DiscussChat:
		return (m_offlineDiscussIds.remove(id) > 0);
	}
	
	return false;
}

void CBuddyMgr::clearOfflineMessages()
{
	m_firstSyncMessages.clear();
	m_offlineMsgCache.clear();
	m_offlineUserIds.clear();
	m_offlineGroupIds.clear();
	m_offlineDiscussIds.clear();
	m_offlineRecved = false;
	foreach (QPointer<ChatBaseDialog> pDialog, m_mapDialogs.values())
	{
		if (!pDialog.isNull())
		{
			pDialog.data()->closeMoreMsgTip();
		}
	}
}

bool CBuddyMgr::getHistoryMsg(bean::MessageType msgType, const QString &id)
{
	OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
	if (!offlineMsgManager)
		return false;

	OfflineMsgManager::FromType fromType = OfflineMsgManager::messageType2OfflineFromType(msgType);
	return offlineMsgManager->requestHistoryMsg(fromType, id);
}

bool CBuddyMgr::needCacheMsg(bean::MessageType msgType, const QString &id)
{
	if (!m_offlineRecved)
		return true;

	switch (msgType)
	{
	case bean::Message_Chat:
		return m_offlineUserIds.value(id, false);
	case bean::Message_GroupChat:
		return m_offlineGroupIds.value(id, false);
	case bean::Message_DiscussChat:
		return m_offlineDiscussIds.value(id, false);
	}

	return false;
}

bool CBuddyMgr::isOfflineReceived() const
{
	return m_offlineRecved;
}

bool CBuddyMgr::resendFailedMessage(const QString &seq, bean::MessageBody *pMsg)
{
	// get message from send message table
	bean::MessageBody msg = m_pSendMessageDB->getMessageBySequence(seq);
	if (msg.to().isEmpty())
	{
		return false;
	}

	// remove from send message table
	m_pSendMessageDB->removeMsgBySequence(seq);

	// send message
	QList<bean::AttachItem> attachs = msg.attachs();
	foreach (bean::AttachItem attach, attachs)
	{
		QString oldUuid = attach.uuid();
		QString newUuid = QUuid::createUuid().toString();
		newUuid = newUuid.mid(1, newUuid.length()-2);
		attach.setUuid(newUuid);

		QString oldBody = msg.body();
		QString newBody = oldBody.replace(oldUuid, newUuid);
		msg.setBody(newBody);
	}
	if (!attachs.isEmpty())
	{
		msg.setAttachs(attachs);
	}
	msg.setTime(CDateTime::currentDateTimeUtcString());
	slot_sendMessage(msg);

	*pMsg = msg;

	return true;
}

void CBuddyMgr::forwardMessage(bean::MessageType msgType, const QString &id, const bean::MessageBody &origMsg)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QString fromId = Account::instance()->id();
	QString fromName = modelManager->userName(fromId);
	QString toName;
	if (msgType == bean::Message_Chat)
	{
		toName = modelManager->userName(id);
	}
	else if (msgType == bean::Message_GroupChat)
	{
		toName = modelManager->groupName(id);
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		toName = modelManager->discussName(id);
	}

	QString newBody = origMsg.body();
	QList<bean::AttachItem> attachs;
	QList<bean::AttachItem> origAttachs = origMsg.attachs();
	for (int i = 0; i < origAttachs.count(); ++i)
	{
		bean::AttachItem origAttach = origAttachs[i];
		bean::AttachItem newAttach(origAttach.filepath());
		newAttach.setMessageType(msgType);
		QString newUuid = QUuid::createUuid().toString();
		newUuid = newUuid.mid(1, newUuid.length()-2);
		newAttach.setUuid(newUuid);
		newAttach.setTransferType(origAttach.transferType());
		newAttach.setFrom(fromId);
		newAttach.setPicWidth(origAttach.picWidth());
		newAttach.setPicHeight(origAttach.picHeight());
		attachs.append(newAttach);
		
		newBody = newBody.replace(origAttach.uuid(), newUuid);
	}

	bean::MessageBody msg = bean::MessageBodyFactory::createMessage(msgType);
	msg.setFrom(fromId);
	msg.setFromName(fromName);
	msg.setTo(id);
	msg.setToName(toName);
	msg.setSend(true);
	msg.setTime("");
	msg.setBody(newBody);
	msg.setAttachs(attachs);
	msg.setExt(bean::MessageExtFactory::create(bean::MessageExt_Chat));

	QPointer<ChatBaseDialog> dlg = GetDialogFromMap(id, msgType);
	if (dlg.isNull())
		dlg = slot_open_dialog(msgType, id, false);

	slot_sendMessage(msg);

	if (!dlg.isNull())
		dlg.data()->appendSendMessage(msg);
}

void CBuddyMgr::appendSessionMessage(const QString &id, const QString &msgText)
{
	bean::MessageBody body = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Session));
	body.setBody(msgText);
	body.setTo(id);
	QString name = qPmApp->getModelManager()->userName(id);
	body.setToName(name);
	body.setSend(false);
	slot_receiveMessage(body);
}

UnionChatDialog * CBuddyMgr::unionChatDialog() const
{
	return m_unionChatDialog.data();
}

void CBuddyMgr::checkSendSecretAcks()
{
	QMap<QString, QString> acks = m_pSendMessageDB->secretAcks();
	if (acks.isEmpty())
		return;

	// clear all acks
	m_pSendMessageDB->clearSecretAcks();

	// send all acks
	foreach (QString stamp, acks.keys())
	{
		QString uid = acks[stamp];
		sendSecretAck(uid, stamp);
	}
}

bool CBuddyMgr::isRecvSecretMessageRead(const QString &stamp, const QString &uid)
{
	bool read = false;
	
	if (stamp.isEmpty() || uid.isEmpty())
		return read;
	
	if (m_readRecvSecretMessages.contains(stamp))
	{
		if (m_readRecvSecretMessages[stamp] == uid)
			read = true;
	}

	return read;
}

void CBuddyMgr::checkRecvSecretMessageDestory(const QString &stamp, const QString &otherId)
{
	if (stamp.isEmpty() || otherId.isEmpty())
		return;

	SecretManager *secretManager = qPmApp->getSecretManager();
	if (secretManager)
		secretManager->requestSecretAck(stamp, Account::instance()->id(), otherId);
}

void CBuddyMgr::checkSendSecretMessageDestroy(const QString &stamp, const QString &uid)
{
	if (stamp.isEmpty() || uid.isEmpty())
		return;

	SecretManager *secretManager = qPmApp->getSecretManager();
	if (secretManager)
		secretManager->requestSecretAck(stamp, uid, Account::instance()->id());
}

void CBuddyMgr::checkRecvSecretMessageDestory()
{
	if (m_unreadRecvSecretMessages.isEmpty())
		return;

	SecretManager *secretManager = qPmApp->getSecretManager();
	if (secretManager)
	{
		QString stamp;
		QString uid;
		foreach (stamp, m_unreadRecvSecretMessages.keys())
		{
			uid = m_unreadRecvSecretMessages[stamp];
			secretManager->requestSecretAck(stamp, Account::instance()->id(), uid);
		}
	}
}

void CBuddyMgr::checkSendSecretMessageDestroy()
{
	if (m_unreadSendSecretMessages.isEmpty())
		return;

	SecretManager *secretManager = qPmApp->getSecretManager();
	if (secretManager)
	{
		QString stamp;
		QString uid;
		foreach (stamp, m_unreadSendSecretMessages.keys())
		{
			uid = m_unreadSendSecretMessages[stamp];
			secretManager->requestSecretAck(stamp, uid, Account::instance()->id());
		}
	}
}

void CBuddyMgr::onRecvSecretMessageRead(const QString &stamp, const QString &uid)
{
	if (stamp.isEmpty() || uid.isEmpty())
		return;

	// update message read state
	setMessageRead(stamp);

	insertReadRecvSecretMessage(stamp, uid);

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		// store to secret ack send table
		storeSecretAck(uid, stamp);
	}
	else
	{
		// send secret ack message
		sendSecretAck(uid, stamp);
	}
}

void CBuddyMgr::addWithdrawMessage(const QString &stamp, const bean::MessageBody &msg /*= bean::MessageBody()*/)
{
	m_withdrawMessages.insert(stamp, msg);
}

void CBuddyMgr::removeWithdrawMessage(const QString &stamp)
{
	m_withdrawMessages.remove(stamp);
}

bool CBuddyMgr::containWithdrawMessage(const QString &stamp)
{
	return m_withdrawMessages.contains(stamp);
}

bean::MessageBody CBuddyMgr::withdrawMessage(const QString &stamp)
{
	bean::MessageBody msg;
	if (m_withdrawMessages.contains(stamp))
		msg = m_withdrawMessages[stamp];
	return msg;
}

void CBuddyMgr::addNewAddedDiscussId(const QString &discussId)
{
	m_newAddedDiscussIds.append(discussId);
}

void CBuddyMgr::clearNewAddedDiscussIds()
{
	m_newAddedDiscussIds.clear();
}

void CBuddyMgr::onSessionRecvInvite( const QString &from, const QString &sid )
{
	if (qPmApp->getModelManager()->isInBlackList(from))
	{
		qDebug() << Q_FUNC_INFO << " recv session invite from black list: " << from;

		// if has session, close session
		rtcsession::Session *s = qPmApp->getRtcSessionManager()->session(sid);
		if (s)
		{
			s->close(rtcsession::Session::CloseOther, tr("You are in black list"));
		}
		return;
	}

	// show chat dialog
	QPointer<ChatBaseDialog> pDialog;

	pDialog = slot_open_dialog(bean::Message_Chat, from);

	pDialog.data()->onSessionInvite(sid);
}

void CBuddyMgr::onOfflineRecvOK()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		qWarning() << Q_FUNC_INFO << "was not logined";
		return;
	}

	// set offline received
	m_offlineRecved = true;
	
	QList<OfflineMsgManager::OfflineItem> offlineItems = qPmApp->getOfflineMsgManager()->offlineItems();
	if (offlineItems.isEmpty())
	{
		qDebug() << "offline received: no offline ids"; 
	}
	else
	{
		QString offlineText("offline received:");
		foreach (OfflineMsgManager::OfflineItem item, offlineItems)
		{
			if (item.m_type == OfflineMsgManager::User)
			{
				m_offlineUserIds[item.m_from] = true;
				offlineText.append(QString(" [user: %1, count: %2, offline-count: %3, ts: %4]")
					.arg(item.m_from).arg(item.m_count).arg(item.m_offlineCount).arg(item.m_ts));
			}
			else if (item.m_type == OfflineMsgManager::Group)
			{
				m_offlineGroupIds[item.m_from] = true;
				offlineText.append(QString(" [group: %1, count: %2, offline-count: %3, ts: %4]")
					.arg(item.m_from).arg(item.m_count).arg(item.m_offlineCount).arg(item.m_ts));
			}
			else if (item.m_type == OfflineMsgManager::Discuss)
			{
				m_offlineDiscussIds[item.m_from] = true;
				offlineText.append(QString(" [discuss: %1, count: %2, offline-count: %3, ts: %4]")
					.arg(item.m_from).arg(item.m_count).arg(item.m_offlineCount).arg(item.m_ts));
			}
		}

		qDebug() << offlineText;
	}
}

void CBuddyMgr::onHistoryMsgRecvOK(int fType, const QString &id, const bean::MessageBodyList &messages, bool offline)
{
	qDebug() << Q_FUNC_INFO << fType << id << messages.count() << offline;

	if (!m_offlineRecved)
		m_offlineRecved = true;

	OfflineMsgManager::FromType fromType = (OfflineMsgManager::FromType)fType;
	bean::MessageType messageType = OfflineMsgManager::offlineFromType2MessageType(fromType);

	if (!qPmApp->GetLoginMgr()->isLogined()) // not logined
	{
		qWarning() << Q_FUNC_INFO << "was not logined: " << fType << id;
		doHistoryMsgFailed(fType, id);
		return;
	}

	AccountSettings *accountSettings = Account::settings();
	bool loadHistory = accountSettings->chatLoadHistory();
	if (messageType == bean::Message_Chat && id == Account::instance()->phoneFullId())
		loadHistory = true;

	OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
	bool firstHistory = false;
	if (fromType == OfflineMsgManager::User)
	{
		if (hasOffline(bean::Message_Chat, id))
		{
			firstHistory = m_offlineUserIds[id];
			if (offlineMsgManager->containOfflineItem(OfflineMsgManager::User, id))
				m_offlineUserIds[id] = false;
			else
				clearOffline(bean::Message_Chat, id);
		}
	}
	else if (fromType == OfflineMsgManager::Group)
	{
		if (hasOffline(bean::Message_GroupChat, id))
		{
			firstHistory = m_offlineGroupIds[id];
			if (offlineMsgManager->containOfflineItem(OfflineMsgManager::Group, id))
				m_offlineGroupIds[id] = false;
			else
				clearOffline(bean::Message_GroupChat, id);
		}
	}
	else if (fromType == OfflineMsgManager::Discuss)
	{
		if (hasOffline(bean::Message_DiscussChat, id))
		{
			firstHistory = m_offlineDiscussIds[id];
			if (offlineMsgManager->containOfflineItem(OfflineMsgManager::Discuss, id))
				m_offlineDiscussIds[id] = false;
			else
				clearOffline(bean::Message_DiscussChat, id);
		}
	}

	// not first history must has dialog request, if this dialog is closed, do not deal with these messages
	QPointer<ChatBaseDialog> dlg = GetDialogFromMap(id, messageType);
	if (!firstHistory && dlg.isNull())
		return;

	// union all the message
	bean::MessageBodyList msgs;
	msgs << messages;
	if (firstHistory)
	{
		foreach (bean::MessageBody cacheMsg, m_offlineMsgCache)
		{
			if ((cacheMsg.messageType() == bean::Message_Chat && (fromType == OfflineMsgManager::User) && (cacheMsg.to() == id)) ||  // chat message
				((cacheMsg.messageType() == bean::Message_GroupChat) && (fromType == OfflineMsgManager::Group) && (cacheMsg.to() == id)) || // group chat message
				((cacheMsg.messageType() == bean::Message_DiscussChat) && (fromType == OfflineMsgManager::Discuss) && (cacheMsg.to() == id))) // discuss chat message
			{
				bool duplicate = false;
				foreach (bean::MessageBody offlineMsg, messages)
				{
					if (offlineMsg.stamp() == cacheMsg.stamp() && offlineMsg.body() == cacheMsg.body())
					{
						duplicate = true;
						break;
					}
				}
				if (!duplicate)
				{
					// cache message should not be store to history again, because the message is stored before
					bean::MessageExt ext = cacheMsg.ext();
					ext.setData(bean::EXT_DATA_HISTORY_NAME, false);     // not add to message history
					cacheMsg.setExt(ext);
					msgs << cacheMsg;
				}
			}
		}
	}

	// first history
	if (firstHistory)
	{
		// if has unread message last time, clear all unread messages
		if (dlg.isNull() && m_unreadMsgModel->containsMsg(id, messageType))
		{
			m_unreadMsgModel->takeMsg(id, messageType);
		}

		// sync messages need to cache, and not set to unread
		if (!offline && dlg.isNull())
		{
			msgs = msgs.mid(messages.count());
			for (int i = 0; i < messages.count(); i++)
			{
				bean::MessageBody msg = messages[i];
				slot_receiveMessage(msg, true, true, true);
			}
		}

		// if the dialog exists, need to append to end
		if (!dlg.isNull())
		{
			dlg.data()->clearMessages();

			int moreCount = msgs.count();
			if (offlineMsgManager->containOfflineItem(fromType, id))
				moreCount += offlineMsgManager->syncMsgCount(fromType, id);

			if (!msgs.isEmpty())
			{
				for (int j = 0; j < msgs.count(); j++)
				{
					bean::MessageBody msg = msgs[j];
					slot_receiveMessage(msg, true, true);
				}

				if (!hasFirstOffline()) // if first offline message finished, need to report ts
				{
					qPmApp->getMaxTsManager()->reportMaxTs();
				}

				// update more count
				moreCount -= msgs.count();
				dlg.data()->setMoreCount(moreCount);

				if (moreCount <= 0)
				{
					QString lastMsgTime = msgs[0].time();
					dlg.data()->setHistoryMsgEndTime(lastMsgTime);
					dlg.data()->setHistoryMsgPageIndex(INT_MAX);
				}
			}

			// deal with more message tip
			if (moreCount <= 0)
			{
				if (loadHistory)
				{
					dlg.data()->appendHistorySeparator();
					dlg.data()->showMoreMsgTip();
				}
			}
			else
			{
				dlg.data()->showMoreMsgTip();
			}
			
			return;
		}
	}

	// do receive history message, insert to the top
	if (!firstHistory)
	{
		for (int j = msgs.count()-1; j >= 0; j--)
		{
			bean::MessageBody msg = msgs[j];
			slot_receiveMessage(msg, true, false);
		}
	}
	else
	{
		for (int j = 0; j < msgs.count(); j++)
		{
			bean::MessageBody msg = msgs[j];
			slot_receiveMessage(msg, true, true);
		}
	}

	if (firstHistory)
	{
		if (!hasFirstOffline()) // if first offline message finished, need to report ts
		{
			qPmApp->getMaxTsManager()->reportMaxTs();
		}
	}

	// deal with more message tip
	if (!dlg.isNull())
	{
		// update more count
		int moreCount = dlg.data()->moreCount();
		moreCount -= msgs.count();
		dlg.data()->setMoreCount(moreCount);

		if (moreCount <= 0)
		{
			QString lastMsgTime;
			if (!msgs.isEmpty())
				lastMsgTime = msgs[0].time();
			dlg.data()->setHistoryMsgEndTime(lastMsgTime);
			dlg.data()->setHistoryMsgPageIndex(INT_MAX);
		}

		// deal with more message tip
		if (!firstHistory)
			dlg.data()->onMoreMsgFinished();

		// deal with more message tip
		if (moreCount <= 0)
		{
			if (loadHistory)
			{
				dlg.data()->appendHistorySeparator();
				dlg.data()->showMoreMsgTip();
			}
			else
				dlg.data()->closeMoreMsgTip();
		}
		else
		{
			dlg.data()->showMoreMsgTip();
		}
	}
}

void CBuddyMgr::onHistoryMsgRecvFailed(int fType, const QString &id)
{
	if (!m_offlineRecved)
		m_offlineRecved = true;

	qWarning() << Q_FUNC_INFO << fType << id;
	doHistoryMsgFailed(fType, id);
}

void CBuddyMgr::checkChatDlgValid(bool tip /*= true*/)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QStringList removedIds;
	foreach (QString mapKey, m_mapDialogs.keys())
	{
		bean::MessageType msgType = bean::Message_Chat;
		QString id;
		if (FromMapKey(mapKey, id, msgType))
		{
			switch (msgType)
			{
			case bean::Message_Chat:
				// do nothing
				break;
			case bean::Message_GroupChat:
				{
					if (!modelManager->hasGroupItem(id))
					{
						CGroupDialog *groupDialog = static_cast<CGroupDialog *>(m_mapDialogs[mapKey].data());
						if (groupDialog)
						{
							QString groupName = groupDialog->groupName();
							m_mapDialogs[mapKey].data()->closeChat();
							if (tip)
							{
								PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
									tr("Group '%1' has been deleted").arg(groupName), 
									QDialogButtonBox::Ok, tr("Tip"), 0);
								WidgetManager::showActivateRaiseWindow(pMB);
							}
						}
						removedIds.append(mapKey);
					}
				}
				break;
			case bean::Message_DiscussChat:
				{
					if (!modelManager->hasDiscussItem(id))
					{
						DiscussDialog *discussDialog = static_cast<DiscussDialog *>(m_mapDialogs[mapKey].data());
						if (discussDialog)
						{
							QString discussName = discussDialog->discussName();
							m_mapDialogs[mapKey].data()->closeChat();
							if (tip)
							{
								PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
									tr("Discuss '%1' has bean deleted, or you have been removed from this discuss").arg(discussName), 
									QDialogButtonBox::Ok, tr("Tip"), 0);
								WidgetManager::showActivateRaiseWindow(pMB);
							}
						}
						removedIds.append(mapKey);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	foreach (QString removedId, removedIds)
	{
		m_mapDialogs.remove(removedId);
	}
}

void CBuddyMgr::checkUnreadMsgValid()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QList<QPair<QString, bean::MessageType>> froms = m_unreadMsgModel->allUnreadMsgsFrom();
	for (int i = 0; i < froms.count(); ++i)
	{
		QPair<QString, bean::MessageType> item = froms[i];
		QString id = item.first;
		bean::MessageType msgType = item.second;
		if (msgType == bean::Message_GroupChat)
		{
			if (!modelManager->hasGroupItem(id))
			{
				m_unreadMsgModel->takeMsg(id, msgType);
			}
		}
		else if (msgType == bean::Message_DiscussChat)
		{
			if (!modelManager->hasDiscussItem(id))
			{
				m_unreadMsgModel->takeMsg(id, msgType);
			}
		}
	}
}

void CBuddyMgr::closeChat(bean::MessageType nMsgType, const QString &rsId)
{
	if (nMsgType == bean::Message_Invalid || rsId.isEmpty())
		return;

	foreach (QString mapKey, m_mapDialogs.keys())
	{
		if (!m_mapDialogs[mapKey].isNull())
		{
			bean::MessageType msgType = bean::Message_Chat;
			QString id;
			if (FromMapKey(mapKey, id, msgType))
			{
				if (nMsgType == msgType && rsId == id)
				{
					m_mapDialogs[mapKey].data()->closeChat();
					m_mapDialogs.remove(mapKey);
					return;
				}
			}
		}
	}
}

void CBuddyMgr::addDiscussMemberChangedTip(const QString &discussId, const QString &tip)
{
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_DiscussChat);
	msgBody.setSend(false);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(discussId);
	msgBody.setTime(CDateTime::currentDateTimeUtcString());
	msgBody.setBody(tip);
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
	ext.setData("level", "info");
	msgBody.setExt(ext);

	QPointer<ChatBaseDialog> dlg = GetDialogFromMap(discussId, bean::Message_DiscussChat);
	if (dlg.isNull())
	{
		bool ignore = true;
		UnreadMsgItem *unreadMsgItem = m_unreadMsgModel->peekUnreadMsg(discussId, bean::Message_DiscussChat);
		if (unreadMsgItem)
		{
			ignore = unreadMsgItem->isIgnoreBefore();
		}
		m_unreadMsgModel->insertMsg(discussId, bean::Message_DiscussChat, msgBody, false, ignore);
	}
	else
	{
		slot_receiveMessage(msgBody);
	}
}

void CBuddyMgr::onSendMessageOK(const QString &seq, const QString &ts)
{
	// get message
	bean::MessageBody msg = m_pSendMessageDB->getMessageBySequence(seq);
	if (msg.to().isEmpty())
	{
		return;
	}

	// secret message
	if (msg.ext().type() == bean::MessageExt_Secret)
	{
		// add to unread
		m_unreadSendSecretMessages.insert(ts, msg.to());
	}

	emit messageSent(msg.messageType(), msg.to(), seq, ts);

	// store message
	msg.setSequence(seq);
	msg.setStamp(ts);
	storeMessage(msg);

	// record the max msg timestamp
	qPmApp->setMaxMsgTs(ts);

	// remove from send message table
	m_pSendMessageDB->removeMsgBySequence(seq);
}

void CBuddyMgr::onSendMessageFailed(const QString &seq)
{
	if (m_pSendMessageDB.isNull())
		return;

	// get failed message
	bean::MessageBody msg = m_pSendMessageDB->getMessageBySequence(seq);
	if (msg.to().isEmpty())
	{
		return;
	}

	emit messageSendError(seq);

	if (qPmApp->GetLoginMgr()->isLogined())
	{
		// add failed tip
		addSendMessageFailedTip(seq, msg);

		// update send message state
		m_pSendMessageDB->updateMessageState(seq);
	}
}

void CBuddyMgr::onSendMessageCanceled(const QString &seq)
{
	if (m_pSendMessageDB.isNull())
		return;

	// get message
	bean::MessageBody msg = m_pSendMessageDB->getMessageBySequence(seq);
	if (msg.to().isEmpty())
	{
		return;
	}

	emit messageSendCancel(seq);

	if (qPmApp->GetLoginMgr()->isLogined())
	{
		// remove from send message table
		m_pSendMessageDB->removeMsgBySequence(seq);

		// add canceled tip
		addSendMessageCancelTip(msg);
	}
}

void CBuddyMgr::checkFailedSendMessages()
{
	// add failed tips
	QMap<QString, bean::MessageBody> failedMessages = m_pSendMessageDB->getMessages();
	foreach (QString seq, failedMessages.keys())
	{
		bean::MessageBody msg = failedMessages[seq];
		addSendMessageFailedTip(seq, msg);
	}

	// clear all failed messages
	if (!failedMessages.isEmpty())
	{
		m_pSendMessageDB->updateMessageStates();
	}
}

void CBuddyMgr::onInterphonesOK(bool ok)
{
	if (ok)
	{
		InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
		QMap<QString, InterphoneInfo> interphones = interphoneManager->allInterphones();
		foreach (QString interphoneId, interphones.keys())
		{
			InterphoneInfo info = interphones[interphoneId];
			bean::MessageType attachType = info.attachType();
			QString attachId = info.attachId();
			addInterphoneStartTip(interphoneId, attachType, attachId);

			emit interphoneStarted(interphoneId);
		}
	}
}

void CBuddyMgr::onInterphoneStarted(const QString &interphoneId, int attachType, const QString &attachId)
{
	addInterphoneStartTip(interphoneId, (bean::MessageType)attachType, attachId);

	emit interphoneStarted(interphoneId);
}

void CBuddyMgr::onInterphoneFinished(const QString &interphoneId)
{
	bean::MessageType msgType = bean::Message_Invalid;
	QString toId;
	InterphoneManager::interphoneId2AttachTypeId(interphoneId, msgType, toId);
	addInterphoneFinishTip(msgType, toId);

	emit interphoneFinished(interphoneId);
}

void CBuddyMgr::onRequestSecretAckFinished(const QString &fromId, const QString &toId, const QString &stamp, int readState)
{
	if (readState == 1)
	{
		if (fromId == Account::instance()->id())
		{
			recvSecretMessageAcked(toId, stamp);
		}
		else
		{
			setSendSecretMessageReadState(fromId, stamp, readState);
		}
	}
}

void CBuddyMgr::setSendSecretMessageReadState(const QString &uid, const QString &stamp, int readState)
{
	if (readState == 1)
	{
		// remove from unread
		m_unreadSendSecretMessages.remove(stamp);

		// set read
		setMessageRead(stamp);

		SecretManager *secretManager = qPmApp->getSecretManager();
		if (secretManager)
			secretManager->setSecretRead(stamp);

		// notify secret message state changed
		emit sendSecretMessageRead(uid, stamp);
	}
}

void CBuddyMgr::recvSecretMessageAcked(const QString &toUid, const QString &stamp)
{
	if (stamp.isEmpty() || toUid.isEmpty())
		return;

	if (isRecvSecretMessageRead(stamp, toUid))
		return;

	// update message read state
	setMessageRead(stamp);

	insertReadRecvSecretMessage(stamp, toUid);

	emit recvSecretMessageRead(toUid, stamp);
}

void CBuddyMgr::onMessageWithdrawOK(bean::MessageType chatType, 
	                                const QString &toId, 
									const QString &fromId, 
									const QString &timeStamp,
									const QString &withdrawId)
{
	bean::MessageBody msg = withdrawMessage(timeStamp);
	if (!msg.isValid())
		return;

	qPmApp->getMessageWithdrawManager()->setLastWithdrawId(withdrawId);

	// replace with tip message in db
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(chatType);
	msgBody.setSend(false);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(toId);
	msgBody.setTime(msg.time());
	if (Account::instance()->id() == Account::idFromFullId(fromId))
	{
		msgBody.setBody(tr("You withdraw a message"));
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QString name = modelManager->userName(Account::idFromFullId(fromId));
		if (msgBody.messageType() == bean::Message_GroupChat)
		{
			name = modelManager->memberNameInGroup(msgBody.to(), Account::idFromFullId(fromId));
		}
		else if (msgBody.messageType() == bean::Message_DiscussChat)
		{
			name = modelManager->memberNameInDiscuss(msgBody.to(), Account::idFromFullId(fromId));
		}

		msgBody.setBody(tr("%1 withdraw a message").arg(name));
	}
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
	ext.setData("level", "info");
	msgBody.setExt(ext);
	qPmApp->getMessageDBStore()->append(msg.messageid(), msgBody);

	// replace tip message in last contact model
	qPmApp->getModelManager()->lastContactModel()->replaceMsg(msgBody, timeStamp);

	// replace in message view
	emit messageWithdrawOK(chatType, toId, fromId, timeStamp, msgBody.body(), msg);

	// remove from withdraw messages
	removeWithdrawMessage(timeStamp);
}

void CBuddyMgr::onMessageWithdrawed(bean::MessageType chatType, 
	                                const QString &toId, 
									const QString &fromId, 
									const QString &timeStamp,
									const QString &withdrawId)
{
	qPmApp->getMessageWithdrawManager()->setLastWithdrawId(withdrawId);

	MessageDBStore *messageDB = qPmApp->getMessageDBStore();
	connect(messageDB, SIGNAL(gotMessageOfStamp(qint64, QString, bean::MessageBody)), 
		this, SLOT(onGotMessageOfStamp(qint64, QString, bean::MessageBody)), Qt::UniqueConnection);
	qint64 seq = messageDB->getMessageByStamp(chatType, toId, timeStamp);
	m_withdrawedMessages.insert(seq, Account::idFromFullId(fromId));
	qDebug() << "add withdraw message: " << seq << timeStamp << fromId;
}

void CBuddyMgr::onMessageWithdrawFailed(bean::MessageType chatType, const QString &toId, const QString &fromId, const QString &timeStamp)
{
	Q_UNUSED(chatType);
	Q_UNUSED(toId);
	Q_UNUSED(fromId)
	bean::MessageBody msg = withdrawMessage(timeStamp);
	if (!msg.isValid())
		return;

	emit messageWithdrawFailed(chatType, toId, fromId, timeStamp);

	// remove from withdraw messages
	removeWithdrawMessage(timeStamp);
}

void CBuddyMgr::onGotMessageOfStamp(qint64 seq, const QString &stamp, const bean::MessageBody &msg)
{
	qDebug() << "got withdraw message" << seq << stamp << msg.isValid() << m_withdrawedMessages.contains(seq) << m_withdrawedMessages;

	if (!m_withdrawedMessages.contains(seq))
		return;

	QString fromId = m_withdrawedMessages[seq];

	m_withdrawedMessages.remove(seq);

	if (!msg.isValid())
		return;

	// replace with tip message in db
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(msg.messageType());
	msgBody.setSend(false);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(msg.to());
	msgBody.setTime(msg.time());
	if (Account::instance()->id() == fromId)
	{
		msgBody.setBody(tr("You withdraw a message"));
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QString name = modelManager->userName(fromId);
		if (msgBody.messageType() == bean::Message_GroupChat)
		{
			name = modelManager->memberNameInGroup(msgBody.to(), fromId);
		}
		else if (msgBody.messageType() == bean::Message_DiscussChat)
		{
			name = modelManager->memberNameInDiscuss(msgBody.to(), fromId);
		}

		msgBody.setBody(tr("%1 withdraw a message").arg(name));
	}
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
	ext.setData("level", "info");
	msgBody.setExt(ext);
	qPmApp->getMessageDBStore()->append(msg.messageid(), msgBody);

	// replace with tip message in unread message model
	UnreadMsgItem *unreadMsgItem = qPmApp->getUnreadMsgModel()->peekUnreadMsg(msg.to(), msg.messageType());
	if (unreadMsgItem)
	{
		bool changed = false;
		QList<bean::MessageBody> newUnreadMsgs;
		QList<bean::MessageBody> unreadMsgs = unreadMsgItem->msgs();
		foreach (bean::MessageBody unreadMsg, unreadMsgs)
		{
			if (unreadMsg.stamp() == stamp)
			{
				changed = true;
				newUnreadMsgs.append(msgBody);
			}
			else
			{
				newUnreadMsgs.append(unreadMsg);
			}
		}
		unreadMsgItem->setMsgs(newUnreadMsgs);
	}

	// replace tip message in last contact model
	qPmApp->getModelManager()->lastContactModel()->replaceMsg(msgBody, stamp);

	emit messageWithdrawOK(msg.messageType(), msg.to(), msg.from(), stamp, msgBody.body(), msg);
}

void CBuddyMgr::forwardImage(const QString &imagePath)
{
	QWidget *w = qobject_cast<QWidget *>(sender());
	if (!w)
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::warning(w, tr("Tip"), tr("Can't forward, you're offline"));
		return;
	}

	SelectChatDialog dlg(tr("Forward"), w);
	if (dlg.exec())
	{
		bean::MessageType msgType;
		QString toId;
		dlg.getSelect(msgType, toId);

		if (msgType == bean::Message_Invalid || toId.isEmpty())
			return;

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::warning(w, tr("Tip"), tr("Can't forward, you're offline"));
			return;
		}

		QString uid = Account::instance()->id();
		ModelManager *modelManager = qPmApp->getModelManager();
		QString name = modelManager->userName(uid);
		QString toName;
		if (msgType == bean::Message_Chat)
		{
			toName = modelManager->userName(toId);
		}
		else if (msgType == bean::Message_GroupChat)
		{
			toName = modelManager->groupName(toId);
		}
		else if (msgType == bean::Message_DiscussChat)
		{
			toName = modelManager->discussName(toId);
		}

		// create image attach
		QList<bean::AttachItem> lstImageAttachs;
		bean::AttachItem attachItem(imagePath);
		attachItem.setMessageType(msgType);
		QString uuidStr = QUuid::createUuid().toString();
		attachItem.setUuid(uuidStr.mid(1, uuidStr.length()-2));
		attachItem.setTransferType(bean::AttachItem::Type_AutoDisplay);
		attachItem.setFrom(uid);
		QImage pic = ImageUtil::readImage(imagePath);
		attachItem.setPicWidth(pic.width());
		attachItem.setPicHeight(pic.height());
		lstImageAttachs.append(attachItem);

		// create message body
		bean::MessageBody msg = bean::MessageBodyFactory::createMessage(msgType);
		msg.setFrom(uid);
		msg.setFromName(name);
		msg.setTo(toId);
		msg.setToName(toName);
		msg.setSend(true);
		msg.setTime("");
		msg.setBody(uuidStr);
		msg.setAttachs(lstImageAttachs);
		msg.setExt(bean::MessageExtFactory::create(bean::MessageExt_Chat));
		
		QPointer<ChatBaseDialog> dlg = GetDialogFromMap(toId, msgType);
		if (dlg.isNull())
			dlg = slot_open_dialog(msgType, toId, false);

		slot_sendMessage(msg);

		if (!dlg.isNull())
			dlg.data()->appendSendMessage(msg);

		/*
		// show tip
		QString tipText = tr("Message has been forwarded");
		PMessageLineTip *lineTip = new PMessageLineTip(PMessageLineTip::Success, tipText, w);
		lineTip->setTipTimeout(2000); // show 2 seconds
		QRect rt(QPoint(), w->size());
		QPoint center = rt.center();
		center.setY(center.y() - 45);
		QPoint globalCenter = w->mapToGlobal(center);
		QSize sz = lineTip->sizeHint();
		QPoint pt;
		pt.setX(globalCenter.x()-sz.width()/2);
		pt.setY(globalCenter.y()-sz.height()/2);
		WidgetManager::showActivateRaiseWindow(lineTip);
		lineTip->move(pt);
		*/
	}
}

void CBuddyMgr::addSendMessageFailedTip(const QString &seq, const bean::MessageBody &msg)
{
	// add a tip message
	QString timeText = makeMessageTimeText(msg);
	QString bodyText = makeMessageBodyText(msg);

	QString msgText = tr("Because of network error, %2 you sent at %1 may fail.").arg(timeText).arg(bodyText);

	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(msg.messageType());
	msgBody.setSend(false);
	msgBody.setFrom(msg.from());
	msgBody.setTo(msg.to());
	msgBody.setTime(CDateTime::currentDateTimeUtcString());
	msgBody.setBody(msgText);
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
	ext.setData("level", "error");
	ext.setData("action", tr("Send again"));
	ext.setData("param", QString("sendmessage:%1").arg(seq));
	msgBody.setExt(ext);

	slot_receiveMessage(msgBody);
}

void CBuddyMgr::addSendMessageCancelTip(const bean::MessageBody &msg)
{
	QString to = msg.to();
	bean::MessageType messageType = msg.messageType();

	if (GetDialogFromMap(to, messageType).isNull())
	{
		// only show tip when this dialog exists
		return;
	}

	// add a tip message
	QString bodyText = makeMessageBodyText(msg);

	QString msgText = tr("You canceled sending of %1, message send failed.").arg(bodyText);

	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(msg.messageType());
	msgBody.setSend(false);
	msgBody.setFrom(msg.from());
	msgBody.setTo(msg.to());
	msgBody.setTime(CDateTime::currentDateTimeUtcString());
	msgBody.setBody(msgText);
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
	ext.setData("level", "error");
	msgBody.setExt(ext);

	slot_receiveMessage(msgBody);
}

QString CBuddyMgr::makeMessageTimeText(const bean::MessageBody &msg)
{
	QString timeText;
	QDateTime timeNow = CDateTime::currentDateTimeUtc();
	QDateTime msgTime = CDateTime::QDateTimeFromString(msg.time());
	msgTime.setTimeSpec(Qt::UTC);
	int seconds = msgTime.secsTo(timeNow);
	if (seconds < 0)
	{
		timeText = "";
	}
	else if (seconds < 60)
	{
		timeText = tr("1 minute ago");
	}
	else if (seconds < 60*60)
	{
		timeText = tr("%1 minutes ago").arg(seconds/60);
	}
	else if (seconds < 24*60*60)
	{
		timeText = tr("%1 hours ago").arg(seconds/60/60);
	}
	else
	{
		timeText = tr("%1 days ago").arg(msgTime.daysTo(timeNow));
	}

	return timeText;
}

QString CBuddyMgr::makeMessageBodyText(const bean::MessageBody &msg)
{
	bool textMsg = true;
	QString bodyText = msg.body();
	foreach (bean::AttachItem attach, msg.attachs())
	{
		if (attach.transferType() == bean::AttachItem::Type_AutoDownload)
		{
			bodyText = tr("voice(%1 seconds)").arg(attach.time());
			textMsg = false;
			break;
		}
		else if (attach.transferType() == bean::AttachItem::Type_Default || attach.transferType() == bean::AttachItem::Type_Dir)
		{
			QString fileSizeStr;
			qint64 size = attach.size();
			if (size < 1024){
				fileSizeStr = QString("%1 B").arg(size);
			} else if (size < 1024*1024){
				fileSizeStr = QString("%1 K").arg(size/1024.0, 0, 'f', 2);
			} else if (size < 1024*1024*1024) {
				fileSizeStr = QString("%1 M").arg(size/(1024.0*1024.0), 0, 'f', 2);
			} else {
				fileSizeStr = QString("%1 G").arg(size/(1024.0*1024.0*1024.0), 0, 'f', 2);
			}
			if (attach.transferType() == bean::AttachItem::Type_Default)
				bodyText = tr("attach \"%1\"(%2)").arg(attach.filename()).arg(fileSizeStr);
			else
				bodyText = tr("dir \"%1\"(%2)").arg(attach.filename()).arg(fileSizeStr);
			textMsg = false;
			break;
		}
	}

	if (textMsg) 
	{
		bodyText = msg.messageBodyText();
		if (bodyText.length() > 30)
		{
			bodyText = bodyText.left(30);
			bodyText.append("...");
		}
		bodyText = tr("message \"%1\"").arg(bodyText);
	}

	return bodyText;
}

void CBuddyMgr::checkToPlayBeep(bean::MessageType msgType, const QString &toId, bean::MessageExtType msgExtType,
					            bool hasDialogBefore, bool hasUnreadBefore, int groupSetting,
					            bool history, bool firstHistory)
{
	if (msgType == bean::Message_Chat)
	{
		if (toId == QString(SUBSCRIPTION_ROSTER_ID)) // subscription tip
		{
			PlayBeep::playRecvSubscriptionMsgBeep();
			return;
		}
		
		if (msgExtType == bean::MessageExt_Chat ||
			msgExtType == bean::MessageExt_Share ||
			msgExtType == bean::MessageExt_Secret ||
			msgExtType == bean::MessageExt_Interphone ||
			msgExtType == bean::MessageExt_Tip)
		{
			// play sound
			if (!history || (history && firstHistory && !hasDialogBefore && !hasUnreadBefore))
			{
				PlayBeep::playRecvBuddyMsgBeep();
			}
		}
	}
	else if (msgType == bean::Message_GroupChat || msgType == bean::Message_DiscussChat)
	{
		if (!hasDialogBefore && !hasUnreadBefore) // this message is first received
		{
			if (groupSetting == (int)(AccountSettings::Tip)) // need to be tipped
			{
				PlayBeep::playRecvBuddyMsgBeep();
			}
		}
	}
}

void CBuddyMgr::addInterphoneStartTip(const QString &interphoneId, bean::MessageType attachType, const QString &attachId)
{
	if (attachType == bean::Message_Invalid)
		return;

	QString toId = attachId;
	QString creator;
	if (attachType == bean::Message_Chat)
	{
		bean::MessageType msgType = bean::Message_Invalid;
		InterphoneManager::interphoneId2AttachTypeId(interphoneId, msgType, toId);
		
		QString selfId = Account::instance()->id();
		if (selfId == attachId)
			creator = toId;
		else
			creator = selfId;
	}

	// if is in black list, do not tip
	ModelManager *modelManager = qPmApp->getModelManager();
	if (attachType == bean::Message_Chat && modelManager->isInBlackList(toId))
		return;

	QString msgText;
	if (attachType == bean::Message_Chat)
	{
		QString name = modelManager->userName(creator);
		msgText = tr("%1 starts an interphone").arg(name);
	}
	else
	{
		msgText = tr("Interphone is running");
	}

	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(attachType);
	msgBody.setSend(false);
	msgBody.setTo(toId);
	msgBody.setTime(CDateTime::currentDateTimeUtcString());
	msgBody.setBody(msgText);
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Interphone);
	msgBody.setExt(ext);

	slot_receiveMessage(msgBody);
}

void CBuddyMgr::addInterphoneFinishTip(bean::MessageType msgType, const QString &toId)
{
	if (msgType == bean::Message_Invalid)
		return;

	// if is in black list, do not tip
	ModelManager *modelManager = qPmApp->getModelManager();
	if (msgType == bean::Message_Chat && modelManager->isInBlackList(toId))
		return;

	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(msgType);
	msgBody.setSend(false);
	msgBody.setTo(toId);
	msgBody.setTime(CDateTime::currentDateTimeUtcString());
	QString msgText = tr("Interphone finished");
	msgBody.setBody(msgText);
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Interphone);
	msgBody.setExt(ext);

	slot_receiveMessage(msgBody);
}

/*
QPoint CBuddyMgr::configDialogPos()
{
	QList<QPoint> dialogPoses;
	foreach (QPointer<ChatBaseDialog> chatInterface, m_mapDialogs)
	{
		if (!chatInterface.isNull() && chatInterface.data())
		{
			FramelessDialog *framelessDialog = qobject_cast<FramelessDialog *>(chatInterface.data());
			if (framelessDialog)
			{
				bool visible = framelessDialog->isVisible(); 
				bool minimized = framelessDialog->isMinimized();
				bool maximized = framelessDialog->isShowMaximized();
				if (visible && !minimized && !maximized)
				{
					dialogPoses.append(framelessDialog->pos());
				}
			}
		}
	}

	foreach (QPointer<MsgMultiSendDlg> dlg, m_mapMsgMultiSendDlgs)
	{
		if (!dlg.isNull())
		{
			FramelessDialog *framelessDialog = dlg.data();
			if (framelessDialog)
			{
				bool visible = framelessDialog->isVisible(); 
				bool minimized = framelessDialog->isMinimized();
				bool maximized = framelessDialog->isShowMaximized();
				if (visible && !minimized && !maximized)
				{
					dialogPoses.append(framelessDialog->pos());
				}
			}
		}
	}

	QRect availableRect = QApplication::desktop()->availableGeometry();
	QPoint middlePt((availableRect.width()-GuiConstants::WidgetSize::ChatMainWidth-GuiConstants::WidgetSize::GroupChatMemberWidth)/2, 
		(availableRect.height()-GuiConstants::WidgetSize::ChatHeight)/2);

	if (dialogPoses.isEmpty())
	{
		return middlePt;
	}

	const int kOffsetX = 20;
	const int kOffsetY = 30;
	const int kTolerate = 10;
	QList<QPoint> offsets;
	offsets << QPoint(-kOffsetX, -kOffsetY) << QPoint(-kOffsetX, 0) << QPoint(-kOffsetX, kOffsetY)
		    << QPoint(0, kOffsetY) << QPoint(kOffsetX, kOffsetY) << QPoint(kOffsetX, 0)
			<< QPoint(kOffsetX, -kOffsetY) << QPoint(0, -kOffsetY);
	for (int i = 1; i <= 10; i++) // 10 should be enough
	{
		foreach (QPoint offset, offsets)
		{
			QPoint newPt = middlePt + QPoint(offset.x()*i, offset.y()*i);
			
			// check if has same pos
			int j = 0;
			for (j = 0; j < dialogPoses.count(); j++)
			{
				QPoint oldPt = dialogPoses[j];
				if (qAbs(oldPt.x() - newPt.x()) <= kTolerate && qAbs(oldPt.y() - newPt.y()) <= kTolerate)
					break;
			}

			if (j >= dialogPoses.count())
				return newPt;
		}
	}

	return middlePt;
}
*/

void CBuddyMgr::sendSecretAck(const QString &uid, const QString &stamp)
{
	if (uid.isEmpty() || stamp.isEmpty())
		return;

	SecretManager *secretManager = qPmApp->getSecretManager();
	secretManager->setSecretAck(Account::instance()->id(), uid, stamp);
}

void CBuddyMgr::storeSecretAck(const QString &uid, const QString &stamp)
{
	if (uid.isEmpty() || stamp.isEmpty())
		return;

	m_pSendMessageDB->storeSecretAck(uid, stamp);
}

void CBuddyMgr::setMessageRead(const QString &stamp)
{
	// store message db
	qPmApp->getMessageDBStore()->append(stamp, 1);
}

void CBuddyMgr::insertReadRecvSecretMessage(const QString &stamp, const QString &uid)
{
	if (stamp.isEmpty() || uid.isEmpty())
		return;

	if (m_unreadRecvSecretMessages.contains(stamp))
		m_unreadRecvSecretMessages.remove(stamp);

	m_readRecvSecretMessages.insert(stamp, uid);
}

void CBuddyMgr::doHistoryMsgFailed(int fType, const QString &id)
{
	OfflineMsgManager::FromType fromType = (OfflineMsgManager::FromType)fType;
	bean::MessageType messageType = OfflineMsgManager::offlineFromType2MessageType(fromType);

	// clear offline
	if (hasOffline(messageType, id))
		clearOffline(messageType, id);

	// deal with more message tip
	QPointer<ChatBaseDialog> dlg = GetDialogFromMap(id, messageType);
	if (!dlg.isNull())
	{
		dlg.data()->onMoreMsgFinished();
		dlg.data()->closeMoreMsgTip();
		dlg.data()->showAutoTip(tr("Check more messages failed"));
	}
}

void CBuddyMgr::cacheFirstSyncMessage(bean::MessageType msgType, const QString &id, const bean::MessageBody &message)
{
	if (msgType == bean::Message_Invalid || id.isEmpty())
		return;

	QList<bean::MessageBody> messages;
	QString cacheKey = GetMapKey(id, msgType);
	if (m_firstSyncMessages.contains(cacheKey))
	{
		messages = m_firstSyncMessages.value(cacheKey);
	}
	messages.append(message);
	m_firstSyncMessages.insert(cacheKey, messages);
}

void CBuddyMgr::cacheFirstSyncMessages(bean::MessageType msgType, const QString &id, const QList<bean::MessageBody> &newMessages)
{
	if (msgType == bean::Message_Invalid || id.isEmpty())
		return;

	QList<bean::MessageBody> messages;
	QString cacheKey = GetMapKey(id, msgType);
	if (m_firstSyncMessages.contains(cacheKey))
	{
		messages = m_firstSyncMessages.value(cacheKey);
	}
	messages.append(newMessages);
	m_firstSyncMessages.insert(cacheKey, messages);
}

QList<bean::MessageBody> CBuddyMgr::takeCacheFirstSyncMessages(bean::MessageType msgType, const QString &id)
{
	QList<bean::MessageBody> msgs;
	
	if (msgType == bean::Message_Invalid || id.isEmpty())
		return msgs;

	QString cacheKey = GetMapKey(id, msgType);
	if (m_firstSyncMessages.contains(cacheKey))
		msgs = m_firstSyncMessages.take(cacheKey);

	for (int i = 0; i < msgs.count(); ++i)
	{
		bean::MessageBody msg = msgs[i];
		if (msg.messageType() == bean::Message_Chat && msg.ext().type() == bean::MessageExt_Secret)
		{
			QString stamp = msg.stamp();
			if (msg.isSend())
			{
				if (!m_unreadSendSecretMessages.contains(stamp))
					msgs[i].setReadState(1);
			}
			else
			{
				if (!m_unreadRecvSecretMessages.contains(stamp))
					msgs[i].setReadState(1);
			}
		}
	}

	return msgs;
}

void CBuddyMgr::checkToAddLastContact(const bean::MessageBody &message)
{
	// add to last contact
	bool addToLastContact = message.ext().data(bean::EXT_DATA_LASTCONTACT_NAME, true).toBool();
	if (addToLastContact)
	{
		qPmApp->getModelManager()->lastContactModel()->appendMsg(message);
	}
}

void CBuddyMgr::checkToStoreMessage(const bean::MessageBody &message)
{
	bool storeMessage = message.ext().data(bean::EXT_DATA_HISTORY_NAME, true).toBool();
	if (storeMessage)
	{
		this->storeMessage(message);
	}
}

bool CBuddyMgr::preprocessReceivedMessage(const bean::MessageBody &rBody, bool history, bean::MessageBody &body)
{
	if (!rBody.isValid())
	{
		qDebug() << Q_FUNC_INFO << " does not valid.";
		return false;
	}

	body = const_cast<bean::MessageBody &>(rBody);
	bean::MessageType messageType = body.messageType();
	QString sId = body.to();
	QString selfId = qPmApp->getAccount()->id();

	// ignore message from self
	if (messageType == bean::Message_Chat &&
		body.toFullId() == Account::instance()->computerFullId())
	{
		qDebug() << Q_FUNC_INFO << " recv message from self: " << sId;
		return false;
	}

	// ignore message from black list
	if (messageType == bean::Message_Chat && 
		qPmApp->getModelManager()->isInBlackList(sId))
	{
		qDebug() << Q_FUNC_INFO << " recv message from black list: " << sId;
		return false;
	}

	// ignore message of group loop
	if (messageType != bean::Message_Chat && 
		body.fromFullId() == Account::instance()->computerFullId() &&
		body.ext().type() != bean::MessageExt_Tip)
	{
		qDebug() << body.from() << "group loop";

		// record the max msg timestamp
		qPmApp->setMaxMsgTs(body.stamp());

		return false;
	}

	// message from phone, time changed to received time, name changed to my phone
	if (messageType == bean::Message_Chat && 
		sId == Account::instance()->phoneFullId() &&
		body.from() == selfId)
	{
		body.setTime(CDateTime::currentDateTimeUtcString());
		body.setToName(Account::phoneName());
	}

	// chat sync message from phone, change to send message
	if (messageType == bean::Message_Chat && 
		sId == Account::instance()->phoneFullId() &&
		body.from() != selfId)
	{
		sId = body.from();
		body.copy();
		body.setTo(sId);
		body.setToName(body.fromName());
		body.setFrom(selfId);
		body.setFromName(qPmApp->getModelManager()->userName(selfId));
		QString sequence = QUuid::createUuid().toString();
		sequence = sequence.mid(1, sequence.length()-2);
		body.setSequence(sequence);
		body.setSync(true);
		body.setSend(true);
	}

	// group chat sync message from phone, change to send message
	if ((messageType == bean::Message_GroupChat || messageType == bean::Message_DiscussChat) && 
		(body.fromFullId() == Account::instance()->phoneFullId()))
	{
		body.copy();
		body.setFrom(selfId);
		QString sequence = QUuid::createUuid().toString();
		sequence = sequence.mid(1, sequence.length()-2);
		body.setSequence(sequence);
		body.setSync(true);
		body.setSend(true);
	}

	// secret receive message need to change to tip message, PC begin to remove received secret message
	if (messageType == bean::Message_Chat && body.ext().type() == bean::MessageExt_Secret && !body.sync())
	{
		body.copy();
		bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
		ext.setData("level", "info");
		ext.setData(bean::EXT_DATA_LASTCONTACT_NAME, true);
		ext.setData(bean::EXT_DATA_HISTORY_NAME, true);
		body.setExt(ext);
		body.setBody(tr("Received a secret message, please check on phone"));
		body.setAttachs(QList<bean::AttachItem>());
	}

	// begin: process off-line message
	if (needCacheMsg(messageType, sId) && !history)
	{
		m_offlineMsgCache.append(rBody); // add original message
	}
	// end: process off-line message

	// begin: get from and to name
	switch (messageType)
	{
	case bean::Message_Chat:
		{
			if (body.toName().isEmpty())
			{
				body.setToName(qPmApp->getModelManager()->userName(sId));
			}
			else
			{
				qPmApp->getModelManager()->addUserName(sId, body.toName());
			}
		}
		break;
	case bean::Message_GroupChat:
		{
			if (!qPmApp->getModelManager()->hasGroupItem(sId))
			{
				// this group is not existed
				qDebug() << QString("(%1)group msg group doesn't exist: %2").arg(sId).arg(body.body());
				return false;
			}

			body.setToName(qPmApp->getModelManager()->groupName(sId));
			if (body.fromName().isEmpty())
				body.setFromName(qPmApp->getModelManager()->userName(body.from()));
		}
		break;
	case bean::Message_DiscussChat:
		{
			if (!qPmApp->getModelManager()->hasDiscussItem(sId))
			{
				// this discuss is not existed
				qDebug() << QString("(%1)discuss msg discuss doesn't exist: %2").arg(sId).arg(body.body());
				return false;
			}

			body.setToName(qPmApp->getModelManager()->discussName(sId));
			if (body.fromName().isEmpty())
				body.setFromName(qPmApp->getModelManager()->userName(body.from()));
		}
		break;
	default:
		{
			// message type error
			qDebug() << QString("(%1)message type invalid: %2").arg(sId).arg((int)(messageType));
			return false;
		}
	}
	// end: get from and to name

	return true;
}