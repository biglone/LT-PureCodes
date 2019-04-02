#include "interphonepacket.h"
#include <QDataStream>
#include "common/crc.h"
#include "Account.h"

namespace interphone
{

	InterphonePacket::InterphonePacket() : AudioPacket()
	{

	}

	InterphonePacket::~InterphonePacket()
	{

	}

	bool InterphonePacket::parse(const QByteArray &ba, int frameTimeInMs, int &packageTimeInMs)
	{
		/* audiopacket
		crc32   |type   |id     |uid    |seq    |audioframe
		4       |4      |4+len  |4+len  |4      |[4+frame]
		*/

		bool bRet = false;
		packageTimeInMs = 0;

		do 
		{
			QDataStream ds(ba);

			int crc;
			ds >> crc;

			int type;
			ds >> type;

			if (type != 1)
				break;

			QString id;
			char *pszId = 0;
			ds >> pszId;
			id = QString::fromLatin1(pszId);
			delete[] pszId;
			setId(id);

			QString uid;
			char *pszUid = 0;
			ds >> pszUid;
			QString fullUid = QString::fromLatin1(pszUid);
			uid = Account::idFromFullId(fullUid);
			delete[] pszUid;
			setUid(uid);

			int seq = -1;
			ds >> seq;
			setSeq(seq);

			if (ds.atEnd())
				break;

			QList<QByteArray> afList;
			setFrameCount(1);
			
			int frameLen = 0;
			ds >> frameLen;

			QByteArray frame;
			frame.resize(frameLen);
			frameLen = ds.readRawData(frame.data(), frameLen);
			if (frameLen != frame.length())
			{
				break;
			}

			packageTimeInMs += frameTimeInMs;

			afList.append(frame);

			setAFList(afList);

			bRet = true;

		} while (0);

		return bRet;

	}

	QByteArray InterphonePacket::make()
	{
		/* audiopacket
		crc32   |type   |id     |uid    |seq    |audioframe
		4       |4      |4+len  |4+len  |4      |[4+frame]
		*/

		QByteArray data;
		QDataStream ds(&data, QIODevice::WriteOnly);

		const int AudioType = 1;
		ds << AudioType;

		QByteArray ba = id().toLatin1();
		ds << ba;

		QString fullUid = Account::fullIdFromIdResource(uid(), RESOURCE_COMPUTER);
		ba = fullUid.toLatin1();
		ds << ba;

		int seq = this->seq();
		ds << seq;

		// audio frame
		QList<QByteArray> afList = AFList();
		foreach (QByteArray frame, afList)
		{
			ds << frame.length();
			ds.writeRawData(frame.data(), frame.length());
		}

		// crc
		QByteArray crcba;
		QDataStream retDs(&crcba, QIODevice::WriteOnly);
		int crc = CRC_32((unsigned char*)data.data(), data.length());
		retDs << crc;

		data.insert(0, crcba);

		return data;
	}

	bool InterphonePacket::isValid() const
	{
		if (seq() < 0 || AFList().isEmpty())
			return false;

		return true;
	}

}