#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include "PsgManager.h"
#include "SettingConstants.h"
#include "cryptsettings.h"
#include "GlobalSettings.h"

static const char DIR_GLOBAL_NAME[]    = "All Users";
static const char GLOBAL_CONFIG_NAME[] = "global2.conf";
static const char DEF_PSG_TAG[]        = "DEF_PSG_1F48FD42-A714-46E1-8163-35AAF9348775";
static const QString ENV_OFFICIAL_NAME = QString::fromLatin1("official");
static const QString ENV_TEST_NAME     = QString::fromLatin1("test");

GlobalSettings* GlobalSettings::s_instance = 0;

QString GlobalSettings::globalHomePath()
{
    QDir dir(appHomePath());
    dir.mkpath(DIR_GLOBAL_NAME);
    dir.cd(DIR_GLOBAL_NAME);
    return dir.absolutePath();
}

QString GlobalSettings::appHomePath()
{
	LoginConfig loginConfig = curLoginConfig();
	QDir dir(loginConfig.storeHome);
	dir.mkpath(loginConfig.storeHome);
	return dir.absolutePath();
}

QString GlobalSettings::title()
{
	return settings().value("title", QObject::tr("Ling Talk")).toString();
}

QString GlobalSettings::company()
{
	return settings().value("company", QObject::tr("Suzhou Grand Lynn Information Technologies Co., Ltd")).toString();
}

QString GlobalSettings::companyUrl()
{
	return settings().value("companyUrl", QString("www.lingtalk.cn")).toString();
}

QString GlobalSettings::companyTel()
{
	return settings().value("companyTel", QString("0512-62982090")).toString();
}

bool GlobalSettings::hideCompanyTel()
{
	return (settings().value("companyTelVisibility", 1).toInt() != 1);
}

GlobalSettings::Language GlobalSettings::language()
{
	return (GlobalSettings::Language)(settings().value("language", (int)(Language_ZH_CN)).toInt());
}

void GlobalSettings::setLanguage(GlobalSettings::Language lang)
{
	settings().setValue("language", (int)lang);
}

bool GlobalSettings::isCloseOptionOn()
{
    int on = settings().value("closeoptionon", 1).toInt();
    if (on == 1)
        return true;
    else
        return false;
}

void GlobalSettings::setCloseOptionOn(bool on)
{
    settings().setValue("closeoptionon", (on ? 1 : 0));
}

bool GlobalSettings::isShortcutConflickTipOn()
{
	int on = settings().value("shortcutconflicttip", 1).toInt();
	if (on == 1)
		return true;
	else
		return false;
}

void GlobalSettings::setShortcutConflictTipOn(bool on)
{
	settings().setValue("shortcutconflicttip", (on ? 1 : 0));
}

GlobalSettings::LoginConfig GlobalSettings::loginConfig(const QString &key)
{
	LoginConfig config = readLoginConfig(key);
	return config;
}

bool GlobalSettings::updateLoginConfig(const QString &key, const LoginConfig &config)
{
	if (!config.isValid())
	{
		qWarning() << Q_FUNC_INFO << "config is not valid";
		return false;
	}

	QStringList allKeys = allLoginConfigKeys();
	if (!allKeys.contains(key))
	{
		qWarning() << Q_FUNC_INFO << key << " is out of: " << allKeys;
		return false;
	}

	writeLoginConfig(key, config);

	// update current login configuration
	if (getCurrentLoginKey() == key)
	{
		instance()->m_curLoginConfig = config;
	}
	return true;
}

GlobalSettings::LoginConfig GlobalSettings::curLoginConfig()
{
	return instance()->m_curLoginConfig;
}

QList<GlobalSettings::LoginConfig> GlobalSettings::allLoginConfigs()
{
	QList<LoginConfig> loginConfigs;
	QStringList allKeys = allLoginConfigKeys();
	foreach (QString key, allKeys)
	{
		LoginConfig config = readLoginConfig(key);
		if (config.isValid())
			loginConfigs.append(config);
	}
	return loginConfigs;
}

QStringList GlobalSettings::allLoginConfigKeys()
{
	return settings().value("login_configs/keys", QStringList()).toStringList();
}

void GlobalSettings::setLoginConfigKeys(const QStringList &keys)
{
	settings().setValue("login_configs/keys", keys);
}

QString GlobalSettings::getCurrentLoginKey()
{
	return instance()->m_curLoginConfig.name;
}

void GlobalSettings::setCurrentLoginKey(const QString &key)
{
	settings().setValue("login_configs/current", key);
	
	// re-read current login configuration
	if (!(key == getCurrentLoginKey()))
		instance()->m_curLoginConfig = readLoginConfig(key);
}

bool GlobalSettings::appendLoginConfig(const QString &key, const LoginConfig &config)
{
	if (key.isEmpty() || !config.isValid())
	{
		qWarning() << Q_FUNC_INFO << "config is not valid";
		return false;
	}

	QStringList allKeys = allLoginConfigKeys();
	if (allKeys.contains(key))
	{
		qWarning() << Q_FUNC_INFO << "already contains login config:" << key;
		return false;
	}

	// append config
	writeLoginConfig(key, config);

	// update all keys
	allKeys.append(key);
	setLoginConfigKeys(allKeys);

	return true;
}

bool GlobalSettings::removeLoginConfig(const QString &key)
{
	QStringList allKeys = allLoginConfigKeys();
	if (!allKeys.contains(key))
	{
		qWarning() << Q_FUNC_INFO << "does not contain login config:" << key;
		return false;
	}

	// remove config
	QString path = QString("login_configs/") + key;
	settings().remove(path);

	// remove key
	allKeys.removeAll(key);
	setLoginConfigKeys(allKeys);

	return true;
}

QString GlobalSettings::getBeepSend()
{
	return settings().value(curBeepConfigPath()+QString("send"), CONFIG_SEND_BEEP_PATH).toString();
}

QString GlobalSettings::getBeepRecv()
{
	return settings().value(curBeepConfigPath()+QString("recv"), CONFIG_RECV_BEEP_PATH).toString();
}

QString GlobalSettings::getBeepAudio()
{
	return settings().value(curBeepConfigPath()+QString("audio"), CONFIG_AUDIO_BEEP_PATH).toString();
}

void GlobalSettings::setPsgs(const QString &id, const QStringList &psgs)
{
	settings().beginWriteArray(curLoginConfigPath()+QString("psgs/%1").arg(id));
	int i = 0;
	foreach (QString psg, psgs)
	{
		settings().setArrayIndex(i++);
		settings().setValue("address", psg);
	}
	settings().endArray();
}

void GlobalSettings::clearPsgs()
{
	settings().remove(curLoginConfigPath()+QString("psgs"));
}

QStringList GlobalSettings::getPsgs(const QString &id)
{
	QStringList psgs;
	int size = settings().beginReadArray(curLoginConfigPath()+QString("psgs/%1").arg(id));
	for (int i = 0; i < size; ++i)
	{
		settings().setArrayIndex(i);
		QString psg = settings().value("address").toString();
		psgs << psg;
	}
	settings().endArray();
	return psgs;
}

void GlobalSettings::setDefPsgs(const QStringList &psgs)
{
	settings().beginWriteArray(curLoginConfigPath()+QString("psgs/%1").arg(QString(DEF_PSG_TAG)));
	int i = 0;
	foreach (QString psg, psgs)
	{
		settings().setArrayIndex(i++);
		settings().setValue("address", psg);
	}
	settings().endArray();
}

QStringList GlobalSettings::defPsgs()
{
	QStringList psgs;
	int size = settings().beginReadArray(curLoginConfigPath()+QString("psgs/%1").arg(DEF_PSG_TAG));
	for (int i = 0; i < size; ++i)
	{
		settings().setArrayIndex(i);
		QString psg = settings().value("address").toString();
		psgs << psg;
	}
	settings().endArray();
	return psgs;
}

GlobalSettings::AudioConfig GlobalSettings::getAudioConfig()
{
	AudioConfig audioConfig;
	audioConfig.type = settings().value(curAudioConfigPath()+QString("type"), CONFIG_AUDIODESC_TYPE).toString();
	if (audioConfig.type.isEmpty())
		audioConfig.type = CONFIG_AUDIODESC_TYPE;

	audioConfig.chan = settings().value(curAudioConfigPath()+QString("chan"), CONFIG_AUDIODESC_CHAN).toInt();
	if (audioConfig.chan <= 0)
		audioConfig.chan = CONFIG_AUDIODESC_CHAN;

	audioConfig.bit = settings().value(curAudioConfigPath()+QString("bit"), CONFIG_AUDIODESC_BIT).toInt();
	if (audioConfig.bit <= 0)
		audioConfig.bit = CONFIG_AUDIODESC_BIT;

	audioConfig.rate = settings().value(curAudioConfigPath()+QString("rate"), CONFIG_AUDIODESC_RATE).toInt();
	if (audioConfig.rate <= 0)
		audioConfig.rate = CONFIG_AUDIODESC_RATE;

	audioConfig.frame = settings().value(curAudioConfigPath()+QString("frame"), CONFIG_AUDIODESC_FRAME).toInt();
	if (audioConfig.frame <= 0)
		audioConfig.frame = CONFIG_AUDIODESC_FRAME;

	return audioConfig;
}

int GlobalSettings::audioFrameTimeInMs()
{
	int frameTimeInMs = settings().value(curAudioConfigPath()+QString("frame"), CONFIG_AUDIODESC_FRAME).toInt();
	return frameTimeInMs;
}

void GlobalSettings::setAudioFrameTimeInMs(int ms)
{
	settings().setValue(curAudioConfigPath()+QString("frame"), ms);
}

QString GlobalSettings::audioEncodeType()
{
	QString encodeType = settings().value(curAudioConfigPath()+QString("type"), CONFIG_AUDIODESC_TYPE).toString();
	return encodeType;
}

void GlobalSettings::setAudioEncodeType(const QString &type)
{
	settings().setValue(curAudioConfigPath()+QString("type"), type);
}

GlobalSettings::VideoConfig GlobalSettings::getVideoConfig()
{
	VideoConfig videoConfig;
	videoConfig.width = settings().value(curVideoConfigPath()+QString("width"), CONFIG_VIDEO_WIDTH).toInt();
	if (videoConfig.width <= 0)
		videoConfig.width = CONFIG_VIDEO_WIDTH;

	videoConfig.height = settings().value(curVideoConfigPath()+QString("height"), CONFIG_VIDEO_HEIGHT).toInt();
	if (videoConfig.height <= 0)
		videoConfig.height = CONFIG_VIDEO_HEIGHT;

	videoConfig.fps = settings().value(curVideoConfigPath()+QString("fps"), CONFIG_VIDEO_FPS).toInt();
	if (videoConfig.fps <= 0)
		videoConfig.fps = CONFIG_VIDEO_FPS;

	videoConfig.codec = settings().value(curVideoConfigPath()+QString("codec"), CONFIG_VIDEO_CODEC).toString();
	if (videoConfig.codec.isEmpty())
		videoConfig.codec = CONFIG_VIDEO_CODEC;

	videoConfig.deviceId = settings().value(curVideoConfigPath()+QString("device"), CONFIG_VIDEO_DEVICEID).toInt();
	if (videoConfig.deviceId <= 0)
		videoConfig.deviceId = CONFIG_VIDEO_DEVICEID;

	return videoConfig;
}

int GlobalSettings::audioBufferedTimeInMs()
{
	int ms = settings().value(curAudioConfigPath()+QString("bufferedTime"), 500).toInt();
	return ms;
}

void GlobalSettings::setAudioBufferedTimeInMs(int ms)
{
	settings().setValue(curAudioConfigPath()+QString("bufferedTime"), ms);
}

int GlobalSettings::audioFramesPerPackage()
{
	int count = settings().value(curAudioConfigPath()+QString("framesPerPackage"), 1).toInt();
	return count;
}

void GlobalSettings::setAudioFramesPerPackage(int count)
{
	settings().setValue(curAudioConfigPath()+QString("framesPerPackage"), count);
}

int GlobalSettings::audioPackageLossCount()
{
	int count = settings().value(curAudioConfigPath()+QString("packageLossCount"), 0).toInt();
	return count;
}

void GlobalSettings::setAudioPackageLossCount(int count)
{
	settings().setValue(curAudioConfigPath()+QString("packageLossCount"), count);
}

bool GlobalSettings::audioLogToFile()
{
	bool logToFile = settings().value(curAudioConfigPath()+QString("logToFile"), false).toBool();
	return logToFile;
}

void GlobalSettings::setAudioLogToFile(bool log)
{
	settings().setValue(curAudioConfigPath()+QString("logToFile"), log);
}

QString GlobalSettings::audioLogFileDirPath()
{
	QString dirPath = ("d:\\pm_audio_test");
	QDir dir(dirPath);
	if (!dir.exists())
	{
		dir.mkpath(dirPath);
	}
	return dirPath;
}

bool GlobalSettings::audioDisabled()
{
	if (instance()->m_companySettings.contains(COMMONSET_AUDIO_DISABLED))
	{
		if (instance()->m_companySettings[COMMONSET_AUDIO_DISABLED] == QString("1"))
			return true;
		else
			return false;
	}

	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_AUDIO_DISABLED), false).toBool();
}

void GlobalSettings::setAudioDisabled(bool disabled)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_AUDIO_DISABLED), disabled);
}

bool GlobalSettings::videoDisabled()
{
	if (instance()->m_companySettings.contains(COMMONSET_VIDEO_DISABLED))
	{
		if (instance()->m_companySettings[COMMONSET_VIDEO_DISABLED] == QString("1"))
			return true;
		else
			return false;
	}

	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_VIDEO_DISABLED), false).toBool();
}

void GlobalSettings::setVideoDisabled(bool disabled)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_VIDEO_DISABLED), disabled);
}

bool GlobalSettings::roamingMsgDisabled()
{
	if (instance()->m_companySettings.contains(COMMONSET_ROAMING_MSG_DISABLED))
	{
		if (instance()->m_companySettings[COMMONSET_ROAMING_MSG_DISABLED] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_ROAMING_MSG_DISABLED), false).toBool();
}

void GlobalSettings::setRoamingMsgDisabled(bool disabled)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_ROAMING_MSG_DISABLED), disabled);
}

int GlobalSettings::introductionViewType()
{
	if (instance()->m_companySettings.contains(COMMONSET_INTRODUCTION_VIEW_TYPE))
	{
		return instance()->m_companySettings[COMMONSET_INTRODUCTION_VIEW_TYPE].toInt();
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_INTRODUCTION_VIEW_TYPE), 0).toInt();
}

void GlobalSettings::setIntroductionViewType(int type)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_INTRODUCTION_VIEW_TYPE), type);
}

bool GlobalSettings::subscriptionDisabled()
{
	if (instance()->m_companySettings.contains(COMMONSET_SUBSCRIPTION_DISABLED))
	{
		if (instance()->m_companySettings[COMMONSET_SUBSCRIPTION_DISABLED] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_SUBSCRIPTION_DISABLED), false).toBool();
}

void GlobalSettings::setSubscriptionDisabled(bool disabled)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_SUBSCRIPTION_DISABLED), disabled);
}

QString GlobalSettings::linkItems()
{
	if (instance()->m_companySettings.contains(COMMONSET_LINK_ITEMS))
	{
		return instance()->m_companySettings[COMMONSET_LINK_ITEMS];
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_LINK_ITEMS), "").toString();
}

void GlobalSettings::setLinkItems(const QString &items)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_LINK_ITEMS), items);
}

QList<GlobalSettings::LinkItem> GlobalSettings::parseLinkItems(const QString &items)
{
	QList<GlobalSettings::LinkItem> linkItems;
	QStringList strs = items.split(";");
	foreach (QString str, strs)
	{
		QStringList parts = str.split("|");
		if (parts.count() != 3)
			continue;

		GlobalSettings::LinkItem linkItem;
		linkItem.name = QString::fromUtf8(QByteArray::fromBase64(parts[0].trimmed().toLatin1()));
		linkItem.iconUrl = QString::fromUtf8(QByteArray::fromBase64(parts[1].trimmed().toLatin1()));
		linkItem.linkUrl = QString::fromUtf8(QByteArray::fromBase64(parts[2].trimmed().toLatin1()));
		linkItems.append(linkItem);
	}
	return linkItems;
}

int GlobalSettings::maxDiscussMemberCount()
{
	if (instance()->m_companySettings.contains(COMMONSET_MAX_DISCUSS_MEMBER_COUNT))
	{
		return instance()->m_companySettings[COMMONSET_MAX_DISCUSS_MEMBER_COUNT].toInt();
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_MAX_DISCUSS_MEMBER_COUNT), 50).toInt();
}

void GlobalSettings::setMaxDiscussMemberCount(int count)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_MAX_DISCUSS_MEMBER_COUNT), count);
}

bool GlobalSettings::isRosterSmallAvatar()
{
	if (instance()->m_companySettings.contains(COMMONSET_ROSTER_SMALL_AVATAR))
	{
		if (instance()->m_companySettings[COMMONSET_ROSTER_SMALL_AVATAR] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_ROSTER_SMALL_AVATAR), false).toBool();
}

void GlobalSettings::setRosterSmallAvatar(bool smallAvatar)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_ROSTER_SMALL_AVATAR), smallAvatar);
}

bool GlobalSettings::isOsLoadAll()
{
	if (instance()->m_companySettings.contains(COMMONSET_OS_LOAD_ALL))
	{
		if (instance()->m_companySettings[COMMONSET_OS_LOAD_ALL] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_OS_LOAD_ALL), false).toBool();
}

void GlobalSettings::setOsLoadAll(bool osLoadAll)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_OS_LOAD_ALL), osLoadAll);
}

bool GlobalSettings::isOfflineSyncMsgEnabled()
{
	if (instance()->m_companySettings.contains(COMMONSET_OFFLINE_SYNC_MSG_ENABLED))
	{
		if (instance()->m_companySettings[COMMONSET_OFFLINE_SYNC_MSG_ENABLED] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_OFFLINE_SYNC_MSG_ENABLED), true).toBool();
}

void GlobalSettings::setOfflineSyncMsgEnabled(bool enabled)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_OFFLINE_SYNC_MSG_ENABLED), enabled);
}

bool GlobalSettings::interphoneDisabled()
{
	if (instance()->m_companySettings.contains(COMMONSET_INTER_PHONE_DISABLED))
	{
		if (instance()->m_companySettings[COMMONSET_INTER_PHONE_DISABLED] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_INTER_PHONE_DISABLED), false).toBool();
}

void GlobalSettings::setInterphoneDisabled(bool disabled)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_INTER_PHONE_DISABLED), disabled);
}

bool GlobalSettings::msgEncrypt()
{
	if (instance()->m_companySettings.contains(COMMONSET_MSG_ENCRYPT))
	{
		if (instance()->m_companySettings[COMMONSET_MSG_ENCRYPT] == QString("1"))
			return true;
		else
			return false;
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_MSG_ENCRYPT), false).toBool();
}

void GlobalSettings::setMsgEncrypt(bool encrypt)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_MSG_ENCRYPT), encrypt);
}

QString GlobalSettings::msgEncryptSeed()
{
	if (instance()->m_companySettings.contains(COMMONSET_MSG_ENCRYPT_SEED))
	{
		return instance()->m_companySettings[COMMONSET_MSG_ENCRYPT_SEED];
	}
	return settings().value(curUiCustomConfigPath()+QString(COMMONSET_MSG_ENCRYPT_SEED), "").toString();
}

void GlobalSettings::setMsgEncryptSeed(const QString &seed)
{
	settings().setValue(curUiCustomConfigPath()+QString(COMMONSET_MSG_ENCRYPT_SEED), seed);
}

void GlobalSettings::setCompanySettings(const QMap<QString, QString> &companySettings)
{
	instance()->m_companySettings = companySettings;
}

int GlobalSettings::proxyType()
{
	return settings().value("proxy/type", 0).toInt();
}

void GlobalSettings::setProxyType(int type)
{
	settings().setValue("proxy/type", type);
}

QString GlobalSettings::proxyAddress()
{
	return settings().value("proxy/address", "").toString();
}

void GlobalSettings::setProxyAddress(const QString &address)
{
	settings().setValue("proxy/address", address);
}

void GlobalSettings::setProxyPort(int port)
{
	settings().setValue("proxy/port", port);
}

int GlobalSettings::proxyPort()
{
	return settings().value("proxy/port", 80).toInt();
}

void GlobalSettings::setProxyUser(const QString &user)
{
	settings().setValue("proxy/user", user);
}

QString GlobalSettings::proxyUser()
{
	return settings().value("proxy/user").toString();
}

void GlobalSettings::setProxyPassword(const QString &password)
{
	settings().setValue("proxy/password", password);
}

QString GlobalSettings::proxyPassword()
{
	return settings().value("proxy/password", "").toString();
}

QString GlobalSettings::curLoginConfigPath()
{
	QString curKey = getCurrentLoginKey();
	return QString("login_configs/%1/").arg(curKey);
}

QString GlobalSettings::curAudioConfigPath()
{
	QString curKey = getCurrentLoginKey();
	return QString("audio/%1/").arg(curKey);
}

QString GlobalSettings::curVideoConfigPath()
{
	QString curKey = getCurrentLoginKey();
	return QString("video/%1/").arg(curKey);
}

QString GlobalSettings::curBeepConfigPath()
{
	QString curKey = getCurrentLoginKey();
	return QString("beep/%1/").arg(curKey);
}

QString GlobalSettings::curUiCustomConfigPath()
{
	QString curKey = getCurrentLoginKey();
	return QString("ui_custom/%1/").arg(curKey);
}

void GlobalSettings::writeDefaultSettings()
{
	m_pGlobalSettings->beginGroup("login_configs"); // begin login_configs
	
	m_pGlobalSettings->setValue("keys", QStringList() << ENV_OFFICIAL_NAME << ENV_TEST_NAME);
#ifndef NDEBUG
	m_pGlobalSettings->setValue("current", ENV_TEST_NAME); // test environment
#else
	m_pGlobalSettings->setValue("current", ENV_OFFICIAL_NAME); // official environment
#endif // NDEBUG

	// official environment : official
	m_pGlobalSettings->setValue(ENV_OFFICIAL_NAME, 1);
	m_pGlobalSettings->setValue(ENV_OFFICIAL_NAME+QString("/manager_url"), OFFICIAL_CONFIG_MANAGER_URL);
	m_pGlobalSettings->setValue(ENV_OFFICIAL_NAME+QString("/removable"), false);
	m_pGlobalSettings->setValue(ENV_OFFICIAL_NAME+QString("/nettype"), QString(OUT_ADDRESS_NAME));
#ifndef NDEBUG
	QString officialStoreHome = QCoreApplication::applicationDirPath() + QString("/official env/") + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName();
	officialStoreHome = QDir::cleanPath(officialStoreHome);
	m_pGlobalSettings->setValue(ENV_OFFICIAL_NAME+QString("/store_home"), officialStoreHome);
#endif // NDEBUG

	// test environment: test
	m_pGlobalSettings->setValue(ENV_TEST_NAME, 1);
	m_pGlobalSettings->setValue(ENV_TEST_NAME+QString("/manager_url"), TEST_CONFIG_MANAGER_URL);
	m_pGlobalSettings->setValue(ENV_TEST_NAME+QString("/removable"), false);
	m_pGlobalSettings->setValue(ENV_TEST_NAME+QString("/nettype"), QString(OUT_ADDRESS_NAME));
#ifndef NDEBUG
	QString testStoreHome = QCoreApplication::applicationDirPath() + QString("/test env/") + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName();
	testStoreHome = QDir::cleanPath(testStoreHome);
	m_pGlobalSettings->setValue(ENV_TEST_NAME+QString("/store_home"), testStoreHome);
#endif // NDEBUG

	m_pGlobalSettings->endGroup(); // end login_configs

	m_pGlobalSettings->sync();
}

GlobalSettings::LoginConfig GlobalSettings::readLoginConfig(QSettings &settings, const QString &key, const QString &path)
{
	LoginConfig config;
	config.name = key;
	config.version = settings.value(path+QString("/version"), "").toString();
	config.transferIp = settings.value(path+QString("/transfer_ip"), "").toString();
	config.transferPort = settings.value(path+QString("/transfer_port"), 0).toInt();
	config.managerUrl = settings.value(path+QString("/manager_url"), "").toString();
	config.removable = settings.value(path+QString("/removable"), true).toBool();
	config.storeHome = settings.value(path+QString("/store_home"), "").toString();
	config.netType = settings.value(path+QString("/nettype"), QString(OUT_ADDRESS_NAME)).toString();
	config.encrypt = (EncryptType)(settings.value(path+QString("/encrypt"), 0).toInt());
	config.loginWithBalance = settings.value(path+QString("/balance"), false).toBool();
	config.trackerServer = settings.value(path + QString("/tracker_server"), "").toString();
	config.fastdfsEnabled = settings.value(path + QString("/fastdfs_enabled"), false).toBool();
	return config;
}

GlobalSettings::LoginConfig GlobalSettings::readLoginConfig(const QString &key)
{
	LoginConfig config;

	QStringList allKeys = allLoginConfigKeys();
	if (!allKeys.contains(key))
	{
		qWarning() << Q_FUNC_INFO << key << " does not exist in keys.";
		return config;
	}

	QString path = QString("login_configs/") + key;
	if (!settings().contains(path))
	{
		qWarning() << Q_FUNC_INFO << key << "does not exist in settings.";
		return config;
	}

	config = readLoginConfig(settings(), key, path);
	return config;
}

bool GlobalSettings::writeLoginConfig(const QString &key, const LoginConfig &loginConfig)
{
	if (key.isEmpty() || !loginConfig.isValid())
	{
		qWarning() << Q_FUNC_INFO << " key is not valid.";
		return false;
	}

	QString path = QString("login_configs/") + key;
	settings().setValue(path, 1);
	settings().setValue(path+QString("/version"), loginConfig.version);
	settings().setValue(path+QString("/transfer_ip"), loginConfig.transferIp);
	settings().setValue(path+QString("/transfer_port"), loginConfig.transferPort);
	settings().setValue(path+QString("/manager_url"), loginConfig.managerUrl);
	settings().setValue(path+QString("/removable"), loginConfig.removable);
	settings().setValue(path+QString("/nettype"), loginConfig.netType);
	settings().setValue(path+QString("/store_home"), loginConfig.storeHome);
	settings().setValue(path+QString("/encrypt"), (int)(loginConfig.encrypt));
	settings().setValue(path+QString("/balance"), loginConfig.loginWithBalance);
	settings().setValue(path+QString("/tracker_server"), loginConfig.trackerServer);
	settings().setValue(path+QString("/fastdfs_enabled"), loginConfig.fastdfsEnabled);

	return true;
}

GlobalSettings::GlobalSettings()
{
	// get global setting path
	QString appdata = QProcessEnvironment::systemEnvironment().value("APPDATA");
	QDir dir(appdata);
#ifndef NDEBUG
	QString path = QCoreApplication::organizationName() + QString("/%1").arg(QCoreApplication::applicationName()) + QString("/test env");
#else
	QString path = QCoreApplication::organizationName() + QString("/%1").arg(QCoreApplication::applicationName());
#endif
	dir.mkpath(path);
	dir.cd(path);
	qWarning() << Q_FUNC_INFO << "global setting path: " << dir.absolutePath();

	m_pGlobalSettings.reset(new QSettings(dir.absoluteFilePath(GLOBAL_CONFIG_NAME), CryptSettings::format()));
	Q_ASSERT(m_pGlobalSettings != 0);

	// check if has necessary setting
	if (m_pGlobalSettings->allKeys().isEmpty())
		writeDefaultSettings();

	// check to write environment store home
#ifdef NDEBUG
	QStringList storeKeys;
	storeKeys << (QString("login_configs/") + ENV_OFFICIAL_NAME + QString("/store_home"));
	storeKeys << (QString("login_configs/") + ENV_TEST_NAME + QString("/store_home"));
	int i = 0;
	foreach (QString storeKey, storeKeys)
	{
		if (!m_pGlobalSettings->contains(storeKey))
		{
			QDir docDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
			QString subDirPath = path;
			if (i == 1)
				subDirPath = subDirPath + "/" + QString("test");
			docDir.mkpath(subDirPath);
			docDir.cd(subDirPath);

			qWarning() << Q_FUNC_INFO << storeKey << " : " << docDir.absolutePath();
			m_pGlobalSettings->setValue(storeKey, docDir.absolutePath());
		}
		++i;
	}
#endif // NDEBUG

	// reading current login configuration
	QString curKey = m_pGlobalSettings->value("login_configs/current").toString();
	QString curPath = QString("login_configs/") + curKey;
	m_curLoginConfig = readLoginConfig(*(m_pGlobalSettings.data()), curKey, curPath);
}

GlobalSettings::~GlobalSettings()
{
	// flush global settings
	settings().sync();
	qWarning() << Q_FUNC_INFO;
}

QSettings& GlobalSettings::settings()
{
	GlobalSettings *gs = instance();
	return *(gs->m_pGlobalSettings.data());
}

GlobalSettings* GlobalSettings::instance()
{
	if (!s_instance)
		s_instance = new GlobalSettings();
	return s_instance;
}