#include "LocalCommMessage.h"

LocalCommMessage::LocalCommMessage()
{
	m_messageType = 0;
	m_requestCode = 0;
	m_responseCode = 0;
}

void LocalCommMessage::setMessageType(quint32 messageType)
{
	m_messageType = messageType;
}

quint32 LocalCommMessage::messageType() const
{
	return m_messageType;
}

void LocalCommMessage::setRequestCode(quint32 requestCode)
{
	m_requestCode = requestCode;
}

quint32 LocalCommMessage::requestCode() const
{
	return m_requestCode;
}

void LocalCommMessage::setResponseCode(quint32 responseCode)
{
	m_responseCode = responseCode;
}

quint32 LocalCommMessage::responseCode() const
{
	return m_responseCode;
}

void LocalCommMessage::setData(const char *data, int size)
{
	m_data = QByteArray(data, size);
}

void LocalCommMessage::setData(const QByteArray &data)
{
	m_data = data;
}

QByteArray LocalCommMessage::data() const
{
	return m_data;
}

void LocalCommMessage::fromStream(const QByteArray &stream)
{
	// the stream format is: 4 byte packet length + 4 byte message type + 4 byte request code + 4 byte response code + data
	int unitLen = sizeof(quint32);
	if (stream.length() < 4*unitLen)
		return;

	quint32 len = *((quint32 *)(stream.left(unitLen).constData()));
	if (len != stream.size())
		return;

	m_messageType = *((quint32 *)(stream.mid(unitLen, unitLen).constData()));
	m_requestCode = *((quint32 *)(stream.mid(2*unitLen, unitLen).constData()));
	m_responseCode = *((quint32 *)(stream.mid(3*unitLen, unitLen).constData()));
	m_data = stream.mid(4*unitLen);
}

QByteArray LocalCommMessage::toStream() const
{
	// the stream format is: 4 byte packet length + 4 byte message type + 4 byte request code + 4 byte response code + data
	int unitLen = sizeof(quint32);
	quint32 streamSize = 4*unitLen + m_data.size();
	char *streamBuffer = new char[streamSize];
	char *p = streamBuffer;
	memcpy(p, &streamSize, unitLen);
	p += unitLen;
	memcpy(p, &m_messageType, unitLen);
	p += unitLen;
	memcpy(p, &m_requestCode, unitLen);
	p += unitLen;
	memcpy(p, &m_responseCode, unitLen);
	p += unitLen;
	if (!m_data.isEmpty())
		memcpy(p, m_data.constData(), m_data.size());
	QByteArray stream(streamBuffer, streamSize);
	delete[] streamBuffer;
	streamBuffer = 0;
	return stream;
}