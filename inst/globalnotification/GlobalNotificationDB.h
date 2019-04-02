#ifndef _GLOBALNOTIFICATIONDB_H_
#define _GLOBALNOTIFICATIONDB_H_

#include "DBBase.h"
#include <QList>
#include <QMap>
#include "globalnotificationdetail.h"
#include "globalnotificationlastmsgmodel.h"

class GlobalNotificationMsg;

namespace DB
{
	class GlobalNotificationDB : public DBBase
	{
	public:
		explicit GlobalNotificationDB(const QString &connSuffix = "");

		bool open();

		bool clearGlobalNotifications();
		bool setGlobalNotifications(const QList<GlobalNotificationDetail> &globalNotifications);
		QList<GlobalNotificationDetail> globalNotifications();

		bool setGlobalNotification(const GlobalNotificationDetail &globalNotification);
		bool delGlobalNotification(const QString &id);
		GlobalNotificationDetail globalNotification(const QString &id);

		bool storeMenu(const QString &globalNotificationId, const QString &menuStr);
		QString getMenu(const QString &globalNotificationId);
		QMap<QString, QString> getMenus();

		bool addLastMsg(const GlobalNotificationLastMsgModel::LastMsgItem &item);
		bool removeLastMsg(const QString &globalNotificationId);
		QList<GlobalNotificationLastMsgModel::LastMsgItem> lastMsgs();
	};
}

#endif //_GLOBALNOTIFICATIONDETAILDB_H_
