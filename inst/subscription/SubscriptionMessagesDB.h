#ifndef _SUBSCRIPTIONMESSAGESDB_H_
#define _SUBSCRIPTIONMESSAGESDB_H_

#include "DBBase.h"
#include <QList>
#include <QMap>
#include "subscriptiondetail.h"
#include "subscriptionmsg.h"

namespace DB
{
	class SubscriptionMessagesDB : public DBBase
	{
	public:
		explicit SubscriptionMessagesDB(const QString &connSuffix = "");

		bool open();

		quint64 maxInnerId();
		quint64 storeMessage(const SubscriptionMsg &msg);
		int getMessageCount(const QString &id, quint64 lastInnerId = 0);
		QList<SubscriptionMsg> getMessages(const QString &id, quint64 lastInnerId = 0, int count = 10);
		bool removeMessages(const QString &subscriptionId);
		
		QMap<QString, int> unreadMsgCount();
		int unreadMsgCount(const QString &subscriptionId);
		bool setUnreadMsgCount(const QString &subscriptionId, int count);
	};
}

#endif // _SUBSCRIPTIONMESSAGESDB_H_
