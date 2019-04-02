#ifndef _GLOBALNOTIFICATIONMESSAGESDB_H_
#define _GLOBALNOTIFICATIONMESSAGESDB_H_

#include "DBBase.h"
#include <QList>
#include <QMap>
#include "globalnotificationdetail.h"
#include "globalnotificationmsg.h"

namespace DB
{
	class GlobalNotificationMessagesDB : public DBBase
	{
	public:
		explicit GlobalNotificationMessagesDB(const QString &connSuffix = "");

		bool open();

		quint64 maxInnerId();
		quint64 storeMessage(const GlobalNotificationMsg &msg);
		int getMessageCount(const QString &id, quint64 lastInnerId = 0);
		QList<GlobalNotificationMsg> getMessages(const QString &id, quint64 lastInnerId = 0, int count = 10);
		bool removeMessages(const QString &globalNotificationId);
		
		QMap<QString, int> unreadMsgCount();
		int unreadMsgCount(const QString &globalNotificationId);
		bool setUnreadMsgCount(const QString &globalNotificationId, int count);
	};
}

#endif // _GLOBALNOTIFICATIONMESSAGESDB_H_
