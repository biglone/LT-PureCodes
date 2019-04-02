#ifndef INTERPHONEAUDIORECVTASK_H
#define INTERPHONEAUDIORECVTASK_H

#include "audiorecvtask.h"
#include <QList>
#include <QByteArray>
#include <QAudio>

class QAudioOutput;
class PmAudioDecoder;
class QIODevice;
class AudioPacket;

namespace interphone
{

	class InterphoneAudioRecvTask : public AudioRecvTask
	{
		Q_OBJECT

	public:
		InterphoneAudioRecvTask(AudioRecv *ar, QObject *parent = 0);
		~InterphoneAudioRecvTask();

	protected:
		void doRecvAndPlay();

	private slots:
		void onAudioOutputStateChanged(QAudio::State state);
		void onAudioOutputNotify();

	private:
		int doFecPacket(const AudioPacket &packet, int nextRecvSeq);
		QByteArray getRecvBeginPacket();
		int play(const QByteArray &pcm);
		int play();
		int playEmpty();
		void decodePacket(const AudioPacket &ap);
		QByteArray decode(const QByteArray &af, int sampleSize, int fec = 0);
		void startAudioOutput();
		void stopAudioOutput();
		int recvedAudioInMs() const;

	private:
		PmAudioDecoder        *m_audioDecoder;
		QAudioOutput          *m_pAudioOutput;
		QIODevice             *m_pIo;
		QList<QByteArray>      m_pcms;
	};

}

#endif // INTERPHONEAUDIORECVTASK_H
