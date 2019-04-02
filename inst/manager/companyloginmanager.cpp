#include "companyloginmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QDebug>
#include "util/JsonParser.h"
#include "Constants.h"
#include "PmApp.h"
#include "logger/logger.h"
#include "settings/GlobalSettings.h"
#include "util/PasswdUtil.h"

CompanyLoginManager::CompanyLoginManager(QObject *parent /*= 0*/)
: QObject(parent)
, m_phonePassStatusNetworkReply(0)
, m_phonePassCodeNetworkReply(0)
, m_setPhonePassNetworkReply(0)
, m_joinCompanyCodeNetworkReply(0)
, m_joinCompanyNetworkReply(0)
, m_loginNetworkReply(0)
, m_expireTime(0)
{
	m_networkAccessManager = new QNetworkAccessManager(this);

	bool connected = false;
	connected = connect(&m_phonePassStatusTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_phonePassCodeTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_setPhonePassTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_joinCompanyCodeTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_joinCompanyTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_loginTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);
}

CompanyLoginManager::~CompanyLoginManager()
{
	if (m_phonePassStatusNetworkReply)
		delete m_phonePassStatusNetworkReply;

	if (m_phonePassCodeNetworkReply)
		delete m_phonePassCodeNetworkReply;

	if (m_setPhonePassNetworkReply)
		delete m_setPhonePassNetworkReply;

	if (m_joinCompanyCodeNetworkReply)
		delete m_joinCompanyCodeNetworkReply;

	if (m_joinCompanyNetworkReply)
		delete m_joinCompanyNetworkReply;

	if (m_loginNetworkReply)
		delete m_loginNetworkReply;
}

void CompanyLoginManager::phonePassStatus(const QString &phone)
{
	if (phone.isEmpty())
		return;

	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/phone/status?phone=%2")
		.arg(loginConfig.managerUrl).arg(phone);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_phonePassStatusNetworkReply = m_networkAccessManager->get(request);
	connect(m_phonePassStatusNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_phonePassStatusTimer.start(20*1000); // 20s timeout
}

void CompanyLoginManager::cancelPhonePassStatus()
{
	m_phonePassStatusTimer.stop();

	if (m_phonePassStatusNetworkReply)
	{
		delete m_phonePassStatusNetworkReply;
		m_phonePassStatusNetworkReply = 0;
	}
}

void CompanyLoginManager::phonePassCode(const QString &phone)
{
	if (phone.isEmpty())
		return;

	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/password/sms/send?phone=%2")
		.arg(loginConfig.managerUrl).arg(phone);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_phonePassCodeNetworkReply = m_networkAccessManager->get(request);
	connect(m_phonePassCodeNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_phonePassCodeTimer.start(20*1000); // 20s timeout
}

void CompanyLoginManager::cancelPhonePassCode()
{
	m_phonePassCodeTimer.stop();

	if (m_phonePassCodeNetworkReply)
	{
		delete m_phonePassCodeNetworkReply;
		m_phonePassCodeNetworkReply = 0;
	}
}

void CompanyLoginManager::setPhonePass(const QString &phone, const QString &code, const QString &pass)
{
	if (phone.isEmpty() || code.isEmpty() || pass.isEmpty())
		return;

	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/setpwd?phone=%2&code=%3&password=%4&type=0")
		.arg(loginConfig.managerUrl).arg(phone).arg(code).arg(pass);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_setPhonePassNetworkReply = m_networkAccessManager->get(request);
	connect(m_setPhonePassNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_setPhonePassTimer.start(20*1000); // 20s timeout
}

void CompanyLoginManager::cancelSetPhonePass()
{
	m_setPhonePassTimer.stop();

	if (m_setPhonePassNetworkReply)
	{
		delete m_setPhonePassNetworkReply;
		m_setPhonePassNetworkReply = 0;
	}
}

void CompanyLoginManager::joinCompanyCode(const QString &phone)
{
	if (phone.isEmpty())
		return;

	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/verification/sms/send?phone=%2")
		.arg(loginConfig.managerUrl).arg(phone);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_joinCompanyCodeNetworkReply = m_networkAccessManager->get(request);
	connect(m_joinCompanyCodeNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_joinCompanyCodeTimer.start(20*1000); // 20s timeout
}

void CompanyLoginManager::cancelJoinCompanyCode()
{
	m_joinCompanyCodeTimer.stop();

	if (m_joinCompanyCodeNetworkReply)
	{
		delete m_joinCompanyCodeNetworkReply;
		m_joinCompanyCodeNetworkReply = 0;
	}
}

void CompanyLoginManager::joinCompany(const QString &phone, const QString &code, const QString &uid)
{
	if (phone.isEmpty() || code.isEmpty() || uid.isEmpty())
		return;

	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/joincompany/verify?phone=%2&code=%3&userId=%4")
		.arg(loginConfig.managerUrl).arg(phone).arg(code).arg(uid);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_joinCompanyNetworkReply = m_networkAccessManager->get(request);
	connect(m_joinCompanyNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_joinCompanyTimer.start(20*1000); // 20s timeout
}

void CompanyLoginManager::cancelJoinCompany()
{
	m_joinCompanyTimer.stop();

	if (m_joinCompanyNetworkReply)
	{
		delete m_joinCompanyNetworkReply;
		m_joinCompanyNetworkReply = 0;
	}
}

void CompanyLoginManager::login(const QString &countryCode, const QString &phone, const QString &password)
{
	if (countryCode.isEmpty() || phone.isEmpty() || password.isEmpty())
		return;

	m_countryCode = countryCode;
	m_phone = phone;
	m_passwd = password;

	QByteArray encryptPwd = PasswdUtil::toCryptogramPasswd(phone, password);

	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/login?phone=%2&password=%3")
		.arg(loginConfig.managerUrl).arg(phone).arg(QString::fromLatin1(encryptPwd));
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_loginNetworkReply = m_networkAccessManager->get(request);
	connect(m_loginNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_loginTimer.start(20*1000); // 20s timeout
}

void CompanyLoginManager::cancelLogin()
{
	m_loginTimer.stop();

	if (m_loginNetworkReply)
	{
		delete m_loginNetworkReply;
		m_loginNetworkReply = 0;
	}
}

QString CompanyLoginManager::countryCode() const
{
	return m_countryCode;
}

QString CompanyLoginManager::phone() const
{
	return m_phone;
}

QString CompanyLoginManager::passwd() const
{
	return m_passwd;
}

QList<CompanyInfo> CompanyLoginManager::companys() const
{
	return m_companys;
}

QString CompanyLoginManager::apiKey() const
{
	return m_apiKey;
}

QString CompanyLoginManager::securityKey() const
{
	return m_securityKey;
}

int CompanyLoginManager::expireTime() const
{
	return m_expireTime;
}

void CompanyLoginManager::reset()
{
	m_companys.clear();
	m_apiKey.clear();
	m_securityKey.clear();
	m_expireTime = 0;
	m_countryCode.clear();
	m_phone.clear();
	m_passwd.clear();
}

void CompanyLoginManager::onReplyFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if (!reply)
		return;

	// stop timer
	m_phonePassStatusTimer.stop();
	m_phonePassCodeTimer.stop();
	m_setPhonePassTimer.stop();
	m_joinCompanyCodeTimer.stop();
	m_joinCompanyTimer.stop();
	m_loginTimer.stop();

	// parse result
	int retCode = 0;
	QString desc;
	QVariantMap datas;
	bool ok = false;
	do {
		if (reply->error() != QNetworkReply::NoError)
		{
			QString netErrorString = reply->errorString();
			int netCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			desc = QString("%1:%2").arg(tr("Network error")).arg(netCode);
			qWarning() << Q_FUNC_INFO << "http error:" << reply->url().toString() << netCode << netErrorString;

			qPmApp->getLogger()->debug(QString("http error: ") + reply->url().toString() + QString::number(netCode) + 
				QString("\n") + netErrorString + QString("\n"));

			retCode = netCode;
			break;
		}

		QByteArray rawData = reply->readAll();
		qPmApp->getLogger()->logReceived(reply->url().toString() + QString("\n") + QString::fromUtf8(rawData) + QString("\n"));

		bool err = true;
		QVariant datasVariant = JsonParser::parse(rawData, err, desc, 0, &retCode);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << retCode << desc;
			break;
		}

		datas = datasVariant.toMap();
		ok = true;

	} while(0);

	if (reply == m_phonePassStatusNetworkReply)
	{
		m_phonePassStatusNetworkReply->deleteLater();
		m_phonePassStatusNetworkReply = 0;

		if (!ok)
		{
			emit phonePassStatusFailed(retCode, desc);
			return;
		}

		int status = datas["status"].toInt();
		emit phonePassStatusOK(status);
	}
	else if (reply == m_phonePassCodeNetworkReply)
	{
		m_phonePassCodeNetworkReply->deleteLater();
		m_phonePassCodeNetworkReply = 0;

		if (!ok)
		{
			emit phonePassCodeFailed(retCode, desc);
			return;
		}

		emit phonePassCodeOK();
	}
	else if (reply == m_setPhonePassNetworkReply)
	{
		m_setPhonePassNetworkReply->deleteLater();
		m_setPhonePassNetworkReply = 0;

		if (!ok)
		{
			emit setPhonePassFailed(retCode, desc);
			return;
		}

		emit setPhonePassOK();
	}
	else if (reply == m_joinCompanyCodeNetworkReply)
	{
		m_joinCompanyCodeNetworkReply->deleteLater();
		m_joinCompanyCodeNetworkReply = 0;

		if (!ok)
		{
			emit joinCompanyCodeFailed(retCode, desc);
			return;
		}

		emit joinCompanyCodeOK();
	}
	else if (reply == m_joinCompanyNetworkReply)
	{
		m_joinCompanyNetworkReply->deleteLater();
		m_joinCompanyNetworkReply = 0;

		if (!ok)
		{
			emit joinCompanyFailed(retCode,desc);
			return;
		}

		emit joinCompanyOK();
	}
	else if (reply == m_loginNetworkReply)
	{
		m_loginNetworkReply->deleteLater();
		m_loginNetworkReply = 0;
		
		if (!ok)
		{
			emit companyLoginFailed(retCode, desc);
			return;
		}

		m_companys.clear();
		m_apiKey.clear();
		m_securityKey.clear();
		m_expireTime = 0;

		QVariantList users = datas["users"].toList();
		for (int k = 0; k < users.count(); ++k)
		{
			CompanyInfo info;
			QVariantMap user = users[k].toMap();
			info.uid = user["id"].toString();
			info.uname = user["username"].toString();
			info.userState = user["userState"].toInt();
			info.frozen = (user["isFrozen"].toInt() == 1);
			QVariantMap company = user["company"].toMap();
			info.companyId = company["id"].toString();
			info.companyName = company["name"].toString();

			QVariantList settingList = company["settingList"].toList();
			foreach (QVariant setting, settingList)
			{
				QVariantMap settingPair = setting.toMap();
				info.settings.insert(settingPair["key"].toString(), settingPair["value"].toString());
			}

			m_companys.append(info);
		}
		
		m_apiKey = datas["api_key"].toString();
		m_securityKey = datas["security_key"].toString();
		m_expireTime = datas["expire_time"].toInt();

		emit companyLoginOK();
	}
}

void CompanyLoginManager::onTimeout()
{
	if (sender() == &m_phonePassStatusTimer)
	{
		qWarning() << Q_FUNC_INFO << "phone password status request time out";

		m_phonePassStatusTimer.stop();
		if (m_phonePassStatusNetworkReply)
		{
			delete m_phonePassStatusNetworkReply;
			m_phonePassStatusNetworkReply = 0;
		}

		emit phonePassStatusFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_phonePassCodeTimer)
	{
		qWarning() << Q_FUNC_INFO << "phone password code request time out";

		m_phonePassCodeTimer.stop();
		if (m_phonePassCodeNetworkReply)
		{
			delete m_phonePassCodeNetworkReply;
			m_phonePassCodeNetworkReply = 0;
		}

		emit phonePassCodeFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_setPhonePassTimer)
	{
		qWarning() << Q_FUNC_INFO << "set phone password request time out";

		m_setPhonePassTimer.stop();
		if (m_setPhonePassNetworkReply)
		{
			delete m_setPhonePassNetworkReply;
			m_setPhonePassNetworkReply = 0;
		}

		emit setPhonePassFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_joinCompanyCodeTimer)
	{
		qWarning() << Q_FUNC_INFO << "join company code request time out";

		m_joinCompanyCodeTimer.stop();
		if (m_joinCompanyCodeNetworkReply)
		{
			delete m_joinCompanyCodeNetworkReply;
			m_joinCompanyCodeNetworkReply = 0;
		}

		emit joinCompanyCodeFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_joinCompanyTimer)
	{
		qWarning() << Q_FUNC_INFO << "join company request time out";

		m_joinCompanyTimer.stop();
		if (m_joinCompanyNetworkReply)
		{
			delete m_joinCompanyNetworkReply;
			m_joinCompanyNetworkReply = 0;
		}

		emit joinCompanyFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_loginTimer)
	{
		qWarning() << Q_FUNC_INFO << "company login request time out";

		m_loginTimer.stop();
		if (m_loginNetworkReply)
		{
			delete m_loginNetworkReply;
			m_loginNetworkReply = 0;
		}

		emit companyLoginFailed(0, tr("request timeout"));
	}
}