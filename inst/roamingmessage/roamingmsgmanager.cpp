#include "roamingmsgmanager.h"
#include "util/JsonParser.h"
#include "iks/iksemel.h"
#include "PmApp.h"
#include "http/HttpPool.h"
#include "settings/GlobalSettings.h"
#include "Constants.h"
#include <QDebug>
#include <QPair>

RoamingMsgManager::RoamingMsgManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

RoamingMsgManager::~RoamingMsgManager()
{

}

void RoamingMsgManager::getChatRoamingMessage(const QString &uid, 
	                                          const QString &selfId, 
											  int pageSize, 
											  int currentPage /*= 0*/, 
											  const QString &beginDate /*= QString()*/, 
											  const QString &endDate /*= QString()*/)
{
	if (uid.isEmpty() || selfId.isEmpty() || pageSize < 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert("self", selfId);
	params.insert("other", uid);
	params.insert("pageSize", QString::number(pageSize));
	if (currentPage > 0)
		params.insert("currentPage", QString::number(currentPage));
	if (!beginDate.isEmpty())
		params.insert("beginDate", beginDate);
	if (!endDate.isEmpty())
		params.insert("endDate", endDate);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/message/chat").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	
	QPair<bean::MessageType, QString> pair = qMakePair(bean::Message_Chat, uid);
	m_requests.insert(requestId, pair);
}

void RoamingMsgManager::getGroupRoamingMessage(const QString &groupId, 
                                               int pageSize, 
											   int currentPage /*= 0*/, 
											   const QString &beginDate /*= QString()*/, 
											   const QString &endDate /*= QString()*/)
{
	if (groupId.isEmpty() || pageSize < 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert("groupId", groupId);
	params.insert("pageSize", QString::number(pageSize));
	if (currentPage > 0)
		params.insert("currentPage", QString::number(currentPage));
	if (!beginDate.isEmpty())
		params.insert("beginDate", beginDate);
	if (!endDate.isEmpty())
		params.insert("endDate", endDate);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/message/group/chat").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	
	QPair<bean::MessageType, QString> pair = qMakePair(bean::Message_GroupChat, groupId);
	m_requests.insert(requestId, pair);
}

void RoamingMsgManager::getDiscussRoamingMessage(const QString &discussId, 
	                                             int pageSize, 
												 int currentPage /*= 0*/, 
												 const QString &beginDate /*= QString()*/, 
												 const QString &endDate /*= QString()*/)
{
	if (discussId.isEmpty() || pageSize < 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert("discussId", discussId);
	params.insert("pageSize", QString::number(pageSize));
	if (currentPage > 0)
		params.insert("currentPage", QString::number(currentPage));
	if (!beginDate.isEmpty())
		params.insert("beginDate", beginDate);
	if (!endDate.isEmpty())
		params.insert("endDate", endDate);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/message/discuss/chat").arg(loginConfig.managerUrl);
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	
	QPair<bean::MessageType, QString> pair = qMakePair(bean::Message_DiscussChat, discussId);
	m_requests.insert(requestId, pair);
}

void RoamingMsgManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_requests.contains(requestId))
		return;

	QPair<bean::MessageType, QString> pair = m_requests.value(requestId);
	bean::MessageType msgType = pair.first;
	QString id = pair.second;

	m_requests.remove(requestId);

	QString errMsg;
	QVariant datasVariant;
	if (error)
	{
		qWarning() << Q_FUNC_INFO << "roaming message failed: " << httpCode << (int)msgType << id;
		errMsg = tr("Network error, code:%1");
	}
	else
	{
		bool err = true;
		datasVariant = JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "roaming message failed: " << errMsg << (int)msgType << id;
		}
	}

	if (!errMsg.isEmpty())
	{
		if (msgType == bean::Message_Chat)
		{
			gotChatRoamingMessageFailed(id);
		}
		else if (msgType == bean::Message_GroupChat)
		{
			gotGroupRoamingMessageFailed(id);
		}
		else if (msgType == bean::Message_DiscussChat)
		{
			gotDiscussRoamingMessageFailed(id);
		}
		return;
	}

	QVariantMap datas = datasVariant.toMap();
	QVariantMap pagination = datas["pagination"].toMap();
	int pageSize = pagination["pageSize"].toInt();
	int currentPage = pagination["currentPage"].toInt();
	int totalPage = pagination["totalPage"].toInt();
	
	QVariantList datasList = datas["datas"].toList();
	bean::MessageBodyList msgs;
	foreach (QVariant data, datasList)
	{
		QVariantMap dataMap = data.toMap();
		QString msgXml = dataMap["msgXml"].toString();
		bean::MessageBody msg = bean::MessageBodyFactory::fromXml(msgXml, true);
		if (msg.isSend())
			msg.setSync(true);
		msgs.insert(0, msg);
	}

	if (msgType == bean::Message_Chat)
	{
		emit gotChatRoamingMessageOK(id, pageSize, currentPage, totalPage, msgs);
	}
	else if (msgType == bean::Message_GroupChat)
	{
		emit gotGroupRoamingMessageOK(id, pageSize, currentPage, totalPage, msgs);
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		emit gotDiscussRoamingMessageOK(id, pageSize, currentPage, totalPage, msgs);
	}
}
