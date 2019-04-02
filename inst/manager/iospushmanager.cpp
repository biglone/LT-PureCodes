#include "iospushmanager.h"
#include "protocol/IOSPushNotification.h"
#include "pmclient/PmClient.h"
#include "PmApp.h"
#include "Account.h"

IOSPushManager::IOSPushManager(QObject *parent)
	: QObject(parent)
{

}

IOSPushManager::~IOSPushManager()
{

}

void IOSPushManager::pushForIOS(const QString &groupType, const QString &groupId/*, const QString &userId*/, int noPush)
{
	if (groupType.isEmpty() || groupId.isEmpty())
	{
		return;
	}
	QString from = qPmApp->getAccount()->id();
	protocol::IOSPushNotification::Out *out = new protocol::IOSPushNotification::Out;
	out->param.type = groupType.toUtf8().constData();
	out->param.to = groupId.toUtf8().constData();
	out->param.from	= from.toUtf8().constData();
	out->param.silence = QString::number(noPush).toUtf8().constData();
	PmClient::instance()->send(out);
}
