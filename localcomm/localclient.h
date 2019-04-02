#ifndef __LOCALCLIENT_H__
#define __LOCALCLIENT_H__

#include <QObject>
#include "LocalCommMessage.h"

class QLocalSocket;

class LocalClient : public QObject
{
	Q_OBJECT

public:
	LocalClient(QObject *parent);
	~LocalClient();

	bool connectToServer(const QString &serverName, int waitMSC = 0);
	bool disconnectFromServer(int waitMSC = 0);
	bool connected() const;
	bool sendMessage(const LocalCommMessage &msg, int waitMSC = 0);

signals:
	void messageReceived(const LocalCommMessage &msg);
	void sessionConnected();
	void sessionDisconnected();

private slots:
	void onConnected();
	void onDisconnected();
	void onReadyRead();

private:
	QLocalSocket *m_localSocket;
	bool m_connected;
};

#endif // __LOCALCLIENT_H__
