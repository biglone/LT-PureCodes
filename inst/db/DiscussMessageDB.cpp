#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include "login/Account.h"
#include "DiscussMessageDB.h"

const char DB_SQL_MESSAGES_CREATE[]        = "create table [%1] ([id] integer primary key not null, [uid], [uname], [type] default 1, [group], [groupname], [method] default 0, [time], [stamp], [subject], [body], [attachscount], [messagexml], [sync] integer default 0, [sequence] default '')";
const char DB_SQL_MESSAGES_CREATE_INDEX[]  = "create unique index [IK_DISCUSSMESSAGESID] on [%1] ( [id] ); ";
const char DB_SQL_MESSAGE_ADD_SYNC[]       = "ALTER TABLE %1 ADD sync integer default 0;";
const char DB_SQL_MESSAGE_ADD_SEQUENCE[]   = "ALTER TABLE %1 ADD sequence default '';";

const char DB_SQL_ATTACHS_CREATE[]         = "create table [%1] ([id] integer primary key not null, [messageid] references [%2] ([id]), [name], [format], [filename], [uuid], [size], [fttype] not null default 0, [ftresult] not null default 0, [time], [source] default '', [picwidth] integer default 0, [picheight] integer default 0)";
const char DB_SQL_ATTACHS_CREATE_INDEX[]   = "create unique index [IK_DISCUSSATTACHMENTSUUID] on [%1] ( [uuid] )";
const char DB_SQL_ATTACHS_ADD_SOURCE[]     = "ALTER TABLE %1 ADD source default '';";
const char DB_SQL_ATTACHS_ADD_PICWIDTH[]   = "ALTER TABLE %1 ADD picwidth integer default 0;";
const char DB_SQL_ATTACHS_ADD_PICHEIGHT[]  = "ALTER TABLE %1 ADD picheight integer default 0;";

namespace DB
{
	const QString DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME = "discussmessages";
	const QString DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME = "discussattachments";

	DiscussMessageDB::DiscussMessageDB(const QString& connSuffix) : MessageDB()
	{
		m_Connname = QString("DiscussMessageDB_%1_%2_%3").arg(DB_DISCUSS_MESSAGE_TABLENAME).arg(DB_DISCUSS_ATTACHS_TABLENAME).arg(connSuffix);
		m_Tag = QString("[DiscussMessageDB] %1").arg(m_Connname);
	}

	bool DiscussMessageDB::open()
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

			m_Filename = pAccount->messageDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			if (!db.tables().contains(DB_DISCUSS_MESSAGE_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_MESSAGES_CREATE).arg(DB_DISCUSS_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_MESSAGES_CREATE_INDEX).arg(DB_DISCUSS_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_DISCUSS_MESSAGE_TABLENAME).contains("sync"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_MESSAGE_ADD_SYNC).arg(DB_DISCUSS_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_DISCUSS_MESSAGE_TABLENAME).contains("sequence"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_MESSAGE_ADD_SEQUENCE).arg(DB_DISCUSS_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!db.tables().contains(DB_DISCUSS_ATTACHS_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_ATTACHS_CREATE).arg(DB_DISCUSS_ATTACHS_TABLENAME).arg(DB_DISCUSS_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_ATTACHS_CREATE_INDEX).arg(DB_DISCUSS_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.finish();
			}

			if (!fields(DB_DISCUSS_ATTACHS_TABLENAME).contains("source"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_SOURCE).arg(DB_DISCUSS_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_DISCUSS_ATTACHS_TABLENAME).contains("picwidth"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_PICWIDTH).arg(DB_DISCUSS_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_DISCUSS_ATTACHS_TABLENAME).contains("picheight"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_PICHEIGHT).arg(DB_DISCUSS_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			{
				const QString DB_DISCUSS_MESSAGE_TRIGGER = "fk_discuss_message";
				QString sql = QString("select name from sqlite_master where type='trigger'");
				QList<QVariantList> data = query(sql);
				bool bFind = false;
				foreach (QVariantList vlist, data)
				{
					if (vlist.contains(DB_DISCUSS_MESSAGE_TRIGGER))
					{
						bFind = true;
						break;
					}
				}

				if (!bFind)
				{
					// create trigger
					sql = QString(" create trigger %1 "
						" before delete on %2 "
						" for each row "
						" begin "
						"	delete from %3 where messageid = old.id; "
						" end;").arg(DB_DISCUSS_MESSAGE_TRIGGER, DB_DISCUSS_MESSAGE_TABLENAME, DB_DISCUSS_ATTACHS_TABLENAME);

					QSqlQuery query(db);
					if (!query.exec(sql))
					{
						qWarning() << m_Tag << query.lastError();
						break;
					}
					query.finish();
				}
			}

			ret = true;
		} while (0);
		return ret;
	}

	QList<bean::AttachItem> DiscussMessageDB::getAttachments(const QString& uid, int msgid)
	{
		QList<bean::AttachItem> lstAttachs;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QStringList fs = fields(DB_DISCUSS_ATTACHS_TABLENAME);

			QString sql = QString("select * from [%1] where [messageid] = ? order by [id] asc").arg(DB_DISCUSS_ATTACHS_TABLENAME);

			QList<QVariantList> data = query(sql, QStringList() << QString("%1").arg(msgid));

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty())
				{
					continue;
				}

				bean::AttachItem item;
				item.fromAttachList(uid, vl);
				item.setMessageType(bean::Message_DiscussChat);

				lstAttachs.append(item);

			}
		} while (0);

		return lstAttachs;
	}

	int DiscussMessageDB::getMessageCount( const QString &uid, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/ )
	{
		int nRet = 0;

		do 
		{
			if (uid.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
					break;
			}

			QString sql = QString("select count(*) from [%1] where [group] like ?").arg(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME);

			if (!begindate.isEmpty())
			{
				sql += QString(" and time >= datetime(\'%1\')").arg(begindate);
			}
			if (!enddate.isEmpty())
			{
				sql += QString(" and time <= datetime(\'%1\')").arg(enddate);
			}
			if (!keyword.isEmpty())
			{
				sql += QString(" and body like  \'%%1%\' and attachscount = 0").arg(keyword);
			}

			QList<QVariantList> data = query(sql, QStringList() << uid);


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

	QList<int> DiscussMessageDB::getMessageIds(const QString &uid, 
		const QString &begindate /* = "" */, const QString &enddate /* = "" */, const QString &keyword /* = "" */)
	{
		QList<int> lstRet;

		do 
		{
			if (uid.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
					break;
			}

			QString sql = QString("select [id] from [%1] where [group] like ?").arg(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME);

			if (!begindate.isEmpty())
			{
				sql += QString(" and time >= datetime(\'%1\')").arg(begindate);
			}
			if (!enddate.isEmpty())
			{
				sql += QString(" and time <= datetime(\'%1\')").arg(enddate);
			}
			if (!keyword.isEmpty())
			{
				sql += QString(" and body like  \'%%1%\' and attachscount = 0").arg(keyword);
			}
			sql += " order by [time] asc, [stamp] asc, [id] asc";

			QList<QVariantList> data = query(sql, QStringList() << uid);

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty())
					continue;

				foreach (QVariant v, vl)
				{
					bool bOk = false;
					int id = v.toInt(&bOk);
					if (bOk)
					{
						lstRet << id;
					}
				}
			}

		} while (0);

		return lstRet;
	}

	QList<bean::MessageBody> DiscussMessageDB::getMessages( const QString &uid, int nOffset, int nLimit, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/ )
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (uid.isEmpty())
				break;

			QString sql = QString("where ");
			sql += "[group] = ?";
			if (!begindate.isEmpty())
			{
				sql += QString(" and time >= datetime(\'%1\')").arg(begindate);
			}
			if (!enddate.isEmpty())
			{
				sql += QString(" and time <= datetime(\'%1\')").arg(enddate);
			}
			if (!keyword.isEmpty())
			{
				sql += QString(" and body like \'%%1%\' and attachscount = 0").arg(keyword);
			}
			sql += " order by [time] asc, [stamp] asc, [id] asc limit ? offset ? ";

			QStringList selectArg;
			selectArg << uid << QString::number(nLimit) << QString::number(nOffset);

			listRet = selectMessages(sql, selectArg);
			
		} while (0);

		return listRet;
	}

	int DiscussMessageDB::getMessageCountBeforeTime(const QString &uid, const QString &rsDateTime)
	{
		int nRet = 0;

		do 
		{
			if (uid.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
					break;
			}

			QString sql = QString("select count(*) from [%1] where [group] like ?").arg(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME);

			if (!rsDateTime.isEmpty())
			{
				sql += QString(" and time < datetime(\'%1\')").arg(rsDateTime);
			}

			QList<QVariantList> data = query(sql, QStringList() << uid);


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

	QList<bean::MessageBody> DiscussMessageDB::getMessagesBeforeTime(const QString &uid, int nOffset, int nLimit, const QString &rsDateTime)
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (uid.isEmpty())
				break;

			QString sql = QString("where ");
			sql += "[group] = ?";
			if (!rsDateTime.isEmpty())
			{
				sql += QString(" and time < datetime(\'%1\')").arg(rsDateTime);
			}
			
			sql += " order by [time] asc, [stamp] asc, [id] asc limit ? offset ? ";

			QStringList selectArg;
			selectArg << uid << QString::number(nLimit) << QString::number(nOffset);

			listRet = selectMessages(sql, selectArg);

		} while (0);

		return listRet;
	}

	QList<int> DiscussMessageDB::getMessageIdBeforeTime(const QString &uid, const QString& rsDateTime)
	{
		QList<int> lstRet;
		do 
		{
			if (uid.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QString sql = QString("select [id] from [%1] where ").arg(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME);
			sql += " [group] like ?  ";
			if (!rsDateTime.isEmpty())
			{
				sql += QString(" and [time] < datetime(\'%1\') ").arg(rsDateTime);
			}
			sql += " order by [time] asc, [stamp] asc, [id] asc";
			QList<QVariantList> data = query(sql, QStringList() << uid);

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty())
					continue;

				foreach (QVariant v, vl)
				{
					bool bOk = false;
					int id = v.toInt(&bOk);
					if (bOk)
					{
						lstRet << id;
					}
				}
			}

		} while (0);

		return lstRet;
	}

	QList<bean::MessageBody> DiscussMessageDB::getMessagesBeforeTs(const QString &uid, const QString &ts, int count)
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (uid.isEmpty())
				break;

			if (count <= 0)
				break;

			QString sql = QString("where ");
			sql += "[group] = ?";
			if (!ts.isEmpty())
			{
				sql += QString(" and stamp < '%1'").arg(ts);
			}
			sql += " order by [stamp] desc limit ? ";

			QStringList selectArg;
			selectArg << uid << QString::number(count);

			QList<bean::MessageBody> result = selectMessages(sql, selectArg);
			foreach (bean::MessageBody item, result)
			{
				listRet.insert(0, item);
			}

		} while (0);

		return listRet;
	}

	bool DiscussMessageDB::removeMsgByMsgId(int nMsgID)
	{
		if (!isOpen())
		{
			if (!open())
			{
				return false;
			}
		}

		QStringList whereArg;
		whereArg << QString::number(nMsgID);
		deleteRows(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME, "id=?", whereArg);
		deleteRows(DB::DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME, "messageid=?", whereArg);
		return true;
	}

	quint64 DiscussMessageDB::storeMessage(const bean::MessageBody &rBody)
	{
		quint64 ret = -1;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			// begin transaction
			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid())
				break;

			db.transaction();

			quint64 nMsgId = -1;
			bool bOk = false;
			do 
			{
				// insert into messages database
				QVariantMap vmap = rBody.toMessageDBMap();
				if (!insert(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME, vmap))
				{
					qDebug() << Q_FUNC_INFO << " insert";
					break;
				}

				bool bOk1 = true;
				// insert into attachments database
				nMsgId = lastInsertId();
				foreach (bean::AttachItem item, rBody.attachs())
				{
					QVariantMap vmap = item.toAttachMap(nMsgId, false);
					if (rBody.isSend() && !rBody.sync())
					{
						vmap["ftresult"] = bean::AttachItem::Transfer_Successful;
					}
					if (!replace(DB::DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME, vmap))
					{
						bOk1 = false;
						qDebug() << Q_FUNC_INFO << " replace";
						break;
					}
				}

				bOk = bOk1;
			} while (0);

			if (bOk)
			{
				bOk = db.commit();
			}

			if (!bOk)
			{
				db.rollback();
			}
			else
			{
				ret = nMsgId;
			}
		} while (0);

		return ret;
	}

	bool DiscussMessageDB::replaceMessage(int nMsgID, const bean::MessageBody &rBody)
	{
		bool ret = false;
		do 
		{
			if (!removeMsgByMsgId(nMsgID))
			{
				qWarning() << Q_FUNC_INFO << "remove message" << nMsgID;
				break;
			}

			// begin transaction
			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid())
				break;

			db.transaction();

			bool bOk = false;
			do 
			{
				// replace into messages database
				QVariantMap vmap = rBody.toMessageDBMap();
				vmap["id"] = nMsgID;
				if (!replace(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME, vmap))
				{
					qDebug() << Q_FUNC_INFO << "replace message" << nMsgID;
					break;
				}

				bool bOk1 = true;
				// replace into attachments database
				foreach (bean::AttachItem item, rBody.attachs())
				{
					QVariantMap vmap = item.toAttachMap(nMsgID, false);
					if (rBody.isSend())
					{
						vmap["ftresult"] = bean::AttachItem::Transfer_Successful;
					}
					if (!replace(DB::DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME, vmap))
					{
						bOk1 = false;
						qDebug() << Q_FUNC_INFO << "replace attach" << nMsgID << item.uuid();
						break;
					}
				}

				bOk = bOk1;
			} while (0);

			if (bOk)
			{
				bOk = db.commit();
			}

			if (!bOk)
			{
				db.rollback();
			}
			else
			{
				ret = true;
			}
		} while (0);

		return ret;
	}

	bool DiscussMessageDB::storeAttachResult(const QString& rsUuid, int nResult)
	{
		bool ret = false;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QVariantMap vmap;
			vmap["ftresult"] = nResult;
			QStringList whereArg;
			whereArg << rsUuid;

			ret = update(DB::DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME, vmap, "uuid=?", whereArg);
		} while (0);

		return ret;
	}

	bool DiscussMessageDB::updateAttachName(const QString &rsUuid, const QString &filePath)
	{
		{
			bool ret = false;
			do 
			{
				if (!isOpen())
				{
					if (!open())
					{
						break;
					}
				}

				QVariantMap vmap;
				vmap["filename"] = filePath;
				QStringList whereArg;
				whereArg << rsUuid;

				ret = update(DB::DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME, vmap, "uuid=?", whereArg);
			} while (0);

			return ret;
		}
	}

	QString DiscussMessageDB::getMaxMessageStamp()
	{
		QString nRet;

		do 
		{
			if (!isOpen())
			{
				if (!open())
					break;
			}

			QString sql = QString("select MAX(stamp) from [%1]").arg(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME);
			QList<QVariantList> data = query(sql);

			if (!data.isEmpty())
			{
				QVariantList& vl = data[0];
				if (!vl.isEmpty())
				{
					nRet = vl[0].toString();
				}
			}

		} while (0);

		return nRet;
	}

	bean::MessageBody DiscussMessageDB::getMessageByStamp(const QString &stamp)
	{
		bean::MessageBody ret;

		do 
		{
			if (stamp.isEmpty())
				break;

			QString sql = QString("where stamp=?");

			QStringList selectArg;
			selectArg << stamp; 

			QList<bean::MessageBody> listRet = selectMessages(sql, selectArg);
			if (!listRet.isEmpty())
				ret = listRet[0];

		} while (0);

		return ret;
	}

	bean::MessageBody DiscussMessageDB::getMessageById(int msgId)
	{
		bean::MessageBody ret;

		do 
		{
			if (msgId < 0)
				break;

			QString sql = QString("where id=?");

			QStringList selectArg;
			selectArg << QString::number(msgId); 

			QList<bean::MessageBody> listRet = selectMessages(sql, selectArg);
			if (!listRet.isEmpty())
				ret = listRet[0];

		} while (0);

		return ret;
	}

	QList<bean::MessageBody> DiscussMessageDB::selectMessages(const QString &whereClause, const QStringList &whereArgs)
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QString sql = QString("select id, uid, attachscount, messagexml, sync, sequence from [%1] ").arg(DB::DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME);
			sql += whereClause;
			qDebug() << Q_FUNC_INFO << sql << whereArgs;

			QList<QVariantList> data = query(sql, whereArgs);

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != 6)
					continue;

				bool bOk = false;
				int nMsgId = vl[0].toInt(&bOk);
				if (bOk && nMsgId == -1)
					continue;

				QString uid = vl[1].toString();
				int attachsCount = vl[2].toInt();
				int sync = vl[4].toInt();
				QString sequence = vl[5].toString();

				bean::MessageBody body = bean::MessageBodyFactory::fromXml(vl[3].toString());
				body.setMessageid(nMsgId);
				body.setAttachsCount(attachsCount);
				body.setSync(sync == 1);
				body.setSequence(sequence);

				if (body.attachsCount() > 0)
				{
					body.setAttachs(getAttachments(body.from(), nMsgId));
				}
				listRet.append(body);
			}
		} while (0);

		return listRet;
	}
}