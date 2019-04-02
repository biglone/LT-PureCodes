#ifndef SUBSCRIPTIONMANAGER_H
#define SUBSCRIPTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include "subscriptiondetail.h"
#include "subscriptionmsg.h"
#include <QVariantList>
#include <QVariantMap>
#include <QScopedPointer>

class HttpPool;
class SubscriptionLogoManager;
class QNetworkAccessManager;

class SubscriptionManager : public QObject
{
	Q_OBJECT

public:
	enum RequestType
	{
		SubscriptionList,
		SearchSubscription,
		SendMsg,
		Detail,
		Subscribe,
		Unsubscribe,
		Report,
		MsgNumber,
		Messages,
		HistoryMessages,
		MenuList,
		MenuClick
	};

public:
	SubscriptionManager(QObject *parent = 0);
	~SubscriptionManager();

public:
	void getSubscriptionList(const QString &userId);
	void searchSubscription(const QString &keyword, int currentPage, int pageSize);
	void sendMsg(const QString &msgId, const QString &userId, const QString &subscriptionId, const QString &content, const QString &createTime);
	void getDetail(const QString &subscriptionId);
	void subscribe(const QString &subscriptionId, const QString &userId, const QString &createTime);
	void unsubscribe(const QString &subscriptionId, const QString &userId, const QString &createTime);
	void report(const QString &userId, const QString &sequence);
	void getMsgNumber(const QString &userId, const QString &lastSequence);
	void getMessages(const QString &userId, const QString &subscriptionId, const QString &from, const QString &to, int count);
	void getHistoryMessages(const QString &userId, const QString &subscriptionId, const QString &to, int count);
	void getMenu(const QString &subscriptionId);
	void clickMenu(const QString &subscriptionId, const QString &userId, const QString &key);
	void getSubscriptionLogo(const QString &subscriptionId, const QString &urlString);

	static QNetworkAccessManager *getSubscriptionWebViewHttpManager();

public Q_SLOTS:
	void onSubscriptionSubscribed(const QString &subscriptionId);
	void onSubscriptionUnsubscribed(const QString &subscriptionId);

Q_SIGNALS:
	void getSubscriptionListFinished(bool ok, const QList<SubscriptionDetail> &subscriptions);
	void searchSubscriptionFinished(bool ok, int currentPage, int pageSize, int rowCount, int totalPage, const QList<SubscriptionDetail> &subscriptions);
	void sendMsgFinished(bool ok, const QString &subscriptionId, const SubscriptionMsg &msg);
	void getDetailFinished(bool ok, const QString &subscriptionId, const SubscriptionDetail &subscription);
	void subscribeFinished(bool ok, const QString &subscriptionId, const SubscriptionMsg &msg);
	void unsubscribeFinished(bool ok, const QString &subscriptionId);
	void reportFinished(bool ok);
	void getMsgNumberFinished(bool ok, const QMap<QString, int> &msgNumbers);
	void getMessagesFinished(bool ok, const QString &subscriptionId, const QList<SubscriptionMsg> &messages);
	void getHistoryMessagesFinished(bool ok, const QString &subscriptionId, const QList<SubscriptionMsg> &messages);
	void getLogoFinished(const QString &subscriptionId, const QString &urlString, const QPixmap &logo, bool save);
	void getMenuFinished(bool ok, const QString &subscriptionId, const QVariantList &menuList);
	void clickMenuFinished(bool ok, const QString &subscriptionId, const QString &key, const SubscriptionMsg &msg);
	void subscriptionSubscribed(const QString &subscriptionId);
	void subscriptionUnsubscribed(const QString &subscriptionId);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	struct RequestData
	{
		RequestType requestType;
		QString     subscriptionId;
		QString     param;

		bool operator==(const RequestData &other) const {
			if (this->requestType == other.requestType && 
				this->subscriptionId == other.subscriptionId &&
				this->param == other.param)
			{
				return true;
			}
			return false;
		}
	};

	QString baseHttpAddress() const;
	void processRequestError(RequestType requestType, const QString &subscriptionId, const QString &errMsg, const QString &param);
	void processSubscriptionList(const QVariantList &vl);
	void processSearchSubscription(const QVariantMap &vm);
	void processSendMsg(const QVariantMap &vm);
	void processDetail(const QVariantMap &vm);
	void processSubscribe(const QString &subscriptionId, const QVariantMap &vm);
	void processMsgNumber(const QVariantMap &vm);
	void processMessages(const QString &subscriptionId, const QVariantList &vl);
	void processHistoryMessages(const QString &subscriptionId, const QVariantList &vl);
	void processMenu(const QString &subscriptionId, const QVariantMap &vm);
	void processClickMenu(const QString &subscriptionId, const QString &key, const QVariantMap &vm);
	SubscriptionDetail parseSubscriptionDetail(const QVariantMap &vm);
	SubscriptionMsg parseSubscriptionMsg(const QVariantMap &vm);
	bool hasRequest(const RequestData &requestDate);

private:
	HttpPool                               *m_httpPool;
	QMap<int, RequestData>                  m_requests;
	QScopedPointer<SubscriptionLogoManager> m_logoManager;

	static QNetworkAccessManager *s_webViewHttpManager;
};

#endif // SUBSCRIPTIONMANAGER_H
