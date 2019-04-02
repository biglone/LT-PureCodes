#ifndef _MESSAGEBASE_H_
#define _MESSAGEBASE_H_

#include <QSharedData>
#include "MessageExt.h"
#include "bean/attachitem.h"
#include "common/datetime.h"
#include "bean/bean.h"

namespace bean
{
	class MessageBase : public QSharedData
	{
	public:
		MessageBase();
		MessageBase(const MessageBase &other);
		~MessageBase() {}

	public:
		bool isValid() const;

		QVariantMap toJson();          // to html display

		QVariantMap toMessageDBMap();  // to messages database

		QString toMessageXml();        // to message xml

		QString toMessageText();       // to short text

		QString messageBodyText(); // body text without speak name
		QString messageSendUid() const;  // group or discuss message send id

		QString toPlainText();         // to plain text

	public:
		static QString methodString(bool send);

	public:
		int           msgId;
		MessageType   msgType;   // 消息类型，单聊，群聊，讨论组

		QString       sequence;  // 消息：sequence

		QString       from;      // 消息：from, 谁发送的
		QString       fromName;  
		QString       to;        // 单聊：自己id；群聊：群id；讨论组：讨论组id
		QString       toName;    
		QString       subject;
		QString       body;
		QString       time;
		QString       stamp;    
		bool          send;      // 是否是发送

		int           readState; // 是否查看过 0: 未查看过 1: 查看过

		bool          sync;      // 是否从手机端同步过来的消息

		MessageExt    ext;       // 辅助

		// 附件属性
		int                             attachsCount;
		QMap<QString, bean::AttachItem> mapAttachs;      // 附件

		// to-do
		QList<QString>                  listAttachsUUid; // 判断是否可以发送
	};
}

#endif //_MESSAGEBASE_H_
