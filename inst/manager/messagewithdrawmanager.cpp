#include "pmclient/PmClient.h"
#include "protocol/ProtocolConst.h"
#include "protocol/ProtocolType.h"
#include "protocol/WithdrawMessage.h"
#include "pmclient/PmClientInterface.h"
#include <QDebug>
#include "Account.h"

#include "messagewithdrawmanager.h"

static const char *TYPE_CHAT         = "chat";
static const char *TYPE_GROUPCHAT    = "groupchat";
static const char *TYPE_DISCUSS      = "discuss";

//////////////////////////////////////////////////////////////////////////
// class WithdrawResHandler
class WithdrawResHandler : public QObject, public IPmClientResponseHandler
{
Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	WithdrawResHandler(MessageWithdrawManager *pMgr);

public:
	void syncWithdraws(const QString &uid, const QString &withdrawId);
	void withdraw(bean::MessageType chatType, const QString &toId, const QString &fromId, const QString &timeStamp);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private:
	void processWithdrawSync(net::Request* req, protocol::Response* res);
	void processWithdraw(net::Request* req, protocol::Response* res);
	bool processResponseError(int type, net::Request* req);

private:
	int                     m_handleId;
	MessageWithdrawManager *m_pMgr;
};

WithdrawResHandler::WithdrawResHandler(MessageWithdrawManager *pMgr)
	: m_handleId(-1)
	, m_pMgr(pMgr)
{
	Q_ASSERT(m_pMgr != NULL);
}

void WithdrawResHandler::syncWithdraws(const QString &uid, const QString &withdrawId)
{
	protocol::WithdrawMessage::WithdrawSyncRequest *pReq = 0;
	pReq = new protocol::WithdrawMessage::WithdrawSyncRequest(
		uid.toUtf8().constData(), withdrawId.toUtf8().constData());
	PmClient::instance()->send(pReq);
}

void WithdrawResHandler::withdraw(bean::MessageType chatType, 
	                              const QString &toId, 
								  const QString &fromId, 
								  const QString &timeStamp)
{
	const char *szType = 0;
	switch (chatType)
	{
	case bean::Message_Chat:
		szType = TYPE_CHAT;
		break;
	case bean::Message_GroupChat:
		szType = TYPE_GROUPCHAT;
		break;
	case bean::Message_DiscussChat:
		szType = TYPE_DISCUSS;
		break;
	default:
		break;
	}

	if (!chatType || toId.isEmpty() || fromId.isEmpty() || timeStamp.isEmpty())
		return;

	protocol::WithdrawMessage::WithdrawRequest *pReq = 0;
	pReq = new protocol::WithdrawMessage::WithdrawRequest(
		szType, toId.toUtf8().constData(), fromId.toUtf8().constData(), timeStamp.toUtf8().constData());
	PmClient::instance()->send(pReq);
}

bool WithdrawResHandler::initObject()
{
	m_handleId = PmClient::instance()->insertResponseHandler(this);
	if (m_handleId < 0)
	{
		qDebug() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void WithdrawResHandler::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_handleId);
	m_handleId = -1;
}

QObject* WithdrawResHandler::instance()
{
	return this;
}

int WithdrawResHandler::handledId() const
{
	return m_handleId;
}

QList<int> WithdrawResHandler::types() const
{
	QList<int> ret;
	ret << protocol::Request_Withdraw_Sync << protocol::Request_Message_Withdraw;
	return ret;
}

bool WithdrawResHandler::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_handleId != handleId)
		return false;

	int type = req->getType();

	// process
	switch (type)
	{
	case protocol::Request_Withdraw_Sync:
		processWithdrawSync(req, res);
		break;
	case protocol::Request_Message_Withdraw:
		processWithdraw(req, res);
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error";
		break;
	}

	return true;
}

void WithdrawResHandler::processWithdrawSync(net::Request* req, protocol::Response* res)
{
	if (processResponseError(protocol::Request_Withdraw_Sync, req)) // error
		return;

	// process withdraws
	protocol::WithdrawMessage::WithdrawSyncResponse *pRes = static_cast<protocol::WithdrawMessage::WithdrawSyncResponse *>(res);
	if (!pRes)
		return;

	std::list<protocol::WithdrawMessage::WithdrawItem> withdrawItems = pRes->withdrawItems();
	std::list<protocol::WithdrawMessage::WithdrawItem>::iterator it;
	for (it = withdrawItems.begin(); it != withdrawItems.end(); ++it)
	{
		protocol::WithdrawMessage::WithdrawItem withdrawItem = *it;
		QString type = QString::fromUtf8(withdrawItem.m_chatType.c_str());
		QString to = QString::fromUtf8(withdrawItem.m_to.c_str());
		QString from = QString::fromUtf8(withdrawItem.m_from.c_str());
		QString timestamp = QString::fromUtf8(withdrawItem.m_timeStamp.c_str());
		QString withdrawId = QString::fromUtf8(withdrawItem.m_withdrawId.c_str());
		bean::MessageType chatType = bean::Message_Chat;
		if (type == QString::fromLatin1(TYPE_GROUPCHAT))
			chatType = bean::Message_GroupChat;
		else if (type == QString::fromLatin1(TYPE_DISCUSS))
			chatType = bean::Message_DiscussChat;

		QMetaObject::invokeMethod(m_pMgr, "messageWithdrawed", Qt::QueuedConnection, 
			Q_ARG(bean::MessageType, chatType), Q_ARG(QString, to), Q_ARG(QString, from), 
			Q_ARG(QString, timestamp), Q_ARG(QString, withdrawId));
	}
}

void WithdrawResHandler::processWithdraw(net::Request* req, protocol::Response* res)
{
	if (processResponseError(protocol::Request_Message_Withdraw, req)) // error
		return;

	// process withdraw
	protocol::WithdrawMessage::WithdrawResponse *pRes = static_cast<protocol::WithdrawMessage::WithdrawResponse *>(res);
	if (!pRes)
		return;

	protocol::WithdrawMessage::WithdrawItem withdrawItem = pRes->withdrawItem();
	QString type = QString::fromUtf8(withdrawItem.m_chatType.c_str());
	QString to = QString::fromUtf8(withdrawItem.m_to.c_str());
	QString from = QString::fromUtf8(withdrawItem.m_from.c_str());
	QString timestamp = QString::fromUtf8(withdrawItem.m_timeStamp.c_str());
	QString withdrawId = QString::fromUtf8(withdrawItem.m_withdrawId.c_str());
	bean::MessageType chatType = bean::Message_Chat;
	if (type == QString::fromLatin1(TYPE_GROUPCHAT))
		chatType = bean::Message_GroupChat;
	else if (type == QString::fromLatin1(TYPE_DISCUSS))
		chatType = bean::Message_DiscussChat;

	QMetaObject::invokeMethod(m_pMgr, "withdrawOK", Qt::QueuedConnection, 
		Q_ARG(bean::MessageType, chatType), Q_ARG(QString, to), Q_ARG(QString, from), 
		Q_ARG(QString, timestamp), Q_ARG(QString, withdrawId));
}

bool WithdrawResHandler::processResponseError(int type, net::Request *req)
{
	bool bError = !req->getResult();
	if (bError)
	{
		if (type == protocol::Request_Withdraw_Sync)
		{
			protocol::WithdrawMessage::WithdrawSyncRequest *pReq;
			pReq = static_cast<protocol::WithdrawMessage::WithdrawSyncRequest *>(req);
			if (!pReq)
				return bError;

			QString uid = QString::fromUtf8(pReq->uid().c_str());
			QString withdrawId = QString::fromUtf8(pReq->withdrawId().c_str());
			qDebug() << Q_FUNC_INFO << "sync withdraw messages failed: " << uid << withdrawId << QString::fromUtf8(req->getMessage().c_str());
		}

		if (type == protocol::Request_Message_Withdraw)
		{
			protocol::WithdrawMessage::WithdrawRequest *pReq;
			pReq = static_cast<protocol::WithdrawMessage::WithdrawRequest *>(req);
			if (!pReq)
				return bError;

			QString type = QString::fromUtf8(pReq->chatType().c_str());
			QString to = QString::fromUtf8(pReq->chatTo().c_str());
			QString from = QString::fromUtf8(pReq->chatFrom().c_str());
			QString timestamp = QString::fromUtf8(pReq->chatTimeStamp().c_str());
			bean::MessageType chatType = bean::Message_Chat;
			if (type == QString::fromLatin1(TYPE_GROUPCHAT))
				chatType = bean::Message_GroupChat;
			else if (type == QString::fromLatin1(TYPE_DISCUSS))
				chatType = bean::Message_DiscussChat;

			QMetaObject::invokeMethod(m_pMgr, "withdrawFailed", Qt::QueuedConnection, 
				Q_ARG(bean::MessageType, chatType), Q_ARG(QString, to), Q_ARG(QString, from), Q_ARG(QString, timestamp));
		}
	}

	return bError;
}

//////////////////////////////////////////////////////////////////////////
// class WithdrawNtfHandler
class WithdrawNtfHandler : public QObject, public IPmClientNotificationHandler
{
Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler);

public:
	WithdrawNtfHandler(MessageWithdrawManager *pMgr);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private:
	int                     m_handleId;
	MessageWithdrawManager *m_pMgr;
};

WithdrawNtfHandler::WithdrawNtfHandler(MessageWithdrawManager *pMgr)
	: m_pMgr(pMgr)
	, m_handleId(-1)
{
	Q_ASSERT(m_pMgr != NULL);
}

bool WithdrawNtfHandler::initObject()
{
	m_handleId = PmClient::instance()->insertNotificationHandler(this);
	if (m_handleId < 0)
	{
		qDebug() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void WithdrawNtfHandler::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_handleId);
	m_handleId = -1;
}

QObject* WithdrawNtfHandler::instance()
{
	return this;
}

QList<int> WithdrawNtfHandler::types() const
{
	return QList<int>() << protocol::MESSAGE_WITHDRAW;
}

int WithdrawNtfHandler::handledId() const
{
	return m_handleId;
}

bool WithdrawNtfHandler::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_handleId != handleId)
		return false;

	protocol::WithdrawMessage::WithdrawNotification *pIn = static_cast<protocol::WithdrawMessage::WithdrawNotification *>(sn);
	if (pIn)
	{
		protocol::WithdrawMessage::WithdrawItem withdrawItem = pIn->withdrawItem();
		QString type = QString::fromUtf8(withdrawItem.m_chatType.c_str());
		QString to = QString::fromUtf8(withdrawItem.m_to.c_str());
		QString from = QString::fromUtf8(withdrawItem.m_from.c_str());
		QString timestamp = QString::fromUtf8(withdrawItem.m_timeStamp.c_str());
		QString withdrawId = QString::fromUtf8(withdrawItem.m_withdrawId.c_str());
		bean::MessageType chatType = bean::Message_Chat;
		if (type == QString::fromLatin1(TYPE_GROUPCHAT))
			chatType = bean::Message_GroupChat;
		else if (type == QString::fromLatin1(TYPE_DISCUSS))
			chatType = bean::Message_DiscussChat;

		QMetaObject::invokeMethod(m_pMgr, "messageWithdrawed", Qt::QueuedConnection, 
			Q_ARG(bean::MessageType, chatType), Q_ARG(QString, to), Q_ARG(QString, from), 
			Q_ARG(QString, timestamp), Q_ARG(QString, withdrawId));
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// class MessageWithdrawManager
MessageWithdrawManager::MessageWithdrawManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
	m_resHandler.reset(new WithdrawResHandler(this));
	m_ntfHandler.reset(new WithdrawNtfHandler(this));
}

MessageWithdrawManager::~MessageWithdrawManager()
{

}

void MessageWithdrawManager::initObject()
{
	m_resHandler->initObject();
	m_ntfHandler->initObject();
}

void MessageWithdrawManager::removeObject()
{
	m_resHandler->removeObject();
	m_ntfHandler->removeObject();
}

void MessageWithdrawManager::syncWithdraws(const QString &uid, const QString &withdrawId)
{
	m_resHandler->syncWithdraws(uid, withdrawId);
}

void MessageWithdrawManager::withdraw(bean::MessageType chatType, 
	                                  const QString &toId, 
									  const QString &fromId, 
									  const QString &timeStamp)
{
	m_resHandler->withdraw(chatType, toId, fromId, timeStamp);
}

void MessageWithdrawManager::setLastWithdrawId(const QString &withdrawId, bool force /*= false*/)
{
	if (force)
	{
		m_withdrawId = withdrawId;
	}
	else
	{
		if (withdrawId > m_withdrawId)
		{
			m_withdrawId = withdrawId;
			Account::settings()->setLastWithdrawId(m_withdrawId);
		}
	}
}

QString MessageWithdrawManager::lastWithdrawId() const
{
	return m_withdrawId;
}

#include "MessageWithdrawManager.moc"