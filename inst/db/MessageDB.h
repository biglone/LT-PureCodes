#ifndef _MESSAGEDB_H_
#define _MESSAGEDB_H_
#include <QList>
#include "DBBase.h"
#include "bean/MessageBody.h"
#include "bean/attachitem.h"

namespace DB
{
	class IMessageOperation
	{
	public:
		// get all the attachment whose message id is equal to 'msgid'
		virtual QList<bean::AttachItem> getAttachments(const QString& uid, int msgid) = 0;

		// get the message count whose uid is 'uid'
		virtual int getMessageCount(const QString &uid, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "") = 0;

		// get the message id whose uid is 'uid'
		virtual QList<int> getMessageIds(const QString &uid,
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "") = 0;

		// get the message which starts from nOffset and has limit of nCount
		virtual QList<bean::MessageBody> getMessages(const QString &uid, int nOffset, int nLimit, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "") = 0;

		// get the message count whose uid is 'uid' and the time is before rsDateTime
		virtual int getMessageCountBeforeTime(const QString &uid, const QString &rsDateTime) = 0;

		// get the message whose uid is 'uid' and the time is before rsDateTime
		virtual QList<bean::MessageBody> getMessagesBeforeTime(const QString &uid, int nOffset, int nLimit, const QString &rsDateTime) = 0;

		// get all the message ids before a specific time
		virtual QList<int> getMessageIdBeforeTime(const QString &uid, const QString &rsDateTime) = 0;

		// get messages whose stamp is less than ts
		virtual QList<bean::MessageBody> getMessagesBeforeTs(const QString &uid, const QString &ts, int count) = 0;

		// remove a message from db
		virtual bool removeMsgByMsgId(int nMsgID) = 0;

		// store a message
		virtual quint64 storeMessage(const bean::MessageBody &rBody) = 0;

		// replace a message with same id
		virtual bool replaceMessage(int nMsgID, const bean::MessageBody &rBody) = 0;

		// store the attach result
		virtual bool storeAttachResult(const QString& rsUuid, int nResult) = 0;

		// update the attach name and path
		virtual bool updateAttachName(const QString &rsUuid, const QString &filePath) = 0;

		// get the max message stamp
		virtual QString getMaxMessageStamp() = 0;

		// store read state by stamp
		virtual bool storeReadState(const QString &stamp, int state) { Q_UNUSED(stamp); Q_UNUSED(state); return true; }

		// get message by stamp
		virtual bean::MessageBody getMessageByStamp(const QString &stamp) = 0;

		// get message by id
		virtual bean::MessageBody getMessageById(int msgId) = 0;
	};

	class MessageDB : public DBBase, public IMessageOperation
	{
	};

	class MessageDBFactory
	{
	public:
		static MessageDB *createMessageDB(bean::MessageType msgType, const QString &connSuffix);
	};
}
#endif //_MESSAGEDB_H_