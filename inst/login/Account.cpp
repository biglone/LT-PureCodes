#include <QApplication>
#include <QDebug>
#include <QScopedPointer>
#include <QDateTime>
#include <QDir>
#include "base/Base.h"
#include "settings/cryptsettings.h"
#include "settings/GlobalSettings.h"
#include "db/UserDB.h"
#include "Account.h"

const char DIR_ATTACH_NAME[]                 = "Attachments";
const char DIR_AUDIO_NAME[]                  = "Audios";
const char DIR_VIDEO_NAME[]                  = "Videos";
const char DIR_IMAGE_NAME[]                  = "Images";
const char DIR_AVATAR_NAME[]                 = "Avatars";
const char DIR_SUBSCRIPTION_NAME[]           = "Subscriptions";
const char DIR_GLOBALNOTIFICATION_NAME[]     = "GlobalNotifications";
const char DIR_CACHE_NAME[]                  = "Caches";
const char DIR_GROUP_NAME[]                  = "Groups";
const char DIR_EMOTION_NAME[]                = "Emotions";
const char DIR_THUMB_NAME[]                  = "Thumbs";
										     
static const char *LOGIN_PLATFORM            = "win32";
static const char *ACCOUNT_ID_SEPERATOR      = "/";

const char *RESOURCE_COMPUTER                = "computer";
const char *RESOURCE_PHONE                   = "phone";
										     
const char USER_DB_FILENAME[]                = "contacts.db";
const char MESSAGE_DB_FILENAME[]             = "messages.db";
										     
const char DB_SUBSCRIPTION_DBNAME[]          = "subscriptions.db";
const char DB_SUBSCRIPTIONMESSAGES_DBNAME[]  = "subscriptionmessages.db";

const char DB_GLOBALNOTIFICATION_DBNAME[]	 = "globalnotifications.db";
const char DB_GLOBALNOTIFICATIONMESSAGES_DBNAME[] = "globalnotificationmessages.db";

class AccountPrivate : public QxtPrivate<Account>
{
public:
	QXT_DECLARE_PUBLIC(Account)

	AccountPrivate() 
	{
		clear();
	}
	~AccountPrivate() {}

	QDir homeDir() const;
	QDir phoneDir() const;

	void clear();
	bool storeLoginPhone();

	bool updateLoginPhoneTime();
	bool clearPassword();

	bool storeUserIds();
	bool setUserIdDefault();

	void createSettings();

	QString countryCode;
	QString loginPhone;
	QString id;
	QString pwd;
	QString cryptoPwd;
	QString resource;
	QString platform;
	bool    voilent;
	int     status;
	bool    savePwd;

	bool    logined;

	bool    autologin;

	bool    settingCreated;

	QString storePath;

	QList<CompanyInfo>                         companyInfos;

	// 登录后的数据保存
	QStringList                                modules;            /// 权限模块列表
	QMap<QString, base::AddressMap>            services;           /// 服务参数表

	// settings
	QScopedPointer<AccountSettings>            settings;

	DB::UserDB                                 userDB;
};

QDir AccountPrivate::homeDir() const
{
	Q_ASSERT_X((!loginPhone.isEmpty())&&(!id.isEmpty()), "homeDir", "Account does not init");

	QDir home(GlobalSettings::appHomePath());
	home.mkdir(loginPhone);
	home.cd(loginPhone);
	home.mkdir(id);
	home.cd(id);

	return home;
}

QDir AccountPrivate::phoneDir() const
{
	Q_ASSERT_X(!loginPhone.isEmpty(), "phoneDir", "Account does not init");

	QDir phone(GlobalSettings::appHomePath());
	phone.mkdir(loginPhone);
	phone.cd(loginPhone);

	return phone;
}

void AccountPrivate::clear()
{
	loginPhone = "";
	id = "";
	pwd = "";
	cryptoPwd = "";
	resource = RESOURCE_COMPUTER;
	platform = LOGIN_PLATFORM;
	voilent = false;
	status = 0;

	savePwd = false;

	autologin = false;

	logined = false;

	companyInfos.clear();

	modules.clear();
	services.clear();

	if (settings)
		settings.reset();

	settingCreated = false;
}

bool AccountPrivate::storeLoginPhone()
{
	DB::UserDB::PhoneItem item;
	item.countryCode = countryCode;
	item.phone = loginPhone;
	item.passwd = pwd;
	item.storePasswd = savePwd;
	item.dateTime = QDateTime::currentDateTimeUtc();
	return userDB.storeAccount(item);
}

bool AccountPrivate::updateLoginPhoneTime()
{
	return userDB.updateAccountTime(loginPhone, QDateTime::currentDateTimeUtc());
}

bool AccountPrivate::clearPassword()
{
	return userDB.clearPasswd(loginPhone);
}

bool AccountPrivate::storeUserIds()
{
	if (!userDB.deleteUserIdsForPhone(loginPhone))
		return false;

	foreach (CompanyInfo compInfo, companyInfos)
	{
		DB::UserDB::UserIdItem item;
		item.uid = compInfo.uid;
		item.uname = compInfo.uname;
		item.cid = compInfo.companyId;
		item.cname = compInfo.companyName;
		item.phone = loginPhone;
		if (!userDB.storeUserId(item))
			return false;
	}
	return true;
}

bool AccountPrivate::setUserIdDefault()
{
	return userDB.setUserIdDefault(id);
}

void AccountPrivate::createSettings()
{
	if (settingCreated)
		return;

	QDir home = homeDir();
	settings.reset(new AccountSettings(home));

	settingCreated = true;
}

static void registerType()
{
	static bool init = false;
	if (!init)
	{
		qRegisterMetaType< QMap<QString, QString> >("QMap<QString,QString>");
		qRegisterMetaType<base::AddressMap>("base::AddressMap");
		qRegisterMetaType< QMap<QString, base::AddressMap> >("QMap<QString,base::AddressMap>");
		init = true;
	}
}

Account* Account::self = 0;

Account* Account::instance()
{
	Q_ASSERT(self != NULL);
	return self;
}

Account::Account()
{
	QXT_INIT_PRIVATE(Account)
	registerType();
	self = this;
}

Account::~Account()
{
	qDebug() << Q_FUNC_INFO;
	self = 0;
}

void Account::setCountryCode(const QString &countryCode)
{
	QXT_D(Account);
	d.countryCode = countryCode;
}

void Account::setLoginPhone(const QString &loginPhone)
{
	QXT_D(Account);
	d.loginPhone = loginPhone;
}

void Account::setId(const QString& id)
{
	QXT_D(Account);
	d.id = id;
}

void Account::setPwd(const QString& pwd)
{
	QXT_D(Account);
	d.pwd = pwd;
}

void Account::setCryptoPwd(const QString &cryptoPwd)
{
	QXT_D(Account);
	d.cryptoPwd = cryptoPwd;
}

void Account::setResource(const QString& resource)
{
	QXT_D(Account);
	d.resource = resource;
}

void Account::setVoilent(bool voilent)
{
	QXT_D(Account);
	d.voilent = voilent;
}

void Account::setStatus(int status)
{
	QXT_D(Account);
	d.status = status;
}

void Account::setSavePwd(bool savePwd)
{
	QXT_D(Account);
	d.savePwd = savePwd;
}

void Account::setAutologin(bool autologin)
{
	QXT_D(Account);
	d.autologin = autologin;
}

void Account::setCompanyInfos(const QList<CompanyInfo> &companyInfos)
{
	QXT_D(Account);
	d.companyInfos = companyInfos;
}

void Account::setLogined()
{
	QXT_D(Account);
	d.logined = true;
}

void Account::setModules(const QStringList& modules)
{
	QXT_D(Account);
	d.modules = modules;
}

void Account::setServiceParameter(const QMap<QString, base::AddressMap>& services)
{
	QXT_D(Account);
	d.services = services;
}

QString Account::countryCode() const
{
	return qxt_d().countryCode;
}

QString Account::loginPhone() const
{
	return qxt_d().loginPhone;
}

QString Account::id() const
{
	return qxt_d().id;
}

QString Account::fullId() const
{
	QString fId = QString("%1%2%3").arg(qxt_d().id).arg(ACCOUNT_ID_SEPERATOR).arg(qxt_d().resource);
	return fId;
}

QString Account::computerFullId() const
{
	QString fId = QString("%1%2%3").arg(qxt_d().id).arg(ACCOUNT_ID_SEPERATOR).arg(RESOURCE_COMPUTER);
	return fId;
}

QString Account::phoneFullId() const
{
	QString fId = QString("%1%2%3").arg(qxt_d().id).arg(ACCOUNT_ID_SEPERATOR).arg(RESOURCE_PHONE);
	return fId;
}

QString Account::idFromFullId(const QString &fullId)
{
	int index = fullId.indexOf(ACCOUNT_ID_SEPERATOR);
	if (index != -1)
	{
		return fullId.left(index);
	}
	else
	{
		return fullId;
	}
}

QString Account::resourceFromFullId(const QString &fullId)
{
	QString resource;
	int index = fullId.indexOf(ACCOUNT_ID_SEPERATOR);
	if (index != -1)
	{
		resource = fullId.mid(index+1);
	}
	return resource;
}

QString Account::fullIdFromIdResource(const QString &id, const QString &resource)
{
	QString fId = QString("%1%2%3").arg(id).arg(ACCOUNT_ID_SEPERATOR).arg(resource);
	return fId;
}

QString Account::computerName()
{
	return tr("My computer");
}

QString Account::phoneName()
{
	return tr("My phone");
}

QString Account::name() const
{
	QString uname;
	QList<CompanyInfo> companyInfos = qxt_d().companyInfos;
	foreach (CompanyInfo companyInfo, companyInfos)
	{
		if (companyInfo.uid == qxt_d().id)
		{
			uname = companyInfo.uname;
			break;
		}
	}
	return uname;
}

QString Account::companyId() const
{
	QString cid;
	QList<CompanyInfo> companyInfos = qxt_d().companyInfos;
	foreach (CompanyInfo companyInfo, companyInfos)
	{
		if (companyInfo.uid == qxt_d().id)
		{
			cid = companyInfo.companyId;
			break;
		}
	}
	return cid;
}

QString Account::companyName() const
{
	QString cname;
	QList<CompanyInfo> companyInfos = qxt_d().companyInfos;
	foreach (CompanyInfo companyInfo, companyInfos)
	{
		if (companyInfo.uid == qxt_d().id)
		{
			cname = companyInfo.companyName;
			break;
		}
	}
	return cname;
}

QString Account::pwd() const
{
	return qxt_d().pwd;
}

QString Account::cryptoPwd() const
{
	return qxt_d().cryptoPwd;
}

QString Account::resource() const
{
	return qxt_d().resource;
}

QString Account::platform() const
{
	return qxt_d().platform;
}

bool Account::voilent() const
{
	return qxt_d().voilent;
}

int Account::stauts() const
{
	return qxt_d().status;
}

bool Account::autoLogin() const
{
	return qxt_d().autologin;
}

QList<CompanyInfo> Account::companyInfos() const
{
	return qxt_d().companyInfos;
}

bool Account::hasModule(const QString& module)
{
	return qxt_d().modules.contains(module);
}

/// 获取服务参数
base::AddressMap Account::getServiceParameter(const QString& service)
{
	return qxt_d().services.value(service);
}

AccountSettings* Account::settings()
{
	Account* pSelf = Account::instance();
	if (pSelf)
	{
		AccountPrivate& d = pSelf->qxt_d();
		if (!d.settingCreated || d.settings.isNull())
		{
			d.createSettings();
		}
		return d.settings.data();
	}

	return 0;
}

QString Account::homePath() const
{
	return homeDir().absolutePath();
}

QString Account::attachPath() const
{
	return attachDir().absolutePath();
}

QString Account::audioPath() const
{
	return audioDir().absolutePath();
}

QString Account::videoPath() const
{
	return videoDir().absolutePath();
}

QString Account::imagePath() const
{
	return imageDir().absolutePath();
}

QString Account::avatarPath() const
{
	return avatarDir().absolutePath();
}

QString Account::subscriptionPath() const
{
	return subscriptionDir().absolutePath();
}

QString Account::globalNotificationPath() const
{
	return globalNotificationDir().absolutePath();
}

QString Account::cachePath() const
{
	return cacheDir().absolutePath();
}

QString Account::groupPath() const
{
	return groupDir().absolutePath();
}

QString Account::emotionPath() const
{
	return emotionDir().absolutePath();
}

QString Account::thumbPath() const
{
	return thumbDir().absolutePath();
}

QDir Account::homeDir() const
{
	return qxt_d().homeDir();
}

QDir Account::attachDir() const
{
	QString sDir = settings()->getDownloadDir();
	QDir ret(sDir);

	if (sDir.isEmpty() || !ret.exists())
	{
		ret = qxt_d().phoneDir();
		if (!ret.exists(DIR_ATTACH_NAME))
		{
			ret.mkdir(DIR_ATTACH_NAME);
		}
		ret.cd(DIR_ATTACH_NAME);

		return ret;
	}

	return ret;
}

QDir Account::audioDir() const
{
	QDir audio = qxt_d().homeDir();
	if (!audio.exists(DIR_AUDIO_NAME))
	{
		audio.mkdir(DIR_AUDIO_NAME);
	}
	audio.cd(DIR_AUDIO_NAME);
	return audio;
}

QDir Account::videoDir() const
{
	QDir video = qxt_d().homeDir();
	if (!video.exists(DIR_VIDEO_NAME))
	{
		video.mkdir(DIR_VIDEO_NAME);
	}
	video.cd(DIR_VIDEO_NAME);
	return video;
}

QDir Account::imageDir() const
{
	QDir image = qxt_d().homeDir();
	if (!image.exists(DIR_IMAGE_NAME))
	{
		image.mkdir(DIR_IMAGE_NAME);
	}
	image.cd(DIR_IMAGE_NAME);
	return image;
}

QDir Account::avatarDir() const
{
	QDir avatar = qxt_d().homeDir();
	if (!avatar.exists(DIR_AVATAR_NAME))
	{
		avatar.mkdir(DIR_AVATAR_NAME);
	}
	avatar.cd(DIR_AVATAR_NAME);
	return avatar;
}

QDir Account::subscriptionDir() const
{
	QDir subscription = qxt_d().homeDir();
	if (!subscription.exists(DIR_SUBSCRIPTION_NAME))
	{
		subscription.mkdir(DIR_SUBSCRIPTION_NAME);
	}
	subscription.cd(DIR_SUBSCRIPTION_NAME);
	return subscription;
}

QDir Account::globalNotificationDir() const
{
	QDir globalNotification = qxt_d().homeDir();
	if (!globalNotification.exists(DIR_GLOBALNOTIFICATION_NAME))
	{
		globalNotification.mkdir(DIR_GLOBALNOTIFICATION_NAME);
	}
	globalNotification.cd(DIR_GLOBALNOTIFICATION_NAME);
	return globalNotification;
}

QDir Account::cacheDir() const
{
	QDir cache = qxt_d().homeDir();
	if (!cache.exists(DIR_CACHE_NAME))
	{
		cache.mkdir(DIR_CACHE_NAME);
	}
	cache.cd(DIR_CACHE_NAME);
	return cache;
}

QDir Account::groupDir() const
{
	QDir group = qxt_d().homeDir();
	if (!group.exists(DIR_GROUP_NAME))
	{
		group.mkdir(DIR_GROUP_NAME);
	}
	group.cd(DIR_GROUP_NAME);
	return group;
}

QDir Account::emotionDir() const
{
	QDir emotion = qxt_d().homeDir();
	if (!emotion.exists(DIR_EMOTION_NAME))
	{
		emotion.mkdir(DIR_EMOTION_NAME);
	}
	emotion.cd(DIR_EMOTION_NAME);
	return emotion;
}

QDir Account::thumbDir() const
{
	QDir thumb = qxt_d().homeDir();
	if (!thumb.exists(DIR_THUMB_NAME))
	{
		thumb.mkdir(DIR_THUMB_NAME);
	}
	thumb.cd(DIR_THUMB_NAME);
	return thumb;
}

QString Account::userDbFilePath() const
{
	QDir dir = qxt_d().homeDir();
	return dir.absoluteFilePath(USER_DB_FILENAME);
}

QString Account::messageDbFilePath() const
{
	QDir dir = qxt_d().homeDir();
	return dir.absoluteFilePath(MESSAGE_DB_FILENAME);
}

QString Account::subscriptionDbFilePath() const
{
	QDir dir = qxt_d().homeDir();
	return dir.absoluteFilePath(DB_SUBSCRIPTION_DBNAME);
}

QString Account::subscriptionMessagesDbFilePath() const
{
	QDir dir = qxt_d().homeDir();
	return dir.absoluteFilePath(DB_SUBSCRIPTIONMESSAGES_DBNAME);
}

QString Account::globalNotificationDbFilePath() const
{
	QDir dir = qxt_d().homeDir();
	return dir.absoluteFilePath(DB_GLOBALNOTIFICATION_DBNAME);
}

QString Account::globalNotificationMessagesDbFilePath() const
{
	QDir dir = qxt_d().homeDir();
	return dir.absoluteFilePath(DB_GLOBALNOTIFICATIONMESSAGES_DBNAME);
}

DB::UserDB& Account::getUserDBref()
{
	return qxt_d().userDB;
}

void Account::clear()
{
	QXT_D(Account);
	d.clear();
}

bool Account::storeLoginPhone()
{
	QXT_D(Account);
	return d.storeLoginPhone();
}

bool Account::updateLoginPhoneTime()
{
	QXT_D(Account);
	return d.updateLoginPhoneTime();
}

bool Account::clearPassword()
{
	QXT_D(Account);
	return d.clearPassword();
}

bool Account::storeUserIds()
{
	QXT_D(Account);
	return d.storeUserIds();
}

bool Account::setUserIdDefault()
{
	QXT_D(Account);
	return d.setUserIdDefault();
}
