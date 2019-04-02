#ifndef COMMONCONFIGMANAGER_H
#define COMMONCONFIGMANAGER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include "Constants.h"

class QNetworkAccessManager;
class QNetworkReply;

class CommonConfigManager : public QObject
{
	Q_OBJECT

public:
	class Config
	{
	public:
		Config()
		{
			reset();
		}

		void reset()
		{
			version = "";
			title = "";
			encrypt = CONFIG_ENCRYPT_NONE;
			transferIp = "";
			transferPort = 0;
			psgs.clear();
			updateUrl = "";
			updateVersion = "";
			updateDesc = "";
			audioDisabled = 0;
			videoDisabled = 0;
			roamingMsgDisabled = 0;
			loginLoadBalance = 0;
			introductionViewType = 0;
			subscriptionDisabled = 0;
			linkItems = "";
			maxDiscussMemberCount = 50;
			trackerServer = "";
			fastdfsEnabled = 0;
			rosterSmallAvatar = 0;
			osLoadAll = 0;
			offlineSyncMsgEnabled = 1;
			msgEncrypt = 0;
			msgEncryptSeed = "";
			interphoneDisabled = 0;
		}

	public:
		QString version;
		QString title;
		QString encrypt;
		QString transferIp;
		int     transferPort;
		QStringList psgs;
		
		QString updateUrl;
		QString updateVersion;
		QString updateDesc;

		int     audioDisabled;      // 1: no audio enabled, 0: audio enabled
		int     videoDisabled;      // 1: no video enabled, 0: video enabled
		
		int     roamingMsgDisabled;

		int     loginLoadBalance;

		int     introductionViewType;

		int     subscriptionDisabled;

		QString linkItems;

		int     maxDiscussMemberCount;

		QString trackerServer;
		int     fastdfsEnabled;

		int     rosterSmallAvatar;

		int     osLoadAll;

		int     offlineSyncMsgEnabled;

		int     msgEncrypt;
		QString msgEncryptSeed;

		int     interphoneDisabled;
	};

public:
	CommonConfigManager(QObject *parent = 0);
	~CommonConfigManager();

	void requestCommonConfig(const QString &oldVersion);
	void cancelCommonConfig();

	Config config() const;

	static bool hasUpdate(const QString &updateVer, const QString &appVer);

signals:
	void commonConfigFinished(bool ok, const QString &desc);

private slots:
	void onReplyFinished();

	void requestVersion();
	void requestConfig();
	void requestUpdateInfo();

	void onRequestTimeout();

private:
	QNetworkAccessManager *m_networkAccessManager;
	QNetworkReply         *m_networkReply;
	Config                 m_config;

	QString                m_oldVersion;

	QTimer                 m_timer;
};

#endif // COMMONCONFIGMANAGER_H
