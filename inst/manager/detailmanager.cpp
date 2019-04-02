#include "detailmanager.h"
#include "settings/GlobalSettings.h"
#include "http/HttpPool.h"
#include "util/JsonParser.h"
#include <QDebug>
#include "bean/DetailItem.h"
#include <QStringList>

DetailManager::DetailManager(HttpPool &httpPool, QObject *parent)
	: QObject(parent), m_httpPool(httpPool)
{
	connect(&m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

DetailManager::~DetailManager()
{

}

void DetailManager::syncVersions(const QStringList &uids)
{
	if (uids.isEmpty())
		return;

	m_versions.clear();
	foreach (QString uid, uids)
	{
		m_versions.insert(uid, 0);
	}

	QString uidsParam = uids.join(",");
	QMultiMap<QString, QString> params;
	params.insert("ids", uidsParam);
	
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/version/query").arg(loginConfig.managerUrl);
	int requestId = m_httpPool.addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_requestVersions.append(requestId);
}

void DetailManager::syncDetail(const QString &uid)
{
	if (uid.isEmpty())
		return;

	if (m_requestDetails.values().contains(uid))
		return;

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/detail/%2").arg(loginConfig.managerUrl).arg(uid);
	int requestId = m_httpPool.addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requestDetails.insert(requestId, uid);
}

void DetailManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (m_requestVersions.contains(requestId))
	{
		m_requestVersions.removeAll(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "detail version request failed: " << httpCode;
			return;
		}

		bool err = true;
		QString errMsg;
		QVariant datas = JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << errMsg;
			return;
		}

		QVariantList userVersionList = datas.toMap().value("userVersionList").toList();
		QString uid;
		int version;
		for (int i = 0; i < userVersionList.count(); i++)
		{
			QVariantMap userVersion = userVersionList[i].toMap();
			uid = userVersion["id"].toString();
			if (userVersion["version"].isNull())
				version = 0;
			else
				version = userVersion["version"].toInt();
			m_versions.insert(uid, version);
		}

		emit getVersionsFinished(m_versions);

		return;
	}

	if (m_requestDetails.contains(requestId))
	{
		QString uid = m_requestDetails[requestId];
		m_requestDetails.remove(requestId);

		bool err = true;
		QString errMsg;
		QVariant datas = JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << errMsg;
			return;
		}
		
		bean::DetailItem *detail = bean::DetailItemFactory::createItem();

		// parse from result
		QVariantMap vmap = datas.toMap();
		if (vmap["version"].isNull())
		{
			detail->setVersion(0);
			detail->setUid(uid);
			detail->setDisabled(true);
		}
		else
		{
			vmap["name"] = vmap["username"];
			vmap.remove("username");

			vmap["uid"] = vmap["id"];

			vmap["updatetime"] = vmap["updateTime"];
			vmap.remove("updateTime");

			vmap["jobdesc"] = vmap["jobDesc"];
			vmap.remove("jobDesc");

			QVariantList deptList = vmap["departmentList"].toList();
			QStringList deptNames;
			QStringList organizationNames;
			for (int i = 0; i < deptList.count(); i++)
			{
				QVariantMap deptMap = deptList[i].toMap();
				deptNames.append(deptMap["name"].toString());
				organizationNames.append(deptMap["organization"].toString());
			}
			deptNames.removeDuplicates();
			QString depart = deptNames.join(",");
			vmap["depart"] = depart;

			organizationNames.removeDuplicates();
			QString organization = organizationNames.join(",");
			vmap["organization"] = organization;

			detail->fromDBMap(vmap);
		}
		
		emit getDetailFinished(detail);
		
		delete detail;
		detail = 0;

		return;
	}
}