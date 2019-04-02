#ifndef __LOCALSERVER_H__
#define __LOCALSERVER_H__

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QLocalSocket>
#include "LocalCommMessage.h"

class QLocalServer;

class LocalServer : public QObject
{
	Q_OBJECT

public:
	LocalServer(const QString &serverName, QObject *parent);
	~LocalServer();

	// start this local server
	bool start();

	// stop this local server
	void stop();

	// check server is running
	bool isRunning() const;

	// all connected session count
	int sessionCount() const;

	// send a message to a session, waitMSC: wait how many milliseconds, default not wait
	bool sendMessage(const QString &sessionId, const LocalCommMessage &msg, int waitMSC = 0);

	// send to first session count messages
	bool sendMessage(int sessionCount, const LocalCommMessage &msg, int waitMSC = 0);

	// send a message to all sessions which current connected, waitMSC: wait how many milliseconds, default not wait
	bool broadcastMessage(const LocalCommMessage &msg, int waitMSC = 0);

	// clear all the sessions
	void clearAllSessions();

signals:
	void newSessionConnected(const QString &sessionId);
	void sessionDisconnected(const QString &sessionId);
	void messageReceived(const QString &sessionId, const LocalCommMessage &msg);

private slots:
	void onNewConnection();
	void onDisconnected();
	void onReadyRead();
	void onSessionError(QLocalSocket::LocalSocketError socketError);

private:
	QLocalSocket* session(const QString &sessionId) const;
	void doDisconnect(QLocalSocket *localSocket);

private:
	QLocalServer                 *m_localServer;
	QString                       m_serverName;
	int                           m_sessionCount;
	QMap<QLocalSocket *, QString> m_sessions;
	QStringList                   m_sessionIds;
};

#endif // __LOCALSERVER_H__
