#ifndef LOGINPROCESS_H
#define LOGINPROCESS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <string>
#include <list>
#include <map>

#include "base/Base.h"

#include "pmclient/PmClientInterface.h"

class ILoginProcess;

class CLoginMgr 
	: public QObject
	, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

	enum LoginState {
		State_Invalid = 0,
		State_Error,
		State_Start,
		State_CommonConfig,
		State_CompanyLogin,
		State_CompanyLogined,
		State_Connect,
		State_Login,
		State_Synctime,
		State_DatabaseUpdate,
		State_Organization,
		State_Logining,
		State_Logined,
		State_Logout
	};

public:
	explicit CLoginMgr(QObject *parent = 0);
	~CLoginMgr();

public:
	bool login();
	void dislogin();
	void logout(bool bSendLogout = true);
	bool isLogining() const;
	bool isLogined() const;
	bool hasError() const;
	bool registerLoginProcess(ILoginProcess *process);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

Q_SIGNALS:
	void loginStatus(const QString& status); // 登录状态改变
	void loginError(const QString& err);     // 登录过程出错

	void aboutLogin();
	void aboutDislogin();
	void aboutLogout();

	void validateFailed(); // 用户名密码验证失败
	void validated();      // 用户名密码验证成功
	void logined();        // 登录成功了
	void dislogined();     // 登录过程中被取消了
	void logouted();       // 登录成功之后登出
	void beLogined();      // 别人已经登录了

	void updateVersion(const QString &updateVer, const QString &updateDesc, const QString &updateUrl);

	void companyLogined();
	void companyLoginFailed(const QString &desc);
	
public Q_SLOTS:
	void resetPsg(const QStringList& psgs);
	void savePsg(const QStringList& psgs);

private Q_SLOTS:
	void onPmclientOpened();
	void onPmclientClosed();
	void onPmclientError(const QString& err);

	void onProcessFinished();
	void onProcessError(const QString &err);

	void onCommonConfigFinished(bool ok, const QString &desc);

	void onCompanyLoginOK();
	void onCompanyLoginFailed(int retCode, const QString &desc);

private Q_SLOTS:
	void callCommonConfig();
	void callCompanyLogin();
	void callConnect();
	void callLogin();                                             /// 登录验证
	void callDatabaseUpdate();                                    /// 检测数据库升级
	void callTimesync();                                          /// 同步时间
	void callOrganization();                                      /// 获取机构
	void callLogout();                                            /// 登出

	void startLoginProcess();

	void onLogouted();

	void onLoginError(const QString& errmsg);

	void onLogined();

	void relogin();

	void onOrgFinished();

	void validateError();

	void sameLogined();

private:
	QString getRequestNameString(int reqType);
	bool processResponseError(net::Request* req);
	void processLogin(net::Request* req, protocol::Response* res);
	void processSynctime(net::Request* req, protocol::Response* res);
	void processLogout(net::Request* req, protocol::Response* res);
	void preparePsg();
	void prepareCompanySetting();

private:
	LoginState    m_eLoginState;
	volatile bool m_bStopLogin;
	int           m_nHandleId;

	QList<QString>                m_listRequestLoginProcess;
	QMap<QString, ILoginProcess*> m_mapLoginProcessor;
};

#endif // LOGINPROCESS_H
