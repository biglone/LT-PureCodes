#include "interphoneaudioudpsender.h"
#include "interphonepacket.h"
#include "bean/AudioDesc.h"
#include "interphoneaudioes.h"

namespace interphone
{

	InterphoneAudioUdpSender::InterphoneAudioUdpSender(AudioES *pAes, int periodSize)
		: AudioUdpSender(pAes, periodSize)
	{

	}

	InterphoneAudioUdpSender::~InterphoneAudioUdpSender()
	{

	}

	QByteArray InterphoneAudioUdpSender::makePacket(int seq, const bean::AudioDesc &adesc, const QList<QByteArray> &encodedFrames)
	{
		Q_ASSERT(encodedFrames.count() == 1);

		InterphoneAudioES *audioES = (InterphoneAudioES *)(this->audioES());

		QByteArray data;
		InterphonePacket packet;
		packet.setId(adesc.id());
		packet.setUid(audioES->uid());
		packet.setSeq(seq);
		packet.setFrameCount(1);
		packet.setAFList(encodedFrames);
		data = packet.make();
		return data;
	}

}
