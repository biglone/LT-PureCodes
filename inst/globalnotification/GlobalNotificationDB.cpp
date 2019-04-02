#include <QThread>
#include <QtSql>

#include "Account.h"
#include "GlobalNotificationDB.h"
#include "globalnotificationmsg.h"

static const char DB_SQL_DETAIL_CREATE[]           = "create table [%1]([id] primary key not null, [name] not null, [type] integer not null, [logo], [num], [introduction], [special] integer default 0);";
static const char DB_SQL_MENU_CREATE[]             = "create table [%1]([id] primary key not null, [menu]);";
static const char DB_SQL_LAST_CREATE[]             = "create table [%1]([id] integer primary key not null, [subid], [subname], [lastbody], [lasttime], [innerid], [send] integer default 0);";
static const char DB_SQL_LAST_CREATE_INDEX[]       = "create unique index [IK_LAST] on [%1] ( [subid] );";
static const char DB_SQL_LAST_ADD_SEND[]           = "ALTER TABLE %1 ADD send integer default 0;";

static const QString DB_GLOBALNOTIFICATIONDETAIL_TABLE = "detail";
static const QString DB_GLOBALNOTIFICATIONMENU_TABLE   = "menu";
static const QString DB_GLOBALNOTIFICATIONLAST_TABLE   = "last";

namespace DB
{
	GlobalNotificationDB::GlobalNotificationDB(const QString &connSuffix /*= ""*/)
	{
		m_Connname = QString("GlobalNotificationDB_%1_%2").arg((int)(QThread::currentThreadId())).arg(connSuffix);
		m_Tag = QString("[GlobalNotificationDB] %1").arg(m_Connname);
	}

	bool GlobalNotificationDB::open()
	{
		bool ret = false;
		do 
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, m_Connname);
			if (!db.isValid())
			{
				break;
			}

			Account* pAccount = Account::instance();
			Q_ASSERT(pAccount != NULL);

			m_Filename = pAccount->globalNotificationDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			QSqlQuery query(db);

			// create detail table
			if (!db.tables().contains(DB_GLOBALNOTIFICATIONDETAIL_TABLE))
			{
				QString sql = QString(DB_SQL_DETAIL_CREATE).arg(DB_GLOBALNOTIFICATIONDETAIL_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// create menu table
			query.clear();
			if (!db.tables().contains(DB_GLOBALNOTIFICATIONMENU_TABLE))
			{
				QString sql = QString(DB_SQL_MENU_CREATE).arg(DB_GLOBALNOTIFICATIONMENU_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// create last table
			if (!db.tables().contains(DB_GLOBALNOTIFICATIONLAST_TABLE))
			{
				query.clear();
				QString sql = QString(DB_SQL_LAST_CREATE).arg(DB_GLOBALNOTIFICATIONLAST_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}

				query.clear();
				sql = QString(DB_SQL_LAST_CREATE_INDEX).arg(DB_GLOBALNOTIFICATIONLAST_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_GLOBALNOTIFICATIONLAST_TABLE).contains("send"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_LAST_ADD_SEND).arg(DB_GLOBALNOTIFICATIONLAST_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			ret = true;
		} while (0);

		return ret;
	}

	bool GlobalNotificationDB::clearGlobalNotifications()
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_GLOBALNOTIFICATIONDETAIL_TABLE, "", QStringList());
	}

	bool GlobalNotificationDB::setGlobalNotifications(const QList<GlobalNotificationDetail> &globalNotifications)
	{
		if (globalNotifications.isEmpty())
			return false;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		QSqlDatabase db = QSqlDatabase::database(m_Connname);
		if (!db.isValid())
		{
			qWarning() << m_Tag << " db is invalid.";
			return false;
		}

		db.transaction();

		bool bOk = false;
		do 
		{
			int count = 0;
			foreach (GlobalNotificationDetail globalNotification, globalNotifications)
			{
				QVariantMap vmap = globalNotification.toDBMap();
				if (!replace(DB_GLOBALNOTIFICATIONDETAIL_TABLE, vmap))
				{
					qDebug() << Q_FUNC_INFO << " replace " << DB_GLOBALNOTIFICATIONDETAIL_TABLE << vmap << " failed.";
					break;
				}
				++count;
			}

			if (count == globalNotifications.count())
			{
				bOk = true;
			}

		} while (0);

		if (bOk)
		{
			db.commit();
		}
		else
		{
			db.rollback();
		}

		return bOk;
	}

	QList<GlobalNotificationDetail> GlobalNotificationDB::globalNotifications()
	{
		QList<GlobalNotificationDetail> result;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return result;
			}
		}

		QString sql = QString("select * from %1").arg(DB_GLOBALNOTIFICATIONDETAIL_TABLE);
		QList<QVariantList> items = query(sql, QStringList());
		int fieldsCount = fields(DB_GLOBALNOTIFICATIONDETAIL_TABLE).count();
		for (int i = 0; i < items.count(); i++)
		{
			QVariantList item = items[i];
			if (item.count() != fieldsCount)
			{
				continue;
			}

			// id  name  type  logo  num  introduction special
			QVariantMap data;
			data["id"] = item[0];
			data["name"] = item[1];
			data["type"] = item[2];
			data["logo"] = item[3];
			data["num"] = item[4];
			data["introduction"] = item[5];
			data["special"] = item[6];
			GlobalNotificationDetail globalNotification;
			globalNotification.fromDBMap(data);
			result.append(globalNotification);
		}

		return result;
	}

	bool GlobalNotificationDB::setGlobalNotification(const GlobalNotificationDetail &globalNotification)
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		QVariantMap vmap = globalNotification.toDBMap();
		if (!replace(DB_GLOBALNOTIFICATIONDETAIL_TABLE, vmap))
		{
			qDebug() << Q_FUNC_INFO << " replace " << DB_GLOBALNOTIFICATIONDETAIL_TABLE << vmap << " failed.";
			return false;
		}

		return true;
	}

	bool GlobalNotificationDB::delGlobalNotification(const QString &id)
	{
		if (id.isEmpty())
			return false;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_GLOBALNOTIFICATIONDETAIL_TABLE, "id = ?", QStringList() << id);
	}

	GlobalNotificationDetail GlobalNotificationDB::globalNotification(const QString &id)
	{
		GlobalNotificationDetail globalNotification;
		if (id.isEmpty())
			return globalNotification;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return globalNotification;
			}
		}

		QString sql = QString("select * from %1 where id = ?").arg(DB_GLOBALNOTIFICATIONDETAIL_TABLE);
		QList<QVariantList> items = query(sql, QStringList() << id);
		if (items.count() != 1)
			return globalNotification;

		int fieldsCount = fields(DB_GLOBALNOTIFICATIONDETAIL_TABLE).count();
		QVariantList item = items[0];
		if (item.count() != fieldsCount)
		{
			return globalNotification;
		}

		// id  name  type  logo  num  introduction special
		QVariantMap data;
		data["id"] = item[0];
		data["name"] = item[1];
		data["type"] = item[2];
		data["logo"] = item[3];
		data["num"] = item[4];
		data["introduction"] = item[5];
		data["special"] = item[6];
		globalNotification.fromDBMap(data);

		return globalNotification;
	}

	bool GlobalNotificationDB::storeMenu(const QString &globalNotificationId, const QString &menuStr)
	{
		bool ret = false;

		do 
		{
			if (globalNotificationId.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QVariantMap vmap;
			vmap["id"] = globalNotificationId;
			vmap["menu"] = menuStr;

			ret = replace(DB_GLOBALNOTIFICATIONMENU_TABLE, vmap);

		} while (0);

		return ret;
	}

	QString GlobalNotificationDB::getMenu(const QString &globalNotificationId)
	{
		QString menuStr;

		do 
		{
			if (globalNotificationId.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QString sql = QString("select menu from [%1] where [id] = ?").arg(DB_GLOBALNOTIFICATIONMENU_TABLE);

			QList<QVariantList> data = query(sql, QStringList() << globalNotificationId);

			if (!data.isEmpty())
			{
				QVariantList &vl = data[0];
				if (!vl.isEmpty())
				{
					menuStr = vl[0].toString();
				}
			}

		} while (0);

		return menuStr;
	}

	QMap<QString, QString> GlobalNotificationDB::getMenus()
	{
		QMap<QString, QString> menus;

		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QString sql = QString("select * from [%1]").arg(DB_GLOBALNOTIFICATIONMENU_TABLE);

			QList<QVariantList> data = query(sql);

			for (int i = 0; i < data.count(); i++)
			{
				QVariantList &vl = data[i];
				if (vl.count() != fields(DB_GLOBALNOTIFICATIONMENU_TABLE).count())
					continue;

				menus.insert(vl[0].toString(), vl[1].toString());
			}

		} while (0);

		return menus;
	}

	bool GlobalNotificationDB::addLastMsg(const GlobalNotificationLastMsgModel::LastMsgItem &item)
	{
		bool ret = false;

		do 
		{
			if (item.subId.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QVariantMap vmap;
			vmap["subid"] = item.subId;
			vmap["subname"] = item.subName;
			vmap["lastbody"] = item.lastBody;
			vmap["lasttime"] = item.lastTime;
			vmap["innerid"] = item.innerId;
			vmap["send"] = (item.send ? 1 : 0);

			ret = replace(DB_GLOBALNOTIFICATIONLAST_TABLE, vmap);

		} while (0);

		return ret;
	}

	bool GlobalNotificationDB::removeLastMsg(const QString &globalNotificationId)
	{
		if (globalNotificationId.isEmpty())
			return false;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_GLOBALNOTIFICATIONLAST_TABLE, "subid = ?", QStringList() << globalNotificationId);
	}

	QList<GlobalNotificationLastMsgModel::LastMsgItem> GlobalNotificationDB::lastMsgs()
	{
		QList<GlobalNotificationLastMsgModel::LastMsgItem> items;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QStringList fileds = fields(DB_GLOBALNOTIFICATIONLAST_TABLE);

			QString sql = QString("select * from [%1] order by [innerid] desc").arg(DB_GLOBALNOTIFICATIONLAST_TABLE);
			QList<QVariantList> data = query(sql);
			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fileds.length())
					continue;

				GlobalNotificationLastMsgModel::LastMsgItem item;
				item.subId = vl[1].toString();
				item.subName = vl[2].toString();
				item.lastBody = vl[3].toString();
				item.lastTime = vl[4].toString();
				item.innerId = vl[5].toString();
				item.send = (vl[6].toInt() == 1 ? true : false);
			
				items.append(item);
			}

		} while (0);

		return items;
	}
}
