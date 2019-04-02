#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QStandardItemModel>

#include "application/Logger.h"
using namespace application;

#include "net/Request.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"
#include "protocol/LoginRequest.h"
#include "protocol/LoginResponse.h"
#include "protocol/TimesyncRequest.h"
#include "protocol/TimesyncResponse.h"
#include "protocol/LogoutRequest.h"
#include "protocol/LogoutResponse.h"

#include "manager/organizationmanager.h"

#include "common/datetime.h"

#include "pmclient/PmClient.h"

#include "model/ModelManager.h"
#include "Constants.h"
#include "Account.h"
#include "settings/GlobalSettings.h"
#include "PsgManager.h"
#include "PmApp.h"
#include "logger/logger.h"
#include "login/ILoginManager.h"
#include "util/PasswdUtil.h"
#include "commonconfigmanager.h"
#include "companyloginmanager.h"
#include "loginmgr.h"
#include "groupsmembermanager.h"
#include "rtc/rtcsessionmanager.h"

CLoginMgr::CLoginMgr( QObject *parent /*= 0*/ ) : QObject(parent)
, m_nHandleId(-1)
, m_eLoginState(State_Invalid)
, m_bStopLogin(false)
{
}

CLoginMgr::~CLoginMgr()
{
	qDebug() << Q_FUNC_INFO;
}

bool CLoginMgr::login()
{
	if (!(m_eLoginState == State_Logout || 
		  m_eLoginState == State_Error || 
		  m_eLoginState == State_Invalid || 
		  m_eLoginState == State_CompanyLogined))
	{
		return false;
	}

	m_bStopLogin = false;

	if (m_eLoginState != State_CompanyLogined)
	{
		m_eLoginState = State_Start;
		emit aboutLogin();

		callCommonConfig();
	}
	else
	{
		prepareCompanySetting();

		preparePsg();

		callConnect();
	}

	return true;
}

void CLoginMgr::dislogin()
{
	if (!isLogining())
		return;

	emit aboutDislogin();

	// do dislogin
	m_bStopLogin = true;
	if (m_eLoginState == State_CommonConfig)
	{
		qPmApp->getCommonConfigManager()->cancelCommonConfig();
	}
	else if (m_eLoginState == State_CompanyLogin)
	{
		qPmApp->getCompanyLoginManager()->cancelLogin();
	}
	else
	{
		if (m_eLoginState == State_Logining)
		{
			qPmApp->getGroupsMemberManager()->stopFetch();
		}
		PmClient::instance()->close();
	}
	m_eLoginState = State_Logout;
	
	emit dislogined();
}

void CLoginMgr::logout(bool bSendLogout /* = true */)
{
	emit aboutLogout();
	if (bSendLogout && PmClient::instance()->isOpened())
	{
		callLogout();
	}
	else
	{
		onLogouted();
	}
}

bool CLoginMgr::isLogining() const
{
	return m_eLoginState >= State_Start && m_eLoginState < State_Logined;
}

bool CLoginMgr::isLogined() const
{
	return m_eLoginState == State_Logined;
}

bool CLoginMgr::hasError() const
{
	return m_eLoginState == State_Error;
}

bool CLoginMgr::initObject()
{
	m_nHandleId = PmClient::instance()->insertResponseHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void CLoginMgr::removeObject()
{
	qDebug() << Q_FUNC_INFO << " handle: " << m_nHandleId;

	PmClient::instance()->removeResponseHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* CLoginMgr::instance()
{
	return this;
}

int CLoginMgr::handledId() const
{
	return m_nHandleId;
}

QList<int> CLoginMgr::types() const
{
	QList<int> ret;
	ret << protocol::Request_Base_Login;
	ret << protocol::Request_Base_Timesync;
	ret << protocol::Request_Base_Logout;

	return ret;
}

bool CLoginMgr::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_nHandleId != handleId)
	{
		return false;
	}

	int type = req->getType();
	do 
	{
		if (m_bStopLogin && type != protocol::Request_Base_Logout)
		{
			break;
		}

		// process
		switch (type)
		{
		case protocol::Request_Base_Login:
			{
				processLogin(req, res);
			}
			break;
		case protocol::Request_Base_Timesync:
			{
				processSynctime(req, res);
			}
			break;
		case protocol::Request_Base_Logout:
			{
				processLogout(req, res);
			}
			break;
		default:
			{
				qWarning() << Q_FUNC_INFO << "error";
			}
		}
	} while (0);
	
	return true;
}

void CLoginMgr::resetPsg(const QStringList& psgs)
{
	if (m_bStopLogin)
		return;

	PmClient::instance()->close();
	m_eLoginState = State_Logout;

	savePsg(psgs);

	// 重置index
	PsgManager::instance().reset();

	// re-login
	QTimer::singleShot(0, this, SLOT(relogin()));
}

void CLoginMgr::savePsg(const QStringList& psgs)
{
	qDebug() << Q_FUNC_INFO << psgs;

	// update all psgs
	PsgManager::instance().setPsgs(psgs);

	// set psgs setting
	GlobalSettings::setPsgs(Account::instance()->id(), psgs);

	// set net type
	GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
	base::Address connAddress = PmClient::instance()->address();
	QList<PsgManager::PSG> allPsgs = PsgManager::instance().psgs();
	foreach (PsgManager::PSG psgSetting, allPsgs)
	{
		if (psgSetting.outAddr.ip.compare(connAddress.ip) == 0)
		{
			curConfig.netType =	QString(OUT_ADDRESS_NAME);
			break;
		}
		else if (psgSetting.inAddr.ip.compare(connAddress.ip) == 0)
		{
			curConfig.netType = QString(IN_ADDRESS_NAME);
			break;
		}
	}
	GlobalSettings::updateLoginConfig(curConfig.name, curConfig);
}

void CLoginMgr::onPmclientOpened()
{
	// log connected address
	base::Address connectAddr = PmClient::instance()->address();
	qPmApp->getLogger()->debug(QString("connected address: %1:%2")
		.arg(QString::fromUtf8(connectAddr.ip.c_str())).arg(connectAddr.port));

	QTimer::singleShot(0, this, SLOT(callLogin()));
}

void CLoginMgr::onPmclientClosed()
{
	m_eLoginState = State_Invalid;
}

void CLoginMgr::onPmclientError(const QString& err)
{
	if (m_eLoginState == State_Connect && !PsgManager::instance().isEnd())
	{
		// relogin
		m_eLoginState = State_Invalid;
		QTimer::singleShot(0, this, SLOT(relogin()));

		return;
	}

	m_eLoginState = State_Error;
	emit loginError(err);
}

void CLoginMgr::callCommonConfig()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_CommonConfig;

	CommonConfigManager *commonConfigManager = qPmApp->getCommonConfigManager();
	connect(commonConfigManager, SIGNAL(commonConfigFinished(bool, QString)), this, SLOT(onCommonConfigFinished(bool, QString)), Qt::UniqueConnection);
	
	commonConfigManager->cancelCommonConfig();

	GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
	commonConfigManager->requestCommonConfig(curConfig.version);

	emit loginStatus(tr("Log in...")); // Get global setting...
}

void CLoginMgr::callCompanyLogin()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_CompanyLogin;
	
	CompanyLoginManager *companyLoginManager = qPmApp->getCompanyLoginManager();
	connect(companyLoginManager, SIGNAL(companyLoginOK()),
		this, SLOT(onCompanyLoginOK()), Qt::UniqueConnection);
	connect(companyLoginManager, SIGNAL(companyLoginFailed(int, QString)),
		this, SLOT(onCompanyLoginFailed(int, QString)), Qt::UniqueConnection);

	companyLoginManager->cancelLogin();

	Account *pAccount = Account::instance();
	companyLoginManager->login(pAccount->countryCode(), pAccount->loginPhone(), pAccount->pwd());

	emit loginStatus(tr("Log in...")); // Company login...
}

void CLoginMgr::callConnect()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_Connect;

	if (PmClient::instance()->isOpened())
	{
		QTimer::singleShot(0, this, SLOT(callLogin()));
	}
	else
	{
		// config connect address
		GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
		base::Address addr;
		do 
		{
			PsgManager::PSG psg = PsgManager::instance().getNextPsg();

			QString sNetType = loginConfig.netType;
			addr = psg.outAddr;
			if (sNetType == QString(IN_ADDRESS_NAME))
				addr = psg.inAddr;

			if (!addr.isValid())
				addr = psg.outAddr;

			break;

		} while (!PsgManager::instance().isEnd());

		qDebug() << Q_FUNC_INFO << QString::fromUtf8(addr.ip.c_str()) << addr.port;
		PmClient::instance()->setAddress(addr);

		// encrypt type
		PmClient::instance()->setEncryptType(loginConfig.encrypt);
		if (loginConfig.encrypt >= Encrypt_TLSV12)
		{
			QString caFilePath = QString::fromUtf8(":/ca/ca.crt");
			PmClient::instance()->setOpensslCertFiles(caFilePath);
		}

		// disconnect & connect
		disconnect(PmClient::instance(), SIGNAL(opened()), this, SLOT(onPmclientOpened()));
		disconnect(PmClient::instance(), SIGNAL(closed()), this, SLOT(onPmclientClosed()));
		disconnect(PmClient::instance(), SIGNAL(error(QString)), this, SLOT(onPmclientError(QString)));
		connect(PmClient::instance(), SIGNAL(opened()), SLOT(onPmclientOpened()));
		connect(PmClient::instance(), SIGNAL(closed()), SLOT(onPmclientClosed()));
		connect(PmClient::instance(), SIGNAL(error(QString)), SLOT(onPmclientError(QString)));

		emit loginStatus(tr("Log in...")); // Connect server...

		PmClient::instance()->open();
	}
}

void CLoginMgr::callLogin()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_Login;
	Account* pAccount = Account::instance();

	QByteArray passwd;
	if (!pAccount->pwd().isEmpty())
	{
		passwd = PasswdUtil::toCryptogramPasswd(pAccount->loginPhone(), pAccount->pwd());
	}
	else
	{
		passwd = QByteArray::fromBase64(pAccount->cryptoPwd().toLatin1());
	}

	GlobalSettings::LoginConfig curLoginConfig = GlobalSettings::curLoginConfig();
	protocol::LoginRequest* req = new protocol::LoginRequest(pAccount->id().toUtf8().constData(), passwd.constData(), 
		pAccount->resource().toUtf8().constData(), pAccount->platform().toUtf8().constData(), 
		pAccount->voilent(), curLoginConfig.loginWithBalance);
	Q_ASSERT(req != NULL);

	emit loginStatus(tr("Log in...")); // Login validating...
	PmClient::instance()->setId(pAccount->id().toUtf8());
	PmClient::instance()->send(req);
}

void CLoginMgr::callDatabaseUpdate()
{
	if (m_bStopLogin)
	{
		return;
	}

	// next step: time sync
	QTimer::singleShot(0, this, SLOT(callTimesync()));
}

void CLoginMgr::callTimesync()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_Synctime;

	protocol::TimesyncRequest* req = new protocol::TimesyncRequest();

	emit loginStatus(tr("Log in...")); // Sync time...

	// record begin time
	CDateTime::setBegin();
	
	PmClient::instance()->send(req);
}

void CLoginMgr::callOrganization()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_Organization;

	qPmApp->getOrganizationManager()->start();
	connect(qPmApp->getOrganizationManager(), SIGNAL(finished()), this, SLOT(onOrgFinished()), Qt::UniqueConnection);
	connect(qPmApp->getOrganizationManager(), SIGNAL(error(QString)), this, SLOT(onProcessError(QString)), Qt::UniqueConnection);

	emit loginStatus(tr("Log in...")); // Sync os...
}

void CLoginMgr::callLogout()
{
	m_eLoginState = State_Logout;
	protocol::LogoutRequest *req = new protocol::LogoutRequest();
	Q_ASSERT(req != NULL);

	emit loginStatus(tr("Log out..."));
	PmClient::instance()->send(req);
}

void CLoginMgr::onLogouted()
{
	// logouted
	PmClient::instance()->close();
	m_eLoginState = State_Logout;

	emit logouted();
}

void CLoginMgr::onLoginError(const QString& errmsg)
{
	// close client and set error state
	PmClient::instance()->close();
	m_eLoginState = State_Error;

	// notify login error
	emit loginError(errmsg);
}

void CLoginMgr::onLogined()
{
	qPmApp->getAccount()->setVoilent(true);
	qPmApp->getAccount()->setLogined();

	m_bStopLogin = false;
	m_eLoginState = State_Logined;
	
	emit logined();
}

void CLoginMgr::relogin()
{
	if (m_bStopLogin)
	{
		return;
	}

	if (!(m_eLoginState == State_Logout || m_eLoginState == State_Error || m_eLoginState == State_Invalid))
	{
		return;
	}

	// reset
	m_eLoginState = State_Start;

	emit aboutLogin();

	QTimer::singleShot(0, this, SLOT(callConnect()));
}

QString CLoginMgr::getRequestNameString(int reqType)
{
	QString ret = "";
	switch (reqType)
	{
	case protocol::Request_Base_Login:
		{
			ret = tr("Log in validate");
		}
		break;
	case protocol::Request_Base_Timesync:
		{
			ret = tr("Sync time");
		}
		break;
	case protocol::Request_Base_Logout:
		{
			ret = tr("Log out");
		}
		break;
	default:
		break;
	}

	return ret;
}

bool CLoginMgr::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		QString sReqTitle = getRequestNameString(req->getType());
		QString sError = QString::fromUtf8(req->getMessage().c_str());

		QString errmsg = QString("%1%2(%3)").arg(sReqTitle).arg(tr(" failed")).arg(sError);
		QMetaObject::invokeMethod(this, "onLoginError", Qt::QueuedConnection, Q_ARG(QString, errmsg));
	}

	return bError;
}

const int OTHER_LOGINED        = 20009;
const int CONNECT_OTHER_PSG    = 10022;
const int USER_PASSWORD_ERROR  = 20001;

void CLoginMgr::processLogin(net::Request* req, protocol::Response* res)
{
	QStringList ret;

	protocol::LoginResponse *pRes = static_cast<protocol::LoginResponse *>(res);
	if (pRes)
	{
		std::list<std::string> psgs = pRes->getPsgs();

		std::list<std::string>::iterator itr = psgs.begin();
		for (; itr != psgs.end(); ++itr)
		{
			ret << (itr->c_str());
		}
	}
	else
	{
		qWarning() << "res is null";
	}

	qDebug() << Q_FUNC_INFO << ret << req->getResult();

	if (!req->getResult())
	{
		if (res)
		{
			int code = QString::fromUtf8(res->getErrcode().c_str()).toInt();
			if (code == CONNECT_OTHER_PSG)
			{
				qDebug() << Q_FUNC_INFO << "connect other psg";

				QMetaObject::invokeMethod(this, "resetPsg", Q_ARG(QStringList, ret));
			}
			else if (code == USER_PASSWORD_ERROR)
			{
				qDebug() << Q_FUNC_INFO << "user password error";

				QMetaObject::invokeMethod(this, "validateError");
			}
			else if (code == OTHER_LOGINED)
			{
				qDebug() << Q_FUNC_INFO << "the user already logined";

				QMetaObject::invokeMethod(this, "sameLogined");
			}
			else
			{
				processResponseError(req);
			}
		}
		else
		{
			processResponseError(req);
		}

		return;
	}

	Q_ASSERT(pRes != NULL);

	// psgs
	QMetaObject::invokeMethod(this, "savePsg", Q_ARG(QStringList, ret));

	// modules
	QStringList modules;
	{
		std::list<std::string> mods = pRes->getModule();
		std::list<std::string>::iterator itr = mods.begin();
		for (; itr != mods.end(); ++itr)
		{
			modules << itr->c_str();
		}
	}

	// services
	QMap<QString, base::AddressMap> services;
	{
		std::map<std::string, base::AddressMap> mapSer = pRes->getMapService();
		std::map<std::string, base::AddressMap>::iterator itr = mapSer.begin();
		for (; itr != mapSer.end(); ++itr)
		{
			services.insert(QString::fromUtf8(itr->first.c_str()), itr->second);
		}
	}

	QMetaObject::invokeMethod(qPmApp->getAccount(), "setModules", Qt::QueuedConnection, Q_ARG(QStringList, modules));
	QMetaObject::invokeMethod(qPmApp->getAccount(), "setServiceParameter", Qt::QueuedConnection, QArgument<QMap<QString,base::AddressMap> >("QMap<QString,base::AddressMap>", services));
	QMetaObject::invokeMethod(PmClient::instance(), "startHeartbeat", Qt::QueuedConnection);

	// rtc param
	protocol::LoginResponse::RtcParam rtcParam = pRes->getRtcParam();
	QStringList iceServers;
	for(int k = 0; k < (int)(rtcParam.urls.size()); ++k)
	{
		iceServers.append(qPrintable(rtcParam.urls[k].c_str()));
	}
	QString username = qPrintable(rtcParam.username.c_str());
	QString credential = qPrintable(rtcParam.credential.c_str());
	qPmApp->getRtcSessionManager()->setIceServerInfo(iceServers, username, credential);

	// notify validated
	QMetaObject::invokeMethod(this, "validated", Qt::QueuedConnection);

	// next step: database update check
	QMetaObject::invokeMethod(this, "callDatabaseUpdate", Qt::QueuedConnection);
}

void CLoginMgr::processSynctime(net::Request* req, protocol::Response* res)
{
	if (processResponseError(req))
	{
		// error
		return;
	}

	protocol::TimesyncResponse* pRes = static_cast<protocol::TimesyncResponse*>(res);
	Q_ASSERT(pRes != NULL);

	QString sServerTime = QString::fromUtf8(pRes->getTime().c_str());
	if (!CDateTime::setBaseDateTime(sServerTime))
	{
		qWarning() << Q_FUNC_INFO << QString("CDateTime::setBaseDateTime failed. server time: %1ms").arg(sServerTime);
	}

	// next step: organization
	QMetaObject::invokeMethod(this, "callOrganization", Qt::QueuedConnection);
}

void CLoginMgr::onOrgFinished()
{
	if (m_bStopLogin)
	{
		return;
	}

	QTimer::singleShot(0, this, SLOT(startLoginProcess()));
}

void CLoginMgr::validateError()
{
	// close client and set error state
	PmClient::instance()->close();
	m_eLoginState = State_Error;

	// notify login error
	emit validateFailed();
}

void CLoginMgr::sameLogined()
{
	// close client and set error state
	PmClient::instance()->close();
	m_eLoginState = State_Error;

	// notify be logined
	emit beLogined();
}

void CLoginMgr::processLogout(net::Request* req, protocol::Response* res)
{
	if (processResponseError(req))
	{
		// error
		return;
	}

	protocol::LogoutResponse* pRes = static_cast<protocol::LogoutResponse*>(res);
	if (!pRes)
		return;

	QMetaObject::invokeMethod(this, "onLogouted", Qt::QueuedConnection);
}

bool CLoginMgr::registerLoginProcess( ILoginProcess *process )
{
	if (!process)
		return false;

	QString name = process->name();
	if (m_mapLoginProcessor.contains(name) && m_mapLoginProcessor[name] == process)
	{
		return true;
	}
	else
	{
		m_mapLoginProcessor[name] = process;
	}

	disconnect(process->instance(), SIGNAL(finished()), this, SLOT(onProcessFinished()));
	disconnect(process->instance(), SIGNAL(error(QString)), this, SLOT(onProcessError(QString)));
	connect(process->instance(), SIGNAL(finished()), this, SLOT(onProcessFinished()));
	connect(process->instance(), SIGNAL(error(QString)), this, SLOT(onProcessError(QString)));

	return true;
}

void CLoginMgr::startLoginProcess()
{
	if (m_bStopLogin)
	{
		return;
	}

	m_eLoginState = State_Logining;

	emit loginStatus(tr("Log in..."));

	qPmApp->getGroupsMemberManager()->startFetch();

	m_listRequestLoginProcess.clear();
	foreach (QString name, m_mapLoginProcessor.keys())
	{
		m_listRequestLoginProcess.append(name);
		m_mapLoginProcessor[name]->start();
	}
}

void CLoginMgr::onProcessFinished()
{
	if (m_bStopLogin)
	{
		return;
	}

	QString key;
	foreach (ILoginProcess *process, m_mapLoginProcessor.values())
	{
		if (process->instance() == sender())
		{
			key = process->name();
			break;
		}
	}

	if (!key.isEmpty() && m_listRequestLoginProcess.contains(key))
	{
		m_listRequestLoginProcess.removeAll(key);

		// login
		if (m_listRequestLoginProcess.isEmpty())
		{
			onLogined();
		}
	}
}

void CLoginMgr::onProcessError( const QString &err )
{
	if (m_eLoginState == State_Error)
		return;

	// close client and set error state
	PmClient::instance()->close();
	m_eLoginState = State_Error;

	emit loginError(err);
}

void CLoginMgr::onCommonConfigFinished(bool ok, const QString &desc)
{
	if (ok)
	{
		// common setting ok
		CommonConfigManager::Config config = qPmApp->getCommonConfigManager()->config();

		// check if need update version
		if (CommonConfigManager::hasUpdate(config.updateVersion, QString::fromLatin1(APP_VERSION)))
		{
			emit updateVersion(config.updateVersion, config.updateDesc, config.updateUrl);
			return;
		}

		// update config
		GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
		if (curConfig.version.isEmpty() || (config.version != curConfig.version))
		{
			curConfig.version = config.version;
			curConfig.transferIp = config.transferIp;
			curConfig.transferPort = config.transferPort;
			if (config.encrypt == CONFIG_ENCRYPT_TLS12)
			{
				curConfig.encrypt = Encrypt_TLSV12;
			}
			else
			{
				curConfig.encrypt = Encrypt_None;
			}
			curConfig.loginWithBalance = ((config.loginLoadBalance == 1) ? true : false);
			curConfig.trackerServer = config.trackerServer;
			curConfig.fastdfsEnabled = config.fastdfsEnabled;
			GlobalSettings::updateLoginConfig(curConfig.name, curConfig);

			GlobalSettings::setAudioDisabled((config.audioDisabled == 1) ? true : false);
			GlobalSettings::setVideoDisabled((config.videoDisabled == 1) ? true : false);
			GlobalSettings::setRoamingMsgDisabled((config.roamingMsgDisabled == 1) ? true : false);
			GlobalSettings::setIntroductionViewType(config.introductionViewType);
			GlobalSettings::setSubscriptionDisabled((config.subscriptionDisabled == 1) ? true : false);
			GlobalSettings::setLinkItems(config.linkItems);
			GlobalSettings::setMaxDiscussMemberCount(config.maxDiscussMemberCount);
			GlobalSettings::setRosterSmallAvatar((config.rosterSmallAvatar == 1) ? true : false);
			GlobalSettings::setOsLoadAll((config.osLoadAll == 1) ? true : false);
			GlobalSettings::setOfflineSyncMsgEnabled((config.offlineSyncMsgEnabled == 1) ? true : false);
			GlobalSettings::setInterphoneDisabled((config.interphoneDisabled == 1) ? true : false);
			GlobalSettings::setMsgEncrypt(((config.msgEncrypt == 1) ? true : false));
			GlobalSettings::setMsgEncryptSeed(config.msgEncryptSeed);

			// common config changed need to reset psgs
			GlobalSettings::clearPsgs();
			GlobalSettings::setDefPsgs(config.psgs);
		}

		// login company
		QTimer::singleShot(0, this, SLOT(callCompanyLogin()));
	}
	else
	{
		// notify login error
		m_eLoginState = State_Error;
		emit loginError(tr("Get global setting failed(%1)").arg(desc));
	}
}

void CLoginMgr::onCompanyLoginOK()
{
	QString errMsg;

	// store all the ids to this phone
	CompanyLoginManager *companyLoginManager = qPmApp->getCompanyLoginManager();
	QList<CompanyInfo> companys = companyLoginManager->companys();
	if (companys.isEmpty())
	{
		// notify login error
		m_eLoginState = State_Error;
		errMsg = tr("Corporation failed(Phone does not join any corporation)");
		emit loginError(errMsg);

		emit companyLoginFailed(errMsg);
		return;
	}

	Account *pAccount = Account::instance();
	pAccount->setCompanyInfos(companys);
	m_eLoginState = State_CompanyLogined;

	if (!pAccount->id().isEmpty()) // already set id
	{
		// check if this company exists
		bool hasId = false;
		CompanyInfo company;
		foreach (company, companys)
		{
			if (company.uid == pAccount->id())
			{
				hasId = true;
				break;
			}
		}
		if (!hasId)
		{
			// notify login error
			m_eLoginState = State_Error;
			errMsg = tr("You are not in this corporation, please contact manager");
			emit loginError(errMsg);

			emit companyLoginFailed(errMsg);
			return;
		}

		if (company.frozen)
		{
			// notify login error
			m_eLoginState = State_Error;
			errMsg = tr("You are frozen in this corporation, please contact manager");
			emit loginError(errMsg);

			emit companyLoginFailed(errMsg);
			return;
		}

		prepareCompanySetting();

		preparePsg();

		QTimer::singleShot(0, this, SLOT(callConnect()));
	}
	else
	{
		// notify to select company to config out id
		emit companyLogined();
	}
}

void CLoginMgr::onCompanyLoginFailed(int retCode, const QString &desc)
{
	m_eLoginState = State_Error;

	// company login failed code is like 361024, 6 digitals & start with 361
	QString codeStr = QString::number(retCode);
	if (codeStr.length() == 6 && codeStr.startsWith("361"))
	{
		QString errMsg = tr("Corporation failed(The phone can't login a corporation)");
		if (codeStr == "361020")
			errMsg = tr("Corporation failed(Password error)");
		emit loginError(errMsg);

		emit companyLoginFailed(errMsg);
	}
	else
	{
		QString errMsg = tr("Corporation failed(%1)").arg(desc);
		emit loginError(errMsg);
	}
}

void CLoginMgr::preparePsg()
{
	QString id = Account::instance()->id();
	QStringList psgs = GlobalSettings::getPsgs(id);
	QStringList defPsgs = GlobalSettings::defPsgs();
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	if (loginConfig.loginWithBalance)
	{
		// login with balance connect only one address
		psgs.clear();
		psgs.append(defPsgs[0]);
	}
	else
	{
		// if psgs don't contain default psgs, add to it
		QString allPsgs = psgs.join(";");
		foreach (QString defPsg, defPsgs)
		{
			if (!allPsgs.contains(defPsg))
			{
				psgs.append(defPsg);
			}
		}
	}

	PsgManager::instance().reset();
	PsgManager::instance().setPsgs(psgs);
}

void CLoginMgr::prepareCompanySetting()
{
	QString id = Account::instance()->id();
	if (id.isEmpty())
		return;

	QList<CompanyInfo> companys = Account::instance()->companyInfos();
	foreach (CompanyInfo company, companys)
	{
		if (company.uid == id)
		{
			GlobalSettings::setCompanySettings(company.settings);
		}
	}
}
