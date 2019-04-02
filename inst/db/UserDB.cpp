#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "settings/GlobalSettings.h"
#include "UserDB.h"

static const char GLOBAL_DB_FILENAME[]                   = "Global.db";
static const char DB_SQL_USER_CREATE_PHONES[]            = "create table [%1] ([id] integer primary key not null, [phone], [passwd], [storepasswd] default 0, [updatetime] default (datetime('now', 'localtime')))";
static const char DB_SQL_USER_CREATE_INDEX_PHONES[]      = "create unique index [USERS_DB_IK_PHONE] on [%1] ( [phone] )";
static const char DB_SQL_USER_ADD_COUNTRYCODE[]          = "alter table %1 add countrycode default '86';";

static const char DB_SQL_USER_CREATE_USERIDS[]           = "create table [%1] ([id] integer primary key not null, [uid], [uname], [cid], [cname], [phone], [status] default 0, [default] default 0)";
static const char DB_SQL_USER_CREATE_INDEX_USERIDS[]     = "create unique index [USERS_DB_IK_UID] on [%1] ( [uid] )";

namespace DB
{
	static const QString DB_USERDB_TABLENAME_PHONES  = "phones";
	static const QString DB_USERDB_TABLENAME_USERIDS = "userids";

	UserDB::UserDB() : DBBase()
	{	
		m_Connname = QString("UserDB_CONN");
		m_Tag = QString("%1 %2_%3").arg(m_Connname).arg(DB_USERDB_TABLENAME_PHONES).arg(DB_USERDB_TABLENAME_USERIDS);
	}

	bool UserDB::open()
	{
		bool bRet = false;
		do 
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, m_Connname);
			if (!db.isValid())
			{
				qWarning("%s %s %s no driver.", Q_FUNC_INFO, qPrintable(m_Tag), DBBase::DB_TYPE);
				break;
			}

			QDir dir(GlobalSettings::globalHomePath());
			m_Filename = dir.absoluteFilePath(GLOBAL_DB_FILENAME);
			qDebug() << m_Tag << m_Filename;
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);

			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			if (!db.tables().contains(DB_USERDB_TABLENAME_PHONES))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_USER_CREATE_PHONES).arg(DB_USERDB_TABLENAME_PHONES);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_USER_CREATE_INDEX_PHONES).arg(DB_USERDB_TABLENAME_PHONES);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();
			}

			if (!db.tables().contains(DB_USERDB_TABLENAME_USERIDS))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_USER_CREATE_USERIDS).arg(DB_USERDB_TABLENAME_USERIDS);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_USER_CREATE_INDEX_USERIDS).arg(DB_USERDB_TABLENAME_USERIDS);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();
			}

			if (!fields(DB_USERDB_TABLENAME_PHONES).contains("countrycode"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_USER_ADD_COUNTRYCODE).arg(DB_USERDB_TABLENAME_PHONES);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();
			}

			bRet = true;
		} while (0);
		return bRet;
	}

	QList<UserDB::PhoneItem> UserDB::getAccountsByUpdateTimeDesc()
	{
		QList<UserDB::PhoneItem> accounts;
		if (!isOpen() && !open())
		{
			return accounts;
		}

		QString sql = QString("select [phone], [passwd], [storepasswd], [updatetime], [countrycode] from [%1] order by [updatetime] desc").arg(DB_USERDB_TABLENAME_PHONES);
		QList<QVariantList> datas = query(sql);
		for (int i = 0; i < datas.count(); ++i)
		{
			QVariantList data = datas[i];
			UserDB::PhoneItem item;
			item.phone = data[0].toString();
			item.passwd = data[1].toString();
			item.storePasswd = data[2].toBool();
			item.dateTime = data[3].toDateTime();
			item.countryCode = data[4].toString();
			accounts.append(item);
		}
		return accounts;
	}

	bool UserDB::storeAccount(const PhoneItem &account)
	{
		bool ret = false;
		if (!isOpen() && !open())
			return ret;

		QVariantMap data;
		data["phone"] = account.phone;
		data["passwd"] = account.passwd;
		data["storepasswd"] = account.storePasswd;
		data["updatetime"] = account.dateTime;
		data["countrycode"] = account.countryCode;
		ret = replace(DB_USERDB_TABLENAME_PHONES, data);
		
		return ret;
	}

	bool UserDB::deleteAccount(const QString &phone)
	{
		bool ret = false;
		if (!isOpen() && !open())
			return ret;

		bool a = deleteRows(DB_USERDB_TABLENAME_PHONES, "[phone]=?", QStringList() << phone);
		bool b = deleteRows(DB_USERDB_TABLENAME_USERIDS, "[phone]=?", QStringList() << phone);
		ret = (a&&b);
		return ret;
	}

	bool UserDB::updateAccountTime(const QString &phone, const QDateTime &dateTime)
	{
		bool ret = false;
		if (!isOpen() && !open())
			return ret;

		QVariantMap vmap;
		vmap["updatetime"] = dateTime;
		ret = update(DB::DB_USERDB_TABLENAME_PHONES, vmap, "[phone]=?", QStringList() << phone);
		
		return ret;
	}

	bool UserDB::clearPasswd(const QString &phone)
	{
		bool ret = false;
		if (!isOpen() && !open())
			return ret;

		QVariantMap vmap;
		vmap["passwd"] = "";
		vmap["storepasswd"] = false;

		ret = update(DB::DB_USERDB_TABLENAME_PHONES, vmap, "[phone]=?", QStringList() << phone);
		
		return ret;
	}

	bool UserDB::deleteUserIdsForPhone(const QString &phone)
	{
		return deleteRows(DB_USERDB_TABLENAME_USERIDS, "[phone]=?", QStringList() << phone);
	}

	bool UserDB::storeUserId(const UserIdItem &userIdItem)
	{
		bool ret = false;
		if (!isOpen() && !open())
			return ret;

		QVariantMap vmap;
		vmap["uid"] = userIdItem.uid;
		vmap["uname"] = userIdItem.uname;
		vmap["cid"] = userIdItem.cid;
		vmap["cname"] = userIdItem.cname;
		vmap["phone"] = userIdItem.phone;
		vmap["status"] = userIdItem.status;
		vmap["default"] = userIdItem.isDefault ? 1 : 0;
		ret = replace(DB_USERDB_TABLENAME_USERIDS, vmap);
		return ret;
	}

	bool UserDB::setUserIdDefault(const QString &uid)
	{
		bool ret = false;
		if (!isOpen() && !open())
			return ret;

		QVariantMap vmap;
		vmap["uid"] = uid;
		vmap["default"] = 1;
		ret = update(DB::DB_USERDB_TABLENAME_USERIDS, vmap, "[uid]=?", QStringList() << uid);
		return ret;
	}

	QList<UserDB::UserIdItem> UserDB::userIdsForPhone(const QString &phone)
	{
		QList<UserDB::UserIdItem> userIdItems;
		if (!isOpen() && !open())
			return userIdItems;

		QString sql = QString("select [uid], [uname], [cid], [cname], [phone], [status], [default] from [%1] where [phone] like '%2'").arg(DB_USERDB_TABLENAME_USERIDS).arg(phone);
		QList<QVariantList> datas = query(sql);
		for (int i = 0; i < datas.count(); ++i)
		{
			QVariantList data = datas[i];
			UserDB::UserIdItem item;
			item.uid = data[0].toString();
			item.uname = data[1].toString();
			item.cid = data[2].toString();
			item.cname = data[3].toString();
			item.phone = data[4].toString();
			item.status = data[5].toInt();
			item.isDefault = (data[6].toInt() == 1) ? true : false;
			userIdItems.append(item);
		}
		return userIdItems;
	}

	QString UserDB::defaultUserId(const QString &phone)
	{
		QString uid;
		QList<UserDB::UserIdItem> items = userIdsForPhone(phone);
		if (items.isEmpty())
			return uid;

		foreach (UserDB::UserIdItem item, items)
		{
			if (item.isDefault)
			{
				uid = item.uid;
				break;
			}
		}
		return uid;
	}
}