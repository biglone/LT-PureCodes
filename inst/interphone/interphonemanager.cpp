#include "interphonemanager.h"
#include <QMetaType>
#include <QDebug>
#include "pmclient/PmClient.h"
#include "protocol/ProtocolConst.h"
#include "protocol/ProtocolType.h"
#include "pmclient/PmClientInterface.h"
#include "Account.h"
#include "protocol/InterphoneRequest.h"
#include "protocol/InterphoneResponse.h"
#include "protocol/InterphoneNotification.h"

static const char *kSpeakOn  = "on";
static const char *kSpeakOff = "off";

static const char *kChat    = "chat";
static const char *kGroup   = "group";
static const char *kDiscuss = "discuss";

static const QString kChatPrefix    = QString("chat_");
static const QString kGroupPrefix   = QString("group_");
static const QString kDiscussPrefix = QString("discuss_");

//////////////////////////////////////////////////////////////////////////
// class InterphoneResHandler
class InterphoneResHandler : public QObject, public IPmClientResponseHandler
{
Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	InterphoneResHandler(InterphoneManager *manager);

public:
	void syncInterphones(const QStringList &groupIds, const QStringList &discussIds);
	void syncInterphoneMember(const QString &interphoneId);
	void addInterphone(const QString &interphoneId, const QString &uid);
	void quitInterphone(const QString &interphoneId, const QString &uid);
	void prepareSpeak(const QString &interphoneId);
	void stopSpeak(const QString &interphoneId);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private:
	void processInterphone(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

private Q_SLOTS:
	void doSyncInterphones(bool force = false);

private:
	int                m_handleId;
	InterphoneManager *m_interphoneManager;
	QStringList        m_syncIds;
};

InterphoneResHandler::InterphoneResHandler(InterphoneManager *manager)
: m_handleId(-1)
, m_interphoneManager(manager)
{
	Q_ASSERT(m_interphoneManager != NULL);
}

void InterphoneResHandler::syncInterphones(const QStringList &groupIds, const QStringList &discussIds)
{
	m_syncIds.clear();

	QString interphoneId;
	foreach (QString groupId, groupIds)
	{
		interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_GroupChat, groupId);
		m_syncIds.append(interphoneId);
	}
	foreach (QString discussId, discussIds)
	{
		interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_DiscussChat, discussId);
		m_syncIds.append(interphoneId);
	}

	doSyncInterphones(true);
}

void InterphoneResHandler::syncInterphoneMember(const QString &interphoneId)
{
	std::vector<std::string> params;
	protocol::InterphoneRequest *pReq = new protocol::InterphoneRequest(protocol::InterphoneRequest::Action_Member, 
		interphoneId.toUtf8().constData(), params);
	PmClient::instance()->send(pReq);
}

void InterphoneResHandler::addInterphone(const QString &interphoneId, const QString &uid)
{
	std::vector<std::string> params;
	params.push_back(uid.toUtf8().constData());
	protocol::InterphoneRequest *pReq = new protocol::InterphoneRequest(protocol::InterphoneRequest::Action_Add, 
		interphoneId.toUtf8().constData(), params);
	PmClient::instance()->send(pReq);
}

void InterphoneResHandler::quitInterphone(const QString &interphoneId, const QString &uid)
{
	std::vector<std::string> params;
	params.push_back(uid.toUtf8().constData());
	protocol::InterphoneRequest *pReq = new protocol::InterphoneRequest(protocol::InterphoneRequest::Action_Quit, 
		interphoneId.toUtf8().constData(), params);
	PmClient::instance()->send(pReq);
}

void InterphoneResHandler::prepareSpeak(const QString &interphoneId)
{
	std::vector<std::string> params;
	params.push_back("on");
	protocol::InterphoneRequest *pReq = new protocol::InterphoneRequest(protocol::InterphoneRequest::Action_Speak, 
		interphoneId.toUtf8().constData(), params);
	PmClient::instance()->send(pReq);
}

void InterphoneResHandler::stopSpeak(const QString &interphoneId)
{
	std::vector<std::string> params;
	params.push_back("off");
	protocol::InterphoneRequest *pReq = new protocol::InterphoneRequest(protocol::InterphoneRequest::Action_Speak, 
		interphoneId.toUtf8().constData(), params);
	PmClient::instance()->send(pReq);
}

bool InterphoneResHandler::initObject()
{
	m_handleId = PmClient::instance()->insertResponseHandler(this);
	if (m_handleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void InterphoneResHandler::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_handleId);
	m_handleId = -1;
}

QObject * InterphoneResHandler::instance()
{
	return this;
}

int InterphoneResHandler::handledId() const
{
	return m_handleId;
}

QList<int> InterphoneResHandler::types() const
{
	return QList<int>() << protocol::Request_Interphone_Interphone;
}

bool InterphoneResHandler::onRequestResult(int handleId, net::Request *req, protocol::Response *res)
{
	if (m_handleId != handleId)
	{
		return false;
	}

	int type = req->getType();
	switch (type)
	{
	case protocol::Request_Interphone_Interphone:
		processInterphone(req, res);
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error";
	}

	return true;
}

void InterphoneResHandler::processInterphone(net::Request *req, protocol::Response *res)
{
	if (processResponseError(req)) // error
		return;

	// process interphone response
	protocol::InterphoneResponse *pRes = static_cast<protocol::InterphoneResponse *>(res);
	Q_ASSERT(pRes != NULL);

	int type = pRes->getActionType();
	if (type == protocol::InterphoneRequest::Action_Sync)
	{
		std::vector<protocol::InterphoneResponse::Item> items = pRes->items();
		QList<InterphoneInfo> interphones;
		for (int i = 0; i < (int)items.size(); i++)
		{
			protocol::InterphoneResponse::Item item = items[i];
			InterphoneInfo info;
			info.setId(QString::fromUtf8(item.m_iid.c_str()));
			bean::MessageType attachType = InterphoneManager::string2AttachType(QString::fromUtf8(item.m_attachType.c_str()));
			QString attachId = QString::fromUtf8(item.m_attachId.c_str());
			info.setAttachType(attachType);
			info.setAttachId(attachId);
			info.setMemberCount(item.m_memberCount);
			interphones.append(info);
		}

		QMetaObject::invokeMethod(m_interphoneManager, "processInterphones", 
			Qt::QueuedConnection, Q_ARG(QList<InterphoneInfo>, interphones));

		// do next sync
		QMetaObject::invokeMethod(this, "doSyncInterphones", Qt::QueuedConnection, Q_ARG(bool, false));
	}
	else if (type == protocol::InterphoneRequest::Action_Member || 
		     type == protocol::InterphoneRequest::Action_Add ||
			 type == protocol::InterphoneRequest::Action_Quit)
	{
		InterphoneInfo info;
		info.setId(QString::fromUtf8(pRes->interphoneId().c_str()));
		bean::MessageType attachType = InterphoneManager::string2AttachType(QString::fromUtf8(pRes->attachType().c_str()));
		QString attachId = QString::fromUtf8(pRes->attachId().c_str());
		info.setAttachType(attachType);
		info.setAttachId(attachId);
		info.setMemberCount(pRes->memberCount());
		QString speakerFullId = QString::fromUtf8(pRes->speakId().c_str());
		info.setSpeakerId(Account::idFromFullId(speakerFullId));
		QStringList memberIds;
		std::vector<std::string> ids = pRes->memberIds();
		for (int i = 0; i < (int)ids.size(); i++)
		{
			QString id = QString::fromUtf8(ids[i].c_str());
			memberIds.append(id);
		}
		info.setMemberIds(memberIds);

		QString audioAddr;
		if (type == protocol::InterphoneRequest::Action_Add)
			audioAddr = QString::fromUtf8(pRes->audioAddr().c_str());

		if (type == protocol::InterphoneRequest::Action_Member)
		{
			QMetaObject::invokeMethod(m_interphoneManager, "processInterphoneMember",
				Qt::QueuedConnection, Q_ARG(InterphoneInfo, info));
		}
		else if (type == protocol::InterphoneRequest::Action_Add)
		{
			QMetaObject::invokeMethod(m_interphoneManager, "processInterphoneAdd",
				Qt::QueuedConnection, Q_ARG(InterphoneInfo, info), Q_ARG(QString, audioAddr));
		}
		else
		{
			QMetaObject::invokeMethod(m_interphoneManager, "processInterphoneQuit",
				Qt::QueuedConnection, Q_ARG(InterphoneInfo, info));
		}
	}
	else if (type == protocol::InterphoneRequest::Action_Speak)
	{
		protocol::InterphoneRequest *interphoneRequest = static_cast<protocol::InterphoneRequest *>(req);
		std::vector<std::string> params = interphoneRequest->params();
		QString param = QString::fromUtf8(params[0].c_str());
		QString interphoneId = QString::fromUtf8(interphoneRequest->interphoneId().c_str());
		QMetaObject::invokeMethod(m_interphoneManager, "processSpeak",
			Qt::QueuedConnection, Q_ARG(QString, interphoneId), Q_ARG(QString, param));
	}
	else // if (type == protocol::InterphoneRequest::Action_None)
	{
		qWarning() << Q_FUNC_INFO << "invalid request action type:" << type;
	}
}

bool InterphoneResHandler::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		protocol::InterphoneRequest *interphoneRequest = static_cast<protocol::InterphoneRequest *>(req);
		int type = interphoneRequest->actionType();
		QString param;
		QString interphoneId = QString::fromUtf8(interphoneRequest->interphoneId().c_str());
		if (type == protocol::InterphoneRequest::Action_Speak)
		{
			std::vector<std::string> params = interphoneRequest->params();
			param = QString::fromUtf8(params[0].c_str());
		}

		QMetaObject::invokeMethod(m_interphoneManager, "setError", Qt::QueuedConnection,
			Q_ARG(int, type), Q_ARG(QString, interphoneId), Q_ARG(QString, param), 
			Q_ARG(QString, QString::fromUtf8(req->getMessage().c_str())));

		if (type == protocol::InterphoneRequest::Action_Sync)
		{
			// do next sync
			QMetaObject::invokeMethod(this, "doSyncInterphones", Qt::QueuedConnection, Q_ARG(bool, false));
		}
	}

	return bError;
}

void InterphoneResHandler::doSyncInterphones(bool force /*= false*/)
{
	const int kMaxOnceRequest = 50;
	std::vector<std::string> params;
	int syncCount = 0;
	QString interphoneId;
	while (!m_syncIds.isEmpty() && syncCount < kMaxOnceRequest)
	{
		interphoneId = m_syncIds.takeFirst();
		params.push_back(interphoneId.toUtf8().constData());
		++syncCount;
	}

	if (!params.empty() || force)
	{
		protocol::InterphoneRequest *pReq = new protocol::InterphoneRequest(protocol::InterphoneRequest::Action_Sync, "", params);
		PmClient::instance()->send(pReq);
	}
}

//////////////////////////////////////////////////////////////////////////
// class InterphoneNtfHandler
class InterphoneNtfHandler : public QObject, public IPmClientNotificationHandler
{
Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler);

public:
	InterphoneNtfHandler(InterphoneManager *manager);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private:
	int                m_handleId;
	InterphoneManager *m_interphoneManager;
};

InterphoneNtfHandler::InterphoneNtfHandler(InterphoneManager *manager)
: m_interphoneManager(manager)
, m_handleId(-1)
{
	Q_ASSERT(m_interphoneManager != NULL);
}

bool InterphoneNtfHandler::initObject()
{
	m_handleId = PmClient::instance()->insertNotificationHandler(this);
	if (m_handleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void InterphoneNtfHandler::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_handleId);
	m_handleId = -1;
}

QObject * InterphoneNtfHandler::instance()
{
	return this;
}

QList<int> InterphoneNtfHandler::types() const
{
	return QList<int>() << protocol::INTERPHONE;
}

int InterphoneNtfHandler::handledId() const
{
	return m_handleId;
}

bool InterphoneNtfHandler::onNotication(int handleId, protocol::SpecificNotification *sn)
{
	if (m_handleId != handleId)
		return false;

	protocol::InterphoneNotification *pIn = static_cast<protocol::InterphoneNotification *>(sn);

	InterphoneInfo info;
	info.setId(QString::fromUtf8(pIn->interphoneId().c_str()));
	bean::MessageType attachType = InterphoneManager::string2AttachType(QString::fromUtf8(pIn->attachType().c_str()));
	QString attachId = QString::fromUtf8(pIn->attachId().c_str());
	info.setAttachType(attachType);
	info.setAttachId(attachId);
	info.setMemberCount(pIn->memberCount());
	QString speakerFullId = QString::fromUtf8(pIn->speakId().c_str());
	info.setSpeakerId(Account::idFromFullId(speakerFullId));

	QMetaObject::invokeMethod(m_interphoneManager, "processInterphoneChanged",
		Qt::QueuedConnection, Q_ARG(InterphoneInfo, info));

	return true;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS InterphoneManager
InterphoneManager::InterphoneManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
	qRegisterMetaType< InterphoneInfo >("InterphoneInfo");
	qRegisterMetaType< QList<InterphoneInfo> >("QList<InterphoneInfo>");

	m_resHandler.reset(new InterphoneResHandler(this));
	m_ntfHandler.reset(new InterphoneNtfHandler(this));
}

InterphoneManager::~InterphoneManager()
{

}

bool InterphoneManager::initObject()
{
	return m_resHandler->initObject() && m_ntfHandler->initObject();
}

void InterphoneManager::removeObject()
{
	m_resHandler->removeObject();
	m_ntfHandler->removeObject();
}

void InterphoneManager::syncInterphones(const QStringList &groupIds, const QStringList &discussIds)
{
	m_resHandler->syncInterphones(groupIds, discussIds);
}

void InterphoneManager::syncInterphoneMember(const QString &interphoneId)
{
	if (interphoneId.isEmpty())
		return;

	m_resHandler->syncInterphoneMember(interphoneId);
}

void InterphoneManager::addInterphone(const QString &interphoneId, const QString &uid)
{
	if (interphoneId.isEmpty())
		return;

	m_resHandler->addInterphone(interphoneId, uid);
}

void InterphoneManager::quitInterphone(const QString &interphoneId, const QString &uid)
{
	if (interphoneId.isEmpty())
		return;

	if (m_currentInterphone == interphoneId)
		m_currentInterphone = QString();

	m_resHandler->quitInterphone(interphoneId, uid);
}

void InterphoneManager::prepareSpeak(const QString &interphoneId)
{
	if (interphoneId.isEmpty())
		return;

	m_resHandler->prepareSpeak(interphoneId);
}

void InterphoneManager::stopSpeak(const QString &interphoneId)
{
	if (interphoneId.isEmpty())
		return;

	m_resHandler->stopSpeak(interphoneId);
}

void InterphoneManager::clearInterphones()
{
	m_currentInterphone = QString();
	m_interphones.clear();

	emit interphonesCleared();
}

void InterphoneManager::quitCurrentInterphone(const QString &uid)
{
	if (!m_currentInterphone.isEmpty())
	{
		QString interphoneId = m_currentInterphone;
		quitInterphone(interphoneId, uid);
	}
}

void InterphoneManager::removeInterphone(const QString &interphoneId)
{
	if (m_interphones.contains(interphoneId))
	{
		m_interphones.remove(interphoneId);

		if (m_currentInterphone == interphoneId)
			m_currentInterphone = QString();
	}
}

bool InterphoneManager::isCurrentInInterphone() const
{
	return !m_currentInterphone.isEmpty();
}

QString InterphoneManager::currentInterphone() const
{
	return m_currentInterphone;
}

bool InterphoneManager::hasInterphone(bean::MessageType attachType, const QString &attachId)
{
	QString id = attachTypeId2InterphoneId(attachType, attachId);
	if (m_interphones.contains(id))
	{
		InterphoneInfo info = m_interphones[id];
		if (info.memberCount() > 0)
		{
			return true;
		}
	}
	return false;
}

bool InterphoneManager::isInInterphone(bean::MessageType attachType, const QString &attachId)
{
	if (m_currentInterphone.isEmpty())
		return false;

	QString id = attachTypeId2InterphoneId(attachType, attachId);
	if (m_currentInterphone == id)
		return true;

	return false;
}

QMap<QString, InterphoneInfo> InterphoneManager::allInterphones() const
{
	return m_interphones;
}

InterphoneInfo InterphoneManager::interphone(const QString &id) const
{
	InterphoneInfo info;
	if (m_interphones.contains(id))
		info = m_interphones[id];
	return info;
}

QString InterphoneManager::interphoneAudioAddr() const
{
	return m_audioAddr;
}

QString InterphoneManager::attachTypeId2InterphoneId(bean::MessageType attachType, const QString &attachId)
{
	QString id;
	if (attachType == bean::Message_Chat)
	{
		QString selfId = Account::instance()->id();
		if (selfId < attachId)
			id = QString("chat_%1:%2").arg(selfId).arg(attachId);
		else
			id = QString("chat_%1:%2").arg(attachId).arg(selfId);
	}
	else if (attachType == bean::Message_GroupChat)
	{
		id = QString("group_%1").arg(attachId);
	}
	else if (attachType == bean::Message_DiscussChat)
	{
		id = QString("discuss_%1").arg(attachId);
	}
	return id;
}

void InterphoneManager::interphoneId2AttachTypeId(const QString &interphoneId, bean::MessageType &attachType, QString &attachId)
{
	attachType = bean::Message_Invalid;
	attachId = "";
	if (interphoneId.startsWith(kChatPrefix))
	{
		QStringList ids = interphoneId.mid(kChatPrefix.length()).split(":");
		if (ids.count() == 2)
		{
			QString id1 = ids[0];
			QString id2 = ids[1];
			QString selfId = Account::instance()->id();
			if (selfId == id1)
				attachId = id2;
			else
				attachId = id1;
			attachType = bean::Message_Chat;
		}
	}
	else if (interphoneId.startsWith(kGroupPrefix))
	{
		attachId = interphoneId.mid(kGroupPrefix.length());
		attachType = bean::Message_GroupChat;
	}
	else if (interphoneId.startsWith(kDiscussPrefix))
	{
		attachId = interphoneId.mid(kDiscussPrefix.length());
		attachType = bean::Message_DiscussChat;
	}
}

bean::MessageType InterphoneManager::string2AttachType(const QString &typeStr)
{
	if (typeStr == kChat)
		return bean::Message_Chat;
	else if (typeStr == kGroup)
		return bean::Message_GroupChat;
	else if (typeStr == kDiscuss)
		return bean::Message_DiscussChat;
	return bean::Message_Invalid;
}

void InterphoneManager::setError(int type, const QString &interphoneId, const QString &param, const QString &errMsg)
{
	if (type == protocol::InterphoneRequest::Action_Sync)
	{
		qWarning() << Q_FUNC_INFO << "sync interphones failed: " << errMsg;
		
		emit syncInterphonesFinished(false);
	}
	else if (type == protocol::InterphoneRequest::Action_Member)
	{
		qWarning() << Q_FUNC_INFO << "sync interphone member failed: " << errMsg;

		emit syncInterphoneMemberFinished(false, interphoneId);
	}
	else if (type == protocol::InterphoneRequest::Action_Add)
	{
		qWarning() << Q_FUNC_INFO << "add interphone failed: " << errMsg;

		emit addInterphoneFinished(false, interphoneId);
	}
	else if (type == protocol::InterphoneRequest::Action_Quit)
	{
		qWarning() << Q_FUNC_INFO << "quit interphone failed: " << errMsg;

		emit quitInterphoneFinished(false, interphoneId);
	}
	else if (type == protocol::InterphoneRequest::Action_Speak)
	{
		qWarning() << Q_FUNC_INFO << "interphone speak failed: " << param << errMsg;

		if (param == kSpeakOn)
		{
			emit prepareSpeakFailed(interphoneId);
		}
		else if (param == kSpeakOff)
		{
			emit stopSpeakFailed(interphoneId);
		}
	}
}

void InterphoneManager::processInterphones(const QList<InterphoneInfo> &interphones)
{
	for (int i = 0; i < interphones.count(); i++)
	{
		InterphoneInfo interphone = interphones[i];
		m_interphones.insert(interphone.id(), interphone);
	}
	emit syncInterphonesFinished(true);
}

void InterphoneManager::processInterphoneMember(const InterphoneInfo &interphone)
{
	QString id = interphone.id();
	if (m_interphones.contains(id))
	{
		m_interphones[id] = interphone;
	}
	else if (interphone.memberCount() > 0)
	{
		m_interphones[id] = interphone;
		emit interphoneStarted(interphone.id(), (int)interphone.attachType(), interphone.attachId());
	}

	emit syncInterphoneMemberFinished(true, id);
}

void InterphoneManager::processInterphoneAdd(const InterphoneInfo &interphone, const QString &audioAddr)
{
	m_audioAddr = audioAddr;

	QString id = interphone.id();
	bool newCreated = false;
	if (!m_interphones.contains(id))
	{
		newCreated = true;
	}

	m_currentInterphone = id;
	m_interphones[id] = interphone;

	if (newCreated)
	{
		emit interphoneStarted(interphone.id(), (int)interphone.attachType(), interphone.attachId());
	}

	emit addInterphoneFinished(true, id);
}

void InterphoneManager::processInterphoneQuit(const InterphoneInfo &interphone)
{
	QString id = interphone.id();
	int memberCount = interphone.memberCount();
	if (m_interphones.contains(id))
	{
		m_currentInterphone = QString();
		if (memberCount <= 0)
		{
			m_interphones.remove(id);

			emit interphoneFinished(id);
		}
		else
		{
			m_interphones[id] = interphone;
		}

		emit quitInterphoneFinished(true, id);
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "id is not in interphones: " << id << m_interphones.keys();
	}
}

void InterphoneManager::processSpeak(const QString &interphoneId, const QString &speakState)
{
	if (speakState == kSpeakOn)
	{
		emit prepareSpeakOK(interphoneId);
	}
	else
	{
		emit stopSpeakOK(interphoneId);
	}
}

void InterphoneManager::processInterphoneChanged(const InterphoneInfo &interphone)
{
	QString id = interphone.id();
	int memberCount = interphone.memberCount();
	if (m_interphones.contains(id))
	{
		if (memberCount <= 0)
		{
			m_interphones.remove(id);

			emit interphoneFinished(id);
		}
		else
		{
			m_interphones[id] = interphone;

			emit interphoneChanged(id, (int)interphone.attachType(), interphone.attachId());
		}
	}
	else if (memberCount > 0)
	{
		m_interphones[id] = interphone;

		emit interphoneStarted(id, (int)(interphone.attachType()), interphone.attachId());
	}
}

#include "interphonemanager.moc"