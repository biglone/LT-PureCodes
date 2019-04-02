#include <QThread>
#include <QtSql>

#include "Account.h"
#include "SubscriptionMessagesDB.h"

static const char DB_SQL_MSG_CREATE[]               = "create table [%1]([innerId] integer primary key autoincrement, [id] not null, [msgId] not null, [userId] not null, [subscriptionId] not null, [type] integer not null, [send] integer default 0, [content], [createTime] varchar(64));";
static const char DB_SQL_UNREAD_CREATE[]            = "create table [%1]([id] primary key not null, [count] integer default 0);";

static const QString DB_SUBSCRIPTIONMSG_TABLE       = "message";
static const QString DB_SUBSCRIPTIONUNREAD_TABLE    = "unread";

namespace DB
{
	SubscriptionMessagesDB::SubscriptionMessagesDB(const QString &connSuffix /*= ""*/)
	{
		m_Connname = QString("SubscriptionMessagesDB_%1_%2").arg((int)(QThread::currentThreadId())).arg(connSuffix);
		m_Tag = QString("[SubscriptionMessagesDB] %1").arg(m_Connname);
	}

	bool SubscriptionMessagesDB::open()
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

			m_Filename = pAccount->subscriptionMessagesDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			QSqlQuery query(db);

			// create message table
			if (!db.tables().contains(DB_SUBSCRIPTIONMSG_TABLE))
			{
				QString sql = QString(DB_SQL_MSG_CREATE).arg(DB_SUBSCRIPTIONMSG_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// create unread table
			query.clear();
			if (!db.tables().contains(DB_SUBSCRIPTIONUNREAD_TABLE))
			{
				QString sql = QString(DB_SQL_UNREAD_CREATE).arg(DB_SUBSCRIPTIONUNREAD_TABLE);
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

	quint64 SubscriptionMessagesDB::maxInnerId()
	{
		quint64 msgId = 0;
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

			QString sql = QString("select MAX(innerId) from [%1]").arg(DB_SUBSCRIPTIONMSG_TABLE);
			QList<QVariantList> data = query(sql);

			if (!data.isEmpty())
			{
				QVariantList& vl = data[0];
				if (!vl.isEmpty())
				{
					msgId = vl[0].toULongLong();
				}
			}

		} while (0);

		return msgId;
	}

	quint64 SubscriptionMessagesDB::storeMessage(const SubscriptionMsg &msg)
	{
		quint64 msgId = 0;
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

			QVariantMap vmap = msg.toDBMap();
			if (!insert(DB_SUBSCRIPTIONMSG_TABLE, vmap))
			{
				qDebug() << Q_FUNC_INFO << " insert failed";
				break;
			}

			msgId = lastInsertId();

		} while (0);

		return msgId;
	}

	int SubscriptionMessagesDB::getMessageCount(const QString &id, quint64 lastInnerId /*= 0*/)
	{
		int nRet = 0;

		do 
		{
			if (id.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QStringList selectionArgs;
			QString sql = QString("select count(*) from [%1] where [subscriptionId] = ?").arg(DB_SUBSCRIPTIONMSG_TABLE);
			selectionArgs << id;

			if (lastInnerId > 0)
			{
				sql += QString(" and [innerId] < ?");
				selectionArgs << QString::number(lastInnerId);
			}

			QList<QVariantList> data = query(sql, selectionArgs);

			if (!data.isEmpty())
			{
				QVariantList &vl = data[0];
				if (!vl.isEmpty())
				{
					nRet = vl[0].toInt();
				}
			}

		} while (0);

		return nRet;
	}

	QList<SubscriptionMsg> SubscriptionMessagesDB::getMessages(const QString &id, quint64 lastInnerId /*= 0*/, int count /*= 10*/)
	{
		QList<SubscriptionMsg> listRet;

		do 
		{
			if (id.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					qDebug() << Q_FUNC_INFO << " open failed";
					break;
				}
			}

			QStringList selectionArgs;
			QString sql = QString("select * from [%1] where ").arg(DB_SUBSCRIPTIONMSG_TABLE);
			sql += "[subscriptionId] = ?";
			selectionArgs << id;
			if (lastInnerId > 0)
			{
				sql += QString(" and [innerId] < ?");
				selectionArgs << QString::number(lastInnerId);
			}
			sql += " order by [innerId] desc limit ? offset 0";
			selectionArgs << QString::number(count);

			int fieldsCount = fields(DB_SUBSCRIPTIONMSG_TABLE).count();
			QList<QVariantList> data = query(sql, selectionArgs);

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fieldsCount)
					continue;

				QVariantMap data;
				data["innerId"] = vl[0];
				data["id"] = vl[1];
				data["msgId"] = vl[2];
				data["userId"] = vl[3];
				data["subscriptionId"] = vl[4];
				data["type"] = vl[5];
				data["send"] = vl[6];
				data["content"] = vl[7];
				data["createTime"] = vl[8];

				SubscriptionMsg msg;
				msg.fromDBMap(data);
				listRet.insert(0, msg);
			}
		} while (0);

		return listRet;
	}

	bool SubscriptionMessagesDB::removeMessages(const QString &subscriptionId)
	{
		if (subscriptionId.isEmpty())
			return false;

		if (!isOpen())
		{
			if (!open())
			{
				qDebug() << Q_FUNC_INFO << " open failed";
				return false;
			}
		}

		QStringList whereArg;
		whereArg << subscriptionId;
		deleteRows(DB_SUBSCRIPTIONMSG_TABLE, "subscriptionId=?", whereArg);
		return true;
	}

	QMap<QString, int> SubscriptionMessagesDB::unreadMsgCount()
	{
		QMap<QString, int> unreadCount;

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

			int fieldsCount = fields(DB_SUBSCRIPTIONUNREAD_TABLE).count();
			QString sql = QString("select * from [%1]").arg(DB_SUBSCRIPTIONUNREAD_TABLE);

			QList<QVariantList> data = query(sql, QStringList());

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fieldsCount)
					continue;

				QString id = vl[0].toString();
				int count = vl[1].toInt();
				unreadCount.insert(id, count);
			}

		} while (0);

		return unreadCount;
	}

	/*
	int SubscriptionMessagesDB::unreadMsgCount(const QString &subscriptionId)
	{
		int nRet = 0;

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

			QString sql = QString("select count from [%1] where [id] = ?").arg(DB_SUBSCRIPTIONUNREAD_TABLE);

			QList<QVariantList> data = query(sql, QStringList() << subscriptionId);

			if (!data.isEmpty())
			{
				QVariantList &vl = data[0];
				if (!vl.isEmpty())
				{
					nRet = vl[0].toInt();
				}
			}

		} while (0);

		return nRet;
	}
	*/

	bool SubscriptionMessagesDB::setUnreadMsgCount(const QString &subscriptionId, int count)
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
			vmap["count"] = count;

			ret = replace(DB_SUBSCRIPTIONUNREAD_TABLE, vmap);

		} while (0);

		return ret;
	}

}
