#ifndef INTERPHONEPACKET_H
#define INTERPHONEPACKET_H

#include "audiopacket.h"
#include <QString>

namespace interphone
{

	/* audiopacket
    crc32   |type   |id     |uid    |seq    |audioframe
    4       |4      |4+len  |4+len  |4      |[4+frame]
	*/

class InterphonePacket : public AudioPacket
{
public:
	InterphonePacket();
	~InterphonePacket();

	void setId(const QString &id) { m_id = id; }
	QString id() const { return m_id; }

	void setUid(const QString &uid) { m_uid = uid; }
	QString uid() const { return m_uid; }

public:
	virtual bool parse(const QByteArray &ba, int frameTimeInMs, int &packageTimeInMs);
	virtual QByteArray make();
	virtual bool isValid() const;

private:
	QString m_id;
	QString m_uid;
};

}

#endif // INTERPHONEPACKET_H
