#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QThread>
#include <QByteArray>

#include "PmApp.h"
#include "logger/logger.h"
#include "Account.h"
#include "ModelManager.h"

#include "db/OrgStructDB.h"
#include "model/orgstructitemdef.h"

#include "settings/GlobalSettings.h"
#include "http/HttpPool.h"
#include "organizationmanager.h"
#include "util/JsonParser.h"

//////////////////////////////////////////////////////////////////////////
// MEMBERS OF CLASS OrganizationManager
OrganizationManager::OrganizationManager(QObject *parent /*= 0*/)
: QObject(parent)
{
	// organization http pool, one time one request
	m_httpPool.reset(new HttpPool());
	m_httpPool->setParallel(1);
	m_httpPool->setRetryCount(0);

	// organization imminent http pool, one time at most three request
	m_imminentHttpPool.reset(new HttpPool());
	m_imminentHttpPool->setParallel(3);
	m_imminentHttpPool->setRetryCount(0);

	m_parser.reset(new StructOrgParser());

	bool connectOK = false;
	connectOK = connect(m_httpPool.data(), SIGNAL(logSent(QString)), qPmApp->getLogger(), SLOT(logSent(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_httpPool.data(), SIGNAL(logReceived(QString)), qPmApp->getLogger(), SLOT(logReceived(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_httpPool.data(), SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_imminentHttpPool.data(), SIGNAL(logSent(QString)), qPmApp->getLogger(), SLOT(logSent(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_imminentHttpPool.data(), SIGNAL(logReceived(QString)), qPmApp->getLogger(), SLOT(logReceived(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_imminentHttpPool.data(), SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_parser.data(), SIGNAL(parseOK(QString, QVariantList, bool)), 
		this, SLOT(onParseOK(QString, QVariantList, bool)), Qt::QueuedConnection);
	Q_ASSERT(connectOK);

	connectOK = connect(m_parser.data(), SIGNAL(parseFailed(QString)),
		this, SLOT(onParseFailed(QString)), Qt::QueuedConnection);
	Q_ASSERT(connectOK);
}

OrganizationManager::~OrganizationManager()
{
}

QObject* OrganizationManager::instance()
{
	return this;
}

QString OrganizationManager::name() const
{
	return "organization";
}

bool OrganizationManager::start()
{
	m_httpPool->start();
	m_imminentHttpPool->start();

	syncOrgStructDept();
	return true;
}

void OrganizationManager::syncOrgStructDept(const QString &deptId /*= QString()*/, 
	                                        bool imminent /*= false*/, 
											bool recursive /*= false*/)
{
	QString uid = Account::instance()->id();
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString;
	if (!deptId.isEmpty())
	{
		urlString = QString("%1/api/contact/%2/%3").arg(loginConfig.managerUrl).arg(uid).arg(deptId);
	}
	else
	{
		urlString = QString("%1/api/contact/%2").arg(loginConfig.managerUrl).arg(uid);
	}
	QUrl url = QUrl::fromUserInput(urlString);
	QMultiMap<QString, QString> params;
	if (recursive)
		params.insert("recursive", "true");
	int requestId = -1;
	if (!imminent)
		requestId = m_httpPool->addRequest(HttpRequest::GetRequest, url, params);
	else
		requestId = m_imminentHttpPool->addRequest(HttpRequest::GetRequest, url, params);
	OrganizationManager::RequestInfo requestInfo;
	requestInfo.deptId = deptId;
	requestInfo.recursive = recursive;
	m_requestDeptIds.insert(requestId, requestInfo);
}

void OrganizationManager::stopSyncOrgStructDept()
{
	m_imminentHttpPool->stop();
	m_httpPool->stop();
}

void OrganizationManager::onHttpRequestFinished(int id, bool err, int httpCode, const QByteArray &recvData)
{
	if (!m_requestDeptIds.contains(id))
		return;

	QString deptId = m_requestDeptIds[id].deptId;
	bool recursive = m_requestDeptIds[id].recursive;
	m_requestDeptIds.remove(id);

	bool imminent = false;
	HttpPool *pool = qobject_cast<HttpPool *>(sender());
	if (pool == m_imminentHttpPool.data())
		imminent = true;

	if (err) // network has error
	{
		QString errmsg = tr("Sync corporation failed(code:%1)").arg(httpCode);
		qWarning() << errmsg << deptId;

		emit syncOrgStructDeptFailed(deptId, errmsg);

		if (deptId.isEmpty())
		{
			emit error(errmsg);
		}

		return;
	}

	// parse the data
	parse(deptId, recvData, recursive, imminent);
}

void OrganizationManager::onParseOK(const QString &deptId, const QVariantList &dbItems, bool recursive)
{
	emit syncOrgStructDeptOk(deptId, dbItems, recursive);

	if (deptId.isEmpty())
	{
		emit finished();

		qPmApp->getModelManager()->setupOrgStructData(dbItems);
	}
}

void OrganizationManager::onParseFailed(const QString &deptId)
{
	QString errmsg = tr("Sync corporation failed(Data error: %1)").arg(deptId);
	emit syncOrgStructDeptFailed(deptId, errmsg);

	if (deptId.isEmpty())
	{
		emit error(errmsg);
	}
}

void OrganizationManager::parse(const QString &deptId, const QByteArray &recvData, bool recursive, bool imminent)
{
	if (recvData.isEmpty())
	{
		onParseOK(deptId, QVariantList(), recursive);
		return;
	}

	if (m_parser)
	{
		QMetaObject::invokeMethod(m_parser.data(), "parse", Qt::QueuedConnection, 
			Q_ARG(QString, deptId), Q_ARG(QByteArray, recvData), Q_ARG(bool, recursive), Q_ARG(bool ,imminent));
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS StructOrgParser
StructOrgParser::StructOrgParser(QObject *parent /*= 0*/)
: QObject(parent)
{
	moveToThread(&m_thread);
	m_thread.start();
}

void StructOrgParser::parse(const QString &deptId, const QByteArray &recvData, bool recursive, bool imminent)
{
	// parse from json data
	bool err = true;
	QString errMsg;
	QVariant var = JsonParser::parse(recvData, err, errMsg);
	if (err)
	{
		qWarning() << Q_FUNC_INFO << errMsg;
		emit parseFailed(deptId);
		return;
	}

	QVariantList items = var.toList();
	doParse(deptId, items, recursive, imminent);
}

void StructOrgParser::doParse(const QString &deptId, const QVariantList &items, bool recursive, bool imminent)
{
	/*
	item:   [id]       [pid] [type] [name] [index] [userState] [isFrozen]
	db map: [id] [uid] [pid] [type] [name] [index] [userstate] [frozen]
	*/
	QString pid = deptId;
	if (pid.isEmpty())
		pid = OrgStructConst::s_invisibleDepartId;

	QMap<QString, QVariantList> childDepts;
	QVariantMap dbMaps;
	foreach (QVariant v, items)
	{
		QVariantMap item = v.toMap();
		QVariantMap dbItem = item;
		dbItem["uid"] = item["id"];
		dbItem["pid"] = pid;
		if (dbItem["type"] == OrgStructConst::s_userFlag)
		{
			dbItem["id"] = QString("%1_%2").arg(pid).arg(dbItem["uid"].toString());
			dbItem["userstate"] = item["userState"];
			dbItem.remove("userState");
			dbItem["frozen"] = item["isFrozen"];
			dbItem.remove("isFrozen");
		}
		else //  OrgStructConst::s_departFlag
		{
			QString childDept = item["id"].toString();
			QVariantList children = item["children"].toList();
			childDepts.insert(childDept, children);
		}

		dbItem.remove("children");
		dbMaps.insert(dbItem["id"].toString(), dbItem);
	}

	QVariantList dbItems = dbMaps.values();
	emit parseOK(deptId, dbItems, (imminent ? true : recursive)); // if imminent do not fetch children

	// parse children
	if (recursive)
	{
		foreach (QString childDept, childDepts.keys())
		{
			QVariantList children = childDepts[childDept];
			doParse(childDept, children, recursive, imminent);
		}
	}
}
