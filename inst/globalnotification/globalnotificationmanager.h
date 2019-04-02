#ifndef GLOBALNOTIFICATIONMANAGER_H
#define GLOBALNOTIFICATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include "globalnotificationdetail.h"
#include "globalnotificationmsg.h"
#include <QVariantList>
#include <QVariantMap>
#include <QScopedPointer>

class HttpPool;
class GlobalNotificationLogoManager;
class QNetworkAccessManager;

class GlobalNotificationManager : public QObject
{
	Q_OBJECT

public:
	enum RequestType
	{
		GlobalNotificationList,
		SearchGlobalNotification,
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
	GlobalNotificationManager(QObject *parent = 0);
	~GlobalNotificationManager();

public:
	void getGlobalNotificationList(const QString &userId);
	void searchGlobalNotification(const QString &keyword, int currentPage, int pageSize);
	void sendMsg(const QString &msgId, const QString &userId, const QString &globalNotificationId, const QString &content, const QString &createTime);
	void getDetail(const QString &globalNotificationId);
	void subscribe(const QString &globalNotificationId, const QString &userId, const QString &createTime);
	void unsubscribe(const QString &globalNotificationId, const QString &userId, const QString &createTime);
	void report(const QString &userId, const QString &sequence);
	void getMsgNumber(const QString &userId, const QString &lastSequence);
	void getMessages(const QString &userId, const QString &globalNotificationId, const QString &from, const QString &to, int count);
	void getHistoryMessages(const QString &userId, const QString &globalNotificationId, const QString &to, int count);
	void getMenu(const QString &globalNotificationId);
	void clickMenu(const QString &globalNotificationId, const QString &userId, const QString &key);
	void getGlobalNotificationLogo(const QString &globalNotificationId, const QString &urlString);

	static QNetworkAccessManager *getGlobalNotificationWebViewHttpManager();

public Q_SLOTS:
	void onGlobalNotificationSubscribed(const QString &globalNotificationId);
	void onGlobalNotificationUnsubscribed(const QString &globalNotificationId);

Q_SIGNALS:
	void getGlobalNotificationListFinished(bool ok, const QList<GlobalNotificationDetail> &globalNotifications);
	void searchGlobalNotificationFinished(bool ok, int currentPage, int pageSize, int rowCount, int totalPage, const QList<GlobalNotificationDetail> &globalNotifications);
	void sendMsgFinished(bool ok, const QString &globalNotificationId, const GlobalNotificationMsg &msg);
	void getDetailFinished(bool ok, const QString &globalNotificationId, const GlobalNotificationDetail &globalNotification);
	void subscribeFinished(bool ok, const QString &globalNotificationId, const GlobalNotificationMsg &msg);
	void unsubscribeFinished(bool ok, const QString &globalNotificationId);
	void reportFinished(bool ok);
	void getMsgNumberFinished(bool ok, const QMap<QString, int> &msgNumbers);
	void getMessagesFinished(bool ok, const QString &globalNotificationId, const QList<GlobalNotificationMsg> &messages);
	void getHistoryMessagesFinished(bool ok, const QString &globalNotificationId, const QList<GlobalNotificationMsg> &messages);
	void getLogoFinished(const QString &globalNotificationId, const QString &urlString, const QPixmap &logo, bool save);
	void getMenuFinished(bool ok, const QString &globalNotificationId, const QVariantList &menuList);
	void clickMenuFinished(bool ok, const QString &globalNotificationId, const QString &key, const GlobalNotificationMsg &msg);
	void globalNotificationSubscribed(const QString &globalNotificationId);
	void globalNotificationUnsubscribed(const QString &globalNotificationId);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	struct RequestData
	{
		RequestType requestType;
		QString     globalNotificationId;
		QString     param;

		bool operator==(const RequestData &other) const {
			if (this->requestType == other.requestType && 
				this->globalNotificationId == other.globalNotificationId &&
				this->param == other.param)
			{
				return true;
			}
			return false;
		}
	};

	QString baseHttpAddress() const;
	void processRequestError(RequestType requestType, const QString &globalNotificationId, const QString &errMsg, const QString &param);
	void processGlobalNotificationList(const QVariantList &vl);
	void processSearchGlobalNotification(const QVariantMap &vm);
	void processSendMsg(const QVariantMap &vm);
	void processDetail(const QVariantMap &vm);
	void processSubscribe(const QString &globalNotificationId, const QVariantMap &vm);
	void processMsgNumber(const QVariantMap &vm);
	void processMessages(const QString &globalNotificationId, const QVariantList &vl);
	void processHistoryMessages(const QString &globalNotificationId, const QVariantList &vl);
	void processMenu(const QString &globalNotificationId, const QVariantMap &vm);
	void processClickMenu(const QString &globalNotificationId, const QString &key, const QVariantMap &vm);
	GlobalNotificationDetail parseGlobalNotificationDetail(const QVariantMap &vm);
	GlobalNotificationMsg parseGlobalNotificationMsg(const QVariantMap &vm);
	bool hasRequest(const RequestData &requestDate);

private:
	HttpPool                               *m_httpPool;
	QMap<int, RequestData>                  m_requests;
	QScopedPointer<GlobalNotificationLogoManager> m_logoManager;

	static QNetworkAccessManager *s_webViewHttpManager;
};

#endif // GLOBALNOTIFICATIONMANAGER_H
