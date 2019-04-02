#ifndef _MESSAGEPROCESSOR_H_
#define _MESSAGEPROCESSOR_H_

#include <QObject>
#include <QByteArray>
#include "pmclient/PmClientInterface.h"
#include "bean/MessageBody.h"
#include "protocol/MessageNotification.h"

class MessageProcessor : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler)

public:
	explicit MessageProcessor(QObject *parent = 0);
	~MessageProcessor();

	static void setMsgPassword(const QByteArray &msgPassword);
	static bean::MessageBody message2MsgBody(protocol::MessageNotification::Message *message);

Q_SIGNALS:
	void receiveMessage(const bean::MessageBody& body);

public:
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private slots:
	void processMessage(void* msg);

private:
	int               m_nHandleId;
	static QByteArray m_msgPassword;
};

#endif // _MESSAGEPROCESSOR_H_
