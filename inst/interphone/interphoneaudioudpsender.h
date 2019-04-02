#ifndef INTERPHONEAUDIOUDPSENDER_H
#define INTERPHONEAUDIOUDPSENDER_H

#include "audioudpsender.h"

namespace interphone
{

	class InterphoneAudioUdpSender : public AudioUdpSender
	{
		Q_OBJECT

	public:
		InterphoneAudioUdpSender(AudioES *pAes, int periodSize);
		virtual ~InterphoneAudioUdpSender();

	protected:
		virtual QByteArray makePacket(int seq, const bean::AudioDesc &adesc, const QList<QByteArray> &encodedFrames);
	};

}

#endif // INTERPHONEAUDIOUDPSENDER_H
