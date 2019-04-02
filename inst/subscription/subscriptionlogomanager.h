#ifndef SUBSCRIPTIONLOGOMANAGER_H
#define SUBSCRIPTIONLOGOMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QPixmap>

class HttpPool;

class SubscriptionLogoManager : public QObject
{
	Q_OBJECT

public:
	SubscriptionLogoManager(HttpPool &httpPool, QObject *parent = 0);
	~SubscriptionLogoManager();

	void getSubscriptionLogo(const QString &subscriptionId, const QString &urlString);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

signals:
	void getLogoFinished(const QString &subscriptionId, const QString &urlString, const QPixmap &logo, bool save);

private:
	struct LogoData
	{
		QString subscriptionId;
		QString urlString;

		LogoData() {subscriptionId = ""; urlString = "";}
		LogoData(const QString &id, const QString &url) {subscriptionId = id; urlString = url;}
		bool operator==(const LogoData &other) const { return (subscriptionId == other.subscriptionId) && (urlString == other.urlString); }
	};

private:
	HttpPool           &m_httpPool;
	QMap<int, LogoData> m_requestLogos;
	QPixmap             m_defaultLogo;
};

#endif // SUBSCRIPTIONLOGOMANAGER_H
