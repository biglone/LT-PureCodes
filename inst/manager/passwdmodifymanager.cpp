#include "passwdmodifymanager.h"
#include <QByteArray>
#include <QDebug>
#include "util/JsonParser.h"
#include "Constants.h"
#include "PmApp.h"
#include "http/HttpPool.h"
#include "settings/GlobalSettings.h"

static const int kInvalidRequestId = -1;

PasswdModifyManager::PasswdModifyManager(QObject *parent /*= 0*/)
	: QObject(parent), m_requestId(kInvalidRequestId)
{
	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

PasswdModifyManager::~PasswdModifyManager()
{

}

void PasswdModifyManager::modify(const QString &phone, const QString &oldPasswd, const QString &newPasswd)
{
	if (m_requestId != kInvalidRequestId)
		return;

	QMultiMap<QString, QString> params;
	params.insert("phone", phone);
	params.insert("oldpassword", oldPasswd);
	params.insert("password", newPasswd);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/companyuser/modifypwd").arg(loginConfig.managerUrl);
	m_requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
}

void PasswdModifyManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (m_requestId != requestId)
		return;

	m_requestId = kInvalidRequestId;

	if (error)
	{
		qWarning() << Q_FUNC_INFO << "request failed: " << httpCode;
		emit passwdModifyFailed(tr("Network error: %1").arg(httpCode));
		return;
	}

	bool err = true;
	QString errMsg;
	QVariant datas = JsonParser::parse(recvData, err, errMsg);
	if (err)
	{
		qWarning() << Q_FUNC_INFO << "request failed: " << errMsg;
		emit passwdModifyFailed(errMsg);
		return;
	}

	emit passwdModifyOK();
}



