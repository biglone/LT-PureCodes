#include "sendmessagemanager.h"
#include "pmclient/PmClient.h"
#include "MessageProcessor.h"
#include "protocol/SendMessage.h"
#include <QDebug>
#include "protocol/ProtocolType.h"
#include "util/MsgEncryptionUtil.h"

SendMessageManager::SendMessageManager(QObject *parent)
	: QObject(parent), m_nHandleId(-1)
{

}

SendMessageManager::~SendMessageManager()
{
	qDeleteAll(m_messages.values());
	m_messages.clear();
}

QString SendMessageManager::setMessage(const bean::MessageBody &msgBody)
{
	protocol::SendMessageRequest *request = new protocol::SendMessageRequest();
	msgBody2Message(msgBody, request->m_Message);
	QString seq = QString::fromUtf8(request->getSeq().c_str());
	m_messages.insert(seq, request);
	return seq;
}

bool SendMessageManager::deliver(const QString &seq, const bean::MessageBody &msgBody)
{
	if (m_messages.contains(seq))
	{
		protocol::SendMessageRequest *request = m_messages.take(seq);

		// update attachment
		if (msgBody.attachsCount() > 0)
		{
			protocol::MessageNotification::Message newMessage;
			msgBody2Message(msgBody, newMessage);
			request->m_Message.attachments = newMessage.attachments;
		}

		PmClient::instance()->send(request);
		return true;
	}

	qWarning() << Q_FUNC_INFO << "can't find message: " << seq;
	return false;
}

bool SendMessageManager::msgBody2Message(const bean::MessageBody &body, protocol::MessageNotification::Message &message)
{
	switch (body.messageType())
	{
	case bean::Message_Chat:
		message.type = protocol::MessageNotification::Message_Chat;
		message.time = body.time().toUtf8().constData();
		break;
	case bean::Message_GroupChat:
		message.type = protocol::MessageNotification::Message_Groupchat;
		message.time = body.time().toUtf8().constData();
		break;
	case bean::Message_DiscussChat:
		message.type = protocol::MessageNotification::Message_Discuss;
		message.time = body.time().toUtf8().constData();
		break;
	default:
		return false;
	}

	if (body.messageType() == bean::Message_Chat)
	{
		message.to = body.to().toUtf8().constData();
	}
	else
	{
		message.group = body.to().toUtf8().constData();
	}
	message.fromName = body.fromName().toUtf8().constData();
	message.toName = body.toName().toUtf8().constData();

	switch (body.ext().type())
	{
	case bean::MessageExt_Chat:
		{
			// do nothing
		}
		break;
	case bean::MessageExt_Session:
		{
			message.chatType = (protocol::MessageNotification::Type_Session);
		}
		break;
	case bean::MessageExt_Shake:
		{
			message.chatType = (protocol::MessageNotification::Type_Shake);
		}
		break;
	case bean::MessageExt_Share:
		{
			message.chatType = (protocol::MessageNotification::Type_Share);
		}
		break;
	case bean::MessageExt_At:
		{
			message.chatType = (protocol::MessageNotification::Type_At);
			message.atIds = body.ext().data("at").toString().toUtf8().constData();
			message.atUid = body.ext().data("atid").toString().toUtf8().constData();
		}
		break;
	case bean::MessageExt_Secret:
		{
			message.chatType = (protocol::MessageNotification::Type_Secret);
		}
		break;
	}
	if (!m_msgEncrypt)
	{
		message.encrypt = false;
		message.body    = body.body().toUtf8().constData();
	}
	else
	{
		message.encrypt = true;
		message.body    = MsgEncryptionUtil::encrypt(body.body().toUtf8(), m_msgPassword);
	}

	foreach(bean::AttachItem item, body.attachs())
	{
		protocol::MessageNotification::Attachment att;

		switch (item.transferType())
		{
		case bean::AttachItem::Type_Default:
			att.ftType = protocol::MessageNotification::Attachment::FtType_Common;
			break;
		case bean::AttachItem::Type_AutoDisplay:
			att.ftType = protocol::MessageNotification::Attachment::FtType_Autodisplay;
			break;
		case bean::AttachItem::Type_AutoDownload:
			att.ftType = protocol::MessageNotification::Attachment::FtType_Autodownload;
			break;
		case bean::AttachItem::Type_Dir:
			att.ftType = protocol::MessageNotification::Attachment::FtType_Dir;
			break;
		}

		att.transType     = protocol::MessageNotification::Attachment::Trans_Upload;
		att.size          = item.size();
		att.time          = item.time();
		att.name          = item.filename().toUtf8().constData();
		att.format        = item.format().toUtf8().constData();
		att.guid          = item.uuid().toUtf8().constData();
		att.path          = item.filepath().toUtf8().constData();
		att.absoluteFile  = item.filepath().toUtf8().constData();
		att.source        = item.source().toUtf8().constData();
		att.picWidth      = item.picWidth();
		att.picHeight     = item.picHeight();

		message.attachments.push_back(att);
	}

	return true;
}

void SendMessageManager::setMsgEncrypt(bool msgEncrypt)
{
	m_msgEncrypt = msgEncrypt;
}

void SendMessageManager::setMsgPassword(const QByteArray &msgPassword)
{
	m_msgPassword = msgPassword;
}

bool SendMessageManager::initObject()
{
	m_nHandleId = PmClient::instance()->insertResponseHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qWarning() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void SendMessageManager::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* SendMessageManager::instance()
{
	return this;
}

int SendMessageManager::handledId() const
{
	return m_nHandleId;
}

QList<int> SendMessageManager::types() const
{
	QList<int> ret;
	ret << protocol::Request_Msg_Send;
	return ret;
}

bool SendMessageManager::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_nHandleId != handleId)
	{
		return false;
	}

	int type = req->getType();
	do 
	{
		// process
		switch (type)
		{
		case protocol::Request_Msg_Send:
			processSendMessage(req, res);
			break;
		default:
			qWarning() << Q_FUNC_INFO << "error";
			break;
		}
	} while (0);

	return true;
}

void SendMessageManager::processSendMessage(net::Request* req, protocol::Response* res)
{
	QString seq = QString::fromUtf8(req->getSeq().c_str());

	if (processResponseError(req)) // error
	{	
		QMetaObject::invokeMethod(this, "sendMessageFailed", Qt::QueuedConnection, Q_ARG(QString, seq));
		return;
	}

	protocol::SendMessageResponse *pRes = static_cast<protocol::SendMessageResponse *>(res);
	QString ts = QString::fromUtf8(pRes->getTimeStamp().c_str());

	QMetaObject::invokeMethod(this, "sendMessageOK", Qt::QueuedConnection, Q_ARG(QString, seq), Q_ARG(QString, ts));
}

bool SendMessageManager::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		QString sReqTitle = "Send message request";
		QString sError = QString::fromUtf8(req->getMessage().c_str());
		QString errmsg = QString("%1-%2%3(%4)").arg(sReqTitle)
			                                   .arg(QString::fromUtf8(req->getSeq().c_str()))
											   .arg(" error: ")
											   .arg(sError);
		qWarning() << errmsg;
	}

	return bError;
}