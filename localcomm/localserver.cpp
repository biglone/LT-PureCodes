#include "localserver.h"
#include <QLocalServer>
#include <QUuid>
#include <QByteArray>

static void registerMetatype()
{
	static bool isInit = false;
	if (!isInit)
	{
		qRegisterMetaType<LocalCommMessage>("LocalCommMessage");
		isInit = true;
	}
}

LocalServer::LocalServer(const QString &serverName, QObject *parent)
	: QObject(parent), m_serverName(serverName), m_sessionCount(0)
{
	registerMetatype();

	m_localServer = new QLocalServer(this);
	
	bool connected = false;
	connected = connect(m_localServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	Q_ASSERT(connected);
}

LocalServer::~LocalServer()
{
	clearAllSessions();

	if (isRunning())
		stop();
}

bool LocalServer::start()
{
	return m_localServer->listen(m_serverName);
}

void LocalServer::stop()
{
	m_localServer->close();
}

bool LocalServer::isRunning() const
{
	return m_localServer->isListening();
}

int LocalServer::sessionCount() const
{
	return m_sessionCount;
}

bool LocalServer::sendMessage(const QString &sessionId, const LocalCommMessage &msg, int waitMSC)
{
	QLocalSocket *localSocket = session(sessionId);
	if (!localSocket)
		return false;

	QByteArray stream = msg.toStream();
	localSocket->write(stream);
	if (waitMSC > 0)
		localSocket->waitForBytesWritten(waitMSC);
	return true;
}

bool LocalServer::sendMessage(int sessionCount, const LocalCommMessage &msg, int waitMSC)
{
	if (sessionCount <= 0)
		return false;

	if (sessionCount > m_sessionCount)
		sessionCount = m_sessionCount;

	for (int i = 0; i < sessionCount; i++)
	{
		QString sessionId = m_sessionIds[i];
		QLocalSocket *localSocket = session(sessionId);
		QByteArray stream = msg.toStream();
		localSocket->write(stream);
		if (waitMSC > 0)
			localSocket->waitForBytesWritten(waitMSC);
	}
	return true;
}

bool LocalServer::broadcastMessage(const LocalCommMessage &msg, int waitMSC)
{
	if (!m_sessionCount)
		return true;

	QList<QLocalSocket *> localSockets = m_sessions.keys();
	QByteArray stream = msg.toStream();
	foreach (QLocalSocket *localSocket, localSockets)
	{
		localSocket->write(stream);
		if (waitMSC > 0)
			localSocket->waitForBytesWritten(waitMSC);
	}
	return true;
}

void LocalServer::clearAllSessions()
{
	QList<QLocalSocket *> localSockets = m_sessions.keys();
	foreach (QLocalSocket *localSocket, localSockets)
	{
		delete localSocket;
		localSocket = 0;
	}
	m_sessions.clear();
	m_sessionIds.clear();
}

void LocalServer::onNewConnection()
{
	while (m_localServer->hasPendingConnections())
	{
		QLocalSocket *localSocket = m_localServer->nextPendingConnection();
		bool connected = false;
		connected = connect(localSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
		Q_ASSERT(connected);
		connected = connect(localSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		Q_ASSERT(connected);
		connected = connect(localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
			this, SLOT(onSessionError(QLocalSocket::LocalSocketError)));
		Q_ASSERT(connected);

		// add to session array
		m_sessionCount++;
		QUuid sid = QUuid::createUuid();
		QString sessionId = sid.toString();
		m_sessions[localSocket] = sessionId;
		m_sessionIds.append(sessionId);

		// notify session connected
		emit newSessionConnected(sessionId);
	}
}

void LocalServer::onDisconnected()
{
	QLocalSocket *localSocket = qobject_cast<QLocalSocket *>(sender());
	if (!localSocket)
		return;
	
	doDisconnect(localSocket);
}

void LocalServer::onReadyRead()
{
	QLocalSocket *localSocket = qobject_cast<QLocalSocket *>(sender());
	if (!localSocket)
		return;

	QString sessionId = m_sessions[localSocket];
	char buffer[2048];
	QByteArray tmpBuffer;
	qint64 readCount = 0;

	// read each message from session
	do {
		readCount = localSocket->read(buffer, sizeof(buffer));
		if (readCount > 0)
		{
			tmpBuffer.append(buffer, readCount);
			while (tmpBuffer.size() > sizeof(quint32))
			{
				quint32 msgStreamLen = *((quint32 *)(tmpBuffer.left(sizeof(quint32)).constData()));
				if (tmpBuffer.size() < msgStreamLen) // not a complete message in tmpBuffer
					break;

				// get a full message from it
				QByteArray msgStream = tmpBuffer.left(msgStreamLen);
				LocalCommMessage msg;
				msg.fromStream(msgStream);

				// notify message received
				// emit messageReceived(sessionId, msg);
				QMetaObject::invokeMethod(this, "messageReceived", Qt::QueuedConnection, 
					Q_ARG(QString, sessionId), Q_ARG(LocalCommMessage, msg));

				// update tmpBuffer
				tmpBuffer = tmpBuffer.mid(msgStreamLen);
			}
		}
	} while (readCount > 0);

	// check data error
	if (tmpBuffer.length() > 0)
	{
		qWarning() << Q_FUNC_INFO << "data error";
		doDisconnect(localSocket);
	}
}

void LocalServer::onSessionError(QLocalSocket::LocalSocketError socketError)
{
	QLocalSocket *localSocket = qobject_cast<QLocalSocket *>(sender());
	if (!localSocket)
		return;

	qWarning() << Q_FUNC_INFO << socketError << localSocket->errorString();
	doDisconnect(localSocket);
}

QLocalSocket* LocalServer::session(const QString &sessionId) const
{
	QLocalSocket *localSocket = m_sessions.key(sessionId);
	return localSocket;
}

void LocalServer::doDisconnect(QLocalSocket *localSocket)
{
	if (!localSocket)
		return;

	if (m_sessions.contains(localSocket))
	{
		// remove from session array
		QString sessionId = m_sessions[localSocket];
		m_sessions.remove(localSocket);
		m_sessionIds.removeOne(sessionId);
		localSocket->deleteLater();
		m_sessionCount--;

		// notify session disconnect
		emit sessionDisconnected(sessionId);
	}
}