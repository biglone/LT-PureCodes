#ifndef _GLOBALSETTINGS_H_
#define _GLOBALSETTINGS_H_

#include <QScopedPointer>
#include <QSettings>
#include <QMap>
#include "../Constants.h"

class GlobalSettings
{
public:
	enum Language
	{
		Language_ZH_CN,
		Language_ENG
	};

	class LoginConfig
	{
	public:
		LoginConfig()
		{	
			transferPort = 0;
			removable = true;
			encrypt = Encrypt_None;
			loginWithBalance = false;
		}

		LoginConfig(const LoginConfig &other)
		{
			name = other.name;
			version = other.version;
			transferIp = other.transferIp;
			transferPort = other.transferPort;
			managerUrl = other.managerUrl;
			storeHome = other.storeHome;
			removable = other.removable;
			netType = other.netType;
			encrypt = other.encrypt;
			loginWithBalance = other.loginWithBalance;
			trackerServer = other.trackerServer;
			fastdfsEnabled = other.fastdfsEnabled;
		}

		LoginConfig & operator=(const LoginConfig &other)
		{
			if (this == &other)
				return *this;

			name = other.name;
			version = other.version;
			transferIp = other.transferIp;
			transferPort = other.transferPort;
			managerUrl = other.managerUrl;
			storeHome = other.storeHome;
			removable = other.removable;
			netType = other.netType;
			encrypt = other.encrypt;
			loginWithBalance = other.loginWithBalance;
			trackerServer = other.trackerServer;
			fastdfsEnabled = other.fastdfsEnabled;
			return *this;
		}

		bool isValid() const { return !name.isEmpty(); }

	public:
		QString     name;
		QString     version;
		QString     transferIp;
		int         transferPort;
		QString     managerUrl;
		QString     storeHome;
		bool        removable;
		QString     netType;
		EncryptType encrypt;
		bool        loginWithBalance;
		QString     trackerServer;
		bool        fastdfsEnabled;
	};

	struct AudioConfig
	{
		QString type;
		int     chan;
		int     bit;
		int     rate;
		int     frame;
	};

	struct VideoConfig
	{
		int     width;
		int     height;
		int     fps;
		QString codec;
		int     deviceId;
	};

	struct LinkItem
	{
		QString name;
		QString iconUrl;
		QString linkUrl;
	};

public:
	static QString globalHomePath();

	static QString appHomePath();

	static QString title();

	static QString company();
	static QString companyUrl();
	static QString companyTel();
	static bool hideCompanyTel();
	static GlobalSettings::Language language();
	static void setLanguage(GlobalSettings::Language lang);

	static bool isCloseOptionOn();
	static void setCloseOptionOn(bool on);

	static bool isShortcutConflickTipOn();
	static void setShortcutConflictTipOn(bool on);

	static LoginConfig loginConfig(const QString &key);
	static bool updateLoginConfig(const QString &key, const LoginConfig &config);

	static LoginConfig curLoginConfig();
	static QList<LoginConfig> allLoginConfigs();

	static QStringList allLoginConfigKeys();
	static void setLoginConfigKeys(const QStringList &keys);

	static QString getCurrentLoginKey();
	static void setCurrentLoginKey(const QString &key);

	static bool appendLoginConfig(const QString &key, const LoginConfig &config);
	static bool removeLoginConfig(const QString &key);

	static QString getBeepSend();
	static QString getBeepRecv();
	static QString getBeepAudio();
	
	static void setPsgs(const QString &id, const QStringList &psgs);
	static void clearPsgs();
	static QStringList getPsgs(const QString &id);

	static void setDefPsgs(const QStringList &psgs);
	static QStringList defPsgs();

	static AudioConfig getAudioConfig();
	static VideoConfig getVideoConfig();

	static int audioFrameTimeInMs();
	static void setAudioFrameTimeInMs(int ms);

	static QString audioEncodeType();
	static void setAudioEncodeType(const QString &type);

	static int audioBufferedTimeInMs();
	static void setAudioBufferedTimeInMs(int ms);

	static int audioFramesPerPackage();
	static void setAudioFramesPerPackage(int count);

	static int audioPackageLossCount();
	static void setAudioPackageLossCount(int count);

	static bool audioLogToFile();
	static void setAudioLogToFile(bool log);
	static QString audioLogFileDirPath();

	static bool audioDisabled();
	static void setAudioDisabled(bool disabled);

	static bool videoDisabled();
	static void setVideoDisabled(bool disabled);

	static bool roamingMsgDisabled();
	static void setRoamingMsgDisabled(bool disabled);

	static int introductionViewType();
	static void setIntroductionViewType(int type);

	static bool subscriptionDisabled();
	static void setSubscriptionDisabled(bool disabled);

	static QString linkItems();
	static void setLinkItems(const QString &items);
	static QList<GlobalSettings::LinkItem> parseLinkItems(const QString &items);

	static int maxDiscussMemberCount();
	static void setMaxDiscussMemberCount(int count);

	static bool isRosterSmallAvatar();
	static void setRosterSmallAvatar(bool smallAvatar);

	static bool isOsLoadAll();
	static void setOsLoadAll(bool osLoadAll);

	static bool isOfflineSyncMsgEnabled();
	static void setOfflineSyncMsgEnabled(bool enabled);

	static bool interphoneDisabled();
	static void setInterphoneDisabled(bool disabled);

	static bool msgEncrypt();
	static void setMsgEncrypt(bool encrypt);

	static QString msgEncryptSeed();
	static void setMsgEncryptSeed(const QString &seed);

	static void setCompanySettings(const QMap<QString, QString> &companySettings);

	static int proxyType();
	static void setProxyType(int type);
	static QString proxyAddress();
	static void setProxyAddress(const QString &address);
	static void setProxyPort(int port);
	static int proxyPort();
	static void setProxyUser(const QString &user);
	static QString proxyUser();
	static void setProxyPassword(const QString &password);
	static QString proxyPassword();
	
private:
	void writeDefaultSettings();

	static QString curLoginConfigPath();
	static QString curAudioConfigPath();
	static QString curVideoConfigPath();
	static QString curBeepConfigPath();
	static QString curUiCustomConfigPath();
	
	static LoginConfig readLoginConfig(QSettings &settings, const QString &key, const QString &path);
	static LoginConfig readLoginConfig(const QString &key);
	static bool writeLoginConfig(const QString &key, const LoginConfig &loginConfig);

	GlobalSettings();
	~GlobalSettings();

	static QSettings& settings();
	static GlobalSettings* instance();

private:
	QScopedPointer<QSettings>  m_pGlobalSettings;
	LoginConfig                m_curLoginConfig;
	QMap<QString, QString>     m_companySettings;
	static GlobalSettings     *s_instance;
};

#endif //_GLOBALSETTINGS_H_