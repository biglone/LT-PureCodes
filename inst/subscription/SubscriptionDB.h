#ifndef _SUBSCRIPTIONDB_H_
#define _SUBSCRIPTIONDB_H_

#include "DBBase.h"
#include <QList>
#include <QMap>
#include "subscriptiondetail.h"
#include "subscriptionlastmsgmodel.h"

class SubscriptionMsg;

namespace DB
{
	class SubscriptionDB : public DBBase
	{
	public:
		explicit SubscriptionDB(const QString &connSuffix = "");

		bool open();

		bool clearSubscriptions();
		bool setSubscriptions(const QList<SubscriptionDetail> &subscriptions);
		QList<SubscriptionDetail> subscriptions();

		bool setSubscription(const SubscriptionDetail &subscription);
		bool delSubscription(const QString &id);
		SubscriptionDetail subscription(const QString &id);

		bool storeMenu(const QString &subscriptionId, const QString &menuStr);
		QString getMenu(const QString &subscriptionId);
		QMap<QString, QString> getMenus();

		bool addLastMsg(const SubscriptionLastMsgModel::LastMsgItem &item);
		bool removeLastMsg(const QString &subscriptionId);
		QList<SubscriptionLastMsgModel::LastMsgItem> lastMsgs();
	};
}

#endif //_SUBSCRIPTIONDETAILDB_H_
