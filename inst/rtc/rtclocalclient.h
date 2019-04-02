#ifndef __RTCLOCALCLIENT_H__
#define __RTCLOCALCLIENT_H__

#include <QObject>

class QLocalSocket;
class QByteArray;

class RtcLocalClient : public QObject
{
	Q_OBJECT

public:
	RtcLocalClient(QObject *parent = 0);
	~RtcLocalClient();

	bool connectToServer(const QString &serverName, int waitMSC = 0);
	bool disconnectFromServer(int waitMSC = 0);
	bool connected() const;
	bool sendMessage(const QByteArray &msg, int waitMSC = 0);

Q_SIGNALS:
	void dataRecved(const QByteArray &msg);
	void sessionConnected();
	void sessionDisconnected();

private Q_SLOTS:
	void onConnected();
	void onDisconnected();
	void onReadyRead();

private:
	QLocalSocket *m_localSocket;
	bool          m_connected;
};

#endif // __RTCLOCALCLIENT_H__
