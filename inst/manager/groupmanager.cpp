#include "groupmanager.h"
#include "protocol/GroupRequest.h"
#include "protocol/GroupResponse.h"
#include "protocol/GroupNotification.h"
#include "pmclient/PmClient.h"
#include "pmclient/PmClientInterface.h"
#include <QDebug>
#include "protocol/ProtocolType.h"
#include "PmApp.h"
#include "model/ModelManager.h"
#include "Account.h"
#include "manager/changenoticemgr.h"
#include "settings/GlobalSettings.h"
#include <QImage>
#include "http/HttpPool.h"
#include <QNetworkReply>
#include "groupsmembermanager.h"

static const int kGroupRequestTimeout = 60; // 60s 

//////////////////////////////////////////////////////////////////////////
// class GroupResHandle
class GroupResHandle : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	GroupResHandle(GroupManager *pMgr);

public:
	void syncGroups();
	void syncGroupMembers(const QString &id);
	void changeCardName(const QString &groupId, const QString &uid, const QString &cardName);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request *req, protocol::Response *res);

private:
	void processGroup(net::Request *req, protocol::Response *res);
	void processGroupCardName(net::Request *req, protocol::Response *res);
	bool processResponseError(net::Request *req);

private:
	int           m_handleId;
	GroupManager *m_pMgr;
};

GroupResHandle::GroupResHandle(GroupManager *pMgr)
	: m_handleId(-1)
	, m_pMgr(pMgr)
{
	Q_ASSERT(m_pMgr != NULL);
}

void GroupResHandle::syncGroups()
{
	protocol::GroupRequest* req = new protocol::GroupRequest();
	Q_ASSERT(req != NULL);
	req->setTimeout(kGroupRequestTimeout);
	PmClient::instance()->send(req);
}

void GroupResHandle::syncGroupMembers(const QString &id)
{
	protocol::GroupRequest* req = new protocol::GroupRequest(id.toUtf8().constData());
	Q_ASSERT(req != NULL);
	req->setTimeout(kGroupRequestTimeout);
	PmClient::instance()->send(req);
}

void GroupResHandle::changeCardName(const QString &groupId, const QString &uid, const QString &cardName)
{
	protocol::GroupCardNameRequest *req = new protocol::GroupCardNameRequest(
		groupId.toUtf8().constData(), uid.toUtf8().constData(), cardName.toUtf8().constData());
	Q_ASSERT(req != NULL);
	req->setTimeout(kGroupRequestTimeout);
	PmClient::instance()->send(req);
}

bool GroupResHandle::initObject()
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

void GroupResHandle::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_handleId);
	m_handleId = -1;
}

QObject* GroupResHandle::instance()
{
	return this;
}

int GroupResHandle::handledId() const
{
	return m_handleId;
}

QList<int> GroupResHandle::types() const
{
	QList<int> ret;
	ret << protocol::Request_IM_Group << protocol::Request_Group_CardName;
	return ret;
}

bool GroupResHandle::onRequestResult(int handleId, net::Request *req, protocol::Response *res)
{
	if (m_handleId != handleId)
		return false;

	// process
	int type = req->getType();
	if (type == protocol::Request_IM_Group)
		processGroup(req, res);
	else if (type == protocol::Request_Group_CardName)
		processGroupCardName(req, res);
	else
		qWarning() << Q_FUNC_INFO << " type error";

	return true;
}

void GroupResHandle::processGroup(net::Request *req, protocol::Response *res)
{
	if (processResponseError(req)) // error
		return;

	protocol::GroupResponse *pRes = static_cast<protocol::GroupResponse *>(res);
	Q_ASSERT(pRes != NULL);

	if (!pRes->isChild())
	{
		// groups
		QStringList ids;
		QStringList names;
		QList<int>  indice;
		QList<int>  logoVersions;
		QStringList annts;
		QStringList versions;

		std::list<protocol::GroupResponse::Item> items = pRes->getItems();
		std::list<protocol::GroupResponse::Item>::iterator itr = items.begin();
		for (; itr != items.end(); ++itr)
		{
			ids.append(QString::fromUtf8(itr->id.c_str()));
			names.append(QString::fromUtf8(itr->name.c_str()));
			indice.append(itr->index);
			logoVersions.append(itr->logoVersion);
			annts.append(QString::fromUtf8(itr->annt.c_str()));
			versions.append(QString::fromUtf8(itr->version.c_str()));
		}
		QMetaObject::invokeMethod(m_pMgr, "onSyncGroupsOK", Qt::QueuedConnection, Q_ARG(QStringList, ids),
			Q_ARG(QStringList, names), Q_ARG(QList<int>, indice), 
			Q_ARG(QList<int>, logoVersions), Q_ARG(QStringList, annts), Q_ARG(QStringList, versions));
	}
	else
	{
		// members
		QStringList ids;
		QStringList names;
		QList<int>  indexes;
		QStringList cardNames;

		std::list<protocol::GroupResponse::Item> items = pRes->getItems();
		std::list<protocol::GroupResponse::Item>::iterator itr = items.begin();
		for (; itr != items.end(); ++itr)
		{
			ids.append(QString::fromUtf8(itr->id.c_str()));
			names.append(QString::fromUtf8(itr->name.c_str()));
			indexes.append(itr->index);
			cardNames.append(QString::fromUtf8(itr->cardName.c_str()));
		}

		QMetaObject::invokeMethod(m_pMgr, "onSyncGroupMembersOK", Qt::QueuedConnection,
			Q_ARG(QString, QString::fromUtf8(pRes->getGroupId().c_str())), 
			Q_ARG(QString, QString::fromUtf8(pRes->getGroupDesc().c_str())),
			Q_ARG(QString, QString::fromUtf8(pRes->getGroupVersion().c_str())), 
			Q_ARG(QStringList, ids), 
			Q_ARG(QStringList, names), 
			Q_ARG(QList<int>, indexes),
			Q_ARG(QStringList, cardNames));
	}
}

void GroupResHandle::processGroupCardName(net::Request *req, protocol::Response *res)
{
	if (processResponseError(req)) // error
		return;

	protocol::GroupCardNameResponse *pRes = static_cast<protocol::GroupCardNameResponse *>(res);
	Q_ASSERT(pRes != NULL);
	QString groupId = QString::fromUtf8(pRes->getGroupId().c_str());
	QMetaObject::invokeMethod(m_pMgr, "changeCardNameOK", Qt::QueuedConnection, Q_ARG(QString, groupId));
}

bool GroupResHandle::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		if (req->getType() == protocol::Request_IM_Group)
		{
			protocol::GroupRequest *groupRequest = static_cast<protocol::GroupRequest *>(req);
			QString groupId = QString::fromUtf8(groupRequest->getGroupId().c_str());
			QString sError = QString::fromUtf8(req->getMessage().c_str());

			if (groupId.isEmpty())
			{
				QString errmsg = tr("Sync group failed(%1)").arg(sError);
				qWarning() << Q_FUNC_INFO << errmsg;

				QMetaObject::invokeMethod(m_pMgr, "error", Qt::QueuedConnection, Q_ARG(QString, errmsg));
			}
			else
			{
				QString errmsg = tr("Sync group member failed(%1)").arg(sError);
				qWarning() << Q_FUNC_INFO << groupId << errmsg;

				QMetaObject::invokeMethod(m_pMgr, "onSyncGroupMembersFailed", Qt::QueuedConnection, Q_ARG(QString, groupId),
					Q_ARG(QString, errmsg));
			}
		}
		else if (req->getType() == protocol::Request_Group_CardName)
		{
			protocol::GroupCardNameRequest *pReq = static_cast<protocol::GroupCardNameRequest *>(req);
			QString groupId = QString::fromUtf8(pReq->getGroupId().c_str());
			QString sError = QString::fromUtf8(pReq->getMessage().c_str());
			qWarning() << Q_FUNC_INFO << sError;
			QMetaObject::invokeMethod(m_pMgr, "changeCardNameFailed", Qt::QueuedConnection, Q_ARG(QString, groupId),
				Q_ARG(QString, sError));
		}
	}

	return bError;
}

//////////////////////////////////////////////////////////////////////////
// class GroupNtfHandle
class GroupNtfHandle : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler);

public:
	GroupNtfHandle(GroupManager *pMgr);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification *sn);

private:
	int           m_handleId;
	GroupManager *m_pMgr;
};

GroupNtfHandle::GroupNtfHandle(GroupManager *pMgr)
	: m_pMgr(pMgr)
	, m_handleId(-1)
{
	Q_ASSERT(m_pMgr != NULL);
}

bool GroupNtfHandle::initObject()
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

void GroupNtfHandle::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_handleId);
	m_handleId = -1;
}

QObject* GroupNtfHandle::instance()
{
	return this;
}

QList<int> GroupNtfHandle::types() const
{
	return QList<int>() << protocol::GROUP;
}

int GroupNtfHandle::handledId() const
{
	return m_handleId;
}

bool GroupNtfHandle::onNotication(int handleId, protocol::SpecificNotification *sn)
{
	if (m_handleId != handleId)
		return false;

	protocol::GroupNotification *pIn = static_cast<protocol::GroupNotification *>(sn);

	if (pIn)
	{
		// sync members
		QStringList members;
		QStringList memberNames;
		QList<int> indexes;
		QStringList cardNames;
		int len = pIn->members.size();
		for (int i = 0; i < len; ++i)
		{
			members << QString::fromUtf8(pIn->members[i].c_str());
			memberNames << QString::fromUtf8(pIn->memberNames[i].c_str());
			indexes << pIn->indice[i];
			cardNames << QString::fromUtf8(pIn->cardNames[i].c_str());
		}

		QString id = QString::fromUtf8(pIn->id.c_str());
		QString version = QString::fromUtf8(pIn->version.c_str());
		QString desc = QString::fromUtf8(pIn->desc.c_str());

		QMetaObject::invokeMethod(m_pMgr, "onSyncGroupMembersOK", Qt::QueuedConnection,
			Q_ARG(QString, id), 
			Q_ARG(QString, desc),
			Q_ARG(QString, version), 
			Q_ARG(QStringList, members), 
			Q_ARG(QStringList, memberNames), 
			Q_ARG(QList<int>, indexes),
			Q_ARG(QStringList, cardNames));
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GroupManager
static void registerMetatype()
{
	static bool isInit = false;
	if (!isInit)
	{
		qRegisterMetaType<QList<int>>("QList<int>");
		isInit = true;
	}
}

GroupManager::GroupManager(QObject *parent)
	: QObject(parent)
{
	registerMetatype();

	m_pResHandle.reset(new GroupResHandle(this));
	m_pNtfHandle.reset(new GroupNtfHandle(this));

	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

GroupManager::~GroupManager()
{

}

void GroupManager::syncGroups()
{
	m_pResHandle->syncGroups();
}

void GroupManager::syncGroupMembers(const QString &id)
{
	m_pResHandle->syncGroupMembers(id);
}

void GroupManager::requestLogo(const QString &id)
{
	if (id.isEmpty())
		return;

	if (m_requestLogoes.values().contains(id))
		return;

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/group/photo/%2").arg(loginConfig.managerUrl).arg(id);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requestLogoes.insert(requestId, id);
}

QMap<QString, int> GroupManager::logoVersions() const 
{
	return m_logoVersions;
}

QString GroupManager::logoPath(const QString &id) const
{
	QDir groupDir = Account::instance()->groupDir();
	QString fileName = id + ".jpg";
	QString filePath = groupDir.absoluteFilePath(fileName);
	return filePath;
}

void GroupManager::changeCardName(const QString &groupId, const QString &uid, const QString &cardName)
{
	m_pResHandle->changeCardName(groupId, uid, cardName);
}

bool GroupManager::initObject()
{
	return m_pResHandle->initObject() && m_pNtfHandle->initObject();
}

void GroupManager::removeObject()
{
	m_pResHandle->removeObject();
	m_pNtfHandle->removeObject();
}

QObject* GroupManager::instance()
{
	return this;
}

QString GroupManager::name() const
{
	return "GroupManager";
}

bool GroupManager::start()
{
	syncGroups();
	return true;
}

void GroupManager::onSyncGroupsOK(const QStringList &ids, const QStringList &names, const QList<int> &indice, 
								  const QList<int> &logoVersions, const QStringList &annts, const QStringList &versions)
{
	m_logoVersions.clear();
	QStringList logoPathes;
	QMap<QString, QString> groupVersions;
	int k = 0;
	foreach (QString id, ids)
	{
		logoPathes << logoPath(id);
		groupVersions.insert(id, versions[k++]);
	}

	// set groups to model
	qPmApp->getModelManager()->setGroups(ids, names, indice, logoPathes, annts);

	// init member versions
	qPmApp->getGroupsMemberManager()->setGroupNewVersions(groupVersions);
	
	// collect logo versions
	if (!ids.isEmpty())
	{
		QMap<QString, QString> oldVersions = qPmApp->getGroupsMemberManager()->groupOldVersions();
		int index = 0;
		int logoVersion = 0;
		QString oldVersion;
		QString newVersion;
		foreach (QString id, ids)
		{
			oldVersion = oldVersions.value(id);
			newVersion = versions[index];
			if (oldVersion != newVersion && !newVersion.isEmpty())
			{
				logoVersion = logoVersions[index];
				m_logoVersions.insert(id, logoVersion);
			}
			++index;
		}
	}

	// notify group ok
	emit groupOK();
	emit finished();

	// start sync all members
	m_syncGroupIds = ids;
	syncNextGroupMembers();
}

void GroupManager::onSyncGroupMembersOK(const QString &gid, const QString &desc, const QString &version, 
										const QStringList &memberIds, const QStringList &memberNames, 
										const QList<int> &indice, const QStringList &cardNames)
{
	QString selfId = Account::instance()->id();
	if (!memberIds.contains(selfId)) // you're deleted from this group
	{
		ChangeNoticeMgr *changeNoticeMgr = qPmApp->getChangeNoticeMgr();
		changeNoticeMgr->postGroupChangeNotice(gid, "delete");
		return;
	}

	// set group members to model
	qPmApp->getModelManager()->setGroupMembers(gid, desc, version, memberIds, memberNames, indice, cardNames);

	// notify group members ok
	emit syncGroupMembersOK(gid);

	// sync next group members
	m_syncGroupIds.removeAll(gid);
	syncNextGroupMembers();
}

void GroupManager::onSyncGroupMembersFailed(const QString &gid, const QString &desc)
{
	// notify group members failed
	emit syncGroupMembersFailed(gid, desc);

	// sync next group members
	m_syncGroupIds.removeAll(gid);
	syncNextGroupMembers();
}

void GroupManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_requestLogoes.contains(requestId))
		return;

	QString gid = m_requestLogoes[requestId];
	m_requestLogoes.remove(requestId);
	int logoVersion = m_logoVersions.value(gid, -1);
	QImage logo;

	if (error)
	{
		qWarning() << Q_FUNC_INFO << "group logo request failed: " << httpCode << gid;

		if (httpCode == 404) // http 404
		{
			emit getGroupLogoFinished(gid, logoVersion, logo);
		}

		return;
	}

	if (recvData.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "group logo response is empty: " << httpCode << gid;
		return;
	}

	logo.loadFromData(recvData);
	if (logo.isNull())
	{
		qWarning() << Q_FUNC_INFO << "group logo response is not a image: " << httpCode << gid;
		return;
	}

	qPmApp->getModelManager()->setGroupLogo(gid, logo);

	emit getGroupLogoFinished(gid, logoVersion, logo);
}

void GroupManager::syncNextGroupMembers()
{
	if (!m_syncGroupIds.isEmpty())
	{
		// get next group members
		QString fetchId = m_syncGroupIds.takeFirst();
		qPmApp->getGroupsMemberManager()->fetchGroupMembers(fetchId);
	}
}

#include "GroupManager.moc"
