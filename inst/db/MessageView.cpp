#include <QtSql>
#include "Account.h"
#include "MessageView.h"
#include "ChatMessageDB.h"
#include "GroupMessageDB.h"
#include "DiscussMessageDB.h"

namespace DB
{
	const QString DB_CHAT_MESSAGE_TABLENAME = "chatmessages";
	const QString DB_GROUP_MESSAGE_TABLENAME = "groupmessages";
	const QString DB_DISCUSS_MESSAGE_TABLENAME = "discussmessages";

	const QString MessageView::DB_MESSAGE_VIEWNAME = "messagesview";

	MessageView::MessageView(const QString &connSuffix)
	{
		m_Connname = QString("MessageView_%1_%2").arg(DB_MESSAGE_VIEWNAME).arg(connSuffix);
		m_Tag = QString("[MessageView] %1").arg(m_Connname);
	}

	bool MessageView::open()
	{
		bool bRet = false;
		do 
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, m_Connname);
			if (!db.isValid())
			{
				break;
			}

			Account *pAccount = Account::instance();
			Q_ASSERT(pAccount != NULL);

			m_selfId = pAccount->id();
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
			QString connSuffix("messageview");
			if (!db.tables().contains(DB_CHAT_MESSAGE_TABLENAME))
			{
				ChatMessageDB *chatMessageDB = new ChatMessageDB(connSuffix);
				chatMessageDB->open();
				delete chatMessageDB;
				chatMessageDB = 0;
			}

			if (!db.tables().contains(DB_GROUP_MESSAGE_TABLENAME))
			{
				GroupMessageDB *groupMessageDB = new GroupMessageDB(connSuffix);
				groupMessageDB->open();
				delete groupMessageDB;
				groupMessageDB = 0;
			}

			if (!db.tables().contains(DB_DISCUSS_MESSAGE_TABLENAME))
			{
				DiscussMessageDB *discussMessageDB = new DiscussMessageDB(connSuffix);
				discussMessageDB->open();
				delete discussMessageDB;
				discussMessageDB = 0;
			}
			//////////////////////////////////////////////////////////////////////////

			if (!db.tables(QSql::Views).contains(DB_MESSAGE_VIEWNAME))
			{
				QSqlQuery query(db);

				QString sql = QString(" create view %1 AS "
					" SELECT 1 AS [msgtype], cm.[id], cm.[uid], cm.[type] AS [exttype], cm.[time], cm.[stamp], cm.[body], cm.[attachscount], cm.[messagexml], cm.[readstate], cm.[sequence], cm.[sync] "
					" FROM %2 AS cm "

					" UNION "

					" SELECT 2 AS [msgtype], gm.[id], gm.[group] AS [uid], gm.[type] AS [exttype], gm.[time], gm.[stamp], gm.[body], gm.[attachscount], gm.[messagexml], 0 AS [readstate], \"\" AS [sequence], gm.[sync] "
					" FROM %3 AS gm "

					" UNION "

					" SELECT 4 AS [msgtype], dm.[id], dm.[group] AS [uid], dm.[type] AS [exttype], dm.[time], dm.[stamp], dm.[body], dm.[attachscount], dm.[messagexml], 0 AS [readstate], \"\" AS [sequence], dm.[sync] "
					" FROM %4 AS dm "
					)
					.arg(DB_MESSAGE_VIEWNAME)
					.arg(DB_CHAT_MESSAGE_TABLENAME)
					.arg(DB_GROUP_MESSAGE_TABLENAME)
					.arg(DB_DISCUSS_MESSAGE_TABLENAME);

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

	int MessageView::getMessageCount(const QString &begindate, const QString &enddate, const QString &keyword)
	{
		int nRet = 0;

		do 
		{
			if (keyword.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
					break;
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

			QString sql = QString(
				"select count(*) from [%1] where body like \'%%2%\' and attachscount = 0"
				" and exttype != 8")
				.arg(DB_MESSAGE_VIEWNAME).arg(keyword); // 8 is secret message

			sql += QString(" and uid not like \'%1\'").arg(Account::fullIdFromIdResource(m_selfId, RESOURCE_PHONE));

			if (!begindate.isEmpty())
			{
				sql += QString(" and time >= datetime(\'%1\')").arg(begindate);
			}
			if (!enddate.isEmpty())
			{
				sql += QString(" and time <= datetime(\'%1\')").arg(enddate);
			}

			QList<QVariantList> data = query(sql, QStringList());

			if (!data.isEmpty())
			{
				QVariantList& vl = data[0];
				if (!vl.isEmpty())
				{
					nRet = vl[0].toInt();
				}
			}

		} while (0);

		return nRet;
	}

	QList<bean::MessageBody> MessageView::getMessages(int nOffset, int nLimit, 
		const QString &begindate, const QString &enddate, const QString &keyword)
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (keyword.isEmpty())
				break;

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

			QString whereClause = QString("where body like \'%%1%\' and attachscount = 0"
				" and exttype != 8")
				.arg(keyword); // 8 is secret message
			
			whereClause += QString(" and uid not like \'%1\'").arg(Account::fullIdFromIdResource(m_selfId, RESOURCE_PHONE));

			if (!begindate.isEmpty())
			{
				whereClause += QString(" and time >= datetime(\'%1\')").arg(begindate);
			}
			if (!enddate.isEmpty())
			{
				whereClause += QString(" and time <= datetime(\'%1\')").arg(enddate);
			}
			whereClause += " order by [time] asc, [stamp] asc limit ? offset ? ";

			QString sql = QString("select msgtype, id, uid, attachscount, messagexml, readstate, sequence, sync from [%1] ")
				.arg(DB_MESSAGE_VIEWNAME);
			sql += whereClause; 
			qDebug() << Q_FUNC_INFO << sql << whereClause;

			QList<QVariantList> data = query(sql, QStringList() << QString::number(nLimit) << QString::number(nOffset));

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != 8)
					continue;

				bool bOk = false;
				int nMsgId = vl[1].toInt(&bOk);
				if (bOk && nMsgId == -1)
					continue;

				QString uid = vl[2].toString();
				int attachsCount = vl[3].toInt();
				int readState = vl[5].toInt();
				QString sequence = vl[6].toString();
				int sync = vl[7].toInt();

				bean::MessageBody body = bean::MessageBodyFactory::fromXml(vl[4].toString());
				body.setMessageid(nMsgId);
				body.setAttachsCount(attachsCount);
				body.setReadState(readState);
				body.setSequence(sequence);
				body.setSync(sync == 1);
				listRet.append(body);
			}

		} while (0);

		return listRet;
	}

}



