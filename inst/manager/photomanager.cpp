#include "photomanager.h"
#include "http/HttpPool.h"
#include "settings/GlobalSettings.h"
#include <QMultiMap>
#include <QDebug>
#include <QNetworkReply>
#include <QFile>
#include "ModelManager.h"

PhotoManager::PhotoManager(HttpPool &httpPool, QObject *parent)
: QObject(parent), m_httpPool(httpPool), m_defaultAvatar(ModelManager::avatarDefaultIcon())
{
	bool connectOK = connect(&m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
	Q_ASSERT(connectOK);
	Q_UNUSED(connectOK);
}

PhotoManager::~PhotoManager()
{

}

void PhotoManager::getAvatar(const QString &uid)
{
	if (uid.isEmpty())
		return;

	if (m_requestAvatars.values().contains(uid))
		return;

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/photo/%2").arg(loginConfig.managerUrl).arg(uid);
	int requestId = m_httpPool.addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requestAvatars.insert(requestId, uid);
}

void PhotoManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_requestAvatars.contains(requestId))
		return;

	QString uid = m_requestAvatars[requestId];
	m_requestAvatars.remove(requestId);

	if (error)
	{
		qWarning() << Q_FUNC_INFO << "avatar request failed: " << httpCode << uid;

		if (httpCode == 404) // http 404
		{
			emit getAvatarFinished(uid, m_defaultAvatar, false);
		}

		return;
	}

	if (recvData.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "avatar response is empty: " << httpCode << uid;

		emit getAvatarFinished(uid, m_defaultAvatar, false);

		return;
	}

	QImage avatar;
	avatar.loadFromData(recvData);
	if (avatar.isNull())
	{
		qWarning() << Q_FUNC_INFO << "avatar response is not a image: " << httpCode << uid;
		return;
	}

	emit getAvatarFinished(uid, avatar, true);
}
