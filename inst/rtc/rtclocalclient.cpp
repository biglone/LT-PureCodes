#include "RtcLocalClient.h"
#include <QLocalSocket>
#include <QDebug>

RtcLocalClient::RtcLocalClient(QObject *parent)
	: QObject(parent), m_connected(false)
{
	m_localSocket = new QLocalSocket(this);
	
	bool connected = false;
	connected = connect(m_localSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	Q_ASSERT(connected);

	connected = connect(m_localSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	Q_ASSERT(connected);

	connected = connect(m_localSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	Q_ASSERT(connected);
}

RtcLocalClient::~RtcLocalClient()
{

}

bool RtcLocalClient::connectToServer(const QString &serverName, int waitMSC)
{
	m_localSocket->connectToServer(serverName);
	if (waitMSC > 0)
		return m_localSocket->waitForConnected(waitMSC);
	return true;
}

bool RtcLocalClient::disconnectFromServer(int waitMSC)
{
	m_localSocket->disconnectFromServer();
	if (waitMSC > 0)
		return m_localSocket->waitForDisconnected(waitMSC);
	return true;
}

bool RtcLocalClient::connected() const
{
	return m_connected;
}

bool RtcLocalClient::sendMessage(const QByteArray &msg, int waitMSC)
{
	if (!m_connected)
		return false;

	m_localSocket->write(msg);
	if (waitMSC > 0)
		m_localSocket->waitForBytesWritten(waitMSC);
	return true;
}

void RtcLocalClient::onConnected()
{
	m_connected = true;
	emit sessionConnected();
}

void RtcLocalClient::onDisconnected()
{
	m_connected = false;
	emit sessionDisconnected();
}

void RtcLocalClient::onReadyRead()
{
	QByteArray data = m_localSocket->readAll();
	// qDebug() << "rtc module ==>" << data;
	emit dataRecved(data);
}