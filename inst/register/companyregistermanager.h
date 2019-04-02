#ifndef COMPANYREGISTERMANAGER_H
#define COMPANYREGISTERMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class CompanyRegisterManager : public QObject
{
	Q_OBJECT

public:
	class CompanyItem
	{
	public:
		QString m_id;
		QString m_name;
	};

	class CustomerItem
	{
	public:
		QString m_id;
		QString m_country;
		QString m_province;
		QString m_region;
		QString m_name;
	};

public:
	CompanyRegisterManager(QObject *parent = 0);
	~CompanyRegisterManager();

public:
	void queryCompanyList();
	void cancelQueryCompanyList();

	void getCustomerList(const QString &language);
	void cancelGetCustomerList();

	void doRegister(const QString &name, const QString &phone, const QString &customerCompanyName, 
		const QString &companyId, const QString &email, const QString &jobTitle, const QString &memo, const QString &departId);
	void cancelRegister();

	QList<CompanyItem> companyList() const;
	QList<CustomerItem> customerList() const;

Q_SIGNALS:
	void queryCompanyListOK();
	void queryCompanyListFailed(int retCode, const QString &desc);

	void getCustomerListOK();
	void getCustomerListFailed(int retCode, const QString &desc);

	void registerOK();
	void registerFailed(int retCode, const QString &desc);

private Q_SLOTS:
	void onReplyFinished();
	void onTimeout();

private:
	QNetworkAccessManager      *m_networkAccessManager;

	QNetworkReply              *m_queryCompanyListNetworkReply;
	QTimer                      m_queryCompanyListTimer;

	QNetworkReply              *m_getCustomerListNetworkReply;
	QTimer                      m_getCustomerListTimer;

	QNetworkReply              *m_registerNetworkReply;
	QTimer                      m_registerTimer;

	QList<CompanyItem>          m_companyList;
	QList<CustomerItem>         m_customerList;
};

#endif // COMPANYREGISTERMANAGER_H
