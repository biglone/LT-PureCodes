#include <QStringList>
#include <QColor>

#include "MessageBase.h"
#include "ChatMessage.h"
#include "GroupMessage.h"
#include "DiscussMessage.h"

namespace bean
{
	MessageBase::MessageBase()
		: msgType(Message_Invalid)
		, attachsCount(0)
		, send(false)
		, msgId(-1)
		, readState(0)
		, sync(false)
	{
		time = CDateTime::currentDateTimeUtcString();
	}

	MessageBase::MessageBase(const MessageBase &other)
	{
		msgId            = other.msgId;
		msgType          = other.msgType;
		from             = other.from;
		fromName         = other.fromName;
		to               = other.to;
		toName           = other.toName;
		subject          = other.subject;
		body             = other.body;
		time             = other.time;
		stamp            = other.stamp;
		send             = other.send;
		readState        = other.readState;
		sync             = other.sync;

		ext              = other.ext;

		attachsCount     = other.attachsCount;
		mapAttachs       = other.mapAttachs;
		listAttachsUUid  = other.listAttachsUUid;
	}

	bool MessageBase::isValid() const
	{
		return msgType != Message_Invalid;
	}

	QVariantMap MessageBase::toJson()
	{
		if (msgType == Message_Chat)
		{
			ChatMessage cm(*this);
			return cm.toJson();
		}
		else if (msgType == Message_GroupChat)
		{
			GroupMessage gm(*this);
			return gm.toJson();
		}
		else if (msgType == Message_DiscussChat)
		{
			DiscussMessage dm(*this);
			return dm.toJson();
		}

		return QVariantMap();
	}

	QVariantMap MessageBase::toMessageDBMap()
	{
		if (msgType == Message_Chat)
		{
			ChatMessage cm(*this);
			return cm.toMessageDBMap();
		}
		else if (msgType == Message_GroupChat)
		{
			GroupMessage gm(*this);
			return gm.toMessageDBMap();
		}
		else if (msgType == Message_DiscussChat)
		{
			DiscussMessage dm(*this);
			return dm.toMessageDBMap();
		}

		return QVariantMap();
	}

	QString MessageBase::toMessageXml()
	{
		if (msgType == Message_Chat)
		{
			ChatMessage cm(*this);
			return cm.toMessageXml();
		}
		else if (msgType == Message_GroupChat)
		{
			GroupMessage gm(*this);
			return gm.toMessageXml();
		}
		else if (msgType == Message_DiscussChat)
		{
			DiscussMessage dm(*this);
			return dm.toMessageXml();
		}

		return QString::null;
	}

	QString MessageBase::toMessageText()
	{
		// this message is shown in last contact list item
		QString bodyText = messageBodyText();

		if ((msgType == Message_GroupChat || msgType == Message_DiscussChat) && 
			(ext.type() != MessageExt_Tip && ext.type() != MessageExt_Interphone))
		{
			bodyText = fromName + ": " + bodyText;
		}

		return bodyText;
	}

	QString MessageBase::messageBodyText()
	{
		QString bodyText = body;
		QString bodyTail;
		foreach (QString key, mapAttachs.keys())
		{
			if (mapAttachs[key].transferType() == bean::AttachItem::Type_AutoDisplay)
			{
				QString attId = QString("{%1}").arg(key);
				bodyText.replace(attId, QObject::tr("[Image]"));
			}
			else if (mapAttachs[key].transferType() == bean::AttachItem::Type_AutoDownload)
			{
				bodyTail.append(QObject::tr("[Voice]"));
			}
			else if (mapAttachs[key].transferType() == bean::AttachItem::Type_Dir)
			{
				bodyTail.append(QObject::tr("[Dir]"));
			}
			else
			{
				bodyTail.append(QObject::tr("[Attach]"));
			}
		}

		bodyText = ext.toText(send, toName, bodyText + bodyTail);
		
		return bodyText;
	}

	QString MessageBase::messageSendUid() const
	{
		QString sendUid;
		if ((msgType == Message_GroupChat || msgType == Message_DiscussChat) && 
			(ext.type() != MessageExt_Tip && ext.type() != MessageExt_Interphone))
		{
			sendUid = from;
		}

		return sendUid;
	}

	QString MessageBase::toPlainText()
	{
		// this message text is plain text, not include secret message
		QString bodyText = body;
		QString bodyTail;
		foreach (QString key, mapAttachs.keys())
		{
			if (mapAttachs[key].transferType() == bean::AttachItem::Type_AutoDisplay)
			{
				QString attId = QString("{%1}").arg(key);
				bodyText.replace(attId, QObject::tr("[Image]"));
			}
			else if (mapAttachs[key].transferType() == bean::AttachItem::Type_AutoDownload)
			{
				bodyTail.append(QObject::tr("[Voice]"));
			}
			else if (mapAttachs[key].transferType() == bean::AttachItem::Type_Dir)
			{
				bodyTail.append(QObject::tr("[Dir]"));
			}
			else
			{
				bodyTail.append(QObject::tr("[Attach]"));
			}
		}

		if (ext.type() != MessageExt_Secret)
			bodyText = ext.toText(send, toName, bodyText + bodyTail);

		return bodyText;
	}

	QString MessageBase::methodString(bool send)
	{
		if (send)
		{
			return bean::kszMethodSend;
		}
		else
		{
			return bean::kszMethodRecv;
		}
	}
}
