#include "subscriptionmsgmanager.h"
#include "SubscriptionMessagesDB.h"
#include "subscriptionmessagesdbstore.h"
#include "PmApp.h"
#include "subscriptionmanager.h"
#include "Account.h"
#include "ModelManager.h"
#include <QUuid>
#include "common/datetime.h"
#include "widgetmanager.h"
#include "subscriptionmsgdialog.h"
#include "subscriptionmodel.h"
#include "Constants.h"
#include "buddymgr.h"
#include <QUrl>
#include <QDesktopServices>
#include "subscriptionwebview.h"
#include "settings/GlobalSettings.h"
#include "subscriptiondownloaddialog.h"
#include <QDebug>
#include "unreadmsgmodel.h"
#include "util/PlayBeep.h"
#include <Windows.h>
#include "subscriptionlastmsgmodel.h"
#include "lastcontactmodeldef.h"
#include "subscriptionlastmsgdialog.h"

SubscriptionMsgManager::SubscriptionMsgManager(SubscriptionManager *subscriptionManager, QObject *parent)
	: QObject(parent), m_subscriptionManager(subscriptionManager), m_running(false), m_lastSequence(0), m_currentMsgInnerId(0)
{
	Q_ASSERT(m_subscriptionManager);

	bool connectOK = false;
	connectOK = connect(m_subscriptionManager, SIGNAL(getMsgNumberFinished(bool, QMap<QString, int>)),
		this, SLOT(onGetMsgNumberFinished(bool, QMap<QString, int>)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_subscriptionManager, SIGNAL(getMessagesFinished(bool, QString, QList<SubscriptionMsg>)),
		this, SLOT(onGetMessagesFinished(bool, QString, QList<SubscriptionMsg>)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_subscriptionManager, SIGNAL(sendMsgFinished(bool, QString, SubscriptionMsg)),
		this, SLOT(onSendMsgFinished(bool, QString, SubscriptionMsg)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_subscriptionManager, SIGNAL(clickMenuFinished(bool, QString, QString, SubscriptionMsg)),
		this, SLOT(onClickMenuFinished(bool, QString, QString, SubscriptionMsg)));
	Q_ASSERT(connectOK);
}

SubscriptionMsgManager::~SubscriptionMsgManager()
{

}

void SubscriptionMsgManager::start()
{
	release();

	// read last sequence
	m_lastSequence = Account::settings()->subscriptionLastSequence();

	DB::SubscriptionMessagesDB *msgDB = new DB::SubscriptionMessagesDB("SubscriptionMsgManager");
	// read all unread message count
	m_unreadMsgCount = msgDB->unreadMsgCount();
	// read current message inner id
	m_currentMsgInnerId = msgDB->maxInnerId();
	delete msgDB;
	msgDB = 0;

	// init messages db
	m_subscriptionMessageDB.reset(new SubscriptionMessagesDBStore());
	m_subscriptionMessageDB->init();
	connect(m_subscriptionMessageDB.data(), SIGNAL(messagesGot(qint64, QList<SubscriptionMsg>, QString, quint64, int)),
		this, SIGNAL(messagesGot(qint64, QList<SubscriptionMsg>)));

	m_running = true;

	foreach (QString subscriptionId, m_unreadMsgCount.keys())
	{
		int msgCount = m_unreadMsgCount[subscriptionId];
		emit unreadMsgChanged(subscriptionId, msgCount);
	}
}

void SubscriptionMsgManager::stop()
{
	m_running = false;
}

void SubscriptionMsgManager::release()
{
	m_running = false;

	m_subscriptionMessageDB.reset(0);
	m_lastSequence = 0;
	m_unreadMsgCount.clear();
	m_currentMsgInnerId = 0;
}

void SubscriptionMsgManager::getMessages()
{
	if (!m_running)
		return;

	if (m_subscriptionMessageDB.isNull())
		return;

	QString selfId = Account::instance()->id();
	m_subscriptionManager->getMsgNumber(selfId, QString::number(m_lastSequence));
}

int SubscriptionMsgManager::unreadMsgCount(const QString &subscriptionId)
{
	int count = 0;
	if (m_unreadMsgCount.contains(subscriptionId))
		count = m_unreadMsgCount[subscriptionId];
	return count;
}

void SubscriptionMsgManager::setUnreadMsgCount(const QString &subscriptionId, int count)
{
	if (m_subscriptionMessageDB.isNull())
		return;

	m_unreadMsgCount[subscriptionId] = count;
	m_subscriptionMessageDB->appendUnreadMsgCount(subscriptionId, count);

	emit unreadMsgChanged(subscriptionId, count);
}

QMap<QString, int> SubscriptionMsgManager::allUnreadMsgCount() const
{
	return m_unreadMsgCount;
}

qint64 SubscriptionMsgManager::getMessagesFromDB(const QString &subscriptionId, 
	quint64 lastInnerId /*= 0*/, int count /*= 10*/)
{
	qint64 seq = -1;
	if (!m_subscriptionMessageDB.isNull())
	{
		seq = m_subscriptionMessageDB->getMessages(subscriptionId, lastInnerId, count);
	}
	return seq;
}

bool SubscriptionMsgManager::removeMessageOfSubscription(const QString &subscriptionId)
{
	if (m_subscriptionMessageDB.isNull())
		return false;

	m_subscriptionMessageDB->removeMessages(subscriptionId);
	return true;
}

void SubscriptionMsgManager::openSubscriptionMsgDialog(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	SubscriptionMsgDialog *dlg = 0;
	if (hasSubscriptionDialog(subscriptionId))
	{
		dlg = m_dialogs[subscriptionId].data();
	}
	else
	{
		dlg = createDialog(subscriptionId);
		dlg->getMessages();
		setUnreadMsgCount(subscriptionId, 0);
	}

	WidgetManager::showActivateRaiseWindow(dlg);
}

void SubscriptionMsgManager::closeSubscriptionMsgDialog(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	SubscriptionMsgDialog *dlg = 0;
	if (hasSubscriptionDialog(subscriptionId))
	{
		dlg = m_dialogs[subscriptionId].data();
		dlg->close();
		m_dialogs.remove(subscriptionId);
	}
}

SubscriptionMsg SubscriptionMsgManager::sendMsg(const QString &subscriptionId, const QString &content)
{
	SubscriptionMsg msg;
	if (!m_running)
		return msg;

	if (m_subscriptionMessageDB.isNull())
		return msg;

	if (subscriptionId.isEmpty() || content.isEmpty())
		return msg;

	// send message
	QUuid uuid = QUuid::createUuid();
	QString uuidString = uuid.toString();
	QString msgId = uuidString.mid(1, uuidString.length()-2);
	QString selfId = Account::instance()->id();
	QString createTime = CDateTime::currentDateTimeUtcString();
	m_subscriptionManager->sendMsg(msgId, selfId, subscriptionId, content, createTime);

	// store message
	msg.setId("0"); // send message do not has id
	msg.setUserId(selfId);
	msg.setSubscriptionId(subscriptionId);
	msg.setMsgId(msgId);
	msg.setContent(content);
	msg.setCreateTime(createTime);
	msg.setSend(true);
	m_subscriptionMessageDB->appendMessage(msg);
	msg.setInnerId(++m_currentMsgInnerId);

	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->subscriptionName(subscriptionId);
	modelManager->subscriptionLastMsgModel()->addLastMsg(msg, name);

	// add a message to last contact
	addLastContact(true, msg);

	return msg;
}

void SubscriptionMsgManager::recvMsg(const SubscriptionMsg &msg)
{
	if (!m_running)
		return;

	if (msg.id().isEmpty())
		return;

	if (m_subscriptionMessageDB.isNull())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
	if (!subscriptionModel)
		return;

	QString subscriptionId = msg.subscriptionId();
	if (!subscriptionModel->hasSubscription(subscriptionId))
		return;

	// save to message db
	m_subscriptionMessageDB->appendMessage(msg);
	quint64 innerId = ++m_currentMsgInnerId;

	SubscriptionMsg dbMsg = msg;
	dbMsg.setInnerId(innerId);

	quint64 msgSequence = msg.id().toULongLong();
	if (msgSequence > m_lastSequence)
		m_lastSequence = msgSequence;

	// add to last
	QString name = modelManager->subscriptionName(subscriptionId);
	modelManager->subscriptionLastMsgModel()->addLastMsg(dbMsg, name);

	// add message
	if (hasSubscriptionDialog(subscriptionId))
	{
		// refresh message
		SubscriptionMsgDialog *dlg = m_dialogs[subscriptionId].data();
		dlg->appendMessage(dbMsg);
		flashTaskBar(dlg);

		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsg);
	}
	else if (SubscriptionLastMsgDialog::hasDialog())
	{
		flashTaskBar(SubscriptionLastMsgDialog::getDialog());

		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsg);

		// save to unread message
		int count = 1;
		if (m_unreadMsgCount.contains(subscriptionId))
			count += m_unreadMsgCount[subscriptionId];
		m_unreadMsgCount[subscriptionId] = count;
		m_subscriptionMessageDB->appendUnreadMsgCount(subscriptionId, count);

		emit unreadMsgChanged(subscriptionId, count);
	}
	else
	{
		// save to unread message
		int count = 1;
		if (m_unreadMsgCount.contains(subscriptionId))
			count += m_unreadMsgCount[subscriptionId];
		m_unreadMsgCount[subscriptionId] = count;
		m_subscriptionMessageDB->appendUnreadMsgCount(subscriptionId, count);

		// tip
		addMessage(dbMsg);

		emit unreadMsgChanged(subscriptionId, count);
	}
	
	// report new sequence
	reportLastSequence();
}

void SubscriptionMsgManager::onGetMsgNumberFinished(bool ok, const QMap<QString, int> &msgNumbers)
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
			m_subscriptionManager->getMessages(selfId, id, QString::number(m_lastSequence), QString(), count);
		}
	}
}

void SubscriptionMsgManager::onGetMessagesFinished(bool ok, const QString &subscriptionId, const QList<SubscriptionMsg> &messages)
{
	if (!m_running)
		return;

	if (!ok)
		return;

	if (messages.isEmpty())
		return;

	if (m_subscriptionMessageDB.isNull())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
	if (!subscriptionModel)
		return;

	if (!subscriptionModel->hasSubscription(subscriptionId))
		return;

	// save to message db
	QList<SubscriptionMsg> dbMsgs;
	foreach (SubscriptionMsg msg, messages)
	{
		SubscriptionMsg dbMsg = msg;
		m_subscriptionMessageDB->appendMessage(msg);
		quint64 innerId = ++m_currentMsgInnerId;
		dbMsg.setInnerId(innerId);
		dbMsgs.append(dbMsg);

		quint64 msgSequence = msg.id().toULongLong();
		if (msgSequence > m_lastSequence)
			m_lastSequence = msgSequence;
	}

	// add to last
	QString name = modelManager->subscriptionName(subscriptionId);
	modelManager->subscriptionLastMsgModel()->addLastMsg(dbMsgs.last(), name);

	// add message
	if (hasSubscriptionDialog(subscriptionId))
	{
		// refresh message
		SubscriptionMsgDialog *dlg = m_dialogs[subscriptionId].data();
		foreach (SubscriptionMsg dbMsg, dbMsgs)
		{
			dlg->appendMessage(dbMsg);
		}
		flashTaskBar(dlg);
	
		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsgs.last());
	}
	else if (SubscriptionLastMsgDialog::hasDialog())
	{
		flashTaskBar(SubscriptionLastMsgDialog::getDialog());

		// play message sound
		PlayBeep::playRecvSubscriptionMsgBeep();

		// add a message to last contact
		addLastContact(false, dbMsgs.last());

		// save to unread message
		int count = messages.count();
		if (m_unreadMsgCount.contains(subscriptionId))
			count += m_unreadMsgCount[subscriptionId];
		m_unreadMsgCount[subscriptionId] = count;
		m_subscriptionMessageDB->appendUnreadMsgCount(subscriptionId, count);

		emit unreadMsgChanged(subscriptionId, count);
	}
	else
	{
		// save to unread message
		int count = messages.count();
		if (m_unreadMsgCount.contains(subscriptionId))
			count += m_unreadMsgCount[subscriptionId];
		m_unreadMsgCount[subscriptionId] = count;
		m_subscriptionMessageDB->appendUnreadMsgCount(subscriptionId, count);

		// tip
		addMessage(dbMsgs.last());

		emit unreadMsgChanged(subscriptionId, count);
	}

	// report new sequence
	reportLastSequence();
}

void SubscriptionMsgManager::onSendMsgFinished(bool ok, const QString &subscriptionId, const SubscriptionMsg &msg)
{
	if (!m_running)
		return;

	if (!ok)
	{
		emit sendMsgFailed(subscriptionId);
		return;
	}

	if (subscriptionId.isEmpty() || msg.id().isEmpty())
		return;

	recvMsg(msg);
}

void SubscriptionMsgManager::onClickMenuFinished(bool ok, const QString &subscriptionId, 
												 const QString & /*key*/, const SubscriptionMsg &msg)
{
	if (!m_running)
		return;

	if (!ok)
	{
		emit clickMenuFailed(subscriptionId);
		return;
	}

	if (subscriptionId.isEmpty() || msg.id().isEmpty())
		return;

	recvMsg(msg);
}

void SubscriptionMsgManager::openUrl(const QString &subscriptionId, const QString &url)
{
	if (subscriptionId.isEmpty() || url.isEmpty())
		return;

	QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void SubscriptionMsgManager::openTitle(const QString &subscriptionId, const QString &idStr, const QString &messageIdStr)
{
	if (subscriptionId.isEmpty() || idStr.isEmpty() || messageIdStr.isEmpty())
		return;

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/subscription/material/article/detail?id=%2&subscriptionId=%3&messageId=%4")
		.arg(loginConfig.managerUrl).arg(idStr).arg(subscriptionId).arg(messageIdStr);
	
	SubscriptionWebView *webView = SubscriptionWebView::getWebView();
	webView->setSubscriptionId(subscriptionId);
	connect(webView, SIGNAL(openAttach(QString, QString, QString)), this, SLOT(openAttach(QString, QString, QString)), Qt::UniqueConnection);
	connect(webView, SIGNAL(openSubscriptionDetail(QString)), this, SIGNAL(openSubscriptionDetail(QString)), Qt::UniqueConnection);
	webView->load(urlString);
	WidgetManager::showActivateRaiseWindow(webView);
}

void SubscriptionMsgManager::clickMenu(const QString &subscriptionId, const QString &key)
{
	if (subscriptionId.isEmpty() || key.isEmpty())
		return;

	m_subscriptionManager->clickMenu(subscriptionId, Account::instance()->id(), key);
}

void SubscriptionMsgManager::openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name)
{
	if (subscriptionId.isEmpty() || urlStr.isEmpty())
		return;

	QString urlString = urlStr;
	if (!urlStr.startsWith("http://") && !urlStr.startsWith("https://"))
	{
		GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
		QUrl baseUrl = QUrl::fromUserInput(loginConfig.managerUrl);
		urlString = QString("%1://%2:%3/%4").arg(baseUrl.scheme()).arg(baseUrl.host()).arg(baseUrl.port(80)).arg(urlStr);
	}

	qDebug() << Q_FUNC_INFO << urlString << name;

	SubscriptionDownloadDialog *downloadDialog = SubscriptionDownloadDialog::getDialog();
	downloadDialog->addDownload(urlString, name);
	WidgetManager::showActivateRaiseWindow(downloadDialog);
}

bool SubscriptionMsgManager::hasSubscriptionDialog(const QString &subscriptionId) const
{
	if (!m_dialogs.contains(subscriptionId))
		return false;

	if (m_dialogs[subscriptionId].isNull())
		return false;

	return true;
}

SubscriptionMsgDialog *SubscriptionMsgManager::createDialog(const QString &subscriptionId)
{
	SubscriptionMsgDialog *dlg = new SubscriptionMsgDialog(subscriptionId);
	m_dialogs[subscriptionId] = dlg;
	connect(dlg, SIGNAL(openSubscriptionDetail(QString)), this, SIGNAL(openSubscriptionDetail(QString)));
	connect(dlg, SIGNAL(openUrl(QString, QString)), this, SLOT(openUrl(QString, QString)));
	connect(dlg, SIGNAL(clickMenu(QString, QString)), this, SLOT(clickMenu(QString, QString)));
	connect(dlg, SIGNAL(openTitle(QString, QString, QString)), this, SLOT(openTitle(QString, QString, QString)));
	connect(dlg, SIGNAL(openAttach(QString, QString, QString)), this, SLOT(openAttach(QString, QString, QString)));
	connect(dlg, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	return dlg;
}

void SubscriptionMsgManager::reportLastSequence()
{
	if (m_lastSequence == 0)
		return;

	QString selfId = Account::instance()->id();
	QString lastSequence = QString::number(m_lastSequence);

	m_subscriptionManager->report(selfId, lastSequence);

	Account::settings()->setSubscriptionLastSequence(m_lastSequence);
}

void SubscriptionMsgManager::flashTaskBar(QWidget *widget)
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

void SubscriptionMsgManager::addLastContact(bool send, const SubscriptionMsg &msg)
{
	// add a message to last contact
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	msgBody.setSend(send);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(QString(SUBSCRIPTION_ROSTER_ID));
	msgBody.setToName(ModelManager::subscriptionShowName());
	msgBody.setTime(msg.createTime());
	msgBody.setStamp(makeSubscriptionStamp(msg.innerId()));
	if (send)
	{
		msgBody.setBody(makeMessageText(msg, tr("I")));
	}
	else
	{
		QString name = modelManager->subscriptionName(msg.subscriptionId());
		msgBody.setBody(makeMessageText(msg, name));
	}
	modelManager->lastContactModel()->appendMsg(msgBody);
}

void SubscriptionMsgManager::addMessage(const SubscriptionMsg &msg)
{
	// add a message to unread message
	bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	msgBody.setSend(false);
	msgBody.setFrom(Account::instance()->id());
	msgBody.setTo(QString(SUBSCRIPTION_ROSTER_ID));
	msgBody.setTime(msg.createTime());
	QString stamp = makeSubscriptionStamp(msg.innerId());
	msgBody.setStamp(stamp);
	QString name = qPmApp->getModelManager()->subscriptionName(msg.subscriptionId());
	QString msgText = makeMessageText(msg, name);
	msgBody.setBody(msgText);
	qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
}

QString SubscriptionMsgManager::makeMessageText(const SubscriptionMsg &msg, const QString &name) const
{	
	if (msg.id().isEmpty())
		return QString();

	QString text = QString("%1: %2").arg(name).arg(msg.bodyText());
	return text;
}

QString SubscriptionMsgManager::makeSubscriptionStamp(quint64 innerId) const
{
	QString stamp = QString("%1").arg(innerId, 29, 10, QChar('0'));
	return stamp;
}
