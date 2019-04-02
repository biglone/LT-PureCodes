#ifndef AUDIOUDPSENDER_H
#define AUDIOUDPSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QList>
#include <QByteArray>

class AudioES;
class PmAudioEncoder;

namespace bean
{
	class AudioDesc;
}

//////////////////////////////////////////////////////////////////////////
// class AudioUdpSender
class AudioUdpSender : public QObject
{
	Q_OBJECT
public:
	explicit AudioUdpSender(AudioES *pAes, int periodSize);
	virtual ~AudioUdpSender();

	AudioES *audioES() const { return m_pAes; }

public slots:
	void onReadyRead();

protected:
	virtual QByteArray makePacket(int seq, const bean::AudioDesc &adesc, const QList<QByteArray> &encodedFrames) = 0;

private:
	void processAudioFrame(const QByteArray &ba);
	QByteArray encodeFrame(const QByteArray &ba);

private:
	QUdpSocket        socket;
	AudioES          *m_pAes;
	int               m_periodSize;
	PmAudioEncoder   *m_audioEncoder;
	int               m_frameCount;
	QList<QByteArray> m_frames;

	//////////////////////////////////////////////////////////////////////////
	// for test
	int               m_lossCount;
	int               m_lossIndex;
	//////////////////////////////////////////////////////////////////////////
};

#endif // AUDIOUDPSENDER_H
