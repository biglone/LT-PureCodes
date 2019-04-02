#include <QtSql>
#include "Account.h"
#include "AttachmentDB.h"
#include "ChatMessageDB.h"
#include "GroupMessageDB.h"
#include "DiscussMessageDB.h"

namespace DB
{
	static const QString DB_CHAT_MESSAGE_TABLENAME         = "chatmessages";
	static const QString DB_GROUP_MESSAGE_TABLENAME        = "groupmessages";
	static const QString DB_DISCUSS_MESSAGE_TABLENAME      = "discussmessages";

	static const QString DB_CHAT_ATTACHS_TABLENAME         = "chatattachments";
	static const QString DB_GROUP_ATTACHS_TABLENAME        = "groupattachments";
	static const QString DB_DISCUSS_ATTACHS_TABLENAME      = "discussattachments";

	const QString AttachmentDB::DB_ATTACH_VIEWNAME         = "attachmentsview";  // old: only attach file
	const QString AttachmentDB::DB_ATTACH_VIEWNAME2        = "attachmentsview2"; // old: attach file and attach dir
	const QString AttachmentDB::DB_ATTACH_VIEWNAME3        = "attachmentsview3"; // new: all attaches

	AttachmentDB::AttachmentDB(const QString &connSuffix)
	{
		m_Connname = QString("AttachmentDB_%1_%2").arg(DB_ATTACH_VIEWNAME3).arg(connSuffix);
		m_Tag = QString("[AttachmentDB] %1").arg(m_Connname);
	}

	bool AttachmentDB::open()
	{
		bool bRet = false;
		do 
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, m_Connname);
			if (!db.isValid())
			{
				break;
			}

			Account* pAccount = Account::instance();
			Q_ASSERT(pAccount != NULL);

			m_Filename = pAccount->messageDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			//////////////////////////////////////////////////////////////////////////
			// prepare for message dbs
			QString connSuffix("AttachmentDB");
			if (!db.tables().contains(DB_CHAT_MESSAGE_TABLENAME) || !db.tables().contains(DB_CHAT_ATTACHS_TABLENAME))
			{
				ChatMessageDB *chatMessageDB = new ChatMessageDB(connSuffix);
				chatMessageDB->open();
				delete chatMessageDB;
				chatMessageDB = 0;
			}

			if (!db.tables().contains(DB_GROUP_MESSAGE_TABLENAME) || !db.tables().contains(DB_GROUP_ATTACHS_TABLENAME))
			{
				GroupMessageDB *groupMessageDB = new GroupMessageDB(connSuffix);
				groupMessageDB->open();
				delete groupMessageDB;
				groupMessageDB = 0;
			}

			if (!db.tables().contains(DB_DISCUSS_MESSAGE_TABLENAME) || !db.tables().contains(DB_DISCUSS_ATTACHS_TABLENAME))
			{
				DiscussMessageDB *discussMessageDB = new DiscussMessageDB(connSuffix);
				discussMessageDB->open();
				delete discussMessageDB;
				discussMessageDB = 0;
			}
			//////////////////////////////////////////////////////////////////////////

			if (db.tables(QSql::Views).contains(DB_ATTACH_VIEWNAME))
			{
				QSqlQuery query(db);
				QString sql = QString("DROP VIEW %1;").arg(DB_ATTACH_VIEWNAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.finish();
			}

			if (db.tables(QSql::Views).contains(DB_ATTACH_VIEWNAME2))
			{
				QSqlQuery query(db);
				QString sql = QString("DROP VIEW %1;").arg(DB_ATTACH_VIEWNAME2);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.finish();
			}

			/* // using left join and where is extremely slow, more than 3 minutes for one query
			if (!db.tables(QSql::Views).contains(DB_ATTACH_VIEWNAME))
			{
				QSqlQuery query(db);

				QString sql = QString(" create view %1 AS "
					" SELECT 1 AS [msgtype], cm.[uid], cm.[uname], cm.[method], cm.[time] AS [date], ca.[id], ca.[messageid], ca.[name], "
					" ca.[format], ca.[filename], ca.[uuid], ca.[size], ca.[fttype], ca.[ftresult], ca.[time] "
					" FROM %2 AS cm LEFT JOIN %3 AS ca ON cm.[id] = ca.[messageid] "
					" WHERE NOT ca.[messageid] IS NULL AND ca.[fttype]=0"

					" UNION "

					" SELECT 2 AS [msgtype], gm.[group] AS [uid], gm.[groupname] AS [uname], gm.[method], gm.[time] AS [date], ga.[id], ga.[messageid], ga.[name], "
					" ga.[format], ga.[filename], ga.[uuid], ga.[size], ga.[fttype], ga.[ftresult], ga.[time] "
					" FROM %4 AS gm LEFT JOIN %5 AS ga ON gm.[id] = ga.[messageid] "
					" WHERE NOT ga.[messageid] IS NULL AND ga.[fttype] = 0"

					" UNION "

					" SELECT 4 AS [msgtype], dm.[group] AS [uid], dm.[groupname] AS [uname], dm.[method], dm.[time] AS [date], da.[id], da.[messageid], da.[name], "
					" da.[format], da.[filename], da.[uuid], da.[size], da.[fttype], da.[ftresult], da.[time] "
					" FROM %6 AS dm LEFT JOIN %7 AS da ON dm.[id] = da.[messageid] "
					" WHERE NOT da.[messageid] IS NULL AND da.[fttype] = 0;")
					.arg(DB_ATTACH_VIEWNAME)
					.arg(DB_CHAT_MESSAGE_TABLENAME)
					.arg(DB_CHAT_ATTACHS_TABLENAME)
					.arg(DB_GROUP_MESSAGE_TABLENAME)
					.arg(DB_GROUP_ATTACHS_TABLENAME)
					.arg(DB_DISCUSS_MESSAGE_TABLENAME)
					.arg(DB_DISCUSS_ATTACHS_TABLENAME);

				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.finish();
			}
			*/

			if (!db.tables(QSql::Views).contains(DB_ATTACH_VIEWNAME3))
			{
				QSqlQuery query(db);

				QString sql = QString(" create view %1 AS "
					" SELECT 1 AS [msgtype], cm.[uid], cm.[uname], cm.[method], cm.[time] AS [date], ca.[id], ca.[messageid], ca.[name], "
					" ca.[format], ca.[filename], ca.[uuid], ca.[size], ca.[fttype], ca.[ftresult], ca.[time] "
					" FROM %2 AS cm INNER JOIN %3 AS ca ON cm.[id] = ca.[messageid] "

					" UNION "

					" SELECT 2 AS [msgtype], gm.[group] AS [uid], gm.[groupname] AS [uname], gm.[method], gm.[time] AS [date], ga.[id], ga.[messageid], ga.[name], "
					" ga.[format], ga.[filename], ga.[uuid], ga.[size], ga.[fttype], ga.[ftresult], ga.[time] "
					" FROM %4 AS gm INNER JOIN %5 AS ga ON gm.[id] = ga.[messageid] "

					" UNION "

					" SELECT 4 AS [msgtype], dm.[group] AS [uid], dm.[groupname] AS [uname], dm.[method], dm.[time] AS [date], da.[id], da.[messageid], da.[name], "
					" da.[format], da.[filename], da.[uuid], da.[size], da.[fttype], da.[ftresult], da.[time] "
					" FROM %6 AS dm INNER JOIN %7 AS da ON dm.[id] = da.[messageid];"
					)
					.arg(DB_ATTACH_VIEWNAME3)
					.arg(DB_CHAT_MESSAGE_TABLENAME)
					.arg(DB_CHAT_ATTACHS_TABLENAME)
					.arg(DB_GROUP_MESSAGE_TABLENAME)
					.arg(DB_GROUP_ATTACHS_TABLENAME)
					.arg(DB_DISCUSS_MESSAGE_TABLENAME)
					.arg(DB_DISCUSS_ATTACHS_TABLENAME);

				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.finish();
			}

			bRet = true;
		} while (0);

		return bRet;
	}

	QVariantList AttachmentDB::getAttachments(int offset /* = 0 */, int limit /* = 60 */, const QString &beginDate /* = "" */, const QString &keyword /* = "" */)
	{
		QVariantList ret;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QString sql = QString ("select * from %1 ").arg(DB_ATTACH_VIEWNAME3);

			QString whereClause("where [ftresult] = 1 and ([fttype] = 0 or fttype = 3) ");
			if (!beginDate.isEmpty())
			{
				whereClause += QString("and [date] > \'%1\' ").arg(beginDate);
			}

			if (!keyword.isEmpty())
			{
				whereClause += QString("and [name] like \'%%1%\' ").arg(keyword);
			}

			sql += whereClause;
			sql += QString("order by [date] desc limit %1 offset %2").arg(limit).arg(offset);

			QSqlQuery query(db);
			if (!query.prepare(sql))
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			if (!query.exec())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			QSqlRecord rec = query.record();
			int nCount = rec.count();
			while (query.next())
			{
				QVariantMap vm;
				for (int i = 0; i < nCount; ++i)
				{
					QString key = rec.field(i).name();
					if (!key.isEmpty())
					{
						vm[key] = query.value(i);
					}
				}
				ret << vm;
			}
		} while (0);

		return ret;
	}

	int AttachmentDB::getAttachmentCount(const QString &beginDate /*= ""*/, const QString &keyword /*= ""*/)
	{
		int nRet = 0;

		do 
		{
			if (!isOpen())
			{
				if (!open())
					break;
			}

			QString sql = QString ("select count(*) from %1 ").arg(DB_ATTACH_VIEWNAME3);

			QString whereClause("where [ftresult] = 1 and ([fttype] = 0 or fttype = 3) ");
			if (!beginDate.isEmpty())
			{
				whereClause += QString("and [date] > \'%1\' ").arg(beginDate);
			}

			if (!keyword.isEmpty())
			{
				whereClause += QString("and [name] like \'%%1%\' ").arg(keyword);
			}

			sql += whereClause;

			QList<QVariantList> data = query(sql);
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

	bool AttachmentDB::remove(int index, bean::MessageType msgType)
	{
		QString table;
		if (msgType == bean::Message_Chat)
			table = DB_CHAT_ATTACHS_TABLENAME;
		else if (msgType == bean::Message_GroupChat)
			table = DB_GROUP_ATTACHS_TABLENAME;
		else if (msgType == bean::Message_DiscussChat)
			table = DB_DISCUSS_ATTACHS_TABLENAME;
		if (table.isEmpty())
			return false;

		bool ret = false;

		do 
		{
			if (!isOpen())
			{
				if (!open())
					break;
			}

			ret = deleteRows(table, "[id]=?", QStringList() << QString("%1").arg(index));
		} while (0);

		return ret;
	}
}



