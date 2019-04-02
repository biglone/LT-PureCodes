#include "addfriendmanager.h"
#include "PmApp.h"
#include "http/HttpPool.h"
#include "settings/GlobalSettings.h"
#include <QDebug>
#include <QUrl>
#include <QVariantMap>
#include "Account.h"
#include "util/JsonParser.h"
#include "qt-json/json.h"

static const char *s_action_request = "request";
static const char *s_action_accept  = "accept";
static const char *s_action_refuse  = "refuse";

AddFriendManager::AddFriendManager(QObject *parent)
	: QObject(parent)
{
	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

AddFriendManager::~AddFriendManager()
{

}

bool AddFriendManager::requestAddFriendList()
{
	if (!m_refreshIds.isEmpty())
		return false;

	QMultiMap<QString, QString> params;
	params.insert("userId", Account::instance()->id());

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/rosteraction/query").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_refreshIds.append(requestId);
	return true;
}

bool AddFriendManager::addFriendAction(AddFriendManager::Action action, const QString &fromId, const QString &toId, 
									   const QString &sId, const QString &message, const QString &group, const QString &group1 /*= QString()*/)
{
	if (fromId.isEmpty() || toId.isEmpty() || sId.isEmpty() || group.isEmpty())
		return false;

	if (m_requestIds.values().contains(sId))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("fromId", fromId);
	params.insert("toId", toId);
	params.insert("message", message);
	params.insert("sessionId", sId);
	params.insert("action", actionToString(action));
	params.insert("group", group);
	if (!group1.isEmpty())
		params.insert("group1", group1);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/rosteraction/add").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_requestIds.insert(requestId, sId);
	return true;
}

bool AddFriendManager::addFriendConfirm(const QString &sId)
{
	if (sId.isEmpty())
		return false;

	if (m_confirmIds.values().contains(sId))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("sessionId", sId);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/rosteraction/confirm").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_confirmIds.insert(requestId, sId);
	return true;
}

bool AddFriendManager::addFriendRead(const QString &sId)
{
	if (sId.isEmpty())
		return false;

	if (m_readIds.values().contains(sId))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("sessionId", sId);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/rosteraction/read").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_readIds.insert(requestId, sId);
	return true;
}

bool AddFriendManager::addFriendDelete(const QString &sId, const QString &by)
{
	if (sId.isEmpty() || by.isEmpty())
		return false;

	if (m_deleteIds.values().contains(sId))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("sessionId", sId);
	params.insert("by", by);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/rosteraction/delete").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_deleteIds.insert(requestId, sId);
	return true;
}

// move to roster manager
bool AddFriendManager::deleteFriend(const QString &selfId, const QString &otherId)
{
	if (selfId.isEmpty() || otherId.isEmpty())
		return false;

	if (m_deleteFriendIds.values().contains(otherId))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("fromId", selfId);
	params.insert("toId", otherId);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/roster/delete").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_deleteFriendIds.insert(requestId, otherId);
	return true;
}
//

QList<AddFriendManager::Item> AddFriendManager::refreshItems() const
{
	return m_refreshItems;
}

AddFriendManager::Action AddFriendManager::stringToAction(const QString &str)
{
	if (str == QString(s_action_accept))
		return AddFriendManager::Accept;
	else if (str == QString(s_action_refuse))
		return AddFriendManager::Refuse;
	else
		return AddFriendManager::Request;
}

QString AddFriendManager::actionToString(AddFriendManager::Action action)
{
	if (action == AddFriendManager::Accept)
		return s_action_accept;
	else if (action == AddFriendManager::Refuse)
		return s_action_refuse;
	else
		return s_action_request;
}

void AddFriendManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (m_requestIds.contains(requestId))
	{
		QString sId = m_requestIds[requestId];
		m_requestIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "request failed: " << httpCode;
			emit addFriendRequestFailed(sId, tr("Network error: %1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		QVariant datas = JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "request failed: " << errMsg;
			emit addFriendRequestFailed(sId, errMsg);
			return;
		}

		QVariantMap vmap = datas.toMap();
		AddFriendManager::Action action = stringToAction(vmap["action"].toString());
		QString group = vmap["group"].toString();
		QString group1 = vmap["group1"].toString();

		emit addFriendRequestOK(sId, (int)action, group, group1);
		return;
	}

	if (m_refreshIds.contains(requestId))
	{
		m_refreshIds.clear();

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "refresh failed: " << httpCode;
			emit refreshFailed(tr("Network error: %1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		QVariant datas = JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "refresh failed: " << errMsg;
			emit refreshFailed(errMsg);
			return;
		}

		m_refreshItems.clear();
		QVariantMap vmap = datas.toMap();
		QVariantList actionList = vmap["actionList"].toList();
		QString fromId;
		QString fromName;
		QString toId;
		QString toName;
		QString sId;
		AddFriendManager::Action action;
		QString message;
		QString time;
		QString group;
		int status = 0;
		int read = 0;
		foreach (QVariant vAction, actionList)
		{
			QVariantMap actionItem = vAction.toMap();
			fromId = actionItem["fromId"].toString();
			fromName = actionItem["fromName"].toString();
			toId = actionItem["toId"].toString();
			toName = actionItem["toName"].toString();
			sId = actionItem["sessionId"].toString();
			action = stringToAction(actionItem["action"].toString());
			message = actionItem["message"].toString();
			time = actionItem["createTime"].toString();
			group = actionItem["group"].toString();
			status = actionItem["status"].toInt();
			read = actionItem["read"].toInt();
			
			AddFriendManager::Item item(fromId, fromName, toId, toName, sId, action, message, time, group, status, read);
			m_refreshItems.append(item);
		}

		emit refreshOK();
		return;
	}

	if (m_confirmIds.contains(requestId))
	{
		QString sId = m_confirmIds[requestId];
		m_confirmIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "confirm failed: " << httpCode;
			emit addFriendConfirmFailed(sId, tr("Network error: %1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "confirm failed: " << errMsg;
			emit addFriendConfirmFailed(sId, errMsg);
			return;
		}

		emit addFriendConfirmOK(sId);
		return;
	}

	if (m_readIds.contains(requestId))
	{
		QString sId = m_readIds[requestId];
		m_readIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "set read failed: " << httpCode;
			return;
		}

		bool err = true;
		QString errMsg;
		JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "set read failed: " << errMsg;
			return;
		}

		return;
	}

	if (m_deleteIds.contains(requestId))
	{
		QString sId = m_deleteIds[requestId];
		m_deleteIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "delete failed: " << httpCode;
			emit addFriendDeleteFailed(sId, tr("Network error: %1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "delete failed: " << errMsg;
			emit addFriendDeleteFailed(sId, errMsg);
			return;
		}

		emit addFriendDeleteOK(sId);
		return;
	}

	// move to roster manager
	if (m_deleteFriendIds.contains(requestId))
	{
		QString otherId = m_deleteFriendIds[requestId];
		m_deleteFriendIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "delete friend failed: " << httpCode;
			emit deleteFriendFailed(otherId, tr("Network error: %1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "delete friend failed: " << errMsg;
			emit deleteFriendFailed(otherId, errMsg);
			return;
		}

		emit deleteFriendOK(otherId);
		return;
	}
	// 
}
