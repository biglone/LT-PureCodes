#ifndef _MESSAGENOTIFICATION_H_
#define _MESSAGENOTIFICATION_H_
#include <list>

#include "SpecificNotification.h"

#include "net/XmlMsg.h"

#include "protocol_global.h"

namespace protocol 
{
	class PROTOCOL_EXPORT MessageNotification
	{
	private:
		MessageNotification() {}
		~MessageNotification() {}

	public:
		enum MessageType
		{
			Message_Chat       = 0x00000000,
			Message_Groupchat  = 0x00000001,
			Message_Discuss    = 0x00000002
		};

		enum ChatType
		{
			Type_Chat          = 0x00000001,
			Type_Shake         = 0x00000002,
			Type_Session       = 0x00000004, // unused here
			Type_Share         = 0x00000008,
			Type_At            = 0x00000010,
			Type_Secret        = 0x00000020
		};

		static std::string MessageTypeToString(MessageType type);
		static MessageType MessageTypeFromString(const std::string& type);

		class Attachment
		{
		public:
			enum FtType
			{
				FtType_Common        = 0x00000000,
				FtType_Autodownload  = 0x00000001,
				FtType_Autodisplay   = 0x00000002,
				FtType_Dir           = 0x00000003
			};

			enum TransType
			{
				Trans_Invaild       = 0x00000000,
				Trans_Download      = 0x00000001,
				Trans_Upload        = 0x00000002
			};

			enum FtResult
			{
				FtResult_Unstart       = 0x00000000,
				FtResult_Success       = 0x00000001,
				FtResult_Fail          = 0x00000002,
				FtResult_Cancel        = 0x00000003,
				FtResult_Invaild       = 0x00000004
			};
			
		public:
			Attachment()
				: progress(0)
				, transType(Trans_Invaild)
				, ftresult(FtResult_Unstart)
				, ftType(FtType_Common)
				, size(0)
				, time(0)
				, name("")
				, format("")
				, guid("")
				, path("")
				, absoluteFile("")
				, source("")
				, picWidth(0)
				, picHeight(0)
			{}

		public:
			/*是否下载或者上传完成**/
			int          progress;//        = 0;
			TransType    transType;//       = null;
			FtResult     ftresult;//        = FtResult.UNSTART;

			FtType       ftType;//          = FtType.COMMON;
			int          size;//            = 0;
			int          time;//            = 0;
			std::string  name;//            = null;
			std::string  format;//          = null;

			/**guid*/
			std::string  guid;//            = null;
			std::string  path;//            = null;
			std::string  absoluteFile;//    = null;

			std::string  source;

			int          picWidth;
			int          picHeight;

		};

		class Message
		{
		public:
			Message()
				: type(Message_Chat)
				, chatType(Type_Chat)
				, sendFail(false)
				, encrypt(false)
			{}
			Message(const Message& other)
			{
				type          = other.type;
				chatType      = other.chatType;
				from          = other.from;
				to            = other.to;
				group         = other.group;
				time          = other.time;
				subject       = other.subject;
				body          = other.body;
				fromName      = other.fromName;
				toName        = other.toName;
				method        = other.method;
				sendFail      = other.sendFail;
				attachments   = other.attachments;
				shareUrl      = other.shareUrl;
				atIds         = other.atIds;
				atUid         = other.atUid;
				stamp         = other.stamp;
				encrypt       = other.encrypt;
			}

			void addAttachment(const std::string& rsPath,
				Attachment::FtType ftType,
				Attachment::TransType transType,
				const std::string& rsName,
				const std::string& rsFormat,
				int size,
				const std::string& rsId,
				int time,
				const std::string& source,
				int picWidth,
				int picHeight)
			{
				Attachment attachment;
				attachment.ftType   = ftType;
				attachment.transType = transType;
				attachment.path = rsPath;
				attachment.name = rsName;
				attachment.format = rsFormat;
				attachment.size = size;
				attachment.guid = rsId;
				attachment.time = time;
				attachment.source = source;
				attachment.picWidth = picWidth;
				attachment.picHeight = picHeight;
				attachments.push_back(attachment);
			}

			void addAttachment(Attachment attachment)
			{
				attachments.push_back(attachment);
			}

		public:
			MessageType                   type;
			ChatType                      chatType;
			std::string                   from;
			std::string                   to;        //one2one会话
			std::string                   group;     //群聊
			std::string                   time;
			std::string                   stamp;
			std::string                   subject;
			std::string                   body;
			std::string                   fromName;
			std::string                   toName;
			std::string                   method;
			std::string                   shareUrl;
			std::string                   atIds;
			std::string                   atUid;
			bool                          sendFail;
			bool                          encrypt;
			std::list<Attachment>         attachments;
		};

		static bool makeImMessage(iks* pnImMessage, const Message& message);

		class In : public SpecificNotification
		{
		public:
			In() {}
			virtual ~In() {}

			bool Parse(iks* pnIks);

			int getNotificationType();

		public:
			Message m_Message;

		};

		class Out : public net::XmlMsg
		{
		public:
			Out(MessageType eType, const std::string& rsTime);

			std::string getBuffer();

		public:
			Message m_Message;
		};
	};
}
#endif
