#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "login/Account.h"
#include "SendMessageDB.h"

const char DB_SQL_SEND_MESSAGES_CREATE[]        = "create table [%1] ([id] integer primary key not null, [sequence], [msgtype], [to], [time], [attachcount] not null default 0, [messagexml], [state] not null default 0)";
const char DB_SQL_SEND_MESSAGES_CREATE_INDEX[]  = "create unique index [IK_SENDMESSAGESSEQUENCE] on [%1] ( [sequence] )";

const char DB_SQL_SEND_ATTACHS_CREATE[]         = "create table [%1] ([id] integer primary key not null, [messageid] references [%2] ([id]), [name], [format], [filename], [uuid], [size], [fttype] not null default 0, [ftresult] not null default 0, [time], [source] default '', [picwidth] integer default 0, [picheight] integer default 0)";
const char DB_SQL_SEND_ATTACHS_CREATE_INDEX[]   = "create unique index [IK_SENDATTACHMENTSUUID] on [%1] ( [uuid] )";
const char DB_SQL_ATTACHS_ADD_SOURCE[]          = "ALTER TABLE %1 ADD source default '';";
const char DB_SQL_ATTACHS_ADD_PICWIDTH[]        = "ALTER TABLE %1 ADD picwidth integer default 0;";
const char DB_SQL_ATTACHS_ADD_PICHEIGHT[]       = "ALTER TABLE %1 ADD picheight integer default 0;";

const char DB_SQL_SEND_SECRETACKS_CREATE[]      = "create table [%1] ([id] integer primary key not null, [uid] not null, [stamp] not null)";
const char DB_SQL_SEND_SECRETACKS_CREATE_INDEX[]= "create unique index [IK_SENDSECRETACKSSTAMP] on [%1] ( [stamp] )";; 

static const char DB_SENDMESSAGE_DBNAME[]       = "sendmessages.db";

namespace DB
{
	const QString SendMessageDB::DB_SEND_MESSAGE_TABLENAME = "sendmessages";
	const QString SendMessageDB::DB_SEND_ATTACHS_TABLENAME = "sendattachments";
	const QString SendMessageDB::DB_SEND_SECRETACKS_TABLENAME = "sendsecretacks";

	SendMessageDB::SendMessageDB(const QString& connSuffix) : DBBase()
	{
		m_Connname = QString("SendMessageDB_%1_%2_%3").arg(DB_SEND_MESSAGE_TABLENAME).arg(DB_SEND_ATTACHS_TABLENAME).arg(connSuffix);
		m_Tag = QString("[SendMessageDB] %1").arg(m_Connname);

		m_sendMessageCache.reset(new SendMessageCache(this));
	}

	bool SendMessageDB::open()
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

			m_Filename = pAccount->homeDir().filePath(DB_SENDMESSAGE_DBNAME);
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			if (!db.tables().contains(DB_SEND_MESSAGE_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_SEND_MESSAGES_CREATE).arg(DB_SEND_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_SEND_MESSAGES_CREATE_INDEX).arg(DB_SEND_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!db.tables().contains(DB_SEND_ATTACHS_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_SEND_ATTACHS_CREATE).arg(DB_SEND_ATTACHS_TABLENAME).arg(DB_SEND_MESSAGE_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_SEND_ATTACHS_CREATE_INDEX).arg(DB_SEND_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_SEND_ATTACHS_TABLENAME).contains("source"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_SOURCE).arg(DB_SEND_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_SEND_ATTACHS_TABLENAME).contains("picwidth"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_PICWIDTH).arg(DB_SEND_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!fields(DB_SEND_ATTACHS_TABLENAME).contains("picheight"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_ATTACHS_ADD_PICHEIGHT).arg(DB_SEND_ATTACHS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			if (!db.tables().contains(DB_SEND_SECRETACKS_TABLENAME))
			{
				QSqlQuery query(db);

				QString sql = QString(DB_SQL_SEND_SECRETACKS_CREATE).arg(DB_SEND_SECRETACKS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_SEND_SECRETACKS_CREATE_INDEX).arg(DB_SEND_SECRETACKS_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			{
				const QString DB_SEND_MESSAGE_TRIGGER = "fk_send_message";
				QString sql = QString("select name from sqlite_master where type='trigger'");
				QList<QVariantList> data = query(sql);
				bool bFind = false;
				foreach (QVariantList vlist, data)
				{
					if (vlist.contains(DB_SEND_MESSAGE_TRIGGER))
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
						" end;").arg(DB_SEND_MESSAGE_TRIGGER, DB_SEND_MESSAGE_TABLENAME, DB_SEND_ATTACHS_TABLENAME);

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

	void SendMessageDB::close()
	{
		m_sendMessageCache->commitMessages();

		DBBase::close();
	}

	void SendMessageDB::storeMessage(const QString &sequence, const bean::MessageBody &rBody)
	{
		m_sendMessageCache.data()->cacheMessage(sequence, rBody);
	}

	quint64 SendMessageDB::commitMessage(const QString &sequence, const bean::MessageBody &rBody)
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
				// [sequence], [msgtype], [to], [time], [attachcount], [messagexml], [state]";
				QVariantMap vmap;
				vmap.insert("sequence", sequence);
				vmap.insert("msgtype", (int)rBody.messageType());
				vmap.insert("to", rBody.to());
				vmap.insert("time", rBody.time());
				vmap.insert("attachcount", rBody.attachs().count());
				vmap.insert("messagexml", rBody.toMessageXml());
				vmap.insert("state", 0);

				if (!insert(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME, vmap))
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
					if (!replace(DB::SendMessageDB::DB_SEND_ATTACHS_TABLENAME, vmap))
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

	bool SendMessageDB::updateMessageState(const QString &sequence, int state /*= 1*/)
	{
		m_sendMessageCache->commitMessages();

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
			vmap["state"] = state;
			QStringList whereArg;
			whereArg << sequence;

			ret = update(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME, vmap, "sequence=?", whereArg);
		} while (0);

		return ret;
	}

	bool SendMessageDB::updateMessageStates(int state /*= 1*/)
	{
		m_sendMessageCache->commitMessages();

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
			vmap["state"] = state;

			ret = update(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME, vmap, "state=0", QStringList());
		} while (0);

		return ret;
	}

	/*
	bool SendMessageDB::storeAttachResult(const QString &rsUuid, int nResult)
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

			ret = update(DB::SendMessageDB::DB_SEND_ATTACHS_TABLENAME, vmap, "uuid=?", whereArg);
		} while (0);

		return ret;
	}
	*/

	bool SendMessageDB::storeAttachSource(const QString &rsUuid, const QString &source)
	{
		m_sendMessageCache->commitMessages();

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
			vmap["source"] = source;
			QStringList whereArg;
			whereArg << rsUuid;

			ret = update(DB::SendMessageDB::DB_SEND_ATTACHS_TABLENAME, vmap, "uuid=?", whereArg);
		} while (0);

		return ret;
	}

	bool SendMessageDB::removeMsgBySequence(const QString &sequence)
	{
		m_sendMessageCache->commitMessages();

		if (!isOpen())
		{
			if (!open())
			{
				return false;
			}
		}

		QStringList whereArg;
		whereArg << sequence;
		deleteRows(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME, "sequence=?", whereArg);
		return true;
	}

	bool SendMessageDB::clearMessages()
	{
		m_sendMessageCache->commitMessages();

		if (!isOpen())
		{
			if (!open())
			{
				return false;
			}
		}

		deleteRows(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME, QString(), QStringList());

		return true;
	}

	bean::MessageBody SendMessageDB::getMessageBySequence(const QString &sequence)
	{
		m_sendMessageCache->commitMessages();

		bean::MessageBody ret;

		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QString sql = QString("select * from [%1] where [sequence] = ?").arg(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME);
			QList<QVariantList> data = query(sql, QStringList() << sequence);
			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fields(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME).count())
					continue;

				bool bOk = false;
				int nMsgId = vl[0].toInt(&bOk);
				if (bOk && nMsgId == -1)
					continue;

				// [id] [sequence], [msgtype], [to], [time], [attachcount] [messagexml], [state]
				int attachsCount = vl[5].toInt();
				ret = bean::MessageBodyFactory::fromXml(vl[6].toString());
				ret.setAttachsCount(attachsCount);

				if (ret.attachsCount() > 0)
				{
					QString attFromUid = ret.isSend() ? ret.from() : ret.to();
					ret.setAttachs(getAttachments(attFromUid, nMsgId));
				}
				break;
			}

		} while (0);

		return ret;
	}

	QMap<QString, bean::MessageBody> SendMessageDB::getMessages(int state /*= 0*/)
	{
		m_sendMessageCache->commitMessages();

		QMap<QString, bean::MessageBody> mapRet;

		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QString sql = QString("select * from [%1] where [state] = %2").arg(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME).arg(state);
			QList<QVariantList> data = query(sql, QStringList());

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fields(DB::SendMessageDB::DB_SEND_MESSAGE_TABLENAME).count())
					continue;

				bool bOk = false;
				int nMsgId = vl[0].toInt(&bOk);
				if (bOk && nMsgId == -1)
					continue;

				// [id] [sequence], [msgtype], [to], [time], [attachcount] [messagexml], [state]
				QString sequence = vl[1].toString();
				int attachsCount = vl[5].toInt();
				bean::MessageBody body = bean::MessageBodyFactory::fromXml(vl[6].toString());
				body.setAttachsCount(attachsCount);

				if (body.attachsCount() > 0)
				{
					QString attFromUid = body.isSend() ? body.from() : body.to();
					body.setAttachs(getAttachments(attFromUid, nMsgId));
				}
				mapRet.insert(sequence, body);
			}
		} while (0);

		return mapRet;
	}

	QList<bean::AttachItem> SendMessageDB::getAttachments(const QString &uid, int msgid)
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

			QStringList fs = fields(DB_SEND_ATTACHS_TABLENAME);

			QString sql = QString("select * from [%1] where [messageid] = ? order by [id] asc").arg(DB_SEND_ATTACHS_TABLENAME);

			QList<QVariantList> data = query(sql, QStringList() << QString("%1").arg(msgid));

			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != fs.length())
				{
					continue;
				}

				bean::AttachItem item;
				item.fromAttachList(uid, vl);

				lstAttachs.append(item);
			}
		} while (0);

		return lstAttachs;
	}

	bool SendMessageDB::storeSecretAck(const QString &uid, const QString &stamp)
	{
		bool ret = false;
		do 
		{
			if (uid.isEmpty() || stamp.isEmpty())
				break;

			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QVariantMap vmap;
			vmap["uid"] = uid;
			vmap["stamp"] = stamp;
			ret = insert(DB::SendMessageDB::DB_SEND_SECRETACKS_TABLENAME, vmap);

		} while (0);

		return ret;
	}

	bool SendMessageDB::clearSecretAcks()
	{
		if (!isOpen())
		{
			if (!open())
			{
				return false;
			}
		}

		deleteRows(DB::SendMessageDB::DB_SEND_SECRETACKS_TABLENAME, QString(), QStringList());

		return true;
	}

	QMap<QString, QString> SendMessageDB::secretAcks()
	{
		QMap<QString, QString> acks;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QString sql = QString("select * from [%1]").arg(DB_SEND_SECRETACKS_TABLENAME);
			QList<QVariantList> data = query(sql, QStringList());


			foreach (QVariantList vl, data)
			{
				if (vl.isEmpty() || vl.length() != 3)
				{
					continue;
				}

				QString stamp = vl[2].toString();
				QString uid = vl[1].toString();
				acks.insert(stamp, uid);
			}

		} while (0);

		return acks;
	}


	SendMessageCache::SendMessageCache(SendMessageDB *db)
		: QObject(), m_sendMessageDB(db)
	{
		m_timer.setSingleShot(true);
		m_timer.setInterval(100);
		bool connected = connect(&m_timer, SIGNAL(timeout()), this, SLOT(commitMessages()));
		Q_ASSERT(connected);
		Q_UNUSED(connected);
	}

	void SendMessageCache::cacheMessage(const QString &seq, const bean::MessageBody &msg)
	{
		if (seq.isEmpty() || !msg.isValid())
			return;

		QPair<QString, bean::MessageBody> pair;
		pair.first = seq;
		pair.second = msg;
		m_messages.append(pair);

		if (!m_timer.isActive())
			m_timer.start();
	}

	void SendMessageCache::commitMessages()
	{
		m_timer.stop();

		if (m_messages.isEmpty())
			return;

		for (int i = 0; i < m_messages.count(); ++i)
		{
			QPair<QString, bean::MessageBody> pair = m_messages[i];
			m_sendMessageDB->commitMessage(pair.first, pair.second);
		}
		m_messages.clear();
	}
}