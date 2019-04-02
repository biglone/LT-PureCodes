#include "globalnotificationmsgmanager.h"
#include "GlobalNotificationMessagesDB.h"
#include "globalnotificationmessagesdbstore.h"
#include "PmApp.h"
#include "globalnotificationmanager.h"
#include "Account.h"
#include "ModelManager.h"
#include <QUuid>
#include "common/datetime.h"
#include "widgetmanager.h"
#include "globalnotificationmsgdialog.h"
#include "globalnotificationmodel.h"
#include "Constants.h"
#include "buddymgr.h"
#include <QUrl>
#include <QDesktopServices>
#include "globalnotificationwebview.h"
#include "settings/GlobalSettings.h"
#include "globalnotificationdownloaddialog.h"
#include <QDebug>
#include "unreadmsgmodel.h"
#include "util/PlayBeep.h"
#include <Windows.h>
#include "globalnotificationlastmsgmodel.h"
#include "lastcontactmodeldef.h"
#include "globalnotificationlastmsgdialog.h"

GlobalNotificationMsgManager::GlobalNotificationMsgManager(GlobalNotificationManager *globalNotificationManager, QObject *parent)
	: QObject(parent), m_globalNotificationManager(globalNotificationManager), m_running(false), m_lastSequence(0), m_currentMsgInnerId(0)
{
	Q_ASSERT(m_globalNotificationManager);

	bool connectOK = false;
	connectOK = connect(m_globalNotificationManager, SIGNAL(getMsgNumberFinished(bool, QMap<QString, int>)),
		this, SLOT(onGetMsgNumberFinished(bool, QMap<QString, int>)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_globalNotificationManager, SIGNAL(getMessagesFinished(bool, QString, QList<GlobalNotificationMsg>)),
		this, SLOT(onGetMessagesFinished(bool, QString, QList<GlobalNotificationMsg>)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_globalNotificationManager, SIGNAL(sendMsgFinished(bool, QString, GlobalNotificationMsg)),
		this, SLOT(onSendMsgFinished(bool, QString, GlobalNotificationMsg)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_globalNotificationManager, SIGNAL(clickMenuFinished(bool, QString, QString, GlobalNotificationMsg)),
		this, SLOT(onClickMenuFinished(bool, QString, QString, GlobalNotificationMsg)));
	Q_ASSERT(connectOK);
}

GlobalNotificationMsgManager::~GlobalNotificationMsgManager()
{

}

void GlobalNotificationMsgManager::start()
{
	release();

	// read last sequence
	//m_lastSequence = Account::settings()->subscriptionLastSequence();
	m_lastSequence = Account::settings()->globalNotificationLastSequence();

	DB::GlobalNotificationMessagesDB *msgDB = new DB::GlobalNotificationMessagesDB("GlobalNotificationMsgManager");
	// read all unread message count
	m_unreadMsgCount = msgDB->unreadMsgCount();
	// read current message inner id
	m_currentMsgInnerId = msgDB->maxInnerId();
	delete msgDB;
	msgDB = 0;

	// init messages db
	m_globalNotificationMessageDB.reset(new GlobalNotificationMessagesDBStore());
	m_globalNotificationMessageDB->init();
	connect(m_globalNotificationMessageDB.data(), SIGNAL(messagesGot(qint64, QList<GlobalNotificationMsg>, QString, quint64, int)),
		this, SIGNAL(messagesGot(qint64, QList<GlobalNotificationMsg>)));

	m_running = true;

	foreach (QString globalNotificationId, m_unreadMsgCount.keys())
	{
		int msgCount = m_unreadMsgCount[globalNotificationId];
		emit unreadMsgChanged(globalNotificationId, msgCount);
	}
}

void GlobalNotificationMsgManager::stop()
{
	m_running = false;
}

void GlobalNotificationMsgManager::release()
{
	m_running = false;

	m_globalNotificationMessageDB.reset(0);
	m_lastSequence = 0;
	m_unreadMsgCount.clear();
	m_currentMsgInnerId = 0;
}

void GlobalNotificationMsgManager::getMessages()
{
	if (!m_running)
		return;

	if (m_globalNotificationMessageDB.isNull())
		return;

	QString selfId = Account::instance()->id();
	m_globalNotificationManager->getMsgNumber(selfId, QString::number(m_lastSequence));
}

int GlobalNotificationMsgManager::unreadMsgCount(const QString &globalNotificationId)
{
	int count = 0;
	if (m_unreadMsgCount.contains(globalNotificationId))
		count = m_unreadMsgCount[globalNotificationId];
	return count;
}

void GlobalNotificationMsgManager::setUnreadMsgCount(const QString &globalNotificationId, int count)
{
	if (m_globalNotificationMessageDB.isNull())
		return;

	m_unreadMsgCount[globalNotificationId] = count;
	m_globalNotificationMessageDB->appendUnreadMsgCount(globalNotificationId, count);

	emit unreadMsgChanged(globalNotificationId, count);
}

QMap<QString, int> GlobalNotificationMsgManager::allUnreadMsgCount() const
{
	return m_unreadMsgCount;
}

qint64 GlobalNotificationMsgManager::getMessagesFromDB(const QString &globalNotificationId, 
	quint64 lastInnerId /*= 0*/, int count /*= 10*/)
{
	qint64 seq = -1;
	if (!m_globalNotificationMessageDB.isNull())
	{
		seq = m_globalNotificationMessageDB->getMessages(globalNotificationId, lastInnerId, count);
	}
	return seq;
}

bool GlobalNotificationMsgManager::removeMessageOfGlobalNotification(const QString &globalNotificationId)
{
	if (m_globalNotificationMessageDB.isNull())
		return false;

	m_globalNotificationMessageDB->removeMessages(globalNotificationId);
	return true;
}

void GlobalNotificationMsgManager::openGlobalNotificationMsgDialog(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	GlobalNotificationMsgDialog *dlg = 0;
	if (hasGlobalNotificationDialog(globalNotificationId))
	{
		dlg = m_dialogs[globalNotificationId].data();
	}
	else
	{
		dlg = createDialog(globalNotificationId);
		dlg->getMessages();
		setUnreadMsgCount(globalNotificationId, 0);
	}

	WidgetManager::showActivateRaiseWindow(dlg);
}

void GlobalNotificationMsgManager::closeGlobalNotificationMsgDialog(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	GlobalNotificationMsgDialog *dlg = 0;
	if (hasGlobalNotificationDialog(globalNotificationId))
	{
		dlg = m_dialogs[globalNotificationId].data();
		dlg->close();
		m_dialogs.remove(globalNotificationId);
	}
}

GlobalNotificationMsg GlobalNotificationMsgManager::sendMsg(const QString &globalNotificationId, const QString &content)
{
	GlobalNotificationMsg msg;
	if (!m_running)
		return msg;

	if (m_globalNotificationMessageDB.isNull())
		return msg;

	if (globalNotificationId.isEmpty() || content.isEmpty())
		return msg;

	// send message
	QUuid uuid = QUuid::createUuid();
	QString uuidString = uuid.toString();
	QString msgId = uuidString.mid(1, uuidString.length()-2);
	QString selfId = Account::instance()->id();
	QString createTime = CDateTime::currentDateTimeUtcString();
	m_globalNotificationManager->sendMsg(msgId, selfId, globalNotificationId, content, createTime);

	// store message
	msg.setId("0"); // send message do not has id
	msg.setUserId(selfId);
	msg.setGlobalNotificationId(globalNotificationId);
	msg.setMsgId(msgId);
	msg.setContent(content);
	msg.setCreateTime(createTime);
	msg.setSend(true);
	m_globalNotificationMessageDB->appendMessage(msg);
	msg.setInnerId(++m_currentMsgInnerId);

	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->globalNotificationName(globalNotificationId);
	modelManager->globalNotificationLastMsgModel()->addLastMsg(msg, name);

	// add a message to last contact
	addLastContact(true, msg);

	return msg;
}

void GlobalNotificationMsgManager::recvMsg(const GlobalNotificationMsg &msg)
{
	if (!m_running)
		return;

	if (msg.id().isEmpty())
		return;

	if (m_globalNotificationMessageDB.isNull())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	GlobalNotificationModel *globalNotificationModel = modelManager->globalNotificationModel();
	if (!globalNotificationModel)
		return;

	QString globalNotificationId = msg.globalNotificationId();
	if (!globalNotificationModel->hasGlobalNotification(globalNotificationId))
		return;

	// save to message db
	m_globalNotificationMessageDB->appendMessage(msg);
	quint64 innerId = ++m_currentMsgInnerId;

	GlobalNotificationMsg dbMsg = msg;
	dbMsg.setInnerId(innerId);

	quint64 msgSequence = msg.id().toULongLong();
	if (msgSequence > m_lastSequence)
		m_lastSequence = msgSequence;

	// add to last
	QString name = modelManager->globalNotificationName(globalNotificationId);
	modelManager->globalNotificationLastMsgModel()->addLastMsg(dbMsg, name);

	// add message
	if (hasGlobalNotificationDialog(globalNotificationId))
	{
		// refresh message
		GlobalNotificationMsgDialog *dlg = m_dialogs[globalNotificationId].data();
		dlg->appendMessage(dbMsg);
		flashTaskBar(dlg);

		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsg);
	}
	else if (GlobalNotificationLastMsgDialog::hasDialog())
	{
		flashTaskBar(GlobalNotificationLastMsgDialog::getDialog());

		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsg);

		// save to unread message
		int count = 1;
		if (m_unreadMsgCount.contains(globalNotificationId))
			count += m_unreadMsgCount[globalNotificationId];
		m_unreadMsgCount[globalNotificationId] = count;
		m_globalNotificationMessageDB->appendUnreadMsgCount(globalNotificationId, count);

		emit unreadMsgChanged(globalNotificationId, count);
	}
	else
	{
		// save to unread message
		int count = 1;
		if (m_unreadMsgCount.contains(globalNotificationId))
			count += m_unreadMsgCount[globalNotificationId];
		m_unreadMsgCount[globalNotificationId] = count;
		m_globalNotificationMessageDB->appendUnreadMsgCount(globalNotificationId, count);

		// tip
		addMessage(dbMsg);

		emit unreadMsgChanged(globalNotificationId, count);
	}
	
	// report new sequence
	reportLastSequence();
}

void GlobalNotificationMsgManager::onGetMsgNumberFinished(bool ok, const QMap<QString, int> &msgNumbers)
{
	if (!m_running)
		return;

	if (!ok)
		return;

	// get all messages
	QString selfId = Account::instance()->id();
	foreach (QString id, msgNumbers.keys())
	{
		int count = msgNumbers[id];
		if (count > 0)
		{
			m_globalNotificationManager->getMessages(selfId, id, QString::number(m_lastSequence), QString(), count);
		}
	}
}

void GlobalNotificationMsgManager::onGetMessagesFinished(bool ok, const QString &globalNotificationId, const QList<GlobalNotificationMsg> &messages)
{
	if (!m_running)
		return;

	if (!ok)
		return;

	if (messages.isEmpty())
		return;

	if (m_globalNotificationMessageDB.isNull())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	GlobalNotificationModel *globalNotificationModel = modelManager->globalNotificationModel();
	if (!globalNotificationModel)
		return;

	if (!globalNotificationModel->hasGlobalNotification(globalNotificationId))
		return;

	// save to message db
	QList<GlobalNotificationMsg> dbMsgs;
	foreach (GlobalNotificationMsg msg, messages)
	{
		GlobalNotificationMsg dbMsg = msg;
		m_globalNotificationMessageDB->appendMessage(msg);
		quint64 innerId = ++m_currentMsgInnerId;
		dbMsg.setInnerId(innerId);
		dbMsgs.append(dbMsg);

		quint64 msgSequence = msg.id().toULongLong();
		if (msgSequence > m_lastSequence)
			m_lastSequence = msgSequence;
	}

	// add to last
	QString name = modelManager->globalNotificationName(globalNotificationId);
	modelManager->globalNotificationLastMsgModel()->addLastMsg(dbMsgs.last(), name);

	// add message
	if (hasGlobalNotificationDialog(globalNotificationId))
	{
		// refresh message
		GlobalNotificationMsgDialog *dlg = m_dialogs[globalNotificationId].data();
		foreach (GlobalNotificationMsg dbMsg, dbMsgs)
		{
			dlg->appendMessage(dbMsg);
		}
		flashTaskBar(dlg);
	
		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsgs.last());
	}
	else if (GlobalNotificationLastMsgDialog::hasDialog())
	{
		flashTaskBar(GlobalNotificationLastMsgDialog::getDialog());

		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsgs.last());

		// save to unread message
		int count = messages.count();
		if (m_unreadMsgCount.contains(globalNotificationId))
			count += m_unreadMsgCount[globalNotificationId];
		m_unreadMsgCount[globalNotificationId] = count;
		m_globalNotificationMessageDB->appendUnreadMsgCount(globalNotificationId, count);

		emit unreadMsgChanged(globalNotificationId, count);
	}
	else
	{
		// save to unread message
		int count = messages.count();
		if (m_unreadMsgCount.contains(globalNotificationId))
			count += m_unreadMsgCount[globalNotificationId];
		m_unreadMsgCount[globalNotificationId] = count;
		m_globalNotificationMessageDB->appendUnreadMsgCount(globalNotificationId, count);

		// tip
		addMessage(dbMsgs.last());

		emit unreadMsgChanged(globalNotificationId, count);
	}

	// report new sequence
	reportLastSequence();
}

void GlobalNotificationMsgManager::onSendMsgFinished(bool ok, const QString &globalNotificationId, const GlobalNotificationMsg &msg)
{
	if (!m_running)
		return;

	if (!ok)
	{
		emit sendMsgFailed(globalNotificationId);
		return;
	}

	if (globalNotificationId.isEmpty() || msg.id().isEmpty())
		return;

	recvMsg(msg);
}

void GlobalNotificationMsgManager::onClickMenuFinished(bool ok, const QString &globalNotificationId, 
												 const QString & /*key*/, const GlobalNotificationMsg &msg)
{
	if (!m_running)
		return;

	if (!ok)
	{
		emit clickMenuFailed(globalNotificationId);
		return;
	}

	if (globalNotificationId.isEmpty() || msg.id().isEmpty())
		return;

	recvMsg(msg);
}

void GlobalNotificationMsgManager::openUrl(const QString &globalNotificationId, const QString &url)
{
	if (globalNotificationId.isEmpty() || url.isEmpty())
		return;

	QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void GlobalNotificationMsgManager::openTitle(const QString &globalNotificationId, const QString &idStr, const QString &messageIdStr)
{
	if (globalNotificationId.isEmpty() || idStr.isEmpty() || messageIdStr.isEmpty())
		return;

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/globalnotification/material/article/detail?id=%2&globalNotificationId=%3&messageId=%4")
		.arg(loginConfig.managerUrl).arg(idStr).arg(globalNotificationId).arg(messageIdStr);
	
	GlobalNotificationWebView *webView = GlobalNotificationWebView::getWebView();
	webView->setGlobalNotificationId(globalNotificationId);
	connect(webView, SIGNAL(openAttach(QString, QString, QString)), this, SLOT(openAttach(QString, QString, QString)), Qt::UniqueConnection);
	connect(webView, SIGNAL(openGlobalNotificationDetail(QString)), this, SIGNAL(openGlobalNotificationDetail(QString)), Qt::UniqueConnection);
	webView->load(urlString);
	WidgetManager::showActivateRaiseWindow(webView);
}

void GlobalNotificationMsgManager::clickMenu(const QString &globalNotificationId, const QString &key)
{
	if (globalNotificationId.isEmpty() || key.isEmpty())
		return;

	m_globalNotificationManager->clickMenu(globalNotificationId, Account::instance()->id(), key);
}

void GlobalNotificationMsgManager::openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name)
{
	if (globalNotificationId.isEmpty() || urlStr.isEmpty())
		return;

	QString urlString = urlStr;
	if (!urlStr.startsWith("http://") && !urlStr.startsWith("https://"))
	{
		GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
		QUrl baseUrl = QUrl::fromUserInput(loginConfig.managerUrl);
		urlString = QString("%1://%2:%3/%4").arg(baseUrl.scheme()).arg(baseUrl.host()).arg(baseUrl.port(80)).arg(urlStr);
	}

	qDebug() << Q_FUNC_INFO << urlString << name;

	GlobalNotificationDownloadDialog *downloadDialog = GlobalNotificationDownloadDialog::getDialog();
	downloadDialog->addDownload(urlString, name);
	WidgetManager::showActivateRaiseWindow(downloadDialog);
}

bool GlobalNotificationMsgManager::hasGlobalNotificationDialog(const QString &globalNotificationId) const
{
	if (!m_dialogs.contains(globalNotificationId))
		return false;

	if (m_dialogs[globalNotificationId].isNull())
		return false;

	return true;
}

GlobalNotificationMsgDialog *GlobalNotificationMsgManager::createDialog(const QString &globalNotificationId)
{
	GlobalNotificationMsgDialog *dlg = new GlobalNotificationMsgDialog(globalNotificationId);
	m_dialogs[globalNotificationId] = dlg;
	connect(dlg, SIGNAL(openGlobalNotificationDetail(QString)), this, SIGNAL(openGlobalNotificationDetail(QString)));
	connect(dlg, SIGNAL(openUrl(QString, QString)), this, SLOT(openUrl(QString, QString)));
	connect(dlg, SIGNAL(clickMenu(QString, QString)), this, SLOT(clickMenu(QString, QString)));
	connect(dlg, SIGNAL(openTitle(QString, QString, QString)), this, SLOT(openTitle(QString, QString, QString)));
	connect(dlg, SIGNAL(openAttach(QString, QString, QString)), this, SLOT(openAttach(QString, QString, QString)));
	connect(dlg, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	return dlg;
}

void GlobalNotificationMsgManager::reportLastSequence()
{
	if (m_lastSequence == 0)
		return;

	QString selfId = Account::instance()->id();
	QString lastSequence = QString::number(m_lastSequence);

	m_globalNotificationManager->report(selfId, lastSequence);

	Account::settings()->setGlobalNotificationLastSequence(m_lastSequence);
}

void GlobalNotificationMsgManager::flashTaskBar(QWidget *widget)
{
	if (!widget)
	{
		return;
	}

	DWORD timeOut = GetCaretBlinkTime();
	if (timeOut <= 0)
	{
		timeOut = 250;
	}

	UINT flashCount = 1;

	FLASHWINFO info;
	info.cbSize = sizeof(info);
	info.hwnd = (HWND)widget->winId();
	info.dwFlags = FLASHW_ALL;
	info.dwTimeout = timeOut;
	info.uCount = flashCount;

	FlashWindowEx(&info);
}

void GlobalNotificationMsgManager::addLastContact(bool send, const GlobalNotificationMsg &msg)
{
	// add a message to last contact
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	msgBody.setSend(send);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(QString(GLOBALNOTIFICATION_ROSTER_ID));
	msgBody.setToName(ModelManager::globalNotificationShowName());
	msgBody.setTime(msg.createTime());
	msgBody.setStamp(makeGlobalNotificationStamp(msg.innerId()));
	if (send)
	{
		msgBody.setBody(makeMessageText(msg, tr("I")));
	}
	else
	{
		QString name = modelManager->globalNotificationName(msg.globalNotificationId());
		msgBody.setBody(makeMessageText(msg, name));
	}
	modelManager->lastContactModel()->appendMsg(msgBody);
}

void GlobalNotificationMsgManager::addMessage(const GlobalNotificationMsg &msg)
{
	// add a message to unread message
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	msgBody.setSend(false);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(QString(GLOBALNOTIFICATION_ROSTER_ID));
	msgBody.setTime(msg.createTime());
	QString stamp = makeGlobalNotificationStamp(msg.innerId());
	msgBody.setStamp(stamp);
	QString name = qPmApp->getModelManager()->globalNotificationName(msg.globalNotificationId());
	QString msgText = makeMessageText(msg, name);
	msgBody.setBody(msgText);
	qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
}

QString GlobalNotificationMsgManager::makeMessageText(const GlobalNotificationMsg &msg, const QString &name) const
{	
	if (msg.id().isEmpty())
		return QString();

	QString text = QString("%1: %2").arg(name).arg(msg.bodyText());
	return text;
}

QString GlobalNotificationMsgManager::makeGlobalNotificationStamp(quint64 innerId) const
{
	QString stamp = QString("%1").arg(innerId, 29, 10, QChar('0'));
	return stamp;
}
