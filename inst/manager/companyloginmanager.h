#ifndef COMPANYLOGINMANAGER_H
#define COMPANYLOGINMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include "Account.h"

class QNetworkAccessManager;
class QNetworkReply;

class CompanyLoginManager : public QObject
{
	Q_OBJECT

public:
	enum PhoneStatus
	{
		NoRegister = -1,
		NoPass = 0,
		HavePass = 1,
		NoAuditNoPass = 2,
		AuditNoPass = 3
	};

public:
	CompanyLoginManager(QObject *parent = 0);
	~CompanyLoginManager();

	void phonePassStatus(const QString &phone);
	void cancelPhonePassStatus();

	void phonePassCode(const QString &phone);
	void cancelPhonePassCode();

	void setPhonePass(const QString &phone, const QString &code, const QString &pass);
	void cancelSetPhonePass();

	void joinCompanyCode(const QString &phone);
	void cancelJoinCompanyCode();

	void joinCompany(const QString &phone, const QString &code, const QString &uid);
	void cancelJoinCompany();

	void login(const QString &countryCode, const QString &phone, const QString &password);
	void cancelLogin();

	QString countryCode() const;
	QString phone() const;
	QString passwd() const;

	QList<CompanyInfo> companys() const;
	QString apiKey() const;
	QString securityKey() const;
	int expireTime() const;
	void reset();

Q_SIGNALS:
	void phonePassStatusOK(int status);
	void phonePassStatusFailed(int retCode, const QString &desc);

	void phonePassCodeOK();
	void phonePassCodeFailed(int retCode, const QString &desc);

	void setPhonePassOK();
	void setPhonePassFailed(int retCode, const QString &desc);

	void joinCompanyCodeOK();
	void joinCompanyCodeFailed(int retCode, const QString &desc);

	void joinCompanyOK();
	void joinCompanyFailed(int retCode, const QString &desc);

	void companyLoginOK();
	void companyLoginFailed(int retCode, const QString &desc);

private Q_SLOTS:
	void onReplyFinished();

	void onTimeout();

private:
	QNetworkAccessManager      *m_networkAccessManager;

	QNetworkReply              *m_phonePassStatusNetworkReply;
	QTimer                      m_phonePassStatusTimer;

	QNetworkReply              *m_phonePassCodeNetworkReply;
	QTimer                      m_phonePassCodeTimer;

	QNetworkReply              *m_setPhonePassNetworkReply;
	QTimer                      m_setPhonePassTimer;

	QNetworkReply              *m_joinCompanyCodeNetworkReply;
	QTimer                      m_joinCompanyCodeTimer;

	QNetworkReply              *m_joinCompanyNetworkReply;
	QTimer                      m_joinCompanyTimer;
	
	QNetworkReply              *m_loginNetworkReply;
	QTimer                      m_loginTimer;
	
	QList<CompanyInfo>          m_companys;
	QString                     m_apiKey;
	QString                     m_securityKey;
	int                         m_expireTime;
	QString                     m_countryCode;
	QString                     m_phone;
	QString                     m_passwd;
};

#endif // COMPANYLOGINMANAGER_H
