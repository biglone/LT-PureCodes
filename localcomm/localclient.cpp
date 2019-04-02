#include "localclient.h"
#include <QLocalSocket>

static void registerMetatype()
{
	static bool isInit = false;
	if (!isInit)
	{
		qRegisterMetaType<LocalCommMessage>("LocalCommMessage");
		isInit = true;
	}
}

LocalClient::LocalClient(QObject *parent)
	: QObject(parent), m_connected(false)
{
	registerMetatype();

	m_localSocket = new QLocalSocket(this);
	
	bool connected = false;
	connected = connect(m_localSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	Q_ASSERT(connected);

	connected = connect(m_localSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	Q_ASSERT(connected);

	connected = connect(m_localSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	Q_ASSERT(connected);
}

LocalClient::~LocalClient()
{

}

bool LocalClient::connectToServer(const QString &serverName, int waitMSC)
{
	m_localSocket->connectToServer(serverName);
	if (waitMSC > 0)
		return m_localSocket->waitForConnected(waitMSC);
	return true;
}

bool LocalClient::disconnectFromServer(int waitMSC)
{
	m_localSocket->disconnectFromServer();
	if (waitMSC > 0)
		return m_localSocket->waitForDisconnected(waitMSC);
	return true;
}

bool LocalClient::connected() const
{
	return m_connected;
}

bool LocalClient::sendMessage(const LocalCommMessage &msg, int waitMSC)
{
	if (!m_connected)
		return false;

	QByteArray stream = msg.toStream();
	m_localSocket->write(stream);
	if (waitMSC > 0)
		m_localSocket->waitForBytesWritten(waitMSC);
	return true;
}

void LocalClient::onConnected()
{
	m_connected = true;
	emit sessionConnected();
}

void LocalClient::onDisconnected()
{
	m_connected = false;
	emit sessionDisconnected();
}

void LocalClient::onReadyRead()
{
	// read each message from session
	char buffer[2048];
	QByteArray tmpBuffer;
	qint64 readCount = 0;

	do {
		readCount = m_localSocket->read(buffer, sizeof(buffer));
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
				// emit messageReceived(msg);
				QMetaObject::invokeMethod(this, "messageReceived", Qt::QueuedConnection, Q_ARG(LocalCommMessage, msg));

				// update tmpBuffer
				tmpBuffer = tmpBuffer.mid(msgStreamLen);
			}
		}
	} while (readCount > 0);
}