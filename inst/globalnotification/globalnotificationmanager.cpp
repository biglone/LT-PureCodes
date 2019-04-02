#include "globalnotificationmanager.h"
#include "settings/GlobalSettings.h"
#include "PmApp.h"
#include "http/HttpPool.h"
#include <QDebug>
#include "qt-json/json.h"
#include "util/JsonParser.h"
#include <QMultiMap>
#include "globalnotificationlogomanager.h"
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

static const char *kTagUserId           = "userId";
static const char *kTagId               = "id";
static const char *kTagNum              = "num";
static const char *kTagName             = "name";
static const char *kTagType             = "type";
static const char *kTagLogo             = "logo";
static const char *kTagIntroduction     = "introduction";
static const char *kTagKeyword          = "keyword";
static const char *kTagCurrentPage      = "currentPage";
static const char *kTagPageSize         = "pageSize";
static const char *kTagRowCount         = "rowCount";
static const char *kTagTotalPage        = "totalPage";
static const char *kTagPagination       = "pagination";
static const char *kTagMsgId            = "msgId";
static const char *kTagGlobalNotificationId   = "globalNotificationId";
static const char *kTagContent          = "content";
static const char *kTagCreateTime       = "createTime";
static const char *kTagSequence         = "sequence";
static const char *kTagLastSequence     = "lastSequence";
static const char *kTagFrom             = "from";
static const char *kTagTo               = "to";
static const char *kTagCount            = "count";
static const char *kTagDatas            = "datas";
static const char *kTagKey              = "key";
static const char *kTagItems            = "items";
static const char *kTagSpecial          = "special";

QNetworkAccessManager *GlobalNotificationManager::s_webViewHttpManager = 0;

GlobalNotificationManager::GlobalNotificationManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));

	m_logoManager.reset(new GlobalNotificationLogoManager(*m_httpPool));
	connect(m_logoManager.data(), SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)),
		this, SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)));
}

GlobalNotificationManager::~GlobalNotificationManager()
{

}

void GlobalNotificationManager::getGlobalNotificationList(const QString &userId)
{
	if (userId.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = GlobalNotificationList;
	requestData.globalNotificationId = QString();
	if (hasRequest(requestData))
		return;

	QString urlString = QString("%1/api/gn/query/%2").arg(baseHttpAddress()).arg(userId);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::searchGlobalNotification(const QString &keyword, int currentPage, int pageSize)
{
	/*
	if (keyword.isEmpty())
		return;
	*/

	QMultiMap<QString, QString> params;
	params.insert(kTagKeyword, keyword);
	params.insert(kTagCurrentPage, QString::number(currentPage));
	params.insert(kTagPageSize, QString::number(pageSize));

	QString urlString = QString("%1/api/gn/query").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = SearchGlobalNotification;
	requestData.globalNotificationId = QString();
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::sendMsg(const QString &msgId, const QString &userId, 
								  const QString &globalNotificationId, const QString &content, const QString &createTime)
{
	if (msgId.isEmpty() || userId.isEmpty() || globalNotificationId.isEmpty() || content.isEmpty() || createTime.isEmpty())
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagMsgId, msgId);
	params.insert(kTagUserId, userId);
	params.insert(kTagGlobalNotificationId, globalNotificationId);
	params.insert(kTagContent, content);
	params.insert(kTagCreateTime, createTime);

	QString urlString = QString("%1/api/gn/text/send").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = SendMsg;
	requestData.globalNotificationId = globalNotificationId;
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::getDetail(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = Detail;
	requestData.globalNotificationId = globalNotificationId;
	if (hasRequest(requestData))
		return;

	QString urlString = QString("%1/api/gn/detail/%2").arg(baseHttpAddress()).arg(globalNotificationId);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::subscribe(const QString &globalNotificationId, const QString &userId, const QString &createTime)
{
	if (globalNotificationId.isEmpty() || userId.isEmpty() || createTime.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = Subscribe;
	requestData.globalNotificationId = globalNotificationId;
	if (hasRequest(requestData))
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagGlobalNotificationId, globalNotificationId);
	params.insert(kTagUserId, userId);
	params.insert(kTagCreateTime, createTime);

	QString urlString = QString("%1/api/gn/subscribe").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::unsubscribe(const QString &globalNotificationId, const QString &userId, const QString &createTime)
{
	if (globalNotificationId.isEmpty() || userId.isEmpty() || createTime.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = Unsubscribe;
	requestData.globalNotificationId = globalNotificationId;
	if (hasRequest(requestData))
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagGlobalNotificationId, globalNotificationId);
	params.insert(kTagUserId, userId);
	params.insert(kTagCreateTime, createTime);

	QString urlString = QString("%1/api/gn/unsubscribe").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::report(const QString &userId, const QString &sequence)
{
	if (userId.isEmpty() || sequence.isEmpty())
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	params.insert(kTagSequence, sequence);

	QString urlString = QString("%1/api/gn/user/report").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = Report;
	requestData.globalNotificationId = QString();
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::getMsgNumber(const QString &userId, const QString &lastSequence)
{
	if (userId.isEmpty())
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	if (!lastSequence.isEmpty())
		params.insert(kTagLastSequence, lastSequence);

	QString urlString = QString("%1/api/gn/message/number").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = MsgNumber;
	requestData.globalNotificationId = QString();
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::getMessages(const QString &userId, const QString &globalNotificationId, 
									  const QString &from, const QString &to, int count)
{
	if (userId.isEmpty() || globalNotificationId.isEmpty() || count <= 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	params.insert(kTagGlobalNotificationId, globalNotificationId);
	if (!from.isEmpty())
		params.insert(kTagFrom, from);
	if (!to.isEmpty())
		params.insert(kTagTo, to);
	params.insert(kTagCount, QString::number(count));

	QString urlString = QString("%1/api/gn/messages").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = Messages;
	requestData.globalNotificationId = globalNotificationId;
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::getHistoryMessages(const QString &userId, const QString &globalNotificationId, const QString &to, int count)
{
	if (userId.isEmpty() || globalNotificationId.isEmpty() || count <= 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	params.insert(kTagGlobalNotificationId, globalNotificationId);
	if (!to.isEmpty())
		params.insert(kTagTo, to);
	params.insert(kTagCount, QString::number(count));

	QString urlString = QString("%1/api/gn/messages").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = HistoryMessages;
	requestData.globalNotificationId = globalNotificationId;
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::getMenu(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = MenuList;
	requestData.globalNotificationId = globalNotificationId;
	if (hasRequest(requestData))
		return;

	QString urlString = QString("%1/api/gn/menu/%2").arg(baseHttpAddress()).arg(globalNotificationId);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::clickMenu(const QString &globalNotificationId, const QString &userId, const QString &key)
{
	if (globalNotificationId.isEmpty() || userId.isEmpty() || key.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = MenuClick;
	requestData.globalNotificationId = globalNotificationId;
	requestData.param = key;
	if (hasRequest(requestData))
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagGlobalNotificationId, globalNotificationId);
	params.insert(kTagUserId, userId);
	params.insert(kTagKey, key);

	QString urlString = QString("%1/api/gn/menu/click").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_requests.insert(requestId, requestData);
}

void GlobalNotificationManager::getGlobalNotificationLogo(const QString &globalNotificationId, const QString &urlString)
{
	if (globalNotificationId.isEmpty() || urlString.isEmpty())
		return;

	m_logoManager->getGlobalNotificationLogo(globalNotificationId, urlString);
}

QNetworkAccessManager *GlobalNotificationManager::getGlobalNotificationWebViewHttpManager()
{
	if (!s_webViewHttpManager)
	{
		QNetworkAccessManager *s_webViewHttpManager = new QNetworkAccessManager();
		QNetworkDiskCache *diskCache = new QNetworkDiskCache(s_webViewHttpManager);
		if (diskCache)
		{
			QString location = QString("%1\\subnetcache").arg(GlobalSettings::globalHomePath());
			diskCache->setCacheDirectory(location);
			s_webViewHttpManager->setCache(diskCache);
		}
	}
	return s_webViewHttpManager;
}

void GlobalNotificationManager::onGlobalNotificationSubscribed(const QString &globalNotificationId)
{
	RequestData requestData;
	requestData.requestType = Subscribe;
	requestData.globalNotificationId = globalNotificationId;
	if (hasRequest(requestData))
		return;

	emit globalNotificationSubscribed(globalNotificationId);
}

void GlobalNotificationManager::onGlobalNotificationUnsubscribed(const QString &globalNotificationId)
{
	RequestData requestData;
	requestData.requestType = Unsubscribe;
	requestData.globalNotificationId = globalNotificationId;
	if (hasRequest(requestData))
		return;

	emit globalNotificationUnsubscribed(globalNotificationId);
}

void GlobalNotificationManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (!m_requests.contains(requestId))
		return;

	QString errMsg;
	if (error)
	{
		errMsg = tr("Network error, code:%1").arg(httpCode);
	}

	bool err = true;
	QVariant datasVariant = JsonParser::parse(recvData, err, errMsg);

	RequestData requestData = m_requests[requestId];
	RequestType requestType = requestData.requestType;
	QString globalNotificationId = requestData.globalNotificationId;
	QString param = requestData.param;
	m_requests.remove(requestId);
	if (!errMsg.isEmpty())
	{
		processRequestError(requestType, globalNotificationId, errMsg, param);
		return;
	}

	switch (requestType)
	{
	case GlobalNotificationList:
		{
			QVariantList vl = datasVariant.toList();
			processGlobalNotificationList(vl);
		}
		break;
	case SearchGlobalNotification:
		{
			QVariantMap vm = datasVariant.toMap();
			processSearchGlobalNotification(vm);
		}
		break;
	case SendMsg:
		{
			QVariantMap vm = datasVariant.toMap();
			processSendMsg(vm);
		}
		break;
	case Detail:
		{
			QVariantMap vm = datasVariant.toMap();
			processDetail(vm);
		}
		break;
	case Subscribe:
		{
			QVariantMap vm = datasVariant.toMap();
			processSubscribe(globalNotificationId, vm);
		}
		break;
	case Unsubscribe:
		{
			emit unsubscribeFinished(true, globalNotificationId);
		}
		break;
	case Report:
		{
			emit reportFinished(true);
		}
		break;
	case MsgNumber:
		{
			QVariantMap vm = datasVariant.toMap();
			processMsgNumber(vm);
		}
		break;
	case Messages:
		{
			QVariantList vl = datasVariant.toList();
			processMessages(globalNotificationId, vl);
		}
		break;
	case HistoryMessages:
		{
			QVariantList vl = datasVariant.toList();
			processHistoryMessages(globalNotificationId, vl);
		}
		break;
	case MenuList:
		{
			QVariantMap vm = datasVariant.toMap();
			processMenu(globalNotificationId, vm);
		}
		break;
	case MenuClick:
		{
			QVariantMap vm = datasVariant.toMap();
			processClickMenu(globalNotificationId, param, vm);
		}
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error request type: " << requestType;
		break;
	}
}

QString GlobalNotificationManager::baseHttpAddress() const
{
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = loginConfig.managerUrl;
	return urlString;
}

void GlobalNotificationManager::processRequestError(RequestType requestType, const QString &globalNotificationId, 
											  const QString &errMsg, const QString &param)
{
	switch (requestType)
	{
	case GlobalNotificationList:
		qWarning() << Q_FUNC_INFO << "get globalNotification list failed:" << errMsg;
		emit getGlobalNotificationListFinished(false, QList<GlobalNotificationDetail>());
		break;
	case SearchGlobalNotification:
		qWarning() << Q_FUNC_INFO << "search globalNotification failed:" << errMsg;
		emit searchGlobalNotificationFinished(false, 0, 0, 0, 0, QList<GlobalNotificationDetail>());
		break;
	case SendMsg:
		qWarning() << Q_FUNC_INFO << "send message failed:" << errMsg;
		emit sendMsgFinished(false, globalNotificationId, GlobalNotificationMsg());
		break;
	case Detail:
		qWarning() << Q_FUNC_INFO << "get detail failed:" << errMsg;
		emit getDetailFinished(false, globalNotificationId, GlobalNotificationDetail());
		break;
	case Subscribe:
		qWarning() << Q_FUNC_INFO << "subscribe failed:" << errMsg;
		emit subscribeFinished(false, globalNotificationId, GlobalNotificationMsg());
		break;
	case Unsubscribe:
		qWarning() << Q_FUNC_INFO << "unsubscribe failed:" << errMsg;
		emit unsubscribeFinished(false, globalNotificationId);
		break;
	case Report:
		qWarning() << Q_FUNC_INFO << "report failed:" << errMsg;
		emit reportFinished(false);
		break;
	case MsgNumber:
		qWarning() << Q_FUNC_INFO << "get message number failed:" << errMsg;
		emit getMsgNumberFinished(false, QMap<QString, int>());
		break;
	case Messages:
		qWarning() << Q_FUNC_INFO << "get message failed:" << errMsg;
		emit getMessagesFinished(false, globalNotificationId, QList<GlobalNotificationMsg>());
		break;
	case HistoryMessages:
		qWarning() << Q_FUNC_INFO << "get history message failed:" << errMsg;
		emit getHistoryMessagesFinished(false, globalNotificationId, QList<GlobalNotificationMsg>());
		break;
	case MenuList:
		qWarning() << Q_FUNC_INFO << "get menu failed:" << errMsg;
		emit getMenuFinished(false, globalNotificationId, QVariantList());
		break;
	case MenuClick:
		qWarning() << Q_FUNC_INFO << "click menu failed:" << errMsg;
		emit clickMenuFinished(false, globalNotificationId, param, GlobalNotificationMsg());
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error request type: " << requestType;
		break;
	}
}

void GlobalNotificationManager::processGlobalNotificationList(const QVariantList &vl)
{
	QList<GlobalNotificationDetail> globalNotifications;
	foreach (QVariant v, vl)
	{
		QVariantMap vm = v.toMap();
		GlobalNotificationDetail globalNotification = parseGlobalNotificationDetail(vm);
		globalNotifications.append(globalNotification);
	}
	emit getGlobalNotificationListFinished(true, globalNotifications);
}

void GlobalNotificationManager::processSearchGlobalNotification(const QVariantMap &vm)
{
	QList<GlobalNotificationDetail> globalNotifications;
	QVariantList subVL = vm[kTagDatas].toList();
	foreach (QVariant v, subVL)
	{
		QVariantMap vm = v.toMap();
		GlobalNotificationDetail globalNotification = parseGlobalNotificationDetail(vm);
		globalNotifications.append(globalNotification);
	}

	QVariantMap pageVM = vm[kTagPagination].toMap();
	int pageSize = pageVM[kTagPageSize].toInt();
	int currentPage = pageVM[kTagCurrentPage].toInt();
	int rowCount = pageVM[kTagRowCount].toInt();
	int totalPage = pageVM[kTagTotalPage].toInt();

	emit searchGlobalNotificationFinished(true, currentPage, pageSize, rowCount, totalPage, globalNotifications);
}

void GlobalNotificationManager::processSendMsg(const QVariantMap &vm)
{
	GlobalNotificationMsg msg = parseGlobalNotificationMsg(vm);
	emit sendMsgFinished(true, msg.globalNotificationId(), msg);
}

void GlobalNotificationManager::processDetail(const QVariantMap &vm)
{
	GlobalNotificationDetail globalNotification = parseGlobalNotificationDetail(vm);
	emit getDetailFinished(true, globalNotification.id(), globalNotification);
}

void GlobalNotificationManager::processSubscribe(const QString &globalNotificationId, const QVariantMap &vm)
{
	GlobalNotificationMsg msg = parseGlobalNotificationMsg(vm);
	emit subscribeFinished(true, globalNotificationId, msg);
}

void GlobalNotificationManager::processMsgNumber(const QVariantMap &vm)
{
	QMap<QString, int> msgNumber;
	foreach (QString subId, vm.keys())
	{
		msgNumber.insert(subId, vm[subId].toInt());
	}
	emit getMsgNumberFinished(true, msgNumber);
}

void GlobalNotificationManager::processMessages(const QString &globalNotificationId, const QVariantList &vl)
{
	QList<GlobalNotificationMsg> messages;
	foreach (QVariant v, vl)
	{
		QVariantMap vm = v.toMap();
		GlobalNotificationMsg msg = parseGlobalNotificationMsg(vm);
		messages.append(msg);
	}
	emit getMessagesFinished(true, globalNotificationId, messages);
}

void GlobalNotificationManager::processHistoryMessages(const QString &globalNotificationId, const QVariantList &vl)
{
	QList<GlobalNotificationMsg> messages;
	foreach (QVariant v, vl)
	{
		QVariantMap vm = v.toMap();
		GlobalNotificationMsg msg = parseGlobalNotificationMsg(vm);
		messages.append(msg);
	}
	emit getHistoryMessagesFinished(true, globalNotificationId, messages);
}

void GlobalNotificationManager::processMenu(const QString &globalNotificationId, const QVariantMap &vm)
{
	QVariantList vl = vm[kTagItems].toList();
	emit getMenuFinished(true, globalNotificationId, vl);
}

void GlobalNotificationManager::processClickMenu(const QString &globalNotificationId, const QString &key, const QVariantMap &vm)
{
	GlobalNotificationMsg msg = parseGlobalNotificationMsg(vm);
	emit clickMenuFinished(true, globalNotificationId, key, msg);
}

GlobalNotificationDetail GlobalNotificationManager::parseGlobalNotificationDetail(const QVariantMap &vm)
{
	GlobalNotificationDetail globalNotification;
	if (vm.isEmpty())
		return globalNotification;

	QString name = vm[kTagName].toString();
	QString id = QString::number(vm[kTagId].toULongLong());
	int type = vm[kTagType].toInt();
	QString logo = vm[kTagLogo].toString();
	QString num = vm[kTagNum].toString();
	QString introduction = vm[kTagIntroduction].toString();
	int special = vm[kTagSpecial].toInt();
	
	globalNotification.setName(name);
	globalNotification.setId(id);
	globalNotification.setType(type);
	globalNotification.setLogo(logo);
	globalNotification.setNum(num);
	globalNotification.setIntroduction(introduction);
	globalNotification.setSpecial(special);
	return globalNotification;
}

GlobalNotificationMsg GlobalNotificationManager::parseGlobalNotificationMsg(const QVariantMap &vm)
{
	GlobalNotificationMsg msg;
	if (vm.isEmpty())
		return msg;

	QString id = QString::number(vm[kTagId].toULongLong());
	int type = vm[kTagType].toInt();
	QString content;
	if (type == GlobalNotificationMsg::kTypeText)
	{
		content = vm[kTagContent].toString();
	}
	else if (type == GlobalNotificationMsg::kTypeImageText)
	{
		content = QtJson::serializeStr(vm[kTagContent]);
	}
	else if (type == GlobalNotificationMsg::kTypeImage)
	{
		content = QtJson::serializeStr(vm[kTagContent]);
	}
	else if (type == GlobalNotificationMsg::kTypeAttach)
	{
		content = QtJson::serializeStr(vm[kTagContent]);
	}
	QString userId = vm[kTagUserId].toString();
	QString globalNotificationId = QString::number(vm[kTagGlobalNotificationId].toULongLong());
	QString msgId = vm[kTagMsgId].toString();
	QString createTime = vm[kTagCreateTime].toString();
	
	msg.setId(id);
	msg.setType(type);
	msg.setContent(content);
	msg.setUserId(userId);
	msg.setGlobalNotificationId(globalNotificationId);
	msg.setMsgId(msgId);
	msg.setCreateTime(createTime);
	return msg;
}

bool GlobalNotificationManager::hasRequest(const RequestData &requestDate)
{
	return m_requests.values().contains(requestDate);
}