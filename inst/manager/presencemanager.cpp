#include "presencemanager.h"
#include "pmclient/PmClient.h"
#include "protocol/ProtocolType.h"
#include <QDebug>
#include "Account.h"

static const QString kCommonResource = "common"; // 为了兼容老版本使用的资源号

PresenceManager::PresenceManager(QObject *parent)
	: QObject(parent), m_nHandleId(-1)
{
}

PresenceManager::~PresenceManager()
{
	reset();
}

QStringList PresenceManager::presenceIds() const
{
	return m_presences.keys();
}

bool PresenceManager::isAvailable(const QString &id) const
{
	bool avaiable = false;
	QString bareId = Account::idFromFullId(id);
	QString resource = Account::resourceFromFullId(id);
	if (m_presences.contains(bareId))
	{
		QMap<QString, Presence> presenceInfo = m_presences[bareId];
		if (resource.isEmpty())
		{
			avaiable = true;
		}
		else if (presenceInfo.contains(resource))
		{
			avaiable = true;
		}
	}

	return avaiable;
}

void PresenceManager::reset()
{
	m_presences.clear();
	emit presenceCleared();
}

void PresenceManager::sendPresence(StatusChanger::Status eStatus)
{
	bool isOffline = (eStatus == StatusChanger::Status_Offline);

	protocol::PresenceNotification::PresenceType eType = isOffline ? protocol::PresenceNotification::Presence_Unavailable
		: protocol::PresenceNotification::Presence_None;

	protocol::PresenceNotification::Out* pOut = new protocol::PresenceNotification::Out(eType);

	pOut->m_PresenceInfo.type = eType;

	if (!isOffline)
	{
		pOut->m_PresenceInfo.ttype = protocol::PresenceNotification::Terminal_PC;

		protocol::PresenceNotification::Show eShow = protocol::PresenceNotification::Show_None;
		switch (eStatus)
		{
		case StatusChanger::Status_Online:
			eShow = protocol::PresenceNotification::Show_None;
			break;
		case StatusChanger::Status_Away:
			eShow = protocol::PresenceNotification::Show_Away;
			break;
		case StatusChanger::Status_Chat:
			eShow = protocol::PresenceNotification::Show_Chat;
			break;
		case StatusChanger::Status_Dnd:
			eShow = protocol::PresenceNotification::Show_DND;
			break;
		case StatusChanger::Status_Xa:
			eShow = protocol::PresenceNotification::Show_XA;
			break;
		default:
			break;
		}

		pOut->m_PresenceInfo.show = eShow;
	}

	PmClient::instance()->send(pOut);
}

PresenceManager::PresenceShow PresenceManager::presenceShow(const QString &id) const
{
	if (id == Account::instance()->phoneFullId())
		return PresenceManager::ShowNone;

	PresenceManager::PresenceShow show = PresenceManager::ShowNone;
	QString bareId = Account::idFromFullId(id);
	QString resource = Account::resourceFromFullId(id);
	if (m_presences.contains(bareId))
	{
		QMap<QString, Presence> presenceInfo = m_presences[bareId];
		if (resource.isEmpty())
		{
			if (presenceInfo.contains(RESOURCE_PHONE))
				show = presenceInfo.value(RESOURCE_PHONE).show;
			else
				show = presenceInfo.values().at(0).show;
		}
		else if (presenceInfo.contains(resource))
		{
			show = presenceInfo[resource].show;
		}
	}
	return show;
}

PresenceManager::PresenceTerminalType PresenceManager::presenceTType(const QString &id) const
{
	if (id == Account::instance()->phoneFullId())
		return PresenceManager::TerminalNone;

	PresenceManager::PresenceTerminalType ttype = PresenceManager::TerminalNone;
	QString bareId = Account::idFromFullId(id);
	QString resource = Account::resourceFromFullId(id);
	if (m_presences.contains(bareId))
	{
		QMap<QString, Presence> presenceInfo = m_presences[bareId];
		if (resource.isEmpty())
		{
			if (presenceInfo.contains(RESOURCE_PHONE))
				ttype = presenceInfo.value(RESOURCE_PHONE).ttype;
			else
				ttype = presenceInfo.values().at(0).ttype;
		}
		else if (presenceInfo.contains(resource))
		{
			ttype = presenceInfo[resource].ttype;
		}
	}
	return ttype;
}

bool PresenceManager::initObject()
{
	m_nHandleId = PmClient::instance()->insertNotificationHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void PresenceManager::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* PresenceManager::instance()
{
	return this;
}

QList<int> PresenceManager::types() const
{
	return QList<int>() << protocol::PRESENCE;
}

int PresenceManager::handledId() const
{
	return m_nHandleId;
}

bool PresenceManager::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_nHandleId != handleId)
		return false;

	protocol::PresenceNotification::In *pIn = static_cast<protocol::PresenceNotification::In *>(sn);

	if (pIn)
	{
		protocol::PresenceNotification::PresenceInfo *pParam = new protocol::PresenceNotification::PresenceInfo(pIn->presenceInfo);
		QMetaObject::invokeMethod(this, "processPresence", Q_ARG(void*, pParam));
	}

	return true;
}

void PresenceManager::processPresence(void *pValue)
{
	protocol::PresenceNotification::PresenceInfo *pParam = static_cast<protocol::PresenceNotification::PresenceInfo*>(pValue);
	if (!pParam)
		return;

	// check if this presence is send from self
	QString fromFullId(QString::fromUtf8(pParam->from.c_str()));
	QString fromId = Account::idFromFullId(fromFullId);
	QString fromResource = Account::resourceFromFullId(fromFullId);
	if (fromResource.isEmpty())
		fromResource = kCommonResource;
	if (!fromId.isEmpty())
	{
		PresenceManager::PresenceType oldType = PresenceManager::PresenceUnavailable;
		if (m_presences.contains(fromId))
		{
			oldType = PresenceManager::PresenceAvailable;
		}
		PresenceManager::PresenceType newType = PresenceManager::PresenceUnavailable;
		if (pParam->type != protocol::PresenceNotification::Presence_Unavailable)
		{
			newType = PresenceManager::PresenceAvailable;
		}

		Presence presence;
		presence.id = fromId;
		presence.resource = fromResource;
		presence.show = (PresenceManager::PresenceShow)(pParam->show);
		presence.ttype = (PresenceManager::PresenceTerminalType)(pParam->ttype);

		if (m_presences.contains(fromId))
		{
			QMap<QString, Presence> presenceInfo = m_presences.value(fromId);
			if (newType != PresenceManager::PresenceUnavailable)
			{
				presenceInfo[fromResource] = presence;
				m_presences[fromId] = presenceInfo;
			}
			else
			{
				presenceInfo.remove(fromResource);
				if (presenceInfo.isEmpty())
				{
					m_presences.remove(fromId);
				}
				else
				{
					m_presences[fromId] = presenceInfo;
				}
			}
		}
		else
		{
			if (newType != PresenceManager::PresenceUnavailable)
			{
				QMap<QString, Presence> presenceInfo;
				presenceInfo.insert(fromResource, presence);
				m_presences.insert(fromId, presenceInfo);
			}
		}
		
		// notify presence received
		emit presenceReceived(fromId, (int)oldType, (int)newType);
	}

	delete pParam;
	pParam = 0;
}

