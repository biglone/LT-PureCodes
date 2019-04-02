#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN
#include <QtCore>
#include <QtGui>
#include <QInputDialog>
#include "cttk/base.h"
#include "Constants.h"
#include "pmclient/PmClient.h"
#include "BaseProcessor.h"
#include "MessageProcessor.h"
#include "filetransfer/attachtransfermgr.h"
#include "gui/debugdlg.h"
#include "gui/logindlg.h"
#include "gui/pscdlg.h"
#include "buddymgr.h"
#include "model/ModelManager.h"
#include "debugdlg.h"
#include "statuschanger/StatusChanger.h"
#include "autostatus/autostatus.h"
#include "login/Account.h"
#include "settings/GlobalSettings.h"
#include "db/DBBase.h"
#include "db/UserDB.h"
#include "db/ComponentMessageDB.h"
#include "BackgroundDlg.h"
#include "logger/logger.h"
#include "manager/presencemanager.h"
#include "manager/detailphotomanager.h"
#include "model/unreadmsgmodel.h"
#include "model/lastcontactmodeldef.h"
#include "model/rostermodeldef.h"
#include "model/groupmodeldef.h"
#include "model/DiscussModeldef.h"
#include "rtc/rtcsessionmanager.h"
#include "MessageDBStore.h"
#include "PmApp.h"
#include "pmessagebox.h"
#include "localclient.h"
#include "LocalCommMessage.h"
#include "widgetkit.h"
#include "ScreenShotDlg.h"
#include <QFontDatabase>
#include "wave/amrrecord.h"
#include "offlinemsgmanager.h"
#include "organizationmanager.h"
#include "rostermanager.h"
#include "groupmanager.h"
#include "DiscussManager.h"
#include "configmanager.h"
#include "sendmessagemanager.h"
#include "maxtsmanager.h"
#include "sessionvideomanager.h"
#include "commonconfigmanager.h"
#include "updatemanager.h"
#include "changenoticemgr.h"
#include "searchmanager.h"
#include "addfriendmanager.h"
#include "tipmanager.h"
#include "interphonemanager.h"
#include "interphonedialog.h"
#include "subscriptionmanager.h"
#include "subscriptionmodel.h"
#include "subscriptionmsgmanager.h"
#include "subscriptiondetailandhistorymanager.h"
#include "subscriptionlastmsgmodel.h"
#include "subscriptionlastmsgdialog.h"
#include "globalnotification/globalnotificationmanager.h"
#include "globalnotification/globalnotificationmsgmanager.h"
#include "globalnotification/globalnotificationdetailandhistorymanager.h"
#include "blacklistmodel.h"
#include "secretmanager.h"
#include "http/HttpPool.h"
#include "emotion/EmotionUtil.h"
#include "onlinereportmanager.h"
#include "util/MsgEncryptionUtil.h"
#include "messagewithdrawmanager.h"
#include "PMImageViewer.h"
#include "widgetmanager.h"
#include "util/PlayBeep.h"
#include "companyloginmanager.h"
#include "roamingmsgmanager.h"
#include "passwdmodifymanager.h"
#include "common/datetime.h"
#include <QTimeZone>
#include <QSslConfiguration>
#include "groupsmembermanager.h"
#include "iospushmanager.h"
#include "companyregistermanager.h"

#ifdef NDEBUG
#define RUN_AS_MANAGER_CLIENT
#endif // NDEBUG

#define START_SHUTDOWN_TIMEOUT      100
#define DELAYED_SHUTDOWN_TIMEOUT    5000

PmApp* PmApp::self = 0;

PmApp::PmApp(QApplication* app)
	: QObject(app)
	, m_bStartClose(false)
	, m_bStartQuit(false)
	, m_bQuitReady(false)
	, m_nShutdownKind(SK_WORK)
	, m_nShutdownDelayCount(0)
	, m_bLogout(false)
	, m_logoutMode(MODE_LOGOUT_COMPANY)
	, m_switchId()
	, m_pPmClient(new PmClient())
	, m_takeSelfPhoto(false)
	, m_pLoginMgr(0)
	, m_dpiScale(1.0)
{
	self = this;

	initDpiScale();

	initApp();

	initTranslator();

	initQss();

	initSslConfiguration();

	// create local client and connect to server
	m_localClient = new LocalClient(this);
	connect(m_localClient, SIGNAL(messageReceived(LocalCommMessage)), this, SLOT(onMessageReceived(LocalCommMessage)));
	connect(m_localClient, SIGNAL(sessionConnected()), this, SLOT(onSessionConnected()));
	connect(m_localClient, SIGNAL(sessionDisconnected()), this, SLOT(onSessionDisconnected()));
}

PmApp::~PmApp()
{
	if (m_pLoginMgr)
	{
		delete m_pLoginMgr;
		m_pLoginMgr = 0;
	}

	qDebug() << Q_FUNC_INFO;
}

void PmApp::construct()
{
	QThreadPool::globalInstance()->setMaxThreadCount(1000);

	m_pAccount.reset(new Account());

	QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::China));

	getLogger()->setLoggingType(Logger::SignalLogging);

	m_pHttpPool.reset(new HttpPool());
	m_pHttpPool->setParallel(5);
	m_pHttpPool->setRetryCount(0);

	m_pAttachMgr.reset(new CAttachTransferMgr());

	m_pBaseProcessor.reset(new BaseProcessor());
	m_pMessageProcessor.reset(new MessageProcessor());

	m_pAmrRecord.reset(new AmrRecord());
	m_pStatusChanger.reset(new StatusChanger());
	m_pModelManager.reset(new ModelManager());
	m_pAutoStatus.reset(new CAutoStatus());
	m_pTray.reset(new CSystemTray());
	m_pPresenceManager.reset(new PresenceManager());
	m_pDetailPhotoManager.reset(new DetailPhotoManager());
	m_pModelManager->setDetailPhotoManager(m_pDetailPhotoManager.data());
	m_pUnreadMsgModel.reset(new UnreadMsgModel());
	m_pBuddyMgr.reset(new CBuddyMgr(m_pUnreadMsgModel.data()));
	m_pRtcSessionManager.reset(new rtcsession::RtcSessionManager());
	m_pMessageDBStore.reset(new MessageDBStore());
	m_pRosterManager.reset(new RosterManager());
	m_pGroupManager.reset(new GroupManager());
	m_pOrganizationManager.reset(new OrganizationManager);
	m_pOfflineMsgManager.reset(new OfflineMsgManager());
	m_pMaxTsManager.reset(new MaxTsManager(*m_pOfflineMsgManager.data()));
	m_pDiscussManager.reset(new DiscussManager());
	m_pConfigManager.reset(new ConfigManager());
	m_pSendMessageManager.reset(new SendMessageManager());
	m_pSessionVideoManager.reset(new SessionVideoManager());
	m_pCommonConfigManager.reset(new CommonConfigManager());
	m_pCompanyLoginManager.reset(new CompanyLoginManager());
	m_pUpdateManager.reset(new UpdateManager());
	m_pChangeNoticeManager.reset(new ChangeNoticeMgr());
	m_pSearchManager.reset(new SearchManager());
	m_pAddFriendManager.reset(new AddFriendManager());
	m_pTipManager.reset(new TipManager());
	m_pInterphoneManager.reset(new InterphoneManager());
	m_pSubscriptionManager.reset(new SubscriptionManager());
	m_pGlobalNotificationManager.reset(new GlobalNotificationManager());
	m_pSubscriptionMsgManager.reset(new SubscriptionMsgManager(m_pSubscriptionManager.data()));
	m_pSubscriptionDetailAndHistoryManager.reset(new SubscriptionDetailAndHistoryManager());
	m_pSecretManager.reset(new SecretManager());
	m_pOnlineReportManager.reset(new OnlineReportManager());
	m_pMessageWithdrawManager.reset(new MessageWithdrawManager());
	m_pRoamingMsgManager.reset(new RoamingMsgManager());
	m_pPasswdModifyManager.reset(new PasswdModifyManager());
	m_pGroupsMemberManager.reset(new GroupsMemberManager(m_pGroupManager.data(), m_pDiscussManager.data()));
	m_pIOSPushManager.reset(new IOSPushManager());
	m_pCompanyRegisterManager.reset(new CompanyRegisterManager());

	// init login manager, register login process into login manager 
	m_pLoginMgr = new CLoginMgr();
	m_pLoginMgr->initObject(); 
	m_pLoginMgr->registerLoginProcess(m_pRosterManager.data());
	m_pLoginMgr->registerLoginProcess(m_pGroupManager.data());
	m_pLoginMgr->registerLoginProcess(m_pDiscussManager.data());

	m_ShutdownTimer.setSingleShot(true);

	// init signals & slots
	connect(&m_ShutdownTimer,SIGNAL(timeout()),SLOT(onShutdownTimerTimeout()));

	connect(qApp, SIGNAL(aboutToQuit()), SLOT(onApplicationAboutToQuit()));

	connect(m_pBaseProcessor.data(), SIGNAL(relogin(QStringList)), m_pLoginMgr, SLOT(resetPsg(QStringList)));

	connect(m_pLoginMgr, SIGNAL(validated()), this, SLOT(slot_loginValidated()));
	connect(m_pLoginMgr, SIGNAL(logined()), this, SLOT(slot_logined()));
	connect(m_pLoginMgr, SIGNAL(loginError(QString)), this, SLOT(slot_loginError()));
	connect(m_pLoginMgr, SIGNAL(logouted()), this, SLOT(slot_logouted()));
	connect(m_pLoginMgr, SIGNAL(aboutLogout()), this, SLOT(slot_aboutLogout()));
	
	connect(m_pUnreadMsgModel.data(), SIGNAL(lastMsgChanged(QString, bean::MessageType)), this, SLOT(slot_lastMsgChanged(QString, bean::MessageType)));
	connect(m_pUnreadMsgModel.data(), SIGNAL(msgToken(QString, bean::MessageType)), this, SIGNAL(removeFlickering(QString, bean::MessageType)));

	connect(m_pPresenceManager.data(), SIGNAL(presenceReceived(QString, int, int)), m_pStatusChanger.data(), SLOT(onPresenceReceived(QString)));
	connect(m_pOfflineMsgManager.data(), SIGNAL(offlineRecvOK()), m_pBuddyMgr.data(), SLOT(onOfflineRecvOK()));
	connect(m_pOfflineMsgManager.data(), SIGNAL(historyMsgRecvOK(int, QString, bean::MessageBodyList, bool)),
		m_pBuddyMgr.data(), SLOT(onHistoryMsgRecvOK(int, QString, bean::MessageBodyList, bool)));
	connect(m_pOfflineMsgManager.data(), SIGNAL(historyMsgRecvFailed(int, QString)),
		m_pBuddyMgr.data(), SLOT(onHistoryMsgRecvFailed(int, QString)));

	connect(m_pConfigManager.data(), SIGNAL(config2GotOk(QStringList)), m_pModelManager.data(), SLOT(onGotBlackListIds(QStringList)));

	connect(m_pModelManager.data(), SIGNAL(addDiscussMemberChangedTip(QString, QString)), m_pBuddyMgr.data(), SLOT(addDiscussMemberChangedTip(QString, QString)));

	connect(m_pSendMessageManager.data(), SIGNAL(sendMessageOK(QString, QString)), m_pBuddyMgr.data(), SLOT(onSendMessageOK(QString, QString)));
	connect(m_pSendMessageManager.data(), SIGNAL(sendMessageFailed(QString)), m_pBuddyMgr.data(), SLOT(onSendMessageFailed(QString)));

	connect(m_pGroupManager.data(), SIGNAL(groupOK()), this, SLOT(checkChatDlgAndUnreadMsgValid()));
	connect(m_pDiscussManager.data(), SIGNAL(discussOK()), this, SLOT(checkChatDlgAndUnreadMsgValid()));

	connect(m_pInterphoneManager.data(), SIGNAL(syncInterphonesFinished(bool)), m_pBuddyMgr.data(), SLOT(onInterphonesOK(bool)));
	connect(m_pInterphoneManager.data(), SIGNAL(interphoneStarted(QString, int, QString)), m_pBuddyMgr.data(), SLOT(onInterphoneStarted(QString, int, QString)));
	connect(m_pInterphoneManager.data(), SIGNAL(interphoneFinished(QString)), m_pBuddyMgr.data(), SLOT(onInterphoneFinished(QString)));

	connect(m_pSubscriptionManager.data(), SIGNAL(getSubscriptionListFinished(bool, QList<SubscriptionDetail>)), 
		m_pModelManager.data(), SLOT(onGetSubscriptionListFinished(bool, QList<SubscriptionDetail>)));

	connect(m_pSubscriptionManager.data(), SIGNAL(getMenuFinished(bool, QString, QVariantList)), 
		m_pModelManager.data(), SLOT(onGetMenuFinished(bool, QString, QVariantList)));

	connect(m_pSubscriptionDetailAndHistoryManager.data(), SIGNAL(openTitle(QString, QString, QString)),
		m_pSubscriptionMsgManager.data(), SLOT(openTitle(QString, QString, QString)));

	connect(m_pSubscriptionDetailAndHistoryManager.data(), SIGNAL(openAttach(QString, QString, QString)),
		m_pSubscriptionMsgManager.data(), SLOT(openAttach(QString, QString, QString)));

	connect(m_pModelManager.data(), SIGNAL(detailChanged(QString)), this, SLOT(onContactDetailChanged(QString)));

	connect(m_pSecretManager.data(), SIGNAL(requestSecretAckFinished(QString, QString, QString, int)), 
		m_pBuddyMgr.data(), SLOT(onRequestSecretAckFinished(QString, QString, QString, int)));

	connect(m_pChangeNoticeManager.data(), SIGNAL(secretAckRecved(QString, QString, int)),
		m_pBuddyMgr.data(), SLOT(setSendSecretMessageReadState(QString, QString, int)));

	connect(m_pChangeNoticeManager.data(), SIGNAL(secretAcked(QString, QString)),
		m_pBuddyMgr.data(), SLOT(recvSecretMessageAcked(QString, QString)));

	connect(m_pMessageWithdrawManager.data(), SIGNAL(withdrawOK(bean::MessageType, QString, QString, QString, QString)),
		m_pBuddyMgr.data(), SLOT(onMessageWithdrawOK(bean::MessageType, QString, QString, QString, QString)));
	connect(m_pMessageWithdrawManager.data(), SIGNAL(messageWithdrawed(bean::MessageType, QString, QString, QString, QString)),
		m_pBuddyMgr.data(), SLOT(onMessageWithdrawed(bean::MessageType, QString, QString, QString, QString)));
	connect(m_pMessageWithdrawManager.data(), SIGNAL(withdrawFailed(bean::MessageType, QString, QString, QString)),
		m_pBuddyMgr.data(), SLOT(onMessageWithdrawFailed(bean::MessageType, QString, QString, QString)));

	connect(m_pHttpPool.data(), SIGNAL(logSent(QString)), getLogger(), SLOT(logSent(QString)));
	connect(m_pHttpPool.data(), SIGNAL(logReceived(QString)), getLogger(), SLOT(logReceived(QString)));
}

Logger* PmApp::getLogger() const
{
	return Logger::getLogger();
}

CDebugDlg* PmApp::getDebugDlg() const
{
#if defined(_HAS_DEBUG_DIALOG)
	return CDebugDlg::getDebugDlg();
#else
	return 0;
#endif
}

StatusChanger* PmApp::getStatusChanger() const
{
	return m_pStatusChanger.data();
}

PmClient* PmApp::getPmClient() const
{
	return m_pPmClient.data();
}

BaseProcessor* PmApp::getBaseProcessor() const
{
	return m_pBaseProcessor.data();
}

MessageProcessor* PmApp::getMessageProcessor() const
{
	return m_pMessageProcessor.data();
}

CSystemTray* PmApp::getSystemTray() const
{
	return m_pTray.data();
}

ModelManager* PmApp::getModelManager() const
{
	return m_pModelManager.data();
}

CBuddyMgr* PmApp::getBuddyMgr() const
{
	return m_pBuddyMgr.data();
}

Account* PmApp::getAccount() const
{
	return m_pAccount.data();
}

PresenceManager* PmApp::getPresenceManager() const
{
	return m_pPresenceManager.data();
}

DetailPhotoManager* PmApp::getDetailPhotoManager() const
{
	return m_pDetailPhotoManager.data();
}

UnreadMsgModel* PmApp::getUnreadMsgModel() const
{
	return m_pUnreadMsgModel.data();
}

rtcsession::RtcSessionManager* PmApp::getRtcSessionManager() const
{
    return m_pRtcSessionManager.data();
}

MessageDBStore* PmApp::getMessageDBStore() const
{
	return m_pMessageDBStore.data();
}

OfflineMsgManager* PmApp::getOfflineMsgManager() const
{
	return m_pOfflineMsgManager.data();
}

MaxTsManager* PmApp::getMaxTsManager() const
{
	return m_pMaxTsManager.data();
}

RosterManager* PmApp::getRosterManager() const
{
	return m_pRosterManager.data();
}

OrganizationManager* PmApp::getOrganizationManager() const
{
	return m_pOrganizationManager.data();
}

GroupManager* PmApp::getGroupManager() const
{
	return m_pGroupManager.data();
}

DiscussManager* PmApp::getDiscussManager() const
{
	return m_pDiscussManager.data();
}

ConfigManager* PmApp::getConfigManager() const
{
	return m_pConfigManager.data();
}

SendMessageManager* PmApp::getSendMessageManager() const
{
	return m_pSendMessageManager.data();
}

SessionVideoManager* PmApp::getSessionVideoManager() const
{
	return m_pSessionVideoManager.data();
}

CommonConfigManager* PmApp::getCommonConfigManager() const
{
	return m_pCommonConfigManager.data();
}

UpdateManager* PmApp::getUpdateManager() const
{
	return m_pUpdateManager.data();
}

ChangeNoticeMgr* PmApp::getChangeNoticeMgr() const
{
	return m_pChangeNoticeManager.data();
}

SearchManager* PmApp::getSearchManager() const
{
	return m_pSearchManager.data();
}

AddFriendManager* PmApp::getAddFriendManager() const
{
	return m_pAddFriendManager.data();
}

AmrRecord* PmApp::getAmrRecord() const
{
	return m_pAmrRecord.data();
}

TipManager* PmApp::getTipManager() const
{
	return m_pTipManager.data();
}

InterphoneManager* PmApp::getInterphoneManager() const
{
	return m_pInterphoneManager.data();
}

SubscriptionManager* PmApp::getSubscriptionManager() const
{
	return m_pSubscriptionManager.data();
}

SubscriptionMsgManager *PmApp::getSubscriptionMsgManager() const
{
	return m_pSubscriptionMsgManager.data();
}

SubscriptionDetailAndHistoryManager *PmApp::getSubscriptionDetailAndHistoryManager() const
{
	return m_pSubscriptionDetailAndHistoryManager.data();
}

GlobalNotificationManager *PmApp::getGlobalNotificationManager() const
{
	return m_pGlobalNotificationManager.data();
}

GlobalNotificationMsgManager *PmApp::getGlobalNotificationMsgManager() const
{
	return m_pGlobalNotificationMsgManager.data();
}

GlobalNotificationDetailAndHistoryManager *PmApp::getGlobalNotificationDetailAndHistoryManager() const
{
	return m_pGlobalNotificationDetailAndHistoryManager.data();
}

SecretManager *PmApp::getSecretManager() const
{
	return m_pSecretManager.data();
}

HttpPool *PmApp::getHttpPool() const
{
	return m_pHttpPool.data();
}

OnlineReportManager *PmApp::getOnlineReportManager() const
{
	return m_pOnlineReportManager.data();
}

MessageWithdrawManager *PmApp::getMessageWithdrawManager() const
{
	return m_pMessageWithdrawManager.data();
}

CompanyLoginManager *PmApp::getCompanyLoginManager() const
{
	return m_pCompanyLoginManager.data();
}

RoamingMsgManager *PmApp::getRoamingMsgManager() const
{
	return m_pRoamingMsgManager.data();
}

PasswdModifyManager *PmApp::getPasswdModifyManager() const
{
	return m_pPasswdModifyManager.data();
}

GroupsMemberManager *PmApp::getGroupsMemberManager() const
{
	return m_pGroupsMemberManager.data();
}

IOSPushManager *PmApp::getIOSPushManager() const
{
	return m_pIOSPushManager.data();
}

CompanyRegisterManager *PmApp::getCompanyRegisterManager() const
{
	return m_pCompanyRegisterManager.data();
}

void PmApp::setMaxMsgTs(const QString &ts)
{
	if (!m_pMaxTsManager.isNull())
	{
		m_pMaxTsManager->setMaxTs(ts);
	}
}

QIcon PmApp::appDefaultIcon() const
{
	QIcon appIcon;
	QString appIconPath = QCoreApplication::applicationDirPath() + "/images/logo.png";
	if (QFile::exists(appIconPath))
		appIcon = QIcon(appIconPath);
	else
		appIcon = QIcon(":/app/app.png");
	return appIcon;
}

qreal PmApp::dpiScale() const
{
	return m_dpiScale; // dpi scale compared to 96DPI
}

int PmApp::scale2DevPos(int visualPos) const
{
	return qFloor(visualPos*m_dpiScale);
}

int PmApp::scale2VisualPos(int devPos) const
{
	return qFloor(devPos/m_dpiScale);
}

bool PmApp::connectToManager()
{
#ifdef RUN_AS_MANAGER_CLIENT
	int i = 0;
	bool connected = false;
	while (i < 3) // try two times
	{
		connected = m_localClient->connectToServer(LOCAL_COMM_SERVER_NAME, 1000);
		if (connected)
			break;
		++i;
	}
	if (!connected)
	{
		qDebug() << "Can't connect to pm deamon manager!";
		return false;
	}
#endif // RUN_AS_MANAGER_CLIENT

	return true;
}

bool PmApp::Initialize()
{
	bool bRet = false;
	do 
	{
		// init date time
		initDateTime();

		// init all handlers
		initHandleObjects();

		// init buddy manager
		m_pBuddyMgr->Init();

		// init debug dialog
		if (!InitDebugDlg())
		{
			qWarning() << Q_FUNC_INFO << "init debug dialog failed";
		}

		// init model
		if (!InitModel())
		{
			break;
		}

		// init background dialog
		m_pBackgroundDlg.reset(new BackgroundDlg());
		connect(m_pLoginMgr, SIGNAL(updateVersion(QString, QString, QString)), 
			m_pBackgroundDlg.data(), SLOT(showUpdateDlg(QString, QString, QString)));
		connect(m_pLoginMgr, SIGNAL(companyLoginFailed(QString)),
			m_pBackgroundDlg.data(), SLOT(onCompanyLoginFailed(QString)));

		if (!InitSystemTray())
		{
			qWarning() << Q_FUNC_INFO << "init system tray failed";
			break;
		}

		m_pTray->setVisible(true);

		m_pBackgroundDlg->show();

		if (isLogout() && logoutMode() == MODE_SWITCH_COMPANY)
		{
			m_pBackgroundDlg->loginWithId(m_switchId);
		}

		bRet = true;
	} while (0);

	if (!bRet)
	{
		Release();
	}
	return bRet;
}

void PmApp::Release()
{
	// remove logined user
	notifyUserLogouted(getAccount()->id());

	// release subscription message
	m_pSubscriptionMsgManager->release();

	// release ppt and video
	m_pRtcSessionManager->release();

	// release the message store
	m_pMessageDBStore->release();

	// clear all the presence here
	m_pPresenceManager->reset();

	// clear all unread message
	m_pUnreadMsgModel->clean();

	// clear all details
	m_pDetailPhotoManager->clear();

	// stop auto status
	m_pAutoStatus->stop();

	// set off line status
	m_pStatusChanger->setStatus(StatusChanger::Status_Offline);

	// update user db
	getAccount()->updateLoginPhoneTime();

	UninitSystemTray();

	m_pBuddyMgr->Release();

	m_pAttachMgr->release();

	UninitModel();

	// init emotions
	EmotionUtil::instance().uninit();

	// close all dbs
	DB::DBBase::closeAllDBs();

	// clear all the account
	getAccount()->clear();

	qDebug() << Q_FUNC_INFO;
}

bool PmApp::isShutingDown() const
{
	return m_nShutdownKind != SK_WORK;
}

bool PmApp::isLogout() const
{
	return m_bLogout;
}

void PmApp::setLogout(bool bLogout)
{
	m_bLogout = bLogout;

	if (m_bLogout)
	{
		m_pAutoStatus->stop();
	}
	else
	{
		setLogoutMode(MODE_LOGOUT_COMPANY);
		setSwitchId("");
		m_pAutoStatus->start();
	}
}

LogoutMode PmApp::logoutMode() const
{
	return m_logoutMode;
}

void PmApp::setLogoutMode(LogoutMode mode)
{
	m_logoutMode = mode;
}

void PmApp::setSwitchId(const QString &uid)
{
	m_switchId = uid;
}

void PmApp::closeSystemTray()
{
	UninitSystemTray();
}

bool PmApp::hasVideoSession() const
{
	return m_pRtcSessionManager.data()->hasVideoSession();
}

bool PmApp::hasSession() const
{
	return m_pRtcSessionManager.data()->hasSession();
}

void PmApp::setTakeSelfPhoto(bool photo)
{
	m_takeSelfPhoto = photo;
}

bool PmApp::isTakeSelfPhoto() const
{
	return m_takeSelfPhoto;
}

void PmApp::quit()
{
	if (!isShutingDown())
	{
		emit prepareForQuit();

		m_nShutdownKind = SK_QUIT;
		startClose();
	}
}

void PmApp::restart()
{
	if (!isShutingDown())
	{
		m_nShutdownKind = SK_RESTART;
		startClose();
	}
}

void PmApp::delayShutdown()
{
	if (isShutingDown())
		m_ShutdownTimer.start(DELAYED_SHUTDOWN_TIMEOUT);
	m_nShutdownDelayCount++;
}

void PmApp::continueShutdown()
{
	if (--m_nShutdownDelayCount<=0 && isShutingDown())
		m_ShutdownTimer.start(START_SHUTDOWN_TIMEOUT);
}

void PmApp::checkIfUserLogined(const QString &id)
{
#ifdef RUN_AS_MANAGER_CLIENT
	LocalCommMessage msg;
	msg.setMessageType(ns_local_comm::MsgApplication);
	msg.setRequestCode(ns_local_comm::AppQueryAccountLogined);
	msg.setData(id.toUtf8());
	m_localClient->sendMessage(msg);
#else
	Q_UNUSED(id);
	emit userLoginCheckFinished(false);
#endif
}

void PmApp::notifyUserLogined(const QString &id)
{
#ifdef RUN_AS_MANAGER_CLIENT
	LocalCommMessage msg;
	msg.setMessageType(ns_local_comm::MsgApplication);
	msg.setRequestCode(ns_local_comm::AppSetLoginAccount);
	msg.setData(id.toUtf8());
	m_localClient->sendMessage(msg);
#else
	Q_UNUSED(id);
#endif
}

void PmApp::notifyUserLogouted(const QString &id)
{
#ifdef RUN_AS_MANAGER_CLIENT
	LocalCommMessage msg;
	msg.setMessageType(ns_local_comm::MsgApplication);
	msg.setRequestCode(ns_local_comm::AppSetLogoutAccount);
	msg.setData(id.toUtf8());
	m_localClient->sendMessage(msg);
#else
	Q_UNUSED(id);
#endif
}

void PmApp::notifyUpdating(bool update)
{
#ifdef RUN_AS_MANAGER_CLIENT
	LocalCommMessage msg;
	msg.setMessageType(ns_local_comm::MsgApplication);
	msg.setRequestCode(ns_local_comm::AppSetUpdate);
	if (update)
		msg.setData("true");
	else
		msg.setData("false");
	m_localClient->sendMessage(msg);
#else
	Q_UNUSED(update);
#endif
}

void PmApp::checkIfUpdating()
{
#ifdef RUN_AS_MANAGER_CLIENT
	LocalCommMessage msg;
	msg.setMessageType(ns_local_comm::MsgApplication);
	msg.setRequestCode(ns_local_comm::AppQueryUpdate);
	m_localClient->sendMessage(msg);
#else
	emit updateCheckFinished(false);
#endif
}

void PmApp::onShutdownTimerTimeout() 
{
	if (m_bStartClose)
	{
		finishClose();
	}
	else if (m_bStartQuit)
	{
		finishQuit();
	}
}

void PmApp::onApplicationAboutToQuit()
{
	if (!m_bQuitReady)
	{
		m_nShutdownKind = SK_QUIT;
		startClose();

		QDateTime closeTimeout = QDateTime::currentDateTime().addMSecs(DELAYED_SHUTDOWN_TIMEOUT);
		while (closeTimeout>QDateTime::currentDateTime() && m_nShutdownDelayCount>0)
			QApplication::processEvents();
		finishClose();

		QDateTime quitTimeout = QDateTime::currentDateTime().addMSecs(DELAYED_SHUTDOWN_TIMEOUT);
		while (quitTimeout>QDateTime::currentDateTime() && m_nShutdownDelayCount>0)
			QApplication::processEvents();
		finishQuit();
	}
}

void PmApp::slot_loginValidated()
{
	// get offline ts
	m_offlineTs = Account::settings()->maxMsgTs();

	// get withdraw
	m_lastWithdrawId = Account::settings()->lastWithdrawId();
	m_pMessageWithdrawManager->setLastWithdrawId(m_lastWithdrawId, true);

	// init attach manager
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString addressStr;
	bool useFastdfs = loginConfig.fastdfsEnabled;
	if (useFastdfs)
	{
		addressStr = loginConfig.trackerServer;
	}
	else
	{
		addressStr = QString("%1:%2").arg(loginConfig.transferIp).arg(loginConfig.transferPort);
	}
	if (!m_pAttachMgr->init(useFastdfs, addressStr))
	{
		qWarning() << Q_FUNC_INFO << "init transfer manager failed: " << useFastdfs << addressStr;
	}

	// set message encrypt
	QString msgEncryptSeed = GlobalSettings::msgEncryptSeed();
	bool msgEncrypt = GlobalSettings::msgEncrypt();
	QByteArray msgPassword = MsgEncryptionUtil::generatePassword(msgEncryptSeed);
	m_pSendMessageManager->setMsgEncrypt(msgEncrypt);
	if (msgEncrypt)
	{
		m_pSendMessageManager->setMsgPassword(msgPassword);
		m_pMessageProcessor->setMsgPassword(msgPassword);
	}
	else
	{
		m_pSendMessageManager->setMsgPassword(QByteArray());
		m_pMessageProcessor->setMsgPassword(QByteArray());
	}

	// message beep
	AccountSettings *accountSettings = Account::settings();
	PlayBeep::setMute(accountSettings->messagePromptMute());
	PlayBeep::setBuddyMute(accountSettings->buddyMsgMuteOn());
	PlayBeep::setSubscriptionMute(accountSettings->subscriptionMsgMuteOn());
	PlayBeep::setBuddyFilePath(accountSettings->buddyMsgPromptFile());
	PlayBeep::setSubscriptionFilePath(accountSettings->subscriptionMsgPromptFile());

	// start http pool
	HttpPool::setApiKey(getCompanyLoginManager()->apiKey());
	HttpPool::setApiSecurity(getCompanyLoginManager()->securityKey());
	HttpPool::setCompany(getAccount()->companyId());
	HttpPool::setApiCheck(true);

	m_pHttpPool->start();
}

void PmApp::slot_logined()
{
	qDebug() << Q_FUNC_INFO << "begin";

	// clear the logout state
	setLogout(false);

	// status change
	m_pStatusChanger->onLoginMgrLogined();

	// set tray tip
	QString trayTip = QString("%2(%3)\n%4")
		.arg(m_pModelManager->userName(getAccount()->id()))
		.arg(getAccount()->loginPhone())
		.arg(getAccount()->companyName());
	m_pTray->setTooltip(trayTip);

	QTimer::singleShot(0, this, SLOT(slot_initSession()));

	// clear all the presence here
	m_pPresenceManager->reset();

	// init last contact model
	m_pModelManager->lastContactModel()->release();
	m_pModelManager->lastContactModel()->init();
	m_pModelManager->lastContactModel()->checkDiscussNameChanged();

	// init subscription model, and get subscription list
	m_pModelManager->subscriptionModel()->readFromDB();
	m_pModelManager->subscriptionLastMsgModel()->readFromDB();
	m_pSubscriptionManager->getSubscriptionList(getAccount()->id());
	m_pSubscriptionMsgManager->start();

	// read&sync detail
	slot_readsyncDetail();

	// do background dlg stuff
	m_pBackgroundDlg->onLogined();

	// start message db
	m_pMessageDBStore->init();

	// get withdraw
	m_pMessageWithdrawManager->syncWithdraws(Account::instance()->id(), m_lastWithdrawId);

	// get all the off-line message
	m_pBuddyMgr->clearOfflineMessages();
	initMaxMsgTs();
	QString maxStamp = m_pMaxTsManager->maxTs();
	if (maxStamp != m_offlineTs)
	{
		qDebug() << "request offline ts is different, before offline received messages: " << m_offlineTs << maxStamp;
	}
	qDebug() << "request offline: " << m_offlineTs;
	m_pOfflineMsgManager->requestOfflineMsg(m_offlineTs);

	// check failed send message
	m_pBuddyMgr->checkFailedSendMessages();

	// check send secret acks
	m_pBuddyMgr->checkSendSecretAcks();
	
	// check if secret is read
	m_pBuddyMgr->checkSendSecretMessageDestroy();
	m_pBuddyMgr->checkRecvSecretMessageDestory();

	// check chat dialog and unread message
	checkChatDlgAndUnreadMsgValid();

	// read black list
	QStringList blackListIds = Account::settings()->blackListIds();
	m_pModelManager->blackListModel()->setBlackList(blackListIds);

	// get config
	m_pConfigManager->getConfig(QList<int>() << 1 << 2 << 3);

	// get add friend list
	m_pAddFriendManager->requestAddFriendList();

	// get interphones
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog::getInterphoneDialog()->quitAndClose();
	}

	QStringList groupIds = m_pModelManager->groupModel()->allGroupIds();
	QStringList discussIds = m_pModelManager->discussModel()->allDiscussIds();
	m_pInterphoneManager->syncInterphones(groupIds, discussIds);

	// request group logoes
	checkToRequestGroupLogoes();

	// init emotions
	EmotionUtil::instance().init();

	// start report online
	m_pOnlineReportManager->start(getAccount()->id());

	// notify account logined
	notifyUserLogined(getAccount()->id());

	qDebug() << Q_FUNC_INFO << "end";
}

void PmApp::slot_loginError()
{
	slot_logouted();
}

void PmApp::slot_logouted()
{
	qDebug() << Q_FUNC_INFO << "begin";

	// stop fetch members
	m_pGroupsMemberManager->stopFetch();

	// clear new added discuss ids
	m_pBuddyMgr->clearNewAddedDiscussIds();

	// stop report online
	m_pOnlineReportManager->stop();

	// clear all interphones
	m_pInterphoneManager->clearInterphones();

	// clear all the off-line message
	m_pOfflineMsgManager->clear();
	m_pBuddyMgr->clearOfflineMessages();

	// clear max ts manager
	m_pMaxTsManager->stop();

	// clear all the presence here
	m_pPresenceManager->reset();

	// release ppt and video
    m_pRtcSessionManager->release();

	// release attachmanager
	m_pAttachMgr->release();

	// stop subscription message
	m_pSubscriptionMsgManager->stop();

	// clear secret ack
	m_pSecretManager->clearSecretRead();

	// stop sync os
	m_pOrganizationManager->stopSyncOrgStructDept();

	// stop http
	m_pHttpPool->stop();

	qDebug() << Q_FUNC_INFO << "end";
}

void PmApp::slot_aboutLogout()
{
	// report offline
	m_pOnlineReportManager->reportOffline();
	m_pOnlineReportManager->stop();

	// close interphone
	m_pInterphoneManager->quitCurrentInterphone(Account::instance()->id());

	// stop auto status
	m_pAutoStatus->stop();

	// report max ts
	m_pMaxTsManager->reportMaxTs();

	// release ppt and video
	m_pRtcSessionManager->release();
}

void PmApp::slot_readsyncDetail()
{
	qPmApp->getModelManager()->initDetailPhotoManager();

	// sync all roster and last contact detail
	QStringList syncIds;
	syncIds << Account::instance()->id();
	syncIds << m_pModelManager->rosterModel()->allRosterIds();
	syncIds << m_pModelManager->lastContactModel()->allContactIds();
	syncIds.removeDuplicates();
	syncIds.removeAll(QString(SUBSCRIPTION_ROSTER_ID));

	qPmApp->getModelManager()->syncDetailWithVersionCheck(syncIds);
}

void PmApp::slot_initSession()
{
    // session
    m_pRtcSessionManager->init();
}

void PmApp::slot_systemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
	{
		if (m_pUnreadMsgModel->hasUnreadMsg()) // has unread messages
		{
			// if has unread messages, take that message
			slot_takeMsg();
		}
		else
		{
			// no unread messages, open the main window
			slot_openMainWindow();
		}
	}
}

void PmApp::slot_lastMsgChanged(const QString &id, bean::MessageType msgType)
{
	if (!id.isEmpty() && msgType != bean::Message_Invalid) // has a new unread message
	{
		if (msgType == bean::Message_GroupChat)
		{
			// start flash with group icon
			QIcon groupIcon(getModelManager()->getGroupLogo(id));
			getSystemTray()->startFlashTray(groupIcon);
		}
		else if (msgType == bean::Message_DiscussChat)
		{
			// start flash with discuss icon
			QIcon discussIcon(QPixmap(":/images/Icon_64_small.png"));
			getSystemTray()->startFlashTray(discussIcon);
		}
		else
		{
			// start flash with user icon
			QIcon userIcon(getModelManager()->getUserAvatar(id));
			getSystemTray()->startFlashTray(userIcon);
		}

		// add flickering of this item
		emit addFlickering(id, msgType);
	}
	else
	{
		// no new message
		getSystemTray()->stopFlashTray();

		// clear all the flickering item
		emit clearFlickering();
	}
}

void PmApp::slot_takeMsg()
{
	QString topMsgId;
	bean::MessageType msgType;
	if (m_pUnreadMsgModel->getTopUnreadMsg(topMsgId, msgType))
	{
		getBuddyMgr()->slot_open_dialog(msgType, topMsgId);
	}
}

void PmApp::slot_openMainWindow()
{
	// open main window
	if (!m_pBackgroundDlg.isNull())
	{
		m_pBackgroundDlg.data()->show();
	}
}

void PmApp::openImages(const QStringList &imagePathes, const QString &imagePath)
{
	if (m_imageViewer.isNull())
	{
		m_imageViewer = new PMImageViewer();
	}
	else
	{
		delete m_imageViewer;
		m_imageViewer = new PMImageViewer();
	}
	m_imageViewer.data()->setSaveDelegate(getBuddyMgr());
	connect(m_imageViewer.data(), SIGNAL(forwardImage(QString)), getBuddyMgr(), SLOT(forwardImage(QString)), Qt::UniqueConnection);
	if (m_imageViewer.data()->showImage(imagePathes, imagePath))
	{
		WidgetManager::showActivateRaiseWindow(m_imageViewer.data());
	}
}

void PmApp::setLanguageEnglish()
{
	GlobalSettings::setLanguage(GlobalSettings::Language_ENG);

	foreach (QTranslator *translator, m_translators)
	{
		QApplication::removeTranslator(translator);
	}

	// remove translator of widget module
	widgetkit::removeTranslator();

	// remove translator of screenshot dlg
	ScreenShotDlg::removeTranslation();

	// remove translator of image viewer
	PMImageViewer::removeTranslator();

	// re-set style sheet
	initQss();
}

void PmApp::setLanguageChinese()
{
	GlobalSettings::setLanguage(GlobalSettings::Language_ZH_CN);

	foreach (QTranslator *translator, m_translators)
	{
		QApplication::installTranslator(translator);
	}

	// install translator of widget module
	widgetkit::installTranslator();

	// install translator of screenshot dlg
	ScreenShotDlg::installTranslation();

	// install translator of image viewer
	PMImageViewer::installTranslator();

	// re-set style sheet
	initQss();
}

void PmApp::onMessageReceived(const LocalCommMessage &msg)
{
	if (msg.messageType() == ns_local_comm::MsgApplication)
	{
		handleApplicationMessage(msg);
	}
}

void PmApp::onSessionConnected()
{
	qDebug() << "PmApp: connected to deamon";
}

void PmApp::onSessionDisconnected()
{
	qDebug() << "PmApp: disconnected to deamon";
	if (isShutingDown())
	{
		qDebug() << "PmApp: is shutting down, change to quit type: " << m_nShutdownKind;
		m_nShutdownKind = SK_QUIT;
	}
	else
	{
		quit();
	}
}

void PmApp::checkChatDlgAndUnreadMsgValid()
{
	if (GetLoginMgr()->isLogined())
	{
		QTimer::singleShot(0, m_pBuddyMgr.data(), SLOT(checkChatDlgValid()));
		QTimer::singleShot(0, m_pBuddyMgr.data(), SLOT(checkUnreadMsgValid()));
	}
}

void PmApp::onContactDetailChanged(const QString &id)
{
	QString topMsgId;
	bean::MessageType msgType;
	if (m_pUnreadMsgModel->getTopUnreadMsg(topMsgId, msgType))
	{
		if (msgType == bean::Message_Chat && id == topMsgId)
		{
			if (m_pTray->isFlashing())
			{
				QIcon userIcon(getModelManager()->getUserAvatar(id));
				m_pTray->startFlashTray(userIcon);
			}
		}
	}
}

bool PmApp::InitDebugDlg()
{
	bool bRet = false;
	do 
	{
#if defined(_HAS_DEBUG_DIALOG)
		CDebugDlg *debugDlg = CDebugDlg::getDebugDlg();
		connect (getLogger(), SIGNAL(message(Logger::MessageType, const QString &)), 
			debugDlg, SLOT(logMessage(Logger::MessageType, const QString &)), Qt::UniqueConnection);
		connect (debugDlg, SIGNAL(showMainWindow()), this, SLOT(slot_openMainWindow()));
#if defined(NDEBUG)
		debugDlg->setVisible(false);
#else
		debugDlg->setVisible(true);
#endif
#endif //_HAS_DEBUG_DIALOG

		bRet = true;
	} while (0);

	return bRet;
}

bool PmApp::InitSystemTray()
{
	assert(m_pTray != NULL);

	m_pTray->setStatus((int)(StatusChanger::Status_Offline));
	m_pTray->setTrayIcon(appDefaultIcon());
	m_pTray->setTooltip(GlobalSettings::title());

	connect(getStatusChanger(), SIGNAL(statusChanged(int)), getSystemTray(), SLOT(slot_statusChanged(int)));
	connect(getSystemTray(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slot_systemTrayActivated(QSystemTrayIcon::ActivationReason)));
	connect(getSystemTray(), SIGNAL(openMainWindow()), this, SLOT(slot_openMainWindow()));
	
	connect(getDebugDlg(), SIGNAL(retriveSystemTrayIconStatus()), getSystemTray(), SLOT(reportStatus()));
	connect(getDebugDlg(), SIGNAL(restartSystemTrayIcon()), getSystemTray(), SLOT(restart()));

	return true;
}

void PmApp::UninitSystemTray()
{
	if (!m_pTray.isNull())
	{
		m_pTray->stopFlashTray();

		m_pTray->setVisible(false);

		disconnect(getStatusChanger(), SIGNAL(statusChanged(int)), getSystemTray(), SLOT(slot_statusChanged(int)));
		disconnect(getSystemTray(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slot_systemTrayActivated(QSystemTrayIcon::ActivationReason)));
		disconnect(getSystemTray(), SIGNAL(openMainWindow()), this, SLOT(slot_openMainWindow()));
	}
}

bool PmApp::InitModel()
{
	bool bRet = false;
	do 
	{
		m_pModelManager->reset();

		ModelManager::setSubscriptionDefaultIcon(QPixmap(":/images/Icon_66.png"));

		QMap<int, QPixmap> mapTerminal;
		mapTerminal[PresenceManager::TerminalPC]      = QPixmap(":/images/Icon_49.png");
		mapTerminal[PresenceManager::TerminalAndroid] = QPixmap(":/images/Icon_50.png");
		mapTerminal[PresenceManager::TerminalIPhone]  = QPixmap(":/images/Icon_50_1.png");
		mapTerminal[PresenceManager::TerminalWeb]     = QPixmap(":/images/Icon_49_1.png");

		ModelManager::setTerminalDefaultIcon(mapTerminal);

		QMap<int, QPixmap> mapShowIcon;
		mapShowIcon[PresenceManager::ShowDND] = QPixmap(":/images/Icon_19.png");
		mapShowIcon[PresenceManager::ShowXA]  = QPixmap(":/images/Icon_23.png");
		ModelManager::setShowDefaultIcon(mapShowIcon);

		QImage avatar;
		QString avatarPath = QCoreApplication::applicationDirPath() + "/images/avatar110.png";
		if (!avatar.load(avatarPath))
			avatar.load(":/images/Icon_60.png");
		ModelManager::setAvatarDefaultIcon(avatar);

		avatarPath = QCoreApplication::applicationDirPath() + "/images/avatar42.png";
		if (!avatar.load(avatarPath))
			avatar.load(":/images/Icon_60_middle.png");
		ModelManager::setAvatarDefaultMiddleIcon(avatar);

		avatarPath = QCoreApplication::applicationDirPath() + "/images/avatar20.png";
		if (!avatar.load(avatarPath))
			avatar.load(":/images/Icon_60_small.png");
		ModelManager::setAvatarDefaultSmallIcon(avatar);
		
		bRet = true;
	} while (0);

	return bRet;
}

void PmApp::UninitModel()
{
	m_pModelManager->reset();
}

void PmApp::closeTopWidget()
{
	qWarning() << Q_FUNC_INFO << "begin";
	foreach (QWidget* widget, QApplication::topLevelWidgets())
	{
		widget->close();
	}
	qWarning() << Q_FUNC_INFO << "end";
}

void PmApp::startClose()
{
	if (!m_bStartClose)
	{
		m_bStartClose = true;
		delayShutdown();

		// remove all handler
		removeHandleObjects();

		// close all video dialogs
		if (!m_pSessionVideoManager.isNull())
		{
			m_pSessionVideoManager.data()->closeAllVideoSessions();
		}

		// close all top windows
		closeTopWidget();

		continueShutdown();
	}
}

void PmApp::finishClose()
{
	m_ShutdownTimer.stop();
	if (m_bStartClose)
	{
		m_bStartClose = false;
		startQuit();
	}
}

void PmApp::startQuit()
{
	if (!m_bStartQuit)
	{
		m_bStartQuit = true;
		delayShutdown();

		Release();

		continueShutdown();
	}
}

void PmApp::finishQuit()
{
	m_ShutdownTimer.stop();
	if (m_bStartQuit)
	{
		m_bStartQuit = false;

		if (m_nShutdownKind == SK_RESTART)
		{
			m_nShutdownKind = SK_WORK;
			m_nShutdownDelayCount = 0;

			if (!Initialize())
			{
				QTimer::singleShot(0,this,SLOT(restart()));
			}
		}
		else if (m_nShutdownKind == SK_QUIT)
		{
			// quit app
			m_bQuitReady = true;
			QTimer::singleShot(0,qApp,SLOT(quit()));
		}
	}
}

void PmApp::handleApplicationMessage(const LocalCommMessage &msg)
{
	if (msg.messageType() != ns_local_comm::MsgApplication)
		return;

	if (msg.requestCode() == ns_local_comm::AppClose)
	{
		quit();
		return;
	}

	if (msg.requestCode() == ns_local_comm::AppQueryAccountLogined)
	{
		bool logined = false;
		QByteArray responseData = msg.data();
		logined = (responseData == "true" ? true : false);
		emit userLoginCheckFinished(logined);
		return;
	}

	if (msg.requestCode() == ns_local_comm::AppQueryUpdate)
	{
		bool isUpdate = false;
		QByteArray responseData = msg.data();
		isUpdate = (responseData == "true" ? true : false);
		emit updateCheckFinished(isUpdate);
		return;
	}
}

void PmApp::initApp()
{
	QApplication::setQuitOnLastWindowClosed(false);

	QApplication::setOrganizationName(QObject::tr(ORG_NAME));
	QApplication::setOrganizationDomain(QObject::tr(ORG_DOMAIN));
	QApplication::setApplicationName(QObject::tr(APP_NAME));
	QApplication::setApplicationVersion(QObject::tr(APP_VERSION));

	qWarning() << __FUNCTION__ << QApplication::libraryPaths() << APP_VERSION;
	QStringList paths = QApplication::libraryPaths();
	QString pluginPath = QApplication::applicationDirPath() + "/plugins";
	paths.append(pluginPath);
	QApplication::setLibraryPaths(paths);
	qWarning() << __FUNCTION__ << QApplication::libraryPaths();

	//Q_INIT_RESOURCE(res);

	QApplication::setWindowIcon(appDefaultIcon());
}

void PmApp::initTranslator()
{
	QString sSystemTranslatorName = QString::fromLatin1("qt_zh_CN");
	QString sWidgetTranslatorName = QString::fromLatin1("qtwidgets_zh_CN");
	QString sGuiTranslatorName = QString::fromLatin1("qtgui_zh_CN");
	QString sUserTranslatorName = QString::fromLatin1("inst_zh_CN");

	QTranslator *pSystemTranslator = new QTranslator(qApp);
	pSystemTranslator->load(sSystemTranslatorName, ":/translator");
	m_translators.append(pSystemTranslator);

	QTranslator *pWidgetTranslator = new QTranslator(qApp);
	pWidgetTranslator->load(sWidgetTranslatorName, ":/translator");
	m_translators.append(pWidgetTranslator);

	QTranslator *pGuiTranslator = new QTranslator(qApp);
	pGuiTranslator->load(sGuiTranslatorName, ":/translator");
	m_translators.append(pGuiTranslator);

	QTranslator *pUserTranslator = new QTranslator(qApp);
	pUserTranslator->load(sUserTranslatorName, ":/translator");
	m_translators.append(pUserTranslator);

	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
	{
		QApplication::installTranslator(pSystemTranslator);
		QApplication::installTranslator(pWidgetTranslator);
		QApplication::installTranslator(pGuiTranslator);
		QApplication::installTranslator(pUserTranslator);

		// install translator of widget module
		widgetkit::installTranslator();

		// install translator of screenshot dlg
		ScreenShotDlg::installTranslation();

		// install translator of image viewer
		PMImageViewer::installTranslator();
	}
}

void PmApp::initQss()
{
	// set qss
	QString sContent;
	QFile file(":/theme/qss/pm_skin.qss");
	if (file.open(QIODevice::ReadOnly))
	{
		sContent.append(file.readAll());
		file.close();
	}

	file.setFileName(":/qss/main.qss");
	if (file.open(QIODevice::ReadOnly))
	{
		sContent.append(file.readAll());
		file.close();
	}

	QString fontFamilyName = QObject::tr("Microsoft YaHei");
	// check if font exists
	QFontDatabase fontDatabase;
	QStringList families = fontDatabase.families();
	bool hasFont = false;
	foreach (QString fontFamily, families)
	{
		if (fontFamily.startsWith(fontFamilyName, Qt::CaseInsensitive))
		{
			hasFont = true;
			break;
		}
	}
	if (!hasFont)
	{
		fontFamilyName = QString::fromLatin1("Tahoma");
	}

	sContent = QString("QWidget {font-family: \"%1\"; font-size: 10pt; color: #333333;}%2").arg(fontFamilyName).arg(sContent);
	qApp->setStyleSheet(sContent);

	QFont appDefaultFont(fontFamilyName, 10);
	qApp->setFont(appDefaultFont);
}

void PmApp::initMaxMsgTs()
{
	QString maxTs = Account::settings()->maxMsgTs();
	m_pMaxTsManager.data()->init(maxTs);
}

void PmApp::initHandleObjects()
{
	m_pRosterManager->initObject();
	m_pGroupManager->initObject();
	m_pPresenceManager->initObject();
	m_pBaseProcessor->initObject();
	m_pMessageProcessor->initObject();
	m_pOfflineMsgManager->initObject();
	m_pDiscussManager->initObject();
	m_pConfigManager->initObject();
	m_pSendMessageManager->initObject();
	m_pChangeNoticeManager->initObject();
	m_pTipManager->initObject();
	m_pInterphoneManager->initObject();
	m_pOnlineReportManager->initObject();
	m_pMessageWithdrawManager->initObject();
	m_pRtcSessionManager->initObject();

	qDebug() << Q_FUNC_INFO << " init objects";
}

void PmApp::removeHandleObjects()
{
	m_pRosterManager->removeObject();
	m_pGroupManager->removeObject();
	m_pPresenceManager->removeObject();
	m_pBaseProcessor->removeObject();
	m_pMessageProcessor->removeObject();
	m_pOfflineMsgManager->removeObject();
	m_pDiscussManager->removeObject();
	m_pConfigManager->removeObject();
	m_pSendMessageManager->removeObject();
	m_pChangeNoticeManager->removeObject();
	m_pTipManager->removeObject();
	m_pInterphoneManager->removeObject();
	m_pOnlineReportManager->removeObject();
	m_pMessageWithdrawManager->removeObject();
	m_pRtcSessionManager->removeObject();

	qDebug() << Q_FUNC_INFO << " remove objects";
}

void PmApp::checkToRequestGroupLogoes()
{
	const int kGroupInitVersion = 1;
	GroupManager *groupManager = m_pGroupManager.data();
	QMap<QString, int> oldLogoVersions = Account::settings()->allGroupLogoVersions();
	QMap<QString, int> newLogoVersions = groupManager->logoVersions();
	foreach (QString gid, newLogoVersions.keys())
	{
		int newVersion = newLogoVersions[gid];
		if (newVersion == kGroupInitVersion)
			continue;

		bool needRequest = true;
		if (oldLogoVersions.contains(gid))
		{
			int oldVersion = oldLogoVersions[gid];
			if (oldVersion == newVersion)
			{
				QString filePath = groupManager->logoPath(gid);
				QImage image;
				if (image.load(filePath, "jpg"))
				{
					m_pModelManager->setGroupLogo(gid, image);
					needRequest = false;
				}
			}
		}

		if (needRequest)
			groupManager->requestLogo(gid);
	}
}

void PmApp::initDateTime()
{
	QDateTime dt = QDateTime::currentDateTime();
	QTimeZone tz = dt.timeZone();
	int seconds = tz.offsetFromUtc(dt);
	CDateTime::setTimeZoneOffsets(seconds);
}

void PmApp::initSslConfiguration()
{
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();  
	config.setPeerVerifyMode(QSslSocket::VerifyPeer);  
	config.setProtocol(QSsl::TlsV1_2);
	QList<QSslCertificate> caCerts = QSslCertificate::fromPath(QString::fromUtf8(":/sslcerts/gd_bundle-g2-g1.crt"));
	config.setCaCertificates(caCerts);
	QSslConfiguration::setDefaultConfiguration(config); 
}

void PmApp::initDpiScale()
{
#ifdef Q_OS_WIN
	bool dpiOK = false;
	do {
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		dm.dmDriverExtra = 0;
		if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
			break;

		int devWidth = dm.dmPelsWidth;
		int visualWidth = GetSystemMetrics(SM_CXSCREEN);
		m_dpiScale = ((qreal)devWidth)/(qreal)(visualWidth);

		dpiOK = true;

	} while(0);

	if (!dpiOK)
		m_dpiScale = 1.0;
#else
	m_dpiScale = 1.0;
#endif // Q_OS_WIN
}
