#ifndef _ACCOUNTSETTINGS_H_
#define _ACCOUNTSETTINGS_H_

#include <QScopedPointer>
#include <QSettings>
#include <QDir>
#include <QColor>
#include <QList>
#include <QMap>

class AccountSettings
{
public:
	class AppInfo
	{
	public:
		AppInfo() {type = 0;}
		AppInfo(const QString &name, const QString &path, int type = 0) {this->name = name; this->path = path; this->type = type;}
	public:
		QString name;
		QString path;
		int     type; // 0 -- app; 1 -- web
	};

	enum RosterAvatarType
	{
		UnknownAvatar = 0,
		BigAvatar = 1,
		SmallAvatar = 2
	};

	enum GroupMsgSettingType
	{
		Tip = 1,
		UnTip = 2
	};

	static const int kDefaultPscWidth  = 288;
	static const int kDefaultPscHeight = 577;

public:
	AccountSettings(const QDir &homeDir);
	~AccountSettings();

public:
	bool getMute() const;
	void setMute(bool mute);

	int getPscX() const;
	void setPscX(int x);

	int getPscY() const;
	void setPscY(int y);

	int getPscWidth() const;
	void setPscWidth(int width);

	int getPscHeight() const;
	void setPscHeight(int height);

	bool isPscCloseHide() const;
	void setPscCloseHide(bool hide);

	bool isPscTopmost() const;
	void setPscTopmost(bool topmost);

	bool isPscEdgeHide() const;
	void setPscEdgeHide(bool edgeHide);

	int getSendType() const;
	void setSendType(int sendType);

	QString getScreenshotKey() const;
	void setScreenshotKey(const QString &key);

	bool hideToScreenshot() const;
	void setHideToScreenshot(bool hide);

	QString getTakeMsgKey() const;
	void setTakeMsgKey(const QString &key);

	void setMessagePromptMute(bool muteOn);
	bool messagePromptMute() const;

	void setBuddyMsgMuteOn(bool muteOn);
	bool buddyMsgMuteOn() const;

	void setSubscriptionMsgMuteOn(bool muteOn);
	bool subscriptionMsgMuteOn() const;

	void setBuddyMsgPromptFile(QString &file);
	QString buddyMsgPromptFile() const;

	void setSubscriptionMsgPromptFile(QString &file);
	QString subscriptionMsgPromptFile() const;

	QString getCurDir() const;
	void setCurDir(const QString &curDir);

	QString getDownloadDir() const;
	void setDownloadDir(const QString &downloadDir);

	bool isAutoRun() const;
	void setAutoRun(bool autoRun);

    QString audioBeepPath() const;
    void setAudioBeepPath(const QString &path);

	QString maxMsgTs() const;
	void setMaxMsgTs(const QString &ts);

	QString lastWithdrawId() const;
	void setLastWithdrawId(const QString &withdrawId);

	bool clearMsgWhenClose() const;
	void setClearMsgWhenClose(bool clear);

	GroupMsgSettingType groupMsgSetting(const QString &id) const;
	void setGroupMsgSetting(const QString &id, GroupMsgSettingType setting);

	GroupMsgSettingType discussMsgSetting(const QString &id) const;
	void setDiscussMsgSetting(const QString &id, GroupMsgSettingType setting);

	QString rosterVersion() const;
	void setRosterVersion(const QString &version);

	QString defaultScreenshotKey() const;

	QString defaultTakeMsgKey() const;

	QStringList blackListIds() const;
	void setBlackListIds(const QStringList &ids);

	QStringList silenceList() const;
	void setSilenceList(const QStringList &silenceList);

	GroupMsgSettingType groupTipSetting(const QString &groupType, const QString &id) const;
	void setGroupTipSetting(const QString &groupType, const QString &id, GroupMsgSettingType setting);

	quint64 subscriptionLastSequence() const;
	void setSubscriptionLastSequence(quint64 sequence);

	quint64 globalNotificationLastSequence() const;
	void setGlobalNotificationLastSequence(quint64 sequence);

	QList<AccountSettings::AppInfo> appInfos() const;
	void setAppInfos(const QList<AccountSettings::AppInfo> &infos);

	QString subscriptionAttachDir() const;
	void setSubscriptionAttachDir(const QString &saveDir);

	bool chatAlwaysCloseAll() const;
	void setChatAlwaysCloseAll(bool closeAll);

	bool chatLoadHistory() const;
	void setChatLoadHistory(bool load);

	RosterAvatarType rosterAvatarType() const;
	void setRosterAvatarType(RosterAvatarType rosterAvatar);

	bool changePwdNotRemind() const;
	void setChangePwdNotRemind(bool notRemind);

	int groupLogoVersion(const QString &gid) const;
	void setGroupLogoVersion(const QString &gid, int version);
	QMap<QString, int> allGroupLogoVersions();

	bool unreadBoxAutoShow() const;
	void setUnreadBoxAutoShow(bool autoShow);

private:
	void writeDefaultSettings();

private:
	QScopedPointer<QSettings> settings;
	QString                   m_defaultScreenshotKey;
	QString                   m_defaultTakeMsgKey;
};

#endif //ACCOUNTSETTINGS