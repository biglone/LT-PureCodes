#ifndef _GROUP_MESSAGEDB_H_
#define _GROUP_MESSAGEDB_H_

#include <QList>
#include "MessageDB.h"
#include "bean/attachitem.h"

namespace DB
{
	class GroupMessageDB : public MessageDB
	{
	public:
		explicit GroupMessageDB(const QString& connSuffix);

		bool open();

	public: // functions from MessageDB
		virtual QList<bean::AttachItem> getAttachments(const QString& uid, int msgid);
		virtual int getMessageCount(const QString &uid, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "");
		virtual QList<int> getMessageIds(const QString &uid, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "");
		virtual QList<bean::MessageBody> getMessages(const QString &uid, int nOffset, int nLimit, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "");
		virtual int getMessageCountBeforeTime(const QString &uid, const QString &rsDateTime);
		virtual QList<bean::MessageBody> getMessagesBeforeTime(const QString &uid, int nOffset, int nLimit, const QString &rsDateTime);
		virtual QList<int> getMessageIdBeforeTime(const QString &uid, const QString& rsDateTime);
		virtual QList<bean::MessageBody> getMessagesBeforeTs(const QString &uid, const QString &ts, int count);
		virtual bool removeMsgByMsgId(int nMsgID);
		virtual quint64 storeMessage(const bean::MessageBody &rBody);
		virtual bool replaceMessage(int nMsgID, const bean::MessageBody &rBody);
		virtual bool storeAttachResult(const QString& rsUuid, int nResult);
		virtual bool updateAttachName(const QString &rsUuid, const QString &filePath);
		virtual QString getMaxMessageStamp();
		virtual bean::MessageBody getMessageByStamp(const QString &stamp);
		virtual bean::MessageBody getMessageById(int msgId);

	private:
		QList<bean::MessageBody> selectMessages(const QString &whereClause, const QStringList &whereArgs);

	public:
		static const QString DB_GROUP_MESSAGE_TABLENAME;
		static const QString DB_GROUP_ATTACHS_TABLENAME;
	};
}
#endif //_GROUP_MESSAGEDB_H_