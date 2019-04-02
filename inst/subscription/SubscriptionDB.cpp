#include <QThread>
#include <QtSql>

#include "Account.h"
#include "SubscriptionDB.h"
#include "subscriptionmsg.h"

static const char DB_SQL_DETAIL_CREATE[]           = "create table [%1]([id] primary key not null, [name] not null, [type] integer not null, [logo], [num], [introduction], [special] integer default 0);";
static const char DB_SQL_MENU_CREATE[]             = "create table [%1]([id] primary key not null, [menu]);";
static const char DB_SQL_LAST_CREATE[]             = "create table [%1]([id] integer primary key not null, [subid], [subname], [lastbody], [lasttime], [innerid], [send] integer default 0);";
static const char DB_SQL_LAST_CREATE_INDEX[]       = "create unique index [IK_LAST] on [%1] ( [subid] );";
static const char DB_SQL_LAST_ADD_SEND[]           = "ALTER TABLE %1 ADD send integer default 0;";

static const QString DB_SUBSCRIPTIONDETAIL_TABLE = "detail";
static const QString DB_SUBSCRIPTIONMENU_TABLE   = "menu";
static const QString DB_SUBSCRIPTIONLAST_TABLE   = "last";

namespace DB
{
	SubscriptionDB::SubscriptionDB(const QString &connSuffix /*= ""*/)
	{
		m_Connname = QString("SubscriptionDB_%1_%2").arg((int)(QThread::currentThreadId())).arg(connSuffix);
		m_Tag = QString("[SubscriptionDB] %1").arg(m_Connname);
	}

	bool SubscriptionDB::open()
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

			m_Filename = pAccount->subscriptionDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			QSqlQuery query(db);

			// create detail table
			if (!db.tables().contains(DB_SUBSCRIPTIONDETAIL_TABLE))
			{
				QString sql = QString(DB_SQL_DETAIL_CREATE).arg(DB_SUBSCRIPTIONDETAIL_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// create menu table
			query.clear();
			if (!db.tables().contains(DB_SUBSCRIPTIONMENU_TABLE))
			{
				QString sql = QString(DB_SQL_MENU_CREATE).arg(DB_SUBSCRIPTIONMENU_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// create last table
			if (!db.tables().contains(DB_SUBSCRIPTIONLAST_TABLE))
			{
				query.clear();
				QString sql = QString(DB_SQL_LAST_CREATE).arg(DB_SUBSCRIPTIONLAST_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}

				query.clear();
				sql = QString(DB_SQL_LAST_CREATE_INDEX).arg(DB_SUBSCRIPTIONLAST_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_SUBSCRIPTIONLAST_TABLE).contains("send"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_LAST_ADD_SEND).arg(DB_SUBSCRIPTIONLAST_TABLE);
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

	bool SubscriptionDB::clearSubscriptions()
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_SUBSCRIPTIONDETAIL_TABLE, "", QStringList());
	}

	bool SubscriptionDB::setSubscriptions(const QList<SubscriptionDetail> &subscriptions)
	{
		if (subscriptions.isEmpty())
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
			foreach (SubscriptionDetail subscription, subscriptions)
			{
				QVariantMap vmap = subscription.toDBMap();
				if (!replace(DB_SUBSCRIPTIONDETAIL_TABLE, vmap))
				{
					qDebug() << Q_FUNC_INFO << " replace " << DB_SUBSCRIPTIONDETAIL_TABLE << vmap << " failed.";
					break;
				}
				++count;
			}

			if (count == subscriptions.count())
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

	QList<SubscriptionDetail> SubscriptionDB::subscriptions()
	{
		QList<SubscriptionDetail> result;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return result;
			}
		}

		QString sql = QString("select * from %1").arg(DB_SUBSCRIPTIONDETAIL_TABLE);
		QList<QVariantList> items = query(sql, QStringList());
		int fieldsCount = fields(DB_SUBSCRIPTIONDETAIL_TABLE).count();
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
			SubscriptionDetail subscription;
			subscription.fromDBMap(data);
			result.append(subscription);
		}

		return result;
	}

	bool SubscriptionDB::setSubscription(const SubscriptionDetail &subscription)
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		QVariantMap vmap = subscription.toDBMap();
		if (!replace(DB_SUBSCRIPTIONDETAIL_TABLE, vmap))
		{
			qDebug() << Q_FUNC_INFO << " replace " << DB_SUBSCRIPTIONDETAIL_TABLE << vmap << " failed.";
			return false;
		}

		return true;
	}

	bool SubscriptionDB::delSubscription(const QString &id)
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

		return deleteRows(DB_SUBSCRIPTIONDETAIL_TABLE, "id = ?", QStringList() << id);
	}

	SubscriptionDetail SubscriptionDB::subscription(const QString &id)
	{
		SubscriptionDetail subscription;
		if (id.isEmpty())
			return subscription;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return subscription;
			}
		}

		QString sql = QString("select * from %1 where id = ?").arg(DB_SUBSCRIPTIONDETAIL_TABLE);
		QList<QVariantList> items = query(sql, QStringList() << id);
		if (items.count() != 1)
			return subscription;

		int fieldsCount = fields(DB_SUBSCRIPTIONDETAIL_TABLE).count();
		QVariantList item = items[0];
		if (item.count() != fieldsCount)
		{
			return subscription;
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
		subscription.fromDBMap(data);

		return subscription;
	}

	bool SubscriptionDB::storeMenu(const QString &subscriptionId, const QString &menuStr)
	{
		bool ret = false;

		do 
		{
			if (subscriptionId.isEmpty())
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
			vmap["id"] = subscriptionId;
			vmap["menu"] = menuStr;

			ret = replace(DB_SUBSCRIPTIONMENU_TABLE, vmap);

		} while (0);

		return ret;
	}

	QString SubscriptionDB::getMenu(const QString &subscriptionId)
	{
		QString menuStr;

		do 
		{
			if (subscriptionId.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QString sql = QString("select menu from [%1] where [id] = ?").arg(DB_SUBSCRIPTIONMENU_TABLE);

			QList<QVariantList> data = query(sql, QStringList() << subscriptionId);

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

	QMap<QString, QString> SubscriptionDB::getMenus()
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

			QString sql = QString("select * from [%1]").arg(DB_SUBSCRIPTIONMENU_TABLE);

			QList<QVariantList> data = query(sql);

			for (int i = 0; i < data.count(); i++)
			{
				QVariantList &vl = data[i];
				if (vl.count() != fields(DB_SUBSCRIPTIONMENU_TABLE).count())
					continue;

				menus.insert(vl[0].toString(), vl[1].toString());
			}

		} while (0);

		return menus;
	}

	bool SubscriptionDB::addLastMsg(const SubscriptionLastMsgModel::LastMsgItem &item)
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

			ret = replace(DB_SUBSCRIPTIONLAST_TABLE, vmap);

		} while (0);

		return ret;
	}

	bool SubscriptionDB::removeLastMsg(const QString &subscriptionId)
	{
		if (subscriptionId.isEmpty())
			return false;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_SUBSCRIPTIONLAST_TABLE, "subid = ?", QStringList() << subscriptionId);
	}

	QList<SubscriptionLastMsgModel::LastMsgItem> SubscriptionDB::lastMsgs()
	{
		QList<SubscriptionLastMsgModel::LastMsgItem> items;
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

			QStringList fileds = fields(DB_SUBSCRIPTIONLAST_TABLE);

			QString sql = QString("select * from [%1] order by [innerid] desc").arg(DB_SUBSCRIPTIONLAST_TABLE);
			QList<QVariantList> data = query(sql);
			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fileds.length())
					continue;

				SubscriptionLastMsgModel::LastMsgItem item;
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
