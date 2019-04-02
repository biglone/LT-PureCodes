#include "rostermanager.h"
#include "protocol/RosterRequest.h"
#include "protocol/RosterResponse.h"
#include "protocol/RosterNotification.h"
#include "pmclient/PmClient.h"
#include <QDebug>
#include "protocol/ProtocolType.h"
#include <QDomDocument>
#include "PmApp.h"
#include "login/Account.h"
#include "ModelManager.h"
#include <vector>

//////////////////////////////////////////////////////////////////////////
// class RosterResHandle
class RosterResHandle : public QObject, public IPmClientResponseHandler
{
Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	RosterResHandle(RosterManager *pMgr);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private:
	void processRoster(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

private:
	RosterManager *m_pMgr;
	int            m_handleId;
};

RosterResHandle::RosterResHandle(RosterManager *pMgr)
: m_pMgr(pMgr), m_handleId(-1)
{
	Q_ASSERT(m_pMgr != NULL);
}

bool RosterResHandle::initObject()
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

void RosterResHandle::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_handleId);
	m_handleId = -1;
}

QObject* RosterResHandle::instance()
{
	return this;
}

int RosterResHandle::handledId() const
{
	return m_handleId;
}

QList<int> RosterResHandle::types() const
{
	QList<int> ret;
	ret << protocol::Request_IM_Roster;
	return ret;
}

bool RosterResHandle::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_handleId != handleId)
	{
		return false;
	}

	// process
	int type = req->getType();
	if (type == protocol::Request_IM_Roster)
		processRoster(req, res);
	else
		qWarning() << Q_FUNC_INFO << " type error";

	return true;
}

void RosterResHandle::processRoster(net::Request* req, protocol::Response* res)
{
	if (processResponseError(req)) // error
	{
		return;
	}

	protocol::RosterResponse *pRes = static_cast<protocol::RosterResponse *>(res);
	Q_ASSERT(pRes != NULL);

	if (pRes->getActionType() == protocol::RosterRequest::Action_Sync)
	{
		QString version = QString::fromUtf8(pRes->getSyncVersion().c_str());
		QList<RosterManager::RosterItem> rosterItems;
		std::vector<protocol::RosterRequest::RosterItem> items = pRes->getRosterItems();
		for (size_t j = 0; j < items.size(); j++)
		{
			protocol::RosterRequest::RosterItem item = items[j];
			RosterManager::RosterItem rosterItem(QString::fromUtf8(item.m_id.c_str()), 
				                                 QString::fromUtf8(item.m_name.c_str()), 
												 QString::fromUtf8(item.m_group.c_str()));
			rosterItems.append(rosterItem);
		}
		QMetaObject::invokeMethod(m_pMgr, "onSyncRosterOK", Qt::QueuedConnection, Q_ARG(QString, version), Q_ARG(QList<RosterManager::RosterItem>, rosterItems));
	}
	else if (pRes->getActionType() == protocol::RosterRequest::Action_Modify)
	{
		QMetaObject::invokeMethod(m_pMgr, "onModifyRosterOK", Qt::QueuedConnection);
	}
}

bool RosterResHandle::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		QString sError = QString::fromUtf8(req->getMessage().c_str());
		qWarning() << Q_FUNC_INFO << sError;

		protocol::RosterRequest *rosterRequest = static_cast<protocol::RosterRequest *>(req);
		if (rosterRequest->getActionType() == protocol::RosterRequest::Action_Sync)
		{
			QMetaObject::invokeMethod(m_pMgr, "syncRosterError", Qt::QueuedConnection, Q_ARG(QString, sError));
			QString errmsg = tr("Sync friends failed(%1)").arg(sError);
			QMetaObject::invokeMethod(m_pMgr, "error", Qt::QueuedConnection, Q_ARG(QString, errmsg));
		}
		else if (rosterRequest->getActionType() == protocol::RosterRequest::Action_Modify)
		{
			int actionType = (int)rosterRequest->clientType();
			QStringList ids;
			QStringList names;
			QStringList groups;
			QList<int>  modifies;
			std::vector<protocol::RosterRequest::RosterItem> rosterItems = rosterRequest->getRosterItems();
			std::vector<protocol::RosterRequest::RosterItem>::reverse_iterator rIter;
			for (rIter = rosterItems.rbegin(); rIter != rosterItems.rend(); rIter++)
			{
				protocol::RosterRequest::RosterItem item = *rIter;
				ids << QString::fromUtf8(item.m_id.c_str());
				names << QString::fromUtf8(item.m_name.c_str());
				groups << QString::fromUtf8(item.m_group.c_str());
				if (item.m_modifyType == protocol::RosterRequest::ModifyAdd)
				{
					modifies << (int)RosterManager::ModifyDelete;
				}
				else if (item.m_modifyType == protocol::RosterRequest::ModifyDelete)
				{
					modifies << (int)RosterManager::ModifyAdd;
				}
				else
				{
					modifies << (int)RosterManager::ModifyNone;
				}
			}
			QMetaObject::invokeMethod(m_pMgr, "modifyRosterError", Qt::QueuedConnection, Q_ARG(QString, sError), Q_ARG(int, actionType), 
				Q_ARG(QStringList, ids), Q_ARG(QStringList, names), Q_ARG(QStringList, groups), Q_ARG(QList<int>, modifies));
		}
	}

	return bError;
}

//////////////////////////////////////////////////////////////////////////
// class RosterNtfHandle
class RosterNtfHandle : public QObject, public IPmClientNotificationHandler
{
Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler);

public:
	RosterNtfHandle(RosterManager *pMgr);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private:
	RosterManager *m_pMgr;
	int            m_handleId;
};

RosterNtfHandle::RosterNtfHandle(RosterManager *pMgr)
: m_pMgr(pMgr), m_handleId(-1)
{
	Q_ASSERT(m_pMgr != NULL);
}

bool RosterNtfHandle::initObject()
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

void RosterNtfHandle::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_handleId);
	m_handleId = -1;
}

QObject* RosterNtfHandle::instance()
{
	return this;
}

QList<int> RosterNtfHandle::types() const
{
	return QList<int>() << protocol::ROSTER;
}

int RosterNtfHandle::handledId() const
{
	return m_handleId;
}

bool RosterNtfHandle::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_handleId != handleId)
		return false;

	protocol::RosterNotification *pIn = static_cast<protocol::RosterNotification *>(sn);
	QStringList ids;
	QStringList names;
	QStringList groups;
	QList<int>  modifies;
	std::vector<protocol::RosterRequest::RosterItem> items = pIn->getRosterItems();
	for (size_t j = 0; j < items.size(); j++)
	{
		protocol::RosterRequest::RosterItem item = items[j];
		ids.append(QString::fromUtf8(item.m_id.c_str()));
		names.append(QString::fromUtf8(item.m_name.c_str()));
		groups.append(QString::fromUtf8(item.m_group.c_str()));
		modifies.append((int)item.m_modifyType);
	}

	QMetaObject::invokeMethod(m_pMgr, "rosterModified", Qt::QueuedConnection, 
		Q_ARG(QStringList, ids), Q_ARG(QStringList, names), Q_ARG(QStringList, groups), Q_ARG(QList<int>, modifies));

	return true;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterManager
RosterManager::RosterManager(QObject *parent)
	: QObject(parent)
{
	qRegisterMetaType<RosterManager::RosterItem>("RosterManager::RosterItem");
	qRegisterMetaType<QList<RosterManager::RosterItem>>("QList<RosterManager::RosterItem>");

	m_pResHandle.reset(new RosterResHandle(this));
	m_pNtfHandle.reset(new RosterNtfHandle(this));
}

RosterManager::~RosterManager()
{

}

bool RosterManager::initObject()
{
	return m_pResHandle->initObject() && m_pNtfHandle->initObject();
}

void RosterManager::removeObject()
{
	m_pResHandle->removeObject();
	m_pNtfHandle->removeObject();
}

void RosterManager::syncRoster(const QString &version /*= QString()*/)
{
	protocol::RosterRequest *request = new protocol::RosterRequest(
		protocol::RosterRequest::Action_Sync, version.toUtf8().constData());
	PmClient::instance()->send(request);
}

void RosterManager::modifyRoster(const RosterManager::RosterItem &item, ActionType actionType)
{
	protocol::RosterRequest *request = new protocol::RosterRequest(protocol::RosterRequest::Action_Modify);
	request->setClientType((protocol::RosterRequest::ClientType)actionType);
	request->addRosterItem((protocol::RosterRequest::ModifyType)(item.m_modifyType), 
		item.m_id.toUtf8().constData(), 
		item.m_name.toUtf8().constData(), 
		item.m_group.toUtf8().constData());
	PmClient::instance()->send(request);
}

void RosterManager::modifyRoster(const QList<RosterManager::RosterItem> &modifyItems, ActionType actionType)
{
	protocol::RosterRequest *request = new protocol::RosterRequest(protocol::RosterRequest::Action_Modify);
	request->setClientType((protocol::RosterRequest::ClientType)actionType);
	foreach (RosterManager::RosterItem item, modifyItems)
	{
		request->addRosterItem((protocol::RosterRequest::ModifyType)(item.m_modifyType), 
			                   item.m_id.toUtf8().constData(), 
							   item.m_name.toUtf8().constData(), 
							   item.m_group.toUtf8().constData());
	}
	PmClient::instance()->send(request);
}

/*
void RosterManager::addRoster(const QString &id, const QString &name, const QString &group)
{
	RosterItem addItem(id, name, group);
	addItem.m_modifyType = ModifyAdd;
	modifyRoster(addItem, ActionAddRoster);
}
*/

void RosterManager::delRoster(const QString &id, const QString &name, const QString &group)
{
	RosterItem delItem(id, name, group);
	delItem.m_modifyType = ModifyDelete;
	modifyRoster(delItem, ActionRemoveRoster);
}

void RosterManager::moveRoster(const QString &id, const QString &name, const QString &oldGroup, const QString &newGroup)
{
	if (id.isEmpty() || name.isEmpty() || oldGroup.isEmpty() || newGroup.isEmpty())
		return;

	if (oldGroup == newGroup)
		return;

	QList<RosterItem> modifyItems;
	RosterItem delItem(id, name, oldGroup);
	delItem.m_modifyType = ModifyDelete;
	RosterItem addItem(id, name, newGroup);
	addItem.m_modifyType = ModifyAdd;
	modifyItems.append(delItem);
	modifyItems.append(addItem);
	modifyRoster(modifyItems, ActionChangeGroup);
}

void RosterManager::moveRosters(const QStringList &ids, const QStringList &names, const QString &oldGroup, const QString &newGroup)
{
	if (ids.isEmpty() || names.isEmpty() || oldGroup.isEmpty() || newGroup.isEmpty())
		return;

	if (ids.count() != names.count())
		return;

	if (oldGroup == newGroup)
		return;

	QList<RosterItem> items;
	for (int i = 0; i < ids.count(); i++)
	{
		QString id = ids[i];
		QString name = names[i];
		
		RosterItem delItem(id, name, oldGroup);
		delItem.m_modifyType = ModifyDelete;

		RosterItem addItem(id, name, newGroup);
		addItem.m_modifyType = ModifyAdd;

		items.append(delItem);
		items.append(addItem);
	}
	modifyRoster(items, ActionChangeGroup);
}

void RosterManager::changeRosterName(const QString &id, const QString &oldName, const QString &newName, const QString &group)
{
	RosterItem delRosterItem(id, oldName, group);
	delRosterItem.m_modifyType = ModifyDelete;
	RosterItem addRosterItem(id, newName, group);
	addRosterItem.m_modifyType = ModifyAdd;
	QList<RosterItem> modifyItems;
	modifyItems << delRosterItem << addRosterItem;
	modifyRoster(modifyItems, ActionChangeName);
}

void RosterManager::onSyncRosterOK(const QString &version, const QList<RosterManager::RosterItem> &rosterItems)
{
	qPmApp->getModelManager()->setRoster(version, rosterItems);

	emit syncRosterOK();
	emit finished();
}

void RosterManager::onModifyRosterOK()
{
	emit modifyRosterOK();
}

QObject* RosterManager::instance()
{
	return this;
}

QString RosterManager::name() const
{
	return "roster";
}

bool RosterManager::start()
{
	// start roster process
	QString rosterVersion = "";
	syncRoster(rosterVersion);

	return true;
}

#include "rostermanager.moc"
