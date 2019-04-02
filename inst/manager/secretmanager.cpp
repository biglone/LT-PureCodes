#include "secretmanager.h"
#include <QDebug>
#include "PmApp.h"
#include "http/HttpPool.h"
#include "settings/GlobalSettings.h"
#include "util/JsonParser.h"

SecretManager::SecretManager(QObject *parent)
	: QObject(parent)
{
	m_httpPool = qPmApp->getHttpPool();
	bool connectOK = false;
	connectOK = connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
	Q_ASSERT(connectOK);
}

SecretManager::~SecretManager()
{

}

void SecretManager::setSecretAck(const QString &fromUid, const QString &toUid, const QString &stamp)
{
	if (fromUid.isEmpty() || toUid.isEmpty() || stamp.isEmpty())
		return;

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/message/readlog/add").arg(loginConfig.managerUrl);
	QUrl url = QUrl::fromUserInput(urlString);

	QMultiMap<QString, QString> params;
	params.insert("ts", stamp);
	params.insert("fromUid", fromUid);
	params.insert("toUid", toUid);
	
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, url, params);
	m_setStamps.insert(requestId, stamp);
}

void SecretManager::requestSecretAck(const QString &stamp, const QString &fromUid, const QString &toId)
{
	if (fromUid.isEmpty() || stamp.isEmpty())
		return;

	// has checked before
	if (m_readStates.contains(stamp))
	{
		if (m_readStates.value(stamp) == 1)
		{
			emit requestSecretAckFinished(fromUid, toId, stamp, 1);
		}
		return;
	}

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/message/readlog/get").arg(loginConfig.managerUrl);
	QUrl url = QUrl::fromUserInput(urlString);

	QMultiMap<QString, QString> params;
	params.insert("ts", stamp);
	params.insert("fromUid", fromUid);

	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, url, params);
	m_requestStamps.insert(requestId, stamp);
}

void SecretManager::clearSecretRead()
{
	m_readStates.clear();
}

void SecretManager::setSecretRead(const QString &stamp, int state /*= 1*/)
{
	if (stamp.isEmpty())
		return;

	m_readStates.insert(stamp, state);
}

void SecretManager::onHttpRequestFinished(int id, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_requestStamps.contains(id) && !m_setStamps.contains(id))
		return;

	QString errMsg;
	QVariant datas;
	bool err = true;
	if (error)
	{
		err = true;
		errMsg = tr("Network error, code:%1").arg(httpCode);
	}
	else
	{
		datas = JsonParser::parse(recvData, err, errMsg);
	}

	if (m_requestStamps.contains(id))
	{
		QString stamp = m_requestStamps[id];
		m_requestStamps.remove(id);

		if (err)
		{
			qDebug() << Q_FUNC_INFO << "request secret ack failed: " << stamp << errMsg;
			emit requestSecretAckFailed(stamp, errMsg);
			return;
		}

		QVariantMap data = datas.toMap();
		int readState = 0;
		if (data.contains("ts"))
		{
			readState = 1;
			QString fromUid= data["fromUid"].toString();
			QString toUid = data["toUid"].toString();
			emit requestSecretAckFinished(fromUid, toUid, stamp, readState);
		}
		
		// add to read states
		m_readStates.insert(stamp, readState);
		return;
	}

	if (m_setStamps.contains(id))
	{
		QString stamp = m_setStamps[id];
		m_setStamps.remove(id);

		if (err)
		{
			qDebug() << Q_FUNC_INFO << "set secret ack failed: " << stamp << errMsg;
			emit setSecretAckFailed(stamp, errMsg);
			return;
		}

		emit setSecretAckOK(stamp);
		return;
	}
}
