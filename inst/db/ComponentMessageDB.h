#ifndef _COMPONENT_MESSAGEDB_H_
#define _COMPONENT_MESSAGEDB_H_

#include <QString>
#include <QList>
#include <QMap>
#include "MessageDB.h"
#include "bean/bean.h"

namespace DB
{
	class AttachmentDB;
	class ComponentMessageDB : public IMessageOperation
	{
	public:
		explicit ComponentMessageDB(const QList<bean::MessageType> &msgTypes, const QString &connSuffix);
		~ComponentMessageDB();

		static bool clearMessages();

	public:
		QList<bean::MessageType> supportMessageTypes() const;
		bool isMessageSupported(bean::MessageType msgType) const;
		bool switchDB(bean::MessageType msgType);
		MessageDB *currentMessageDB() const;
		AttachmentDB *attachmentDB() const;

	public: // functions from IMessageOperation
		virtual QList<bean::AttachItem> getAttachments(const QString& uid, int msgid);
		virtual int getMessageCount(const QString &uid, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "");
		virtual QList<int> getMessageIds(const QString &uid,
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "");
		virtual QList<bean::MessageBody> getMessages(const QString &uid, int nOffset, int nLimit, 
			const QString &begindate = "", const QString &enddate = "", const QString &keyword = "");
		virtual int getMessageCountBeforeTime(const QString &uid, const QString &rsDateTime);
		virtual QList<bean::MessageBody> getMessagesBeforeTime(const QString &uid, int nOffset, int nLimit, const QString &rsDateTime);
		virtual QList<int> getMessageIdBeforeTime(const QString &uid, const QString &rsDateTime);
		virtual QList<bean::MessageBody> getMessagesBeforeTs(const QString &uid, const QString &ts, int count);
		virtual bool removeMsgByMsgId(int nMsgID);
		virtual quint64 storeMessage(const bean::MessageBody &rBody);
		virtual bool replaceMessage(int nMsgID, const bean::MessageBody &rBody);
		virtual bool storeAttachResult(const QString &rsUuid, int nResult);
		virtual bool updateAttachName(const QString &rsUuid, const QString &filePath);
		virtual QString getMaxMessageStamp();
		virtual bool storeReadState(const QString &stamp, int state);
		virtual bean::MessageBody getMessageByStamp(const QString &stamp);
		virtual bean::MessageBody getMessageById(int msgId);

	private:
		AttachmentDB                        *m_attachmentDB;
		QMap<bean::MessageType, MessageDB *> m_messageDBs;
		bean::MessageType                    m_currentDB;
	};
}
#endif // _COMPONENT_MESSAGEDB_H_