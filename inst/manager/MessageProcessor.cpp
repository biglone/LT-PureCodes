#include <QDebug>
#include "cttk/base.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "login/Account.h"
#include "pmclient/PmClient.h"

#include "bean/ChatMessageExt.h"
#include "bean/SessionMessageExt.h"
#include "bean/ShareMessageExt.h"
#include "bean/ShakeMessageExt.h"

#include "util/MsgEncryptionUtil.h"

#include "MessageProcessor.h"

static void registerMetaType()
{
	static bool bRegister = false;
	if (bRegister)
		return;

	qRegisterMetaType<bean::MessageType>("bean::MessageType");
	qRegisterMetaType<bean::MessageExtType>("bean::MessageExtType");
	qRegisterMetaType<bean::MessageBody>();
	qRegisterMetaType<bean::MessageBodyList>();
	bRegister = true;
}

QByteArray MessageProcessor::m_msgPassword;

MessageProcessor::MessageProcessor(QObject *parent)
	: QObject(parent), m_nHandleId(-1)
{
	registerMetaType();
}

MessageProcessor::~MessageProcessor()
{
}

void MessageProcessor::setMsgPassword(const QByteArray &msgPassword)
{
	m_msgPassword = msgPassword;
}

bool MessageProcessor::initObject()
{
	m_nHandleId = PmClient::instance()->insertNotificationHandler(this);
	qWarning() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void MessageProcessor::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* MessageProcessor::instance()
{
	return this;
}

int MessageProcessor::handledId() const
{
	return m_nHandleId;
}

QList<int> MessageProcessor::types() const
{
	return QList<int>() << protocol::MESSAGE;
}

bool MessageProcessor::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_nHandleId != handleId)
	{
		return false;
	}

	protocol::MessageNotification::In* pIn = static_cast<protocol::MessageNotification::In*>(sn);
	if (!pIn)
		return false;

	protocol::MessageNotification::Message* pMsg = new protocol::MessageNotification::Message(pIn->m_Message);

	QMetaObject::invokeMethod(this, "processMessage", Qt::QueuedConnection, Q_ARG(void*, pMsg));

	return true;
}

void MessageProcessor::processMessage(void* msg)
{
	protocol::MessageNotification::Message* pMsg = static_cast<protocol::MessageNotification::Message*>(msg);
	if (!pMsg)
		return;

	// process
	bean::MessageBody body = message2MsgBody(pMsg);

	emit receiveMessage(body);

	// delete
	SAFE_DELETE(pMsg);
}

bean::MessageBody MessageProcessor::message2MsgBody(protocol::MessageNotification::Message *message)
{
	if (!message)
		return bean::MessageBody();

	protocol::MessageNotification::Message* pMsg = message;

	// get message type
	bean::MessageType type = bean::Message_Invalid;
	switch (pMsg->type)
	{
	case protocol::MessageNotification::Message_Chat:
		type = bean::Message_Chat;
		break;
	case protocol::MessageNotification::Message_Groupchat:
		type = bean::Message_GroupChat;
		break;
	case protocol::MessageNotification::Message_Discuss:
		type = bean::Message_DiscussChat;
		break;
	}

	// create attachments
	QList<bean::AttachItem> listAttachs;
	Account* pAccount = Account::instance();

	std::list<protocol::MessageNotification::Attachment>::iterator itr = pMsg->attachments.begin();
	for (; itr != pMsg->attachments.end(); ++itr)
	{
		bean::AttachItem item;
		item.setMessageType(type);
		item.setFrom(QString::fromUtf8(pMsg->from.c_str()));
		item.setFilename(QString::fromUtf8(itr->name.c_str()));
		item.setFormat(QString::fromUtf8(itr->format.c_str()));
		item.setUuid(QString::fromUtf8(itr->guid.c_str()));
		item.setSize(itr->size);
		item.setTime(itr->time);
		item.setSource(QString::fromUtf8(itr->source.c_str()));
		item.setPicWidth(itr->picWidth);
		item.setPicHeight(itr->picHeight);

		int transferType = bean::AttachItem::Type_Default;

		switch (itr->ftType)
		{
		case protocol::MessageNotification::Attachment::FtType_Common:
			transferType = bean::AttachItem::Type_Default;
			item.setFilePath(pAccount->attachDir().absoluteFilePath(item.filename()));
			break;
		case protocol::MessageNotification::Attachment::FtType_Dir:
			transferType = bean::AttachItem::Type_Dir;
			item.setFilePath(pAccount->attachDir().absoluteFilePath(item.filename()));
			break;;
		case protocol::MessageNotification::Attachment::FtType_Autodownload:
			transferType = bean::AttachItem::Type_AutoDownload;
			item.setFilePath(pAccount->audioDir().absoluteFilePath(item.filename()));
			break;
		case protocol::MessageNotification::Attachment::FtType_Autodisplay:
			{
				transferType = bean::AttachItem::Type_AutoDisplay;
				QString imageFileName = QString("%1.%2").arg(item.uuid()).arg(item.format());
				item.setFilename(imageFileName);
				item.setFilePath(pAccount->imageDir().absoluteFilePath(imageFileName));
			}
			break;
		}

		item.setTransferType(transferType);

		listAttachs.append(item);
	}

	// create ext
	bean::MessageExt ext;
	switch (pMsg->chatType)
	{
	case protocol::MessageNotification::Type_Chat:
		{
			ext = bean::MessageExtFactory::create(bean::MessageExt_Chat);
		}
		break;
	case protocol::MessageNotification::Type_Session:
		{
			ext = bean::MessageExtFactory::create(bean::MessageExt_Session);
		}
		break;
	case protocol::MessageNotification::Type_Shake:
		{
			ext = bean::MessageExtFactory::create(bean::MessageExt_Shake);
		}
		break;
	case protocol::MessageNotification::Type_Share:
		{
			ext = bean::MessageExtFactory::create(bean::MessageExt_Share);
			ext.setData("shareurl", QString::fromUtf8(pMsg->shareUrl.c_str()));
		}
		break;
	case protocol::MessageNotification::Type_At:
		{
			ext = bean::MessageExtFactory::create(bean::MessageExt_At);
			ext.setData("at", QString::fromUtf8(pMsg->atIds.c_str()));
			ext.setData("atid", QString::fromUtf8(pMsg->atUid.c_str()));
		}
		break;
	case protocol::MessageNotification::Type_Secret:
		{
			ext = bean::MessageExtFactory::create(bean::MessageExt_Secret);
		}
		break;
	}

	// create message
	bean::MessageBody body = bean::MessageBodyFactory::createMessage(type);
	body.setSend(false);
	if (type == bean::Message_Chat)
	{
		body.setFrom(QString::fromUtf8(pMsg->to.c_str()));         // 自己的uid
		body.setFromName(QString::fromUtf8(pMsg->toName.c_str())); // 自己的name 
		body.setTo(QString::fromUtf8(pMsg->from.c_str()));         // 对方的uid
		body.setToName(QString::fromUtf8(pMsg->fromName.c_str())); // 对方的name
	}
	else
	{
		body.setFrom(QString::fromUtf8(pMsg->from.c_str()));         // 发送者的uid
		body.setFromName(QString::fromUtf8(pMsg->fromName.c_str())); // 发送者的name
		body.setTo(QString::fromUtf8(pMsg->group.c_str()));          // 群或讨论组的id
	}

	body.setTime(QString::fromUtf8(pMsg->time.c_str()));
	body.setStamp(QString::fromUtf8(pMsg->stamp.c_str()));
	body.setSubject(QString::fromUtf8(pMsg->subject.c_str()));
	if (!pMsg->encrypt)
	{
		body.setBody(QString::fromUtf8(pMsg->body.c_str()));
	}
	else
	{
		QByteArray bodyBuffer = MsgEncryptionUtil::decrypt(pMsg->body.c_str(), m_msgPassword);
		body.setBody(QString::fromUtf8(bodyBuffer.constData(), bodyBuffer.length()));
	}
	body.setExt(ext);
	body.setAttachs(listAttachs);

	return body;
}
