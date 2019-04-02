#include <QStringList>
#include <QStandardPaths>
#include "Constants.h"
#include "cryptsettings.h"
#include "AccountSettings.h"
#include "settings/GlobalSettings.h"
#include <QApplication>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN

static const char *USER_CONFIG_FILENAME = "usersettings.conf";

AccountSettings::AccountSettings(const QDir &homeDir)
{
	QString file = homeDir.absoluteFilePath(USER_CONFIG_FILENAME);
	settings.reset(new QSettings(file, CryptSettings::format()));

	m_defaultScreenshotKey = "Ctrl+Shift+C";
	m_defaultTakeMsgKey    = "Ctrl+Shift+Z";

	// init with default settings
	QStringList allKeys = settings->allKeys();
	if (allKeys.isEmpty())
	{
		writeDefaultSettings();
	}

	if (!allKeys.contains("appinfos/size"))
	{
#ifdef Q_OS_WIN
		char buffer[256];
		GetSystemDirectoryA(buffer, 256);
		QString systemDir = QString::fromLocal8Bit(buffer);
		QList<AccountSettings::AppInfo> infos;
		infos << AppInfo(QObject::tr("Remote desktop"), systemDir + QString::fromLatin1("\\mstsc.exe"))
			<< AppInfo(QObject::tr("Calculate"), systemDir + QString::fromLatin1("\\calc.exe"))
			<< AppInfo(QObject::tr("Paint"), systemDir + QString::fromLatin1("\\mspaint.exe"))
			<< AppInfo(QObject::tr("Notebook"), systemDir + QString::fromLatin1("\\notepad.exe"));
		setAppInfos(infos);
#endif // Q_OS_WIN
	}
}

AccountSettings::~AccountSettings()
{
}

bool AccountSettings::getMute() const
{
	return settings->value("system/mute", false).toBool();
}

void AccountSettings::setMute(bool mute)
{
	settings->setValue("system/mute", mute);
}

int AccountSettings::getPscX() const
{
	return settings->value("pscdlg/x", 724).toInt();
}

void AccountSettings::setPscX(int x)
{
	settings->setValue("pscdlg/x", x);
}

int AccountSettings::getPscY() const
{
	return settings->value("pscdlg/y", 24).toInt();
}

void AccountSettings::setPscY(int y)
{
	settings->setValue("pscdlg/y", y);
}

int AccountSettings::getPscWidth() const
{
	return settings->value("pscdlg/width", kDefaultPscWidth).toInt();
}

void AccountSettings::setPscWidth(int width)
{
	settings->setValue("pscdlg/width", width);
}

int AccountSettings::getPscHeight() const
{
	return settings->value("pscdlg/height", kDefaultPscHeight).toInt();
}

void AccountSettings::setPscHeight(int height)
{
	settings->setValue("pscdlg/height", height);
}

bool AccountSettings::isPscCloseHide() const
{
	int closeType = settings->value("pscdlg/closetype", 0).toInt();
	if (closeType == 0)
		return true;
	else
		return false;
}

void AccountSettings::setPscCloseHide(bool hide)
{
	settings->setValue("pscdlg/closetype", hide ? 0 : 1);
}

bool AccountSettings::isPscTopmost() const
{
	int topmost = settings->value("pscdlg/topmost", 0).toInt();
	if (topmost == 0)
		return false;
	else
		return true;
}

void AccountSettings::setPscTopmost(bool topmost)
{
	settings->setValue("pscdlg/topmost", topmost ? 1 : 0);
}

bool AccountSettings::isPscEdgeHide() const
{
	int edgeHide = settings->value("pscdlg/edgehide", 1).toInt();
	if (edgeHide == 0)
		return false;
	else
		return true;
}

void AccountSettings::setPscEdgeHide(bool edgeHide)
{
	settings->setValue("pscdlg/edgehide", edgeHide ? 1 : 0);
}

int AccountSettings::getSendType() const
{
	return settings->value("shortcut/sendtype", 0).toInt();
}

void AccountSettings::setSendType(int sendType)
{
	settings->setValue("shortcut/sendtype", sendType);
}

QString AccountSettings::getScreenshotKey() const
{
	return settings->value("shortcut/screenshot", m_defaultScreenshotKey).toString();
}

void AccountSettings::setScreenshotKey(const QString &key)
{
	settings->setValue("shortcut/screenshot", key);
}

bool AccountSettings::hideToScreenshot() const
{
	return settings->value("shortcut/hidewhenscreenshot", false).toBool();
}

void AccountSettings::setHideToScreenshot(bool hide)
{
	settings->setValue("shortcut/hidewhenscreenshot", hide);
}

QString AccountSettings::getTakeMsgKey() const
{
	return settings->value("shortcut/takemsg", m_defaultTakeMsgKey).toString();
}

void AccountSettings::setTakeMsgKey(const QString &key)
{
	settings->setValue("shortcut/takemsg", key);
}

void AccountSettings::setMessagePromptMute(bool muteOn)
{
	settings->setValue("sound/muteon", muteOn);
}

bool AccountSettings::messagePromptMute() const
{
	return settings->value("sound/muteon", false).toBool();
}

void AccountSettings::setBuddyMsgMuteOn(bool muteOn)
{
	settings->setValue("sound/buddymsgmuteon", muteOn);
}

bool AccountSettings::buddyMsgMuteOn() const
{
	return settings->value("sound/buddymsgmuteon", false).toBool();
} 

void AccountSettings::setSubscriptionMsgMuteOn(bool muteOn)
{
	settings->setValue("sound/subscriptionmsgmuteon", muteOn);
}

bool AccountSettings::subscriptionMsgMuteOn() const
{
	return settings->value("sound/subscriptionmsgmuteon", false).toBool();
}

void AccountSettings::setBuddyMsgPromptFile(QString &file)
{
	settings->setValue("beep/recvbuddy", file);
}

QString AccountSettings::buddyMsgPromptFile() const
{
	QString file = GlobalSettings::getBeepRecv();
	QDir appDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(appDir, file);
	file = fi.absoluteFilePath();
	file = QDir::toNativeSeparators(file);
	return settings->value("beep/recvbuddy", file).toString();
}

void AccountSettings::setSubscriptionMsgPromptFile(QString &file)
{
	settings->setValue("beep/recvsubscription", file);
}

QString AccountSettings::subscriptionMsgPromptFile() const
{
	QString file = GlobalSettings::getBeepRecv();
	QDir appDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(appDir, file);
	file = fi.absoluteFilePath();
	file = QDir::toNativeSeparators(file);
	return settings->value("beep/recvsubscription", file).toString();
}

QString AccountSettings::getCurDir() const
{
	return settings->value("filedialog/curdir", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
}

void AccountSettings::setCurDir(const QString &curDir)
{
	settings->setValue("filedialog/curdir", curDir);
}

QString AccountSettings::getDownloadDir() const
{
	return settings->value("filedialog/downloaddir", "").toString();
}

void AccountSettings::setDownloadDir(const QString &downloadDir)
{
	settings->setValue("filedialog/downloaddir", downloadDir);
}

void AccountSettings::writeDefaultSettings()
{
	settings->beginGroup("system");
	settings->setValue("mute", false);
	settings->endGroup();

	settings->beginGroup("pscdlg");
	settings->setValue("x", 724);
	settings->setValue("y", 24);
	settings->setValue("closetype", 0);
	settings->setValue("topmost", 0);
	settings->setValue("edgehide", 1);
	settings->endGroup();

	settings->beginGroup("shortcut");
	settings->setValue("sendtype", 0);
	settings->setValue("screenshot", m_defaultScreenshotKey);
	settings->setValue("takemsg", m_defaultTakeMsgKey);
	settings->setValue("hidewhenscreenshot", false);
	settings->endGroup();

	settings->beginGroup("filedialog");
	settings->setValue("curdir", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
	settings->setValue("downloaddir", "");
	settings->endGroup();

	settings->beginGroup("message");
	settings->setValue("max-ts", "");
	settings->setValue("clearwhenclose", false);
	settings->setValue("closeallchat", false);
	settings->setValue("loadhistory", false);
	settings->setValue("unreadboxautoshow", false);
	settings->setValue("withdraw-id", "");
	settings->endGroup();

	settings->beginGroup("version");
	settings->setValue("roster", "");
	settings->endGroup();

	settings->beginGroup("subscription");
	settings->setValue("lastsequence", 0);
	settings->endGroup();

	settings->beginGroup("roster");
	settings->setValue("avatartype", (int)AccountSettings::UnknownAvatar);
	settings->endGroup();
}

bool AccountSettings::isAutoRun() const
{
	QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
	return reg.contains(qApp->applicationName());
}

void AccountSettings::setAutoRun(bool autoRun)
{
	QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
	if (autoRun)
	{
		QString appFilePath = qApp->applicationDirPath() + "/" + APP_DEAMON_EXE_NAME;
		QVariant autoRunValue = reg.value(qApp->applicationName());
		if (autoRunValue.isValid())
		{
			QString autoRunStr = autoRunValue.toString();
			if (autoRunStr == QDir::toNativeSeparators(appFilePath))
			{
				return;
			}
		}

		reg.setValue(qApp->applicationName(), QDir::toNativeSeparators(appFilePath));
	}
	else
	{
		if (reg.contains(qApp->applicationName()))
		{
			reg.remove(qApp->applicationName());
		}
	}
}

QString AccountSettings::defaultScreenshotKey() const
{
	return m_defaultScreenshotKey;
}

QString AccountSettings::defaultTakeMsgKey() const
{
	return m_defaultTakeMsgKey;
}

QStringList AccountSettings::blackListIds() const
{
	return settings->value("blacklist/ids", QStringList()).toStringList();
}

void AccountSettings::setBlackListIds(const QStringList &ids)
{
	settings->setValue("blacklist/ids", ids);
}

QStringList AccountSettings::silenceList() const
{
	return settings->value("silencelist", QStringList()).toStringList();
}

void AccountSettings::setSilenceList(const QStringList &silenceList)
{
	settings->setValue("silencelist", silenceList);
}

quint64 AccountSettings::subscriptionLastSequence() const
{
	return settings->value("subscription/lastsequence", 0).toULongLong();
}

void AccountSettings::setSubscriptionLastSequence(quint64 sequence)
{
	settings->setValue("subscription/lastsequence", sequence);
}

quint64 AccountSettings::globalNotificationLastSequence() const
{
	return settings->value("globalnotification/lastsequence", 0).toULongLong();
}

void AccountSettings::setGlobalNotificationLastSequence(quint64 sequence)
{
	settings->setValue("globalnotification/lastsequence", sequence);
}

QList<AccountSettings::AppInfo> AccountSettings::appInfos() const
{
	QList<AccountSettings::AppInfo> infos;
	int size = settings->beginReadArray("appinfos");
	for (int i = 0; i < size; ++i)
	{
		settings->setArrayIndex(i);
		AccountSettings::AppInfo info;
		info.name = settings->value("name").toString();
		info.path = settings->value("path").toString();
		info.type = settings->value("type").toInt();
		infos.append(info);
	}
	settings->endArray();
	return infos;
}

void AccountSettings::setAppInfos(const QList<AccountSettings::AppInfo> &infos)
{
	settings->beginWriteArray("appinfos");
	for (int i = 0; i < infos.length(); ++i)
	{
		settings->setArrayIndex(i);
		settings->setValue("name", infos.at(i).name);
		settings->setValue("path", infos.at(i).path);
		settings->setValue("type", infos.at(i).type);
	}
	settings->endArray();
}

QString AccountSettings::subscriptionAttachDir() const
{
	QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	defaultDir = QDir::toNativeSeparators(defaultDir);
	return settings->value("subscription/attachdir", defaultDir).toString();
}

void AccountSettings::setSubscriptionAttachDir(const QString &saveDir)
{
	settings->setValue("subscription/attachdir", saveDir);
}

QString AccountSettings::maxMsgTs() const
{
	return settings->value("message/max-ts", QString()).toString();
}

void AccountSettings::setMaxMsgTs(const QString &ts)
{
	settings->setValue("message/max-ts", ts);
}

QString AccountSettings::lastWithdrawId() const
{
	return settings->value("message/withdraw-id", QString()).toString();
}

void AccountSettings::setLastWithdrawId(const QString &withdrawId)
{
	settings->setValue("message/withdraw-id", withdrawId);
}

bool AccountSettings::clearMsgWhenClose() const
{
	return settings->value("message/clearwhenclose", false).toBool();
}

void AccountSettings::setClearMsgWhenClose(bool clear)
{
	settings->setValue("message/clearwhenclose", clear);
}

bool AccountSettings::chatAlwaysCloseAll() const
{
	return settings->value("message/closeallchat", false).toBool();
}

void AccountSettings::setChatAlwaysCloseAll(bool closeAll)
{
	settings->setValue("message/closeallchat", closeAll);
}

bool AccountSettings::chatLoadHistory() const
{
	return settings->value("message/loadhistory", false).toBool();
}

void AccountSettings::setChatLoadHistory(bool load)
{
	settings->setValue("message/loadhistory", load);
}

AccountSettings::RosterAvatarType AccountSettings::rosterAvatarType() const
{
	return (AccountSettings::RosterAvatarType)settings->value("roster/avatartype", (int)AccountSettings::UnknownAvatar).toInt();
}

void AccountSettings::setRosterAvatarType(RosterAvatarType rosterAvatar)
{
	settings->setValue("roster/avatartype", (int)rosterAvatar);
}

bool AccountSettings::changePwdNotRemind() const
{
	return settings->value("password/changenotremind", false).toBool();
}

void AccountSettings::setChangePwdNotRemind(bool notRemind)
{
	settings->setValue("password/changenotremind", notRemind);
}

AccountSettings::GroupMsgSettingType AccountSettings::groupMsgSetting(const QString &id) const
{
	return groupTipSetting("group", id);
}

void AccountSettings::setGroupMsgSetting(const QString &id, GroupMsgSettingType setting)
{
	setGroupTipSetting("group", id, setting);
}

AccountSettings::GroupMsgSettingType AccountSettings::groupTipSetting(const QString &groupType, const QString &id) const
{
	if (groupType.isEmpty() || id.isEmpty())
	{
		return Tip;
	}

	QStringList curSilenceList = silenceList();
	if (curSilenceList.isEmpty())
	{
		return Tip;
	}

	foreach (QString silence, curSilenceList)
	{
		QStringList silenceParts = silence.split(":");
		if (silenceParts.count() == 3 && silenceParts[0] == groupType && silenceParts[1] == id)
		{
			if ("1" == silenceParts[2])
			{
				return UnTip;
			}
			else
			{
				return Tip;
			}
		}
	}

	return Tip;
}

void AccountSettings::setGroupTipSetting(const QString &groupType, const QString &id, GroupMsgSettingType setting)
{
	if (groupType.isEmpty() || id.isEmpty())
	{
		return;
	}

	QStringList sl = silenceList();
	QStringList silenceParts;
	QStringList drops;

	foreach(QString silence, sl)
	{
		silenceParts = silence.split(":");
		if (silenceParts.count() != 3)
		{
			 // delete unnormal silence
			drops.append(silence);
			continue;
		}
		if (silence.contains(id) && silenceParts[0] == groupType)
		{
			drops.append(silence);
		}
	}

	foreach (QString drop, drops)
	{
		sl.removeOne(drop);
	}

	QString newSilence = groupType + QString(":") + id + QString(":") + QString::number((int)setting - 1);
	sl.append(newSilence);
	setSilenceList(sl);
}

AccountSettings::GroupMsgSettingType AccountSettings::discussMsgSetting(const QString &id) const
{
	return groupTipSetting("discuss", id);
}

void AccountSettings::setDiscussMsgSetting(const QString &id, GroupMsgSettingType setting)
{
	setGroupTipSetting("discuss", id, setting);
}

QString AccountSettings::rosterVersion() const
{
	return settings->value("version/roster", QString()).toString();
}

void AccountSettings::setRosterVersion(const QString &version)
{
	settings->setValue("version/roster", version);
}

int AccountSettings::groupLogoVersion(const QString &gid) const
{
	QString key = QString("grouplogoversion/%1").arg(gid);
	return settings->value(key, -1).toInt();
}

void AccountSettings::setGroupLogoVersion(const QString &gid, int version)
{
	QString key = QString("grouplogoversion/%1").arg(gid);
	settings->setValue(key, version);
}

QMap<QString, int> AccountSettings::allGroupLogoVersions()
{
	QMap<QString, int> logoVersions;
	settings->beginGroup("grouplogoversion");
	QStringList gids = settings->childKeys();
	foreach (QString gid, gids)
	{
		int logoVersion = settings->value(gid, -1).toInt();
		logoVersions.insert(gid, logoVersion);
	}
	settings->endGroup();
	return logoVersions;
}

bool AccountSettings::unreadBoxAutoShow() const
{
	return settings->value("message/unreadboxautoshow", false).toBool();
}

void AccountSettings::setUnreadBoxAutoShow(bool autoShow)
{
	settings->setValue("message/unreadboxautoshow", autoShow);
}