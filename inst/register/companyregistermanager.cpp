#include "companyregistermanager.h"
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

CompanyRegisterManager::CompanyRegisterManager(QObject *parent)
	: QObject(parent)
	, m_queryCompanyListNetworkReply(0)
	, m_getCustomerListNetworkReply(0)
	, m_registerNetworkReply(0)
{
	m_networkAccessManager = new QNetworkAccessManager(this);

	bool connected = false;
	connected = connect(&m_queryCompanyListTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_getCustomerListTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);

	connected = connect(&m_registerTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	Q_ASSERT(connected);
}

CompanyRegisterManager::~CompanyRegisterManager()
{
	if (m_queryCompanyListNetworkReply)
		delete m_queryCompanyListNetworkReply;

	if (m_getCustomerListNetworkReply)
		delete m_getCustomerListNetworkReply;

	if (m_registerNetworkReply)
		delete m_registerNetworkReply;
}

void CompanyRegisterManager::queryCompanyList()
{
	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/company/querycompanylist").arg(loginConfig.managerUrl);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_queryCompanyListNetworkReply = m_networkAccessManager->get(request);
	connect(m_queryCompanyListNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_queryCompanyListTimer.start(20*1000); // 20s timeout
}

void CompanyRegisterManager::cancelQueryCompanyList()
{
	m_queryCompanyListTimer.stop();

	if (m_queryCompanyListNetworkReply)
	{
		delete m_queryCompanyListNetworkReply;
		m_queryCompanyListNetworkReply = 0;
	}
}

void CompanyRegisterManager::getCustomerList(const QString &language)
{
	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/ork/api/customers/GetCustomerList?language=%2")
		.arg(loginConfig.managerUrl).arg(language);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_getCustomerListNetworkReply = m_networkAccessManager->get(request);
	connect(m_getCustomerListNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_getCustomerListTimer.start(20*1000); // 20s timeout
}

void CompanyRegisterManager::cancelGetCustomerList()
{
	m_getCustomerListTimer.stop();

	if (m_getCustomerListNetworkReply)
	{
		delete m_getCustomerListNetworkReply;
		m_getCustomerListNetworkReply = 0;
	}
}

void CompanyRegisterManager::doRegister(const QString &name, const QString &phone, const QString &customerCompanyName, 
	const QString &companyId, const QString &email, const QString &jobTitle, const QString &memo, const QString &departId)
{
	QNetworkRequest request;
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/ork/registerUser/register?code=%2&name=%3&phone=%4&customerCompanyName=%5&companyId=%6&email=%7&jobTitle=%8&memo=%9&departId=%10")
							 .arg(loginConfig.managerUrl)
							 .arg("1234")
							 .arg(name)
							 .arg(phone)
							 .arg(customerCompanyName)
							 .arg(companyId)
							 .arg(email)
							 .arg(jobTitle)
							 .arg(memo)
							 .arg(departId);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_registerNetworkReply = m_networkAccessManager->get(request);
	connect(m_registerNetworkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_registerTimer.start(20*1000); // 20s timeout
}

void CompanyRegisterManager::cancelRegister()
{
	m_registerTimer.stop();

	if (m_registerNetworkReply)
	{
		delete m_registerNetworkReply;
		m_registerNetworkReply = 0;
	}
}

QList<CompanyRegisterManager::CompanyItem> CompanyRegisterManager::companyList() const
{
	return m_companyList;
}

QList<CompanyRegisterManager::CustomerItem> CompanyRegisterManager::customerList() const
{
	return m_customerList;
}

void CompanyRegisterManager::onReplyFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if (!reply)
		return;

	// stop timer
	m_queryCompanyListTimer.stop();
	m_getCustomerListTimer.stop();
	m_registerTimer.stop();

	// parse result
	int retCode = 0;
	QString desc;
	QVariant datasVariant;
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
		datasVariant = JsonParser::parse(rawData, err, desc, 0, &retCode);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << retCode << desc;
			break;
		}

		ok = true;

	} while(0);

	if (reply == m_queryCompanyListNetworkReply)
	{
		m_queryCompanyListNetworkReply->deleteLater();
		m_queryCompanyListNetworkReply = 0;

		if (!ok)
		{
			emit queryCompanyListFailed(retCode, desc);
			return;
		}

		// parse
		m_companyList.clear();
		QVariantMap datas = datasVariant.toMap();
		QVariantList companyList = datas["companyList"].toList();
		foreach (QVariant companyV, companyList)
		{
			QVariantMap companyM = companyV.toMap();
			CompanyItem item;
			item.m_id = QString::number(companyM["id"].toInt());
			item.m_name = companyM["name"].toString().trimmed();
			m_companyList.append(item);
		}

		emit queryCompanyListOK();
	}
	else if (reply == m_getCustomerListNetworkReply)
	{
		m_customerList.clear();

		m_getCustomerListNetworkReply->deleteLater();
		m_getCustomerListNetworkReply = 0;

		if (!ok)
		{
			emit getCustomerListFailed(retCode, desc);
			return;
		}

		// parse
		m_customerList.clear();
		QVariantList customerList = datasVariant.toList();
		foreach (QVariant customerV, customerList)
		{
			QVariantMap customerM = customerV.toMap();
			CustomerItem item;
			item.m_id = QString::number(customerM["Id"].toInt());
			item.m_country = customerM["Country"].toString().trimmed();
			item.m_province = customerM["Province"].toString().trimmed();
			item.m_region = customerM["Region"].toString().trimmed();
			item.m_name = customerM["Name"].toString().trimmed();
			m_customerList.append(item);
		}

		emit getCustomerListOK();
	}
	else if (reply == m_registerNetworkReply)
	{
		m_registerNetworkReply->deleteLater();
		m_registerNetworkReply = 0;

		if (!ok)
		{
			emit registerFailed(retCode, desc);
			return;
		}

		emit registerOK();
	}
}

void CompanyRegisterManager::onTimeout()
{
	if (sender() == &m_queryCompanyListTimer)
	{
		qWarning() << Q_FUNC_INFO << "query company list request time out";

		m_queryCompanyListTimer.stop();
		if (m_queryCompanyListNetworkReply)
		{
			delete m_queryCompanyListNetworkReply;
			m_queryCompanyListNetworkReply = 0;
		}

		emit queryCompanyListFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_getCustomerListTimer)
	{
		qWarning() << Q_FUNC_INFO << "get customer list request time out";

		m_getCustomerListTimer.stop();
		if (m_getCustomerListNetworkReply)
		{
			delete m_getCustomerListNetworkReply;
			m_getCustomerListNetworkReply = 0;
		}

		emit getCustomerListFailed(0, tr("request timeout"));
	}
	else if (sender() == &m_registerTimer)
	{
		qWarning() << Q_FUNC_INFO << "register request time out";

		m_registerTimer.stop();
		if (m_registerNetworkReply)
		{
			delete m_registerNetworkReply;
			m_registerNetworkReply = 0;
		}

		emit registerFailed(0, tr("request timeout"));
	}
}