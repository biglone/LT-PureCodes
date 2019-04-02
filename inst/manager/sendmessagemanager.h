#ifndef SENDMESSAGEMANAGER_H
#define SENDMESSAGEMANAGER_H

#include <QObject>
#include <QByteArray>
#include "pmclient/PmClientInterface.h"
#include "protocol/MessageNotification.h"
#include "bean/MessageBody.h"

namespace protocol
{
	class SendMessageRequest;
};

class SendMessageManager : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	SendMessageManager(QObject *parent = 0);
	~SendMessageManager();

	QString setMessage(const bean::MessageBody &msgBody);
	bool deliver(const QString &seq, const bean::MessageBody &msgBody);

	bool msgBody2Message(const bean::MessageBody &body, protocol::MessageNotification::Message &message);

	void setMsgEncrypt(bool msgEncrypt);
	void setMsgPassword(const QByteArray &msgPassword);

signals:
	void sendMessageOK(const QString &seq, const QString &ts);
	void sendMessageFailed(const QString &seq);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private:
	void processSendMessage(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

private:
	int                                          m_nHandleId;
	QMap<QString, protocol::SendMessageRequest*> m_messages;
	QByteArray                                   m_msgPassword;
	bool                                         m_msgEncrypt;
};

#endif // SENDMESSAGEMANAGER_H
