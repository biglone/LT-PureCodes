#include "ComponentMessageDB.h"
#include "ChatMessageDB.h"
#include "GroupMessageDB.h"
#include "DiscussMessageDB.h"
#include "AttachmentDB.h"
#include "Account.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace DB
{
	ComponentMessageDB::ComponentMessageDB(const QList<bean::MessageType> &msgTypes, const QString &connSuffix)
	{
		m_attachmentDB = new AttachmentDB(connSuffix);
		foreach (bean::MessageType msgType, msgTypes)
		{
			MessageDB *messageDB = MessageDBFactory::createMessageDB(msgType, connSuffix);
			m_messageDBs[msgType] = messageDB;
		}
		m_currentDB = bean::Message_Invalid;
	}

	ComponentMessageDB::~ComponentMessageDB()
	{
		qDeleteAll(m_messageDBs);
		delete m_attachmentDB;
	}

	bool ComponentMessageDB::clearMessages()
	{
		bool ok = false;
		do 
		{
			QString connectName("ComponentMessageDB_ClearMessages");
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, connectName);
			if (!db.isValid())
			{
				qWarning() << connectName << "db is not valid.";
				break;
			}

			Account* pAccount = Account::instance();
			Q_ASSERT(pAccount != NULL);

			QString fileName = pAccount->messageDbFilePath();
			db.setDatabaseName(fileName);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << connectName << db.lastError();
				break;
			}

			QSqlQuery query(db);
			QString sql;

			// drop all views
			QString dropViewFormat("drop view [%1]");
			QStringList dropViewNames;
			dropViewNames << AttachmentDB::DB_ATTACH_VIEWNAME << AttachmentDB::DB_ATTACH_VIEWNAME2 << AttachmentDB::DB_ATTACH_VIEWNAME3;
			foreach (QString viewName, dropViewNames)
			{
				if (db.tables(QSql::Views).contains(viewName))
				{
					sql = dropViewFormat.arg(viewName);
					if (!query.exec(sql))
					{
						qWarning() << connectName << query.lastError();
						break;
					}
					sql.clear();
				}
			}

			// drop all message table
			QString dropTableFormat("drop table [%1]");
			QStringList dropTableNames;
			dropTableNames << ChatMessageDB::DB_CHAT_ATTACHS_TABLENAME << ChatMessageDB::DB_CHAT_MESSAGE_TABLENAME
				<< GroupMessageDB::DB_GROUP_ATTACHS_TABLENAME << GroupMessageDB::DB_GROUP_MESSAGE_TABLENAME
				<< DiscussMessageDB::DB_DISCUSS_ATTACHS_TABLENAME << DiscussMessageDB::DB_DISCUSS_MESSAGE_TABLENAME;
			foreach (QString tableName, dropTableNames)
			{
				if (db.tables().contains(tableName))
				{
					sql = dropTableFormat.arg(tableName);
					if (!query.exec(sql))
					{
						qWarning() << connectName << query.lastError();
						break;
					}
					sql.clear();
				}
			}

			ok = true;

		} while(0);

		return ok;
	}

	QList<bean::MessageType> ComponentMessageDB::supportMessageTypes() const
	{
		return m_messageDBs.keys();
	}

	bool ComponentMessageDB::isMessageSupported(bean::MessageType msgType) const
	{
		QList<bean::MessageType> msgTypes = supportMessageTypes();
		if (msgTypes.contains(msgType))
			return true;
		else
			return false;
	}

	bool ComponentMessageDB::switchDB(bean::MessageType msgType)
	{
		if (isMessageSupported(msgType))
		{
			m_currentDB = msgType;
			return true;
		}
		else
		{
			qDebug() << Q_FUNC_INFO << msgType << "is not supported.";
			return false;
		}
	}

	MessageDB *ComponentMessageDB::currentMessageDB() const
	{
		if (m_messageDBs.contains(m_currentDB))
			return m_messageDBs[m_currentDB];
		else
			return 0;
	}

	AttachmentDB *ComponentMessageDB::attachmentDB() const
	{
		return m_attachmentDB;
	}

	QList<bean::AttachItem> ComponentMessageDB::getAttachments(const QString& uid, int msgid)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getAttachments(uid, msgid);
	}

	int ComponentMessageDB::getMessageCount(const QString &uid, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessageCount(uid, begindate, enddate, keyword);
	}

	QList<int> ComponentMessageDB::getMessageIds(const QString &uid, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessageIds(uid, begindate, enddate, keyword);
	}

	QList<bean::MessageBody> ComponentMessageDB::getMessages(const QString &uid, int nOffset, int nLimit, 
		const QString &begindate /*= ""*/, const QString &enddate /*= ""*/, const QString &keyword /*= ""*/)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessages(uid, nOffset, nLimit, begindate, enddate, keyword);
	}

	int ComponentMessageDB::getMessageCountBeforeTime(const QString &uid, const QString &rsDateTime)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessageCountBeforeTime(uid, rsDateTime);
	}

	QList<bean::MessageBody> ComponentMessageDB::getMessagesBeforeTime(const QString &uid, int nOffset, int nLimit, const QString &rsDateTime)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessagesBeforeTime(uid, nOffset, nLimit, rsDateTime);
	}

	QList<int> ComponentMessageDB::getMessageIdBeforeTime(const QString &uid, const QString &rsDateTime)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessageIdBeforeTime(uid, rsDateTime);
	}

	QList<bean::MessageBody> ComponentMessageDB::getMessagesBeforeTs(const QString &uid, const QString &ts, int count)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessagesBeforeTs(uid, ts, count);
	}

	bool ComponentMessageDB::removeMsgByMsgId(int nMsgID)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->removeMsgByMsgId(nMsgID);
	}

	quint64 ComponentMessageDB::storeMessage(const bean::MessageBody &rBody)
	{
		if (!switchDB(rBody.messageType()))
		{
			qDebug("%s :switch db failed. type: %d", Q_FUNC_INFO, rBody.messageType());
			return -1;
		}

		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->storeMessage(rBody);
	}

	bool ComponentMessageDB::replaceMessage(int nMsgID, const bean::MessageBody &rBody)
	{
		if (!switchDB(rBody.messageType()))
		{
			qDebug("%s :switch db failed. type: %d", Q_FUNC_INFO, rBody.messageType());
			return false;
		}

		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->replaceMessage(nMsgID, rBody);
	}

	bool ComponentMessageDB::storeAttachResult(const QString &rsUuid, int nResult)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->storeAttachResult(rsUuid, nResult);
	}

	bool ComponentMessageDB::updateAttachName(const QString &rsUuid, const QString &filePath)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->updateAttachName(rsUuid, filePath);
	}

	QString ComponentMessageDB::getMaxMessageStamp()
	{
		bean::MessageType origCurrent = m_currentDB;
		QString maxStamp;
		QList<bean::MessageType> msgTypes = supportMessageTypes();
		foreach (bean::MessageType msgType, msgTypes)
		{
			switchDB(msgType);
			MessageDB *currentDB = currentMessageDB();
			Q_ASSERT(currentDB);
			QString stamp = currentDB->getMaxMessageStamp();
			if (maxStamp < stamp)
				maxStamp = stamp;
		}
		switchDB(origCurrent);
		return maxStamp;
	}

	bool ComponentMessageDB::storeReadState(const QString &stamp, int state)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->storeReadState(stamp, state);
	}

	bean::MessageBody ComponentMessageDB::getMessageByStamp(const QString &stamp)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessageByStamp(stamp);
	}

	bean::MessageBody ComponentMessageDB::getMessageById(int msgId)
	{
		MessageDB *currentDB = currentMessageDB();
		Q_ASSERT(currentDB);
		return currentDB->getMessageById(msgId);
	}
}