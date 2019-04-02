#ifndef __LOCAL_COMM_MESSAGE_H__
#define __LOCAL_COMM_MESSAGE_H__

#include <QByteArray>

class LocalCommMessage 
{
public:
	LocalCommMessage();

	void setMessageType(quint32 messageType);
	quint32 messageType() const;

	void setRequestCode(quint32 requestCode);
	quint32 requestCode() const;

	void setResponseCode(quint32 responseCode);
	quint32 responseCode() const;

	void setData(const char *data, int size);
	void setData(const QByteArray &data);
	QByteArray data() const;

	void fromStream(const QByteArray &stream);
	QByteArray toStream() const;

private:
	quint32 m_messageType;
	quint32 m_requestCode;
	quint32 m_responseCode;
	QByteArray m_data;
};

#endif // __LOCAL_COMM_MESSAGE_H__