#include "protocol/ProtocolConst.h"
#include "protocol/ProtocolType.h"

#include "protocol/KickNotification.h"
#include "protocol/ReloginNotification.h"

#include "BaseProcessor.h"

#include "pmclient/PmClient.h"

#include <QDebug>

BaseProcessor::BaseProcessor(QObject *parent)
	: QObject(parent)
	, m_nHandleId(-1)
	, m_bKicked(false)
{
}

BaseProcessor::~BaseProcessor()
{
}

bool BaseProcessor::initObject()
{
	m_nHandleId = PmClient::instance()->insertNotificationHandler(this);
	qWarning() << Q_FUNC_INFO << " handle: " << m_nHandleId;

	disconnect(PmClient::instance(), SIGNAL(opened()), this, SLOT(onPmClientOpened()));
	connect(PmClient::instance(), SIGNAL(opened()), this, SLOT(onPmClientOpened()));

	return true;
}

void BaseProcessor::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_nHandleId);
	m_nHandleId = -1;
}

bool BaseProcessor::isKicked()
{
	return m_bKicked;
}

QObject* BaseProcessor::instance()
{
	return this;
}

int BaseProcessor::handledId() const
{
	return m_nHandleId;
}

QList<int> BaseProcessor::types() const
{
	return QList<int>() << protocol::KICK << protocol::RELOGIN;
}

bool BaseProcessor::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_nHandleId != handleId)
		return false;

	if (protocol::KICK == sn->getNotificationType())
	{
		QMetaObject::invokeMethod(this, "onKicked", Qt::QueuedConnection);
	}
	else if (protocol::RELOGIN == sn->getNotificationType())
	{
		protocol::ReloginNotification* pRelogin = static_cast<protocol::ReloginNotification*>(sn);
		if (pRelogin)
		{
			QStringList ret;
			std::list<std::string> psgs = pRelogin->getPsgs();
			std::list<std::string>::iterator itr = psgs.begin();
			for (; itr != psgs.end(); ++itr)
			{
				ret << (itr->c_str());
			}

			QMetaObject::invokeMethod(this, "relogin", Qt::QueuedConnection, Q_ARG(QStringList, ret));
		}

	}

	return true;
}

void BaseProcessor::onKicked()
{
	m_bKicked = true;

	emit kicked();
}

void BaseProcessor::onPmClientOpened()
{
	m_bKicked = false;
}
