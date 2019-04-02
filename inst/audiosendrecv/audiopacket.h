#ifndef AUDIOPACKET_H
#define AUDIOPACKET_H

#include <QByteArray>
#include <QList>

class AudioPacket
{
public:
	AudioPacket();
	virtual ~AudioPacket() {}

	int seq() const { return m_nSeq; }
	void setSeq(int nSeq) { m_nSeq = nSeq; } 

	QList<QByteArray> AFList() const { return m_AFlist; }
	void setAFList(const QList<QByteArray> &afList) { m_AFlist = afList; }

	int frameCount() const { return m_nFrameCount; }
	void setFrameCount(int count) { m_nFrameCount = count; }

public:
	// parse the audio package of "ba"
	// @param [in]  ba:               audio package data
	// @param [in]  frameTimeInMs:    how many milliseconds of a frame
	// @param [out] packageTimeInMs:  how many milliseconds of this package
	virtual bool parse(const QByteArray &ba, int frameTimeInMs, int &packageTimeInMs) = 0;

	// to make a packet
	virtual QByteArray make() = 0;

	// check if this packet is valid
	virtual bool isValid() const = 0;

private:
	int               m_nSeq;
	int               m_nFrameCount;
	QList<QByteArray> m_AFlist;
};

#endif // AUDIOPACKET_H
