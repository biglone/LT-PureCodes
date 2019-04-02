#ifndef _MESSAGEBODY_H_
#define _MESSAGEBODY_H_

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include "attachitem.h"
#include "MessageExt.h"
#include "MessageBase.h"
#include "bean.h"

class QDomElement;

namespace bean
{
	class MessageBody
	{
	public:
		explicit MessageBody();
		MessageBody(const MessageBody& other);

		MessageBody& operator=(const MessageBody& other);

		void copy();

	public:
		// for send
		int setTransferFinish(const QString& rsUuid, int result);
		bool canSend();

	public:
		void setSend(bool send);

		void setMessageid(int nMsgId);
		void setFrom(const QString& id);
		void setFromName(const QString& name);
		void setTo(const QString& id);
		void setToName(const QString& name);
		void setTime(const QString& time);
		void setStamp(const QString& stamp);
		void setSubject(const QString& subject);
		void setBody(const QString& body);

		void setAttachs(const QList<bean::AttachItem>& rlistAttach);
		void setAttachsCount(int nCount);

		void setExt(const MessageExt &ext);

		void setSequence(const QString &sequence);

		void setSync(bool sync);

		// type
		MessageType messageType() const;
		void setMessageType(MessageType msgType);

		bool isSend() const;
		QString methodstring() const;

		int messageid() const;
		QString from() const;     // 单聊：自己的uid，群聊：发消息的uid
		QString fromFullId() const;
		QString fromName() const;
		QString to() const;       // 单聊：对方的uid，群聊：群或者讨论组的id
		QString toFullId() const;
		QString toName() const;
		QString time() const;
		QString stamp() const;

		QString subject() const;
		QString body() const;
		QString logBody() const;

		QList<bean::AttachItem> attachs() const;
		int attachsCount() const;

		MessageExt ext() const;

		QString sequence() const;

		bool sync() const;

		bool containAttach(const QString &uuid) const;

		void setReadState(int state);
		int readState() const;

	public:
		bool isValid() const;

		QVariantMap toJson() const;          // to html display

		QVariantMap toMessageDBMap() const;  // to messages database

		QString toMessageXml() const;        // to message xml

		QString toMessageText() const;       // to short text

		QString messageBodyText() const; // body text without speak name
		QString messageSendUid() const;   // group or discuss message send id

	private:
		QExplicitlySharedDataPointer<MessageBase> d;

		friend class MessageBodyFactory;
	};

	typedef QList<MessageBody> MessageBodyList;


	class MessageBodyFactory
	{
	private:
		MessageBodyFactory() {}
		virtual ~MessageBodyFactory() {}
	
	public:
		static MessageBody fromXml(const QString &xml, bool includeAttachs = false);
		static MessageBody createMessage(MessageType type);
		static QList<bean::AttachItem> attachsFromXml(MessageType msgType, 
			                                          const QString &attachFrom,
													  const QDomElement &xml);
	};
}

Q_DECLARE_METATYPE(bean::MessageBody)
Q_DECLARE_METATYPE(bean::MessageBodyList)

#endif // _MESSAGEBODY_H_
