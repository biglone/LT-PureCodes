#include "subscriptionlogomanager.h"
#include "http/HttpPool.h"
#include <QDebug>
#include <QNetworkReply>
#include <QFile>
#include "ModelManager.h"

SubscriptionLogoManager::SubscriptionLogoManager(HttpPool &httpPool, QObject *parent)
	: QObject(parent), m_httpPool(httpPool)
{
	bool connectOK = connect(&m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
	Q_ASSERT(connectOK);
	Q_UNUSED(connectOK);
}

SubscriptionLogoManager::~SubscriptionLogoManager()
{

}


void SubscriptionLogoManager::getSubscriptionLogo(const QString &subscriptionId, const QString &urlString)
{
	if (subscriptionId.isEmpty() || urlString.isEmpty())
		return;

	if (!urlString.startsWith("http"))
	{
		if (m_defaultLogo.isNull())
		{
			QIcon &defaultLogo = ModelManager::subscriptionDefaultIcon();
			m_defaultLogo = defaultLogo.pixmap(QSize(110, 110));
		}

		qDebug() << Q_FUNC_INFO << "url is invalid: " << urlString;
		emit getLogoFinished(subscriptionId, urlString, m_defaultLogo, false);
		return;
	}

	LogoData logoData(subscriptionId, urlString);
	if (m_requestLogos.values().contains(logoData))
		return;

	int requestId = m_httpPool.addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requestLogos.insert(requestId, logoData);
}

void SubscriptionLogoManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_requestLogos.contains(requestId))
		return;

	if (m_defaultLogo.isNull())
	{
		QIcon &defaultLogo = ModelManager::subscriptionDefaultIcon();
		m_defaultLogo = defaultLogo.pixmap(QSize(110, 110));
	}

	LogoData logoData = m_requestLogos[requestId];
	m_requestLogos.remove(requestId);

	QString subscriptionId = logoData.subscriptionId;
	QString urlString = logoData.urlString;

	if (error)
	{
		qWarning() << Q_FUNC_INFO << "subscription logo request failed: " << httpCode << subscriptionId << urlString;

		if (httpCode == 404) // http 404
		{
			emit getLogoFinished(subscriptionId, urlString, m_defaultLogo, false);
		}

		return;
	}

	if (recvData.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "subscription logo response is empty: " << httpCode << subscriptionId << urlString;

		emit getLogoFinished(subscriptionId, urlString, m_defaultLogo, false);

		return;
	}

	QImage image;
	image.loadFromData(recvData);
	if (image.isNull())
	{
		qWarning() << Q_FUNC_INFO << "subscription response is not a image: " << httpCode << subscriptionId << urlString;

		emit getLogoFinished(subscriptionId, urlString, m_defaultLogo, false);

		return;
	}

	emit getLogoFinished(subscriptionId, urlString, QPixmap::fromImage(image), true);
}
