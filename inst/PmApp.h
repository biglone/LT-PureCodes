#ifndef PMAPP_H
#define PMAPP_H

#include <string>

#include <QApplication>
#include <QScopedPointer>
#include <QPointer>

#include "loginmgr.h"
#include "systemtray.h"

class PmApp;

#if defined(qPmApp)
#undef qPmApp
#endif
#define qPmApp PmApp::instance()

class CAttachTransferMgr;

class PmClient;
class BaseProcessor;
class MessageProcessor;

class CBuddyMgr;
class CDebugDlg;
class CRosterModel;
class CLastContactModel;
class CGroupMgr;
class StatusChanger;
class ModelManager;
class CAutoStatus;
class Account;
class QSettings;
class BackgroundDlg;
class Logger;
class PresenceManager;
class DetailPhotoManager;
class UnreadMsgModel;
class MessageDBStore;
class LocalClient;
class LocalCommMessage;
class OfflineMsgManager;
class RosterManager;
class OrganizationManager;
class GroupManager;
class DiscussManager;
class ConfigManager;
class SendMessageManager;
class MaxTsManager;
class SessionVideoManager;
class CommonConfigManager;
class UpdateManager;
class ChangeNoticeMgr;
class SearchManager;
class AddFriendManager;
class AmrRecord;
class TipManager;
class InterphoneManager;
class SubscriptionManager;
class SubscriptionMsgManager;
class SubscriptionDetailAndHistoryManager;
class GlobalNotificationManager;
class GlobalNotificationMsgManager;
class GlobalNotificationDetailAndHistoryManager;
class SecretManager;
class HttpPool;
class OnlineReportManager;
class MessageWithdrawManager;
class GroupsMemberManager;
class PMImageViewer;
class CompanyLoginManager;
class RoamingMsgManager;
class PasswdModifyManager;
class IOSPushManager;
class CompanyRegisterManager;

namespace rtcsession { class RtcSessionManager; }

enum ShutdownKind {
	SK_QUIT,
	SK_WORK,
	SK_RESTART
};

enum LogoutMode {
	MODE_LOGOUT_COMPANY,
	MODE_SWITCH_COMPANY
};

class PmApp : public QObject
{
	Q_OBJECT
public:
	PmApp(QApplication* app);
	~PmApp();

public:
	static PmApp* instance() { return self; }

	bool connectToManager();

	void construct();

	Logger* getLogger() const;

	CDebugDlg* getDebugDlg() const;

	StatusChanger* getStatusChanger() const;

	PmClient* getPmClient() const;

	BaseProcessor* getBaseProcessor() const;

	MessageProcessor* getMessageProcessor() const;

	CSystemTray* getSystemTray() const;

	CLastContactModel* getLastContactModel() const;

	ModelManager* getModelManager() const;

	CBuddyMgr* getBuddyMgr() const;

	CLoginMgr* GetLoginMgr() { return m_pLoginMgr; }

	CAttachTransferMgr& rAttachMgr() { return *(m_pAttachMgr.data()); }

	Account* getAccount() const;

	PresenceManager* getPresenceManager() const;

	DetailPhotoManager* getDetailPhotoManager() const;

	UnreadMsgModel* getUnreadMsgModel() const;

    rtcsession::RtcSessionManager* getRtcSessionManager() const;

	MessageDBStore* getMessageDBStore() const;

	OfflineMsgManager* getOfflineMsgManager() const;

	MaxTsManager* getMaxTsManager() const;

	OrganizationManager* getOrganizationManager() const;
	RosterManager* getRosterManager() const;

	GroupManager* getGroupManager() const;

	DiscussManager* getDiscussManager() const;

	ConfigManager* getConfigManager() const;

	SendMessageManager* getSendMessageManager() const;

	SessionVideoManager* getSessionVideoManager() const;

	CommonConfigManager* getCommonConfigManager() const;

	UpdateManager* getUpdateManager() const;

	ChangeNoticeMgr* getChangeNoticeMgr() const;

	SearchManager* getSearchManager() const;

	AddFriendManager* getAddFriendManager() const;

	AmrRecord* getAmrRecord() const;

	TipManager* getTipManager() const;

	InterphoneManager* getInterphoneManager() const;

	SubscriptionManager* getSubscriptionManager() const;

	SubscriptionMsgManager *getSubscriptionMsgManager() const;

	SubscriptionDetailAndHistoryManager *getSubscriptionDetailAndHistoryManager() const;

	GlobalNotificationManager* getGlobalNotificationManager() const;

	GlobalNotificationMsgManager *getGlobalNotificationMsgManager() const;

	GlobalNotificationDetailAndHistoryManager *getGlobalNotificationDetailAndHistoryManager() const;

	SecretManager *getSecretManager() const;

	HttpPool *getHttpPool() const;

	OnlineReportManager *getOnlineReportManager() const;

	MessageWithdrawManager *getMessageWithdrawManager() const;

	CompanyLoginManager *getCompanyLoginManager() const;

	RoamingMsgManager *getRoamingMsgManager() const;

	PasswdModifyManager *getPasswdModifyManager() const;

	GroupsMemberManager *getGroupsMemberManager() const;

	IOSPushManager *getIOSPushManager() const;

	CompanyRegisterManager *getCompanyRegisterManager() const;

	void setMaxMsgTs(const QString &ts);

	QIcon appDefaultIcon() const;

	qreal dpiScale() const;
	int scale2DevPos(int visualPos) const;
	int scale2VisualPos(int devPos) const;

public:
	bool Initialize();
	void Release();

public:
	bool isShutingDown() const;
	
	bool isLogout() const;

	void setLogout(bool bLogout);

	LogoutMode logoutMode() const;

	void setLogoutMode(LogoutMode mode);

	void setSwitchId(const QString &uid);

	void closeSystemTray();

	bool hasVideoSession() const;

	bool hasSession() const;

	void setTakeSelfPhoto(bool photo);

	bool isTakeSelfPhoto() const;

Q_SIGNALS:
	void prepareForQuit();

	void addFlickering(const QString &id, bean::MessageType msgType);
	void removeFlickering(const QString &id, bean::MessageType msgType);
	void clearFlickering();

	void userLoginCheckFinished(bool logined);
	void updateCheckFinished(bool update);

	void shortcutKeyChanged();

public slots:
	void quit();
	void restart();
	void delayShutdown();
	void continueShutdown();

	void checkIfUserLogined(const QString &id);
	void notifyUserLogined(const QString &id);
	void notifyUserLogouted(const QString &id);

	void notifyUpdating(bool update);
	void checkIfUpdating();

	void slot_takeMsg();
	void slot_openMainWindow();

	void openImages(const QStringList &imagePathes, const QString &imagePath);

	void setLanguageEnglish();
	void setLanguageChinese();

private slots:
	void onShutdownTimerTimeout();
	void onApplicationAboutToQuit();
	void slot_loginValidated();
	void slot_logined();
	void slot_loginError();
	void slot_logouted();
	void slot_aboutLogout();
	void slot_readsyncDetail();

	void slot_initSession();

	void slot_systemTrayActivated(QSystemTrayIcon::ActivationReason reason);

	void slot_lastMsgChanged(const QString &id, bean::MessageType msgType);

	void onMessageReceived(const LocalCommMessage &msg);
	void onSessionConnected();
	void onSessionDisconnected();

	void checkChatDlgAndUnreadMsgValid();

	void onContactDetailChanged(const QString &id);

private:
	/// 处理启动参数
	bool ParseRunCommand();

private:
	bool InitDebugDlg();

	bool InitSystemTray();
	void UninitSystemTray();

	bool InitModel();
	void UninitModel();

	//inline void AdjustAddress();

	void closeTopWidget();

	void startClose();
	void finishClose();

	void startQuit();
	void finishQuit();

	void handleApplicationMessage(const LocalCommMessage &msg);
	void handleOcxMessage(const LocalCommMessage &msg);

	void initApp();
	void initTranslator();
	void initQss();

	void initMaxMsgTs();

	void initHandleObjects();
	void removeHandleObjects();

	void checkToRequestGroupLogoes();

	void initDateTime();

	void initSslConfiguration();

	void initDpiScale();

private:

	static PmApp*               self;

	LocalClient                *m_localClient;

	bool                        m_bStartClose;
	bool                        m_bStartQuit;
	bool                        m_bQuitReady;

	bool                        m_bLogout;
	LogoutMode                  m_logoutMode;
	QString                     m_switchId;

	int                         m_nShutdownKind;
	int                         m_nShutdownDelayCount;

	QTimer                      m_ShutdownTimer;

	bool                        m_takeSelfPhoto;

	QString                     m_offlineTs;

	QString                     m_lastWithdrawId; 

	QPointer<PMImageViewer>     m_imageViewer;

	CLoginMgr                  *m_pLoginMgr;

	qreal                       m_dpiScale;

	QList<QTranslator *>        m_translators;

	QScopedPointer<CBuddyMgr>                  m_pBuddyMgr;

	QScopedPointer<CSystemTray>                m_pTray;                   /// 系统托盘

	QScopedPointer<CAttachTransferMgr>         m_pAttachMgr;

	QScopedPointer<PmClient>                   m_pPmClient;

	QScopedPointer<BaseProcessor>              m_pBaseProcessor;

	QScopedPointer<MessageProcessor>           m_pMessageProcessor;

	QScopedPointer<StatusChanger>              m_pStatusChanger;

	QScopedPointer<ModelManager>               m_pModelManager;

	QScopedPointer<CAutoStatus>                m_pAutoStatus;

	QScopedPointer<Account>                    m_pAccount;

	QScopedPointer<BackgroundDlg>              m_pBackgroundDlg;

	QScopedPointer<PresenceManager>            m_pPresenceManager;

	QScopedPointer<DetailPhotoManager>         m_pDetailPhotoManager;

	QScopedPointer<UnreadMsgModel>             m_pUnreadMsgModel;

    QScopedPointer<rtcsession::RtcSessionManager> m_pRtcSessionManager;

	QScopedPointer<MessageDBStore>             m_pMessageDBStore;

	QScopedPointer<OfflineMsgManager>          m_pOfflineMsgManager;

	QScopedPointer<MaxTsManager>               m_pMaxTsManager;

	QScopedPointer<OrganizationManager>        m_pOrganizationManager;

	QScopedPointer<RosterManager>              m_pRosterManager;

	QScopedPointer<GroupManager>               m_pGroupManager;

	QScopedPointer<DiscussManager>             m_pDiscussManager;

	QScopedPointer<ConfigManager>              m_pConfigManager;
	
	QScopedPointer<SendMessageManager>         m_pSendMessageManager;

	QScopedPointer<SessionVideoManager>        m_pSessionVideoManager;

	QScopedPointer<CommonConfigManager>        m_pCommonConfigManager;

	QScopedPointer<UpdateManager>              m_pUpdateManager;

	QScopedPointer<ChangeNoticeMgr>            m_pChangeNoticeManager;

	QScopedPointer<SearchManager>              m_pSearchManager;

	QScopedPointer<AddFriendManager>           m_pAddFriendManager;

	QScopedPointer<AmrRecord>                  m_pAmrRecord;

	QScopedPointer<TipManager>                 m_pTipManager;

	QScopedPointer<InterphoneManager>          m_pInterphoneManager;

	QScopedPointer<SubscriptionManager>        m_pSubscriptionManager;

	QScopedPointer<SubscriptionMsgManager>     m_pSubscriptionMsgManager;

	QScopedPointer<SubscriptionDetailAndHistoryManager> m_pSubscriptionDetailAndHistoryManager;

	QScopedPointer<GlobalNotificationManager>  m_pGlobalNotificationManager;

	QScopedPointer<GlobalNotificationMsgManager>     m_pGlobalNotificationMsgManager;

	QScopedPointer<GlobalNotificationDetailAndHistoryManager> m_pGlobalNotificationDetailAndHistoryManager;

	QScopedPointer<SecretManager>              m_pSecretManager;

	QScopedPointer<HttpPool>                   m_pHttpPool;

	QScopedPointer<OnlineReportManager>        m_pOnlineReportManager;

	QScopedPointer<MessageWithdrawManager>     m_pMessageWithdrawManager;

	QScopedPointer<CompanyLoginManager>        m_pCompanyLoginManager;

	QScopedPointer<RoamingMsgManager>          m_pRoamingMsgManager;

	QScopedPointer<PasswdModifyManager>        m_pPasswdModifyManager;

	QScopedPointer<GroupsMemberManager>        m_pGroupsMemberManager;

	QScopedPointer<IOSPushManager>			   m_pIOSPushManager;

	QScopedPointer<CompanyRegisterManager>     m_pCompanyRegisterManager;
 
	friend class BackgroundDlg;
};

#endif // PMAPP_H
