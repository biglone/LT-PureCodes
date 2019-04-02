#include "subscriptionmanager.h"
#include "settings/GlobalSettings.h"
#include "PmApp.h"
#include "http/HttpPool.h"
#include <QDebug>
#include "qt-json/json.h"
#include "util/JsonParser.h"
#include <QMultiMap>
#include "subscriptionlogomanager.h"
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
static const char *kTagSubscriptionId   = "subscriptionId";
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

QNetworkAccessManager *SubscriptionManager::s_webViewHttpManager = 0;

SubscriptionManager::SubscriptionManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
	m_httpPool = qPmApp->getHttpPool();
	connect(m_httpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));

	m_logoManager.reset(new SubscriptionLogoManager(*m_httpPool));
	connect(m_logoManager.data(), SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)),
		this, SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)));
}

SubscriptionManager::~SubscriptionManager()
{

}

void SubscriptionManager::getSubscriptionList(const QString &userId)
{
	if (userId.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = SubscriptionList;
	requestData.subscriptionId = QString();
	if (hasRequest(requestData))
		return;

	QString urlString = QString("%1/api/subscription/query/%2").arg(baseHttpAddress()).arg(userId);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::searchSubscription(const QString &keyword, int currentPage, int pageSize)
{
	/*
	if (keyword.isEmpty())
		return;
	*/

	QMultiMap<QString, QString> params;
	params.insert(kTagKeyword, keyword);
	params.insert(kTagCurrentPage, QString::number(currentPage));
	params.insert(kTagPageSize, QString::number(pageSize));

	QString urlString = QString("%1/api/subscription/query").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = SearchSubscription;
	requestData.subscriptionId = QString();
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::sendMsg(const QString &msgId, const QString &userId, 
								  const QString &subscriptionId, const QString &content, const QString &createTime)
{
	if (msgId.isEmpty() || userId.isEmpty() || subscriptionId.isEmpty() || content.isEmpty() || createTime.isEmpty())
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagMsgId, msgId);
	params.insert(kTagUserId, userId);
	params.insert(kTagSubscriptionId, subscriptionId);
	params.insert(kTagContent, content);
	params.insert(kTagCreateTime, createTime);

	QString urlString = QString("%1/api/subscription/text/send").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = SendMsg;
	requestData.subscriptionId = subscriptionId;
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::getDetail(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = Detail;
	requestData.subscriptionId = subscriptionId;
	if (hasRequest(requestData))
		return;

	QString urlString = QString("%1/api/subscription/detail/%2").arg(baseHttpAddress()).arg(subscriptionId);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::subscribe(const QString &subscriptionId, const QString &userId, const QString &createTime)
{
	if (subscriptionId.isEmpty() || userId.isEmpty() || createTime.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = Subscribe;
	requestData.subscriptionId = subscriptionId;
	if (hasRequest(requestData))
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagSubscriptionId, subscriptionId);
	params.insert(kTagUserId, userId);
	params.insert(kTagCreateTime, createTime);

	QString urlString = QString("%1/api/subscription/subscribe").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::unsubscribe(const QString &subscriptionId, const QString &userId, const QString &createTime)
{
	if (subscriptionId.isEmpty() || userId.isEmpty() || createTime.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = Unsubscribe;
	requestData.subscriptionId = subscriptionId;
	if (hasRequest(requestData))
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagSubscriptionId, subscriptionId);
	params.insert(kTagUserId, userId);
	params.insert(kTagCreateTime, createTime);

	QString urlString = QString("%1/api/subscription/unsubscribe").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::report(const QString &userId, const QString &sequence)
{
	if (userId.isEmpty() || sequence.isEmpty())
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	params.insert(kTagSequence, sequence);

	QString urlString = QString("%1/api/subscription/user/report").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = Report;
	requestData.subscriptionId = QString();
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::getMsgNumber(const QString &userId, const QString &lastSequence)
{
	if (userId.isEmpty())
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	if (!lastSequence.isEmpty())
		params.insert(kTagLastSequence, lastSequence);

	QString urlString = QString("%1/api/subscription/message/number").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = MsgNumber;
	requestData.subscriptionId = QString();
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::getMessages(const QString &userId, const QString &subscriptionId, 
									  const QString &from, const QString &to, int count)
{
	if (userId.isEmpty() || subscriptionId.isEmpty() || count <= 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	params.insert(kTagSubscriptionId, subscriptionId);
	if (!from.isEmpty())
		params.insert(kTagFrom, from);
	if (!to.isEmpty())
		params.insert(kTagTo, to);
	params.insert(kTagCount, QString::number(count));

	QString urlString = QString("%1/api/subscription/messages").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = Messages;
	requestData.subscriptionId = subscriptionId;
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::getHistoryMessages(const QString &userId, const QString &subscriptionId, const QString &to, int count)
{
	if (userId.isEmpty() || subscriptionId.isEmpty() || count <= 0)
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagUserId, userId);
	params.insert(kTagSubscriptionId, subscriptionId);
	if (!to.isEmpty())
		params.insert(kTagTo, to);
	params.insert(kTagCount, QString::number(count));

	QString urlString = QString("%1/api/subscription/messages").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	RequestData requestData;
	requestData.requestType = HistoryMessages;
	requestData.subscriptionId = subscriptionId;
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::getMenu(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = MenuList;
	requestData.subscriptionId = subscriptionId;
	if (hasRequest(requestData))
		return;

	QString urlString = QString("%1/api/subscription/menu/%2").arg(baseHttpAddress()).arg(subscriptionId);
	int requestId = m_httpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString));
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::clickMenu(const QString &subscriptionId, const QString &userId, const QString &key)
{
	if (subscriptionId.isEmpty() || userId.isEmpty() || key.isEmpty())
		return;

	RequestData requestData;
	requestData.requestType = MenuClick;
	requestData.subscriptionId = subscriptionId;
	requestData.param = key;
	if (hasRequest(requestData))
		return;

	QMultiMap<QString, QString> params;
	params.insert(kTagSubscriptionId, subscriptionId);
	params.insert(kTagUserId, userId);
	params.insert(kTagKey, key);

	QString urlString = QString("%1/api/subscription/menu/click").arg(baseHttpAddress());
	int requestId = m_httpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_requests.insert(requestId, requestData);
}

void SubscriptionManager::getSubscriptionLogo(const QString &subscriptionId, const QString &urlString)
{
	if (subscriptionId.isEmpty() || urlString.isEmpty())
		return;

	m_logoManager->getSubscriptionLogo(subscriptionId, urlString);
}

QNetworkAccessManager *SubscriptionManager::getSubscriptionWebViewHttpManager()
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

void SubscriptionManager::onSubscriptionSubscribed(const QString &subscriptionId)
{
	RequestData requestData;
	requestData.requestType = Subscribe;
	requestData.subscriptionId = subscriptionId;
	if (hasRequest(requestData))
		return;

	emit subscriptionSubscribed(subscriptionId);
}

void SubscriptionManager::onSubscriptionUnsubscribed(const QString &subscriptionId)
{
	RequestData requestData;
	requestData.requestType = Unsubscribe;
	requestData.subscriptionId = subscriptionId;
	if (hasRequest(requestData))
		return;

	emit subscriptionUnsubscribed(subscriptionId);
}

void SubscriptionManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
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
	QString subscriptionId = requestData.subscriptionId;
	QString param = requestData.param;
	m_requests.remove(requestId);
	if (!errMsg.isEmpty())
	{
		processRequestError(requestType, subscriptionId, errMsg, param);
		return;
	}

	switch (requestType)
	{
	case SubscriptionList:
		{
			QVariantList vl = datasVariant.toList();
			processSubscriptionList(vl);
		}
		break;
	case SearchSubscription:
		{
			QVariantMap vm = datasVariant.toMap();
			processSearchSubscription(vm);
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
			processSubscribe(subscriptionId, vm);
		}
		break;
	case Unsubscribe:
		{
			emit unsubscribeFinished(true, subscriptionId);
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
			processMessages(subscriptionId, vl);
		}
		break;
	case HistoryMessages:
		{
			QVariantList vl = datasVariant.toList();
			processHistoryMessages(subscriptionId, vl);
		}
		break;
	case MenuList:
		{
			QVariantMap vm = datasVariant.toMap();
			processMenu(subscriptionId, vm);
		}
		break;
	case MenuClick:
		{
			QVariantMap vm = datasVariant.toMap();
			processClickMenu(subscriptionId, param, vm);
		}
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error request type: " << requestType;
		break;
	}
}

QString SubscriptionManager::baseHttpAddress() const
{
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = loginConfig.managerUrl;
	return urlString;
}

void SubscriptionManager::processRequestError(RequestType requestType, const QString &subscriptionId, 
											  const QString &errMsg, const QString &param)
{
	switch (requestType)
	{
	case SubscriptionList:
		qWarning() << Q_FUNC_INFO << "get subscription list failed:" << errMsg;
		emit getSubscriptionListFinished(false, QList<SubscriptionDetail>());
		break;
	case SearchSubscription:
		qWarning() << Q_FUNC_INFO << "search subscription failed:" << errMsg;
		emit searchSubscriptionFinished(false, 0, 0, 0, 0, QList<SubscriptionDetail>());
		break;
	case SendMsg:
		qWarning() << Q_FUNC_INFO << "send message failed:" << errMsg;
		emit sendMsgFinished(false, subscriptionId, SubscriptionMsg());
		break;
	case Detail:
		qWarning() << Q_FUNC_INFO << "get detail failed:" << errMsg;
		emit getDetailFinished(false, subscriptionId, SubscriptionDetail());
		break;
	case Subscribe:
		qWarning() << Q_FUNC_INFO << "subscribe failed:" << errMsg;
		emit subscribeFinished(false, subscriptionId, SubscriptionMsg());
		break;
	case Unsubscribe:
		qWarning() << Q_FUNC_INFO << "unsubscribe failed:" << errMsg;
		emit unsubscribeFinished(false, subscriptionId);
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
		emit getMessagesFinished(false, subscriptionId, QList<SubscriptionMsg>());
		break;
	case HistoryMessages:
		qWarning() << Q_FUNC_INFO << "get history message failed:" << errMsg;
		emit getHistoryMessagesFinished(false, subscriptionId, QList<SubscriptionMsg>());
		break;
	case MenuList:
		qWarning() << Q_FUNC_INFO << "get menu failed:" << errMsg;
		emit getMenuFinished(false, subscriptionId, QVariantList());
		break;
	case MenuClick:
		qWarning() << Q_FUNC_INFO << "click menu failed:" << errMsg;
		emit clickMenuFinished(false, subscriptionId, param, SubscriptionMsg());
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error request type: " << requestType;
		break;
	}
}

void SubscriptionManager::processSubscriptionList(const QVariantList &vl)
{
	QList<SubscriptionDetail> subscriptions;
	foreach (QVariant v, vl)
	{
		QVariantMap vm = v.toMap();
		SubscriptionDetail subscription = parseSubscriptionDetail(vm);
		subscriptions.append(subscription);
	}
	emit getSubscriptionListFinished(true, subscriptions);
}

void SubscriptionManager::processSearchSubscription(const QVariantMap &vm)
{
	QList<SubscriptionDetail> subscriptions;
	QVariantList subVL = vm[kTagDatas].toList();
	foreach (QVariant v, subVL)
	{
		QVariantMap vm = v.toMap();
		SubscriptionDetail subscription = parseSubscriptionDetail(vm);
		subscriptions.append(subscription);
	}

	QVariantMap pageVM = vm[kTagPagination].toMap();
	int pageSize = pageVM[kTagPageSize].toInt();
	int currentPage = pageVM[kTagCurrentPage].toInt();
	int rowCount = pageVM[kTagRowCount].toInt();
	int totalPage = pageVM[kTagTotalPage].toInt();

	emit searchSubscriptionFinished(true, currentPage, pageSize, rowCount, totalPage, subscriptions);
}

void SubscriptionManager::processSendMsg(const QVariantMap &vm)
{
	SubscriptionMsg msg = parseSubscriptionMsg(vm);
	emit sendMsgFinished(true, msg.subscriptionId(), msg);
}

void SubscriptionManager::processDetail(const QVariantMap &vm)
{
	SubscriptionDetail subscription = parseSubscriptionDetail(vm);
	emit getDetailFinished(true, subscription.id(), subscription);
}

void SubscriptionManager::processSubscribe(const QString &subscriptionId, const QVariantMap &vm)
{
	SubscriptionMsg msg = parseSubscriptionMsg(vm);
	emit subscribeFinished(true, subscriptionId, msg);
}

void SubscriptionManager::processMsgNumber(const QVariantMap &vm)
{
	QMap<QString, int> msgNumber;
	foreach (QString subId, vm.keys())
	{
		msgNumber.insert(subId, vm[subId].toInt());
	}
	emit getMsgNumberFinished(true, msgNumber);
}

void SubscriptionManager::processMessages(const QString &subscriptionId, const QVariantList &vl)
{
	QList<SubscriptionMsg> messages;
	foreach (QVariant v, vl)
	{
		QVariantMap vm = v.toMap();
		SubscriptionMsg msg = parseSubscriptionMsg(vm);
		messages.append(msg);
	}
	emit getMessagesFinished(true, subscriptionId, messages);
}

void SubscriptionManager::processHistoryMessages(const QString &subscriptionId, const QVariantList &vl)
{
	QList<SubscriptionMsg> messages;
	foreach (QVariant v, vl)
	{
		QVariantMap vm = v.toMap();
		SubscriptionMsg msg = parseSubscriptionMsg(vm);
		messages.append(msg);
	}
	emit getHistoryMessagesFinished(true, subscriptionId, messages);
}

void SubscriptionManager::processMenu(const QString &subscriptionId, const QVariantMap &vm)
{
	QVariantList vl = vm[kTagItems].toList();
	emit getMenuFinished(true, subscriptionId, vl);
}

void SubscriptionManager::processClickMenu(const QString &subscriptionId, const QString &key, const QVariantMap &vm)
{
	SubscriptionMsg msg = parseSubscriptionMsg(vm);
	emit clickMenuFinished(true, subscriptionId, key, msg);
}

SubscriptionDetail SubscriptionManager::parseSubscriptionDetail(const QVariantMap &vm)
{
	SubscriptionDetail subscription;
	if (vm.isEmpty())
		return subscription;

	QString name = vm[kTagName].toString();
	QString id = QString::number(vm[kTagId].toULongLong());
	int type = vm[kTagType].toInt();
	QString logo = vm[kTagLogo].toString();
	QString num = vm[kTagNum].toString();
	QString introduction = vm[kTagIntroduction].toString();
	int special = vm[kTagSpecial].toInt();
	
	subscription.setName(name);
	subscription.setId(id);
	subscription.setType(type);
	subscription.setLogo(logo);
	subscription.setNum(num);
	subscription.setIntroduction(introduction);
	subscription.setSpecial(special);
	return subscription;
}

SubscriptionMsg SubscriptionManager::parseSubscriptionMsg(const QVariantMap &vm)
{
	SubscriptionMsg msg;
	if (vm.isEmpty())
		return msg;

	QString id = QString::number(vm[kTagId].toULongLong());
	int type = vm[kTagType].toInt();
	QString content;
	if (type == SubscriptionMsg::kTypeText)
	{
		content = vm[kTagContent].toString();
	}
	else if (type == SubscriptionMsg::kTypeImageText)
	{
		content = QtJson::serializeStr(vm[kTagContent]);
	}
	else if (type == SubscriptionMsg::kTypeImage)
	{
		content = QtJson::serializeStr(vm[kTagContent]);
	}
	else if (type == SubscriptionMsg::kTypeAttach)
	{
		content = QtJson::serializeStr(vm[kTagContent]);
	}
	QString userId = vm[kTagUserId].toString();
	QString subscriptionId = QString::number(vm[kTagSubscriptionId].toULongLong());
	QString msgId = vm[kTagMsgId].toString();
	QString createTime = vm[kTagCreateTime].toString();
	
	msg.setId(id);
	msg.setType(type);
	msg.setContent(content);
	msg.setUserId(userId);
	msg.setSubscriptionId(subscriptionId);
	msg.setMsgId(msgId);
	msg.setCreateTime(createTime);
	return msg;
}

bool SubscriptionManager::hasRequest(const RequestData &requestDate)
{
	return m_requests.values().contains(requestDate);
}