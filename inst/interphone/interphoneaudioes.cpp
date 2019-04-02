#include "interphoneaudioes.h"
#include "interphoneaudioudpsender.h"

namespace interphone
{

	InterphoneAudioES::InterphoneAudioES(const QString &uid)
		: AudioES(), m_uid(uid)
	{

	}

	InterphoneAudioES::~InterphoneAudioES()
	{

	}

	AudioUdpSender *InterphoneAudioES::createSender(AudioES *audioES, int periodSize)
	{
		return new InterphoneAudioUdpSender(audioES, periodSize);
	}

}