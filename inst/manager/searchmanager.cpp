#include "searchmanager.h"
#include "pmclient/PmClient.h"
#include <QDebug>
#include "PmApp.h"
#include "http/HttpPool.h"
#include "util/JsonParser.h"
#include "settings/GlobalSettings.h"
#include <QMultiMap>

SearchManager::SearchManager(QObject *parent)
	: QObject(parent)
{
	m_searchResult = new SearchResult();
	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

SearchManager::~SearchManager()
{
	delete m_searchResult;
	m_searchResult = 0;
}

void SearchManager::conditionSearch(const QString &name, const QString &sex, const QString &phone, int page, int size)
{
	QMultiMap<QString, QString> params;
	params.insert("username", name);
	params.insert("sex", sex);
	params.insert("phone", phone);
	params.insert("currentPage", QString::number(page));
	params.insert("pageSize", QString::number(size));

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/search").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_searchIds.append(requestId);
}

void SearchManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_searchIds.contains(requestId))
	{
		return;
	}

	m_searchIds.removeAll(requestId);

	if (error)
	{
		qWarning() << Q_FUNC_INFO << "search network error: " << httpCode;
		m_lastError = tr("Network error, code:%1").arg(httpCode);
		emit searchFinished(false);
		return;
	}

	bool err = true;
	QString errMsg;
	QVariant datas = JsonParser::parse(recvData, err, errMsg);
	if (err)
	{
		qWarning() << Q_FUNC_INFO << errMsg;
		m_lastError = errMsg;
		emit searchFinished(false);
		return;
	}

	m_lastError.clear();
	QVariantMap vmap = datas.toMap();
	m_searchResult->m_searchName = vmap["searchUsername"].toString();
	m_searchResult->m_searchSex = vmap["searchSex"].toString();
	m_searchResult->m_searchPhone = vmap["searchPhone"].toString();
	m_searchResult->m_currentPage = vmap["currentPage"].toInt();
	m_searchResult->m_pageSize = vmap["pageSize"].toInt();
	m_searchResult->m_count = vmap["count"].toInt();
	m_searchResult->m_items.clear();

	QString id;
	QString name;
	QString sSex;
	int     sex;
	QString depart;
	QString phone;
	QVariantList items = vmap["searchResult"].toList();
	foreach (QVariant vItem, items)
	{
		QVariantMap mapItem = vItem.toMap();
		id = mapItem["id"].toString();
		name = mapItem["username"].toString();
		sSex = mapItem["sex"].toString();
		if (sSex.isEmpty())
			sex = 9;
		else 
			sex = sSex.toInt();
		phone = mapItem["cellNumber"].toString();
		QVariantList departList = mapItem["departmentList"].toList();
		depart.clear();
		foreach (QVariant vDepart, departList)
		{
			QVariantMap mapDepart = vDepart.toMap();
			depart.append(mapDepart["name"].toString());
			depart.append(",");
		}
		if (!depart.isEmpty())
			depart = depart.left(depart.length()-1);

		SearchManager::SearchItem item(id, name, sex, depart, phone);
		item.m_userState = mapItem["userState"].toInt();
		item.m_frozen = mapItem["isFrozen"].toInt();
		m_searchResult->m_items.append(item);
	}
	
	emit searchFinished(true);
}
