#ifndef ORGANIZATIONMANAGER_H
#define ORGANIZATIONMANAGER_H

#include <QVariant>
#include <QObject>
#include <QScopedPointer>
#include "pmclient/PmClientInterface.h"
#include "login/ILoginManager.h"
#include <QThread>

class OrgStructModel;
class HttpPool;
class StructOrgParser;

class OrganizationManager : public QObject, public ILoginProcess
{
	Q_OBJECT
	Q_INTERFACES(ILoginProcess);
	
	struct RequestInfo
	{
		QString deptId;
		bool    recursive;  
	};

public:
	explicit OrganizationManager(QObject *parent = 0);
	virtual ~OrganizationManager();

public:
	// ILoginProcess
	virtual QObject* instance();
	virtual QString name() const;
	virtual bool start();

Q_SIGNALS:
	void finished();
	void error(const QString &err);

public slots:
	void syncOrgStructDept(const QString &deptId = QString(), bool imminent = false, bool recursive = false);
	void stopSyncOrgStructDept();

Q_SIGNALS:
	void syncOrgStructDeptOk(const QString &deptId, const QVariantList &dbItems, bool recursive);
	void syncOrgStructDeptFailed(const QString &deptId, const QString &err);

private slots:
	void onHttpRequestFinished(int id, bool err, int httpCode, const QByteArray &recvData);
	void onParseOK(const QString &deptId, const QVariantList &dbItems, bool recursive);
	void onParseFailed(const QString &deptId);

private:
	void parse(const QString &deptId, const QByteArray &recvData, bool recursive, bool imminent);

private:
	QScopedPointer<HttpPool>                    m_httpPool;
	QScopedPointer<HttpPool>                    m_imminentHttpPool;
	QMap<int, OrganizationManager::RequestInfo> m_requestDeptIds;
	QScopedPointer<StructOrgParser>             m_parser;
};

class StructOrgParser : public QObject
{
	Q_OBJECT
public:
	StructOrgParser(QObject *parent = 0);

Q_SIGNALS:
	void parseOK(const QString &deptId, const QVariantList &dbItems, bool recursive);
	void parseFailed(const QString &deptId);

public slots:
	void parse(const QString &deptId, const QByteArray &recvData, bool recursive, bool imminent);

private:
	void doParse(const QString &deptId, const QVariantList &items, bool recursive, bool imminent);

private:
	QThread m_thread;
};

#endif // ORGANIZATIONMANAGER_H
