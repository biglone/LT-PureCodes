#ifndef GLOBALNOTIFICATIONLOGOMANAGER_H
#define GLOBALNOTIFICATIONLOGOMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QPixmap>

class HttpPool;

class GlobalNotificationLogoManager : public QObject
{
	Q_OBJECT

public:
	GlobalNotificationLogoManager(HttpPool &httpPool, QObject *parent = 0);
	~GlobalNotificationLogoManager();

	void getGlobalNotificationLogo(const QString &globalNotificationId, const QString &urlString);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

signals:
	void getLogoFinished(const QString &globalNotificationId, const QString &urlString, const QPixmap &logo, bool save);

private:
	struct LogoData
	{
		QString globalNotificationId;
		QString urlString;

		LogoData() {globalNotificationId = ""; urlString = "";}
		LogoData(const QString &id, const QString &url) {globalNotificationId = id; urlString = url;}
		bool operator==(const LogoData &other) const { return (globalNotificationId == other.globalNotificationId) && (urlString == other.urlString); }
	};

private:
	HttpPool           &m_httpPool;
	QMap<int, LogoData> m_requestLogos;
	QPixmap             m_defaultLogo;
};

#endif // GLOBALNOTIFICATIONLOGOMANAGER_H
