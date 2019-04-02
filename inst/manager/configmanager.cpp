#include "configmanager.h"
#include "pmclient/PmClient.h"
#include "protocol/ProtocolType.h"
#include "protocol/ConfigRequest.h"
#include "protocol/ConfigResponse.h"
#include <QDebug>

static const QString kGroupsName = QString("groups");
static const QString kIdsName    = QString("ids");
static const QString kSilenceName = QString("silence");

ConfigManager::ConfigManager(QObject *parent)
	: QObject(parent), m_nHandleId(-1)
{

}

ConfigManager::~ConfigManager()
{

}

void ConfigManager::getConfig(const QList<int> &configNums)
{
	protocol::ConfigRequest *req = new protocol::ConfigRequest(protocol::ConfigRequest::Action_Get);
	Q_ASSERT(req != 0);
	if (configNums.contains(1))
	{
		req->setConfigData(kGroupsName.toUtf8().constData(), "");
	}
	if (configNums.contains(2))
	{
		req->setConfigData(kIdsName.toUtf8().constData(), "");
	}
	if (configNums.contains(3))
	{
		req->setConfigData(kSilenceName.toUtf8().constData(), "");
	}
	PmClient::instance()->send(req);
}

void ConfigManager::setConfig1(const QStringList &groupNames)
{
	protocol::ConfigRequest *req = new protocol::ConfigRequest(protocol::ConfigRequest::Action_Set);
	Q_ASSERT(req != 0);
	QByteArray groupNamesText = encodeGroupNames(groupNames);
	req->setConfigData(kGroupsName.toUtf8().constData(), groupNamesText.constData());
	PmClient::instance()->send(req);
}

void ConfigManager::setConfig2(const QStringList &ids)
{
	protocol::ConfigRequest *req = new protocol::ConfigRequest(protocol::ConfigRequest::Action_Set);
	Q_ASSERT(req != 0);
	QByteArray idsText = ids.join(",").toUtf8();
	req->setConfigData(kIdsName.toUtf8().constData(), idsText.constData());
	PmClient::instance()->send(req);
}

void ConfigManager::setConfig3(const QStringList &silenceList)
{
	protocol::ConfigRequest *req = new protocol::ConfigRequest(protocol::ConfigRequest::Action_Set);
	Q_ASSERT(req != 0);
	QByteArray typeIdSilenceText = silenceList.join(";").toUtf8();
	req->setConfigData(kSilenceName.toUtf8().constData(), typeIdSilenceText.constData());
	PmClient::instance()->send(req);
}

bool ConfigManager::initObject()
{
	m_nHandleId = PmClient::instance()->insertResponseHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qWarning() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void ConfigManager::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* ConfigManager::instance()
{
	return this;
}

int ConfigManager::handledId() const
{
	return m_nHandleId;
}

QList<int> ConfigManager::types() const
{
	QList<int> ret;
	ret << protocol::Request_Config_Config;
	return ret;
}

bool ConfigManager::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_nHandleId != handleId)
	{
		return false;
	}

	// process
	int type = req->getType();
	if (type == protocol::Request_Config_Config)
		processConfig(req, res);
	else
		qWarning() << Q_FUNC_INFO << " type error";

	return true;
}

void ConfigManager::processConfig(net::Request* req, protocol::Response* res)
{
	if (processResponseError(req)) // error
	{
		return;
	}

	protocol::ConfigResponse *pRes = static_cast<protocol::ConfigResponse *>(res);
	Q_ASSERT(pRes != 0);
	
	if (pRes->actionType() == protocol::ConfigRequest::Action_Get)
	{
		if (pRes->hasConfig(1))
		{
			QByteArray base64GroupNames(pRes->configData(kGroupsName.toUtf8().constData()).c_str());
			QStringList groupNames;
			if (!base64GroupNames.isEmpty())
				groupNames = decodeGroupNames(base64GroupNames);
			QMetaObject::invokeMethod(this, "config1GotOk", Qt::QueuedConnection, Q_ARG(QStringList, groupNames));
		}

		if (pRes->hasConfig(2))
		{
			QString idsText(QString::fromUtf8(pRes->configData(kIdsName.toUtf8().constData()).c_str()));
			QStringList ids;
			if (!idsText.isEmpty())
				ids = idsText.split(",");
			QMetaObject::invokeMethod(this, "config2GotOk", Qt::QueuedConnection, Q_ARG(QStringList, ids));
		}

		if (pRes->hasConfig(3))
		{
			QString silenceText(QString::fromUtf8(pRes->configData(kSilenceName.toUtf8().constData()).c_str()));
			QStringList silenceList;
			if (!silenceText.isEmpty())
			{
				silenceList = silenceText.split(";");
			}
			QMetaObject::invokeMethod(this, "config3GotOk", Qt::QueuedConnection, Q_ARG(QStringList, silenceList));
		}
	}
}

bool ConfigManager::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		QString sError = QString::fromUtf8(req->getMessage().c_str());
		qWarning() << Q_FUNC_INFO << sError;
		QMetaObject::invokeMethod(this, "configError", Qt::QueuedConnection);
	}

	return bError;
}

QByteArray ConfigManager::encodeGroupNames(const QStringList &groupNames)
{
	QByteArray groupNamesText;
	foreach (QString groupName, groupNames)
	{
		QByteArray baGroupName = groupName.toUtf8();
		QByteArray base64GroupName = baGroupName.toBase64();
		groupNamesText.append(base64GroupName);
		groupNamesText.append(";");
	}
	groupNamesText.remove(groupNamesText.length()-1, 1);
	return groupNamesText;
}

QStringList ConfigManager::decodeGroupNames(const QByteArray &groupNamesText)
{
	QStringList groupNames;
	QList<QByteArray> base64GroupNames = groupNamesText.split(';');
	foreach (QByteArray base64GroupName, base64GroupNames)
	{
		QByteArray baGroupName = QByteArray::fromBase64(base64GroupName);
		QString groupName = QString::fromUtf8(baGroupName.constData());
		groupNames.append(groupName);
	}
	return groupNames;
}