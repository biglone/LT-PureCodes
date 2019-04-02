#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QDir>
#include "qxtglobal.h"

#include "settings/AccountSettings.h"
#include "base/Base.h"

extern const char DIR_ATTACH_NAME[];
extern const char DIR_AUDIO_NAME[];
extern const char DIR_VIDEO_NAME[];
extern const char DIR_IMAGE_NAME[];
extern const char DIR_AVATAR_NAME[];
extern const char DIR_SUBSCRIPTION_NAME[];
extern const char DIR_CACHE_NAME[];
extern const char DIR_GROUP_NAME[];
extern const char DIR_EMOTION_NAME[];
extern const char DIR_THUMB_NAME[];

extern const char *RESOURCE_COMPUTER;
extern const char *RESOURCE_PHONE;

extern const char USER_DB_FILENAME[];
extern const char MESSAGE_DB_FILENAME[];

extern const char DB_SUBSCRIPTION_DBNAME[];
extern const char DB_SUBSCRIPTIONMESSAGES_DBNAME[];

namespace DB
{
	class UserDB;
}

class CompanyInfo
{
public:
	QString                uid;
	QString                uname;
	QString                companyId;
	QString                companyName;
	int                    userState;
	bool                   frozen;
	QMap<QString, QString> settings;

	CompanyInfo() : uid(), uname(), companyId(), companyName(), userState(0), frozen(false) {}
};

class AccountPrivate;

class Account : public QObject
{
	Q_OBJECT
	QXT_DECLARE_PRIVATE(Account)

public:
	static Account* instance();

private:
	static Account* self;

public:
	Account();
	~Account();

public:
	void setCountryCode(const QString &countryCode);
	void setLoginPhone(const QString &loginPhone);
	void setId(const QString& id);
	void setPwd(const QString& pwd);
	void setCryptoPwd(const QString &cryptoPwd);
	void setResource(const QString& resource);
	void setVoilent(bool voilent);
	void setStatus(int status);
	void setSavePwd(bool savePwd);
	void setAutologin(bool autologin);
	void setCompanyInfos(const QList<CompanyInfo> &companyInfos);

	void setLogined();

public:
	/// 获取账号
	QString countryCode() const;
	QString loginPhone() const;
	QString id() const;
	QString fullId() const;
	QString computerFullId() const;
	QString phoneFullId() const;

	static QString idFromFullId(const QString &fullId);
	static QString resourceFromFullId(const QString &fullId);
	static QString fullIdFromIdResource(const QString &id, const QString &resource);

	static QString computerName();
	static QString phoneName();

	QString name() const;

	QString companyId() const;

	QString companyName() const;

	QString pwd() const;

	QString cryptoPwd() const;

	QString resource() const;

	QString platform() const;

	bool voilent() const;

	int stauts() const;

	bool autoLogin() const;

	QList<CompanyInfo> companyInfos() const;

	/// 判断模块ID是否存在
	bool hasModule(const QString& module);

	/// 获取服务参数
	base::AddressMap getServiceParameter(const QString& service);

	static AccountSettings* settings();

	/// store path
	QString homePath() const;
	QString attachPath() const;
	QString audioPath() const;
	QString videoPath() const;
	QString imagePath() const;
	QString avatarPath() const;
	QString subscriptionPath() const;
	QString globalNotificationPath() const;
	QString cachePath() const;
	QString groupPath() const;
	QString emotionPath() const;
	QString thumbPath() const;
	
	QDir    homeDir() const;
	QDir    attachDir() const;
	QDir    audioDir() const;
	QDir    videoDir() const;
	QDir    imageDir() const;
	QDir    avatarDir() const;
	QDir    subscriptionDir() const;
	QDir	globalNotificationDir() const;
	QDir    cacheDir() const;
	QDir    groupDir() const;
	QDir    emotionDir() const;
	QDir    thumbDir() const;

	QString userDbFilePath() const;
	QString messageDbFilePath() const;

	QString subscriptionDbFilePath() const;
	QString subscriptionMessagesDbFilePath() const;

	QString globalNotificationDbFilePath() const;
	QString globalNotificationMessagesDbFilePath() const;

	DB::UserDB& getUserDBref();

public slots:
	void setModules(const QStringList& modules);
	void setServiceParameter(const QMap<QString, base::AddressMap>& services);

	void clear();
	bool storeLoginPhone();
	bool updateLoginPhoneTime();
	bool clearPassword();

	bool storeUserIds();
	bool setUserIdDefault();
};
#endif // _ACCOUNT_H_