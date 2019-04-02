#ifndef _USERDB_H_
#define _USERDB_H_

#include "DBBase.h"
#include <QString>
#include <QDateTime>
#include <QList>

namespace DB
{
	class UserDB : public DBBase
	{
	public:
		struct PhoneItem
		{
			PhoneItem() {countryCode = ""; phone=""; passwd=""; storePasswd=false; dateTime=QDateTime::currentDateTimeUtc();}

			QString   countryCode;
			QString   phone;
			QString   passwd;
			bool      storePasswd;
			QDateTime dateTime;
		};

		struct UserIdItem
		{
			UserIdItem() {uid=""; uname=""; cid=""; cname=""; phone=""; status=0; isDefault=false;}

			QString   uid;
			QString   uname;
			QString   cid;
			QString   cname;
			QString   phone;
			int       status;
			bool      isDefault;
		};

	public:
		UserDB();
		bool open();

		QList<UserDB::PhoneItem> getAccountsByUpdateTimeDesc();
		bool storeAccount(const PhoneItem &account);
		bool deleteAccount(const QString &phone);
		bool updateAccountTime(const QString &phone, const QDateTime &dateTime);
		bool clearPasswd(const QString &phone);

		bool deleteUserIdsForPhone(const QString &phone);
		bool storeUserId(const UserIdItem &userIdItem);
		bool setUserIdDefault(const QString &uid);
		QList<UserDB::UserIdItem> userIdsForPhone(const QString &phone);
		QString defaultUserId(const QString &phone);
	};
}
#endif // _USERDB_H_