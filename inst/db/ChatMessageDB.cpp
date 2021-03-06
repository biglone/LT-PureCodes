#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "login/Account.h"
#include "ChatMessageDB.h"

const char DB_SQL_MESSAGES_CREATE[]        = "create table [%1] ([id] integer primary key not null, [uid], [uname], [type] default 1, [method] default 0, [time], [stamp], [subject], [body], [attachscount], [messagexml], [readstate] integer default 0, [sequence] default '', [sync] integer default 0 )";
const char DB_SQL_MESSAGES_CREATE_INDEX[]  = "create unique index [IK_CHATMESSAGESID] on [%1] ( [id] ); ";
const char DB_SQL_MESSAGE_ADD_READSTATE[]  = "ALTER TABLE %1 ADD readstate integer default 0;";
const char DB_SQL_MESSAGE_ADD_SEQUENCE[]   = "ALTER TABLE %1 ADD sequence default '';";
const char DB_SQL_MESSAGE_ADD_SYNC[]       = "ALTER TABLE %1 ADD sync integer default 0;";

const char DB_SQL_ATTACHS_CREATE[]         = "create table [%1] ([id] integer primary key not null, [messageid] references [%2] ([id]), [name], [format], [filename], [uuid], [size], [fttype] not null default 0, [ftresult] not null default 0, [time], [source] default '', [picwidth] integer default 0, [picheight] integer default 0)";
const char DB_SQL_ATTACHS_CREATE_INDEX[]   = "create unique index [IK_CHATATTACHMENTSUUID] on [%1] ( [uuid] )";
const char DB_SQL_ATTACHS_ADD_SOURCE[]     = "ALTER TABLE %1 ADD source default '';";
const char DB_SQL_ATTACHS_ADD_PICWIDTH[]   = "ALTER TABLE %1 ADD picwidth integer default 0;";
const char DB_SQL_ATTACHS_ADD_PICHEIGHT[]  = "ALTER TABLE %1 ADD picheight integer default 0;";

namespace DB
{
	const QString ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME = "chatmessages";
	const QString ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME = "chatattachments";

	ChatMessageDB::ChatMessageDB(const QString& connSuffix) : MessageDB()
	{
		m_Connname = QString("ChatMessageDB_%1_%2_%3").arg(DB_CHAT_MESSAGE_TABLENAME).arg(DB_CHAT_ATTACHS_TABLENAME).arg(connSuffix);
		m_Tag = QString("[ChatMessageDB] %1").arg(m_Connname);
	}

	bool ChatMessageDB::open()
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

			if (!db.tables().contains(DB_CHAT_MESSAGE_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_MESSAGES_CREATE).arg(DB_CHAT_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_MESSAGES_CREATE_INDEX).arg(DB_CHAT_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_CHAT_MESSAGE_TABLENAME).contains("readstate"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_MESSAGE_ADD_READSTATE).arg(DB_CHAT_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_CHAT_MESSAGE_TABLENAME).contains("sequence"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_MESSAGE_ADD_SEQUENCE).arg(DB_CHAT_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_CHAT_MESSAGE_TABLENAME).contains("sync"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_MESSAGE_ADD_SYNC).arg(DB_CHAT_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!db.tables().contains(DB_CHAT_ATTACHS_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_ATTACHS_CREATE).arg(DB_CHAT_ATTACHS_TABLENAME).arg(DB_CHAT_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_ATTACHS_CREATE_INDEX).arg(DB_CHAT_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_CHAT_ATTACHS_TABLENAME).contains("source"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_SOURCE).arg(DB_CHAT_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_CHAT_ATTACHS_TABLENAME).contains("picwidth"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_PICWIDTH).arg(DB_CHAT_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_CHAT_ATTACHS_TABLENAME).contains("picheight"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_PICHEIGHT).arg(DB_CHAT_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			{
				const QString DB_CHAT_MESSAGE_TRIGGER = "fk_chat_message";
				QString sql = QString("select name from sqlite_master where type='trigger'");
				QList<QVariantList> data = query(sql);
				bool bFind = false;
				foreach (QVariantList vlist, data)
				{
					if (vlist.contains(DB_CHAT_MESSAGE_TRIGGER))
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
						" end;").arg(DB_CHAT_MESSAGE_TRIGGER, DB_CHAT_MESSAGE_TABLENAME, DB_CHAT_ATTACHS_TABLENAME);

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

	QList<bean::AttachItem> ChatMessageDB::getAttachments(const QString& uid, int msgid)
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

			QStringList fs = fields(DB_CHAT_ATTACHS_TABLENAME);

			QString sql = QString("select * from [%1] where [messageid] = ? order by [id] asc").arg(DB_CHAT_ATTACHS_TABLENAME);

			QList<QVariantList> data = query(sql, QStringList() << QString("%1").arg(msgid));

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fs.length())
				{
					continue;
				}

				bean::AttachItem item;
				item.fromAttachList(uid, vl);
				item.setMessageType(bean::Message_Chat);

				lstAttachs.append(item);
			}
		} while (0);

		return lstAttachs;
	}

	int ChatMessageDB::getMessageCount( const QString &uid, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/)
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

			QString sql = QString("select count(*) from [%1] where [uid] like ?").arg(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME);
			
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
				sql += QString(" and body like \'%%1%\' and attachscount = 0 and type != 8").arg(keyword); // 8 is secret
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

	QList<int> ChatMessageDB::getMessageIds(const QString &uid, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/)
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

			QString sql = QString("select [id] from [%1] where [uid] like ?").arg(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME);

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
				sql += QString(" and body like \'%%1%\' and attachscount = 0 and type != 8").arg(keyword); // 8 is secret
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

	QList<bean::MessageBody> ChatMessageDB::getMessages( const QString &uid, int nOffset, int nLimit, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/ )
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (uid.isEmpty())
				break;

			QString sql = QString("where ");
			sql += " [uid] like ?";
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
				sql += QString(" and body like \'%%1%\' and attachscount = 0 and type != 8").arg(keyword); // 8 is secret
			}
			sql += " order by [time] asc, [stamp] asc, [id] asc limit ? offset ? ";

			QStringList selectArg;
			selectArg << uid << QString::number(nLimit) << QString::number(nOffset); 

			listRet = selectMessages(sql, selectArg);

		} while (0);

		return listRet;
	}

	int ChatMessageDB::getMessageCountBeforeTime(const QString &uid, const QString &rsDateTime)
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

			QString sql = QString("select count(*) from [%1] where [uid] like ?").arg(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME);

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

	QList<bean::MessageBody> ChatMessageDB::getMessagesBeforeTime(const QString &uid, int nOffset, int nLimit, const QString &rsDateTime)
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (uid.isEmpty())
				break;

			QString sql = QString("where ");
			sql += " [uid] like ?";
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

	QList<int> ChatMessageDB::getMessageIdBeforeTime(const QString &uid, const QString& rsDateTime)
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

			QString sql = QString("select [id] from [%1] where ").arg(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME);
			sql += " [uid] like ?  ";
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

	QList<bean::MessageBody> ChatMessageDB::getMessagesBeforeTs(const QString &uid, const QString &ts, int count)
	{
		QList<bean::MessageBody> listRet;

		do 
		{
			if (uid.isEmpty())
				break;

			if (count <= 0)
				break;

			QString sql = QString("where ");
			sql += " [uid] like ?";
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

	bool ChatMessageDB::removeMsgByMsgId(int nMsgID)
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
		deleteRows(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME, "id=?", whereArg);
		deleteRows(DB::ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME, "messageid=?", whereArg);
		return true;
	}

	quint64 ChatMessageDB::storeMessage(const bean::MessageBody &rBody)
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
				if (!insert(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME, vmap))
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
					if (!replace(DB::ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME, vmap))
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

	bool ChatMessageDB::replaceMessage(int nMsgID, const bean::MessageBody &rBody)
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
				if (!replace(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME, vmap))
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
					if (!replace(DB::ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME, vmap))
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

	bool ChatMessageDB::storeAttachResult(const QString& rsUuid, int nResult)
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

			ret = update(DB::ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME, vmap, "uuid=?", whereArg);
		} while (0);

		return ret;
	}

	bool ChatMessageDB::updateAttachName(const QString &rsUuid, const QString &filePath)
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

				ret = update(DB::ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME, vmap, "uuid=?", whereArg);
			} while (0);

			return ret;
		}
	}

	QString ChatMessageDB::getMaxMessageStamp()
	{
		QString nRet;

		do 
		{
			if (!isOpen())
			{
				if (!open())
					break;
			}

			QString sql = QString("select MAX(stamp) from [%1]").arg(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME);
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

	bool ChatMessageDB::storeReadState(const QString &stamp, int state)
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
			vmap["readstate"] = state;
			QStringList whereArg;
			whereArg << stamp;

			ret = update(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME, vmap, "stamp=?", whereArg);
		} while (0);

		return ret;
	}

	bean::MessageBody ChatMessageDB::getMessageByStamp(const QString &stamp)
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

	bean::MessageBody ChatMessageDB::getMessageById(int msgId)
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

	QList<bean::MessageBody> ChatMessageDB::selectMessages(const QString &whereClause, const QStringList &whereArgs)
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

			QString sql = QString("select id, uid, attachscount, messagexml, readstate, sequence, sync from [%1] ").arg(DB::ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME);
			sql += whereClause; 
			qDebug() << Q_FUNC_INFO << sql << whereArgs;

			QList<QVariantList> data = query(sql, whereArgs);

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != 7)
					continue;

				bool bOk = false;
				int nMsgId = vl[0].toInt(&bOk);
				if (bOk && nMsgId == -1)
					continue;

				QString uid = vl[1].toString();
				int attachsCount = vl[2].toInt();
				int readState = vl[4].toInt();
				QString sequence = vl[5].toString();
				int sync = vl[6].toInt();

				bean::MessageBody body = bean::MessageBodyFactory::fromXml(vl[3].toString());
				body.setMessageid(nMsgId);
				body.setAttachsCount(attachsCount);
				body.setReadState(readState);
				body.setSequence(sequence);
				body.setSync(sync == 1);

				if (body.attachsCount() > 0)
				{
					QString attFromUid = body.isSend() ? body.from() : body.to();
					body.setAttachs(getAttachments(attFromUid, nMsgId));
				}
				listRet.append(body);
			}
		} while (0);

		return listRet;
	}
}