#ifndef INTERPHONEAUDIOES_H
#define INTERPHONEAUDIOES_H

#include "AudioES.h"

namespace interphone
{

	class InterphoneAudioES : public AudioES
	{
		Q_OBJECT

	public:
		InterphoneAudioES(const QString &uid);
		virtual ~InterphoneAudioES();

		QString uid() const { return m_uid; }

	protected:
		AudioUdpSender *createSender(AudioES *audioES, int periodSize);

	private:
		QString m_uid;
	};

}

#endif // INTERPHONEAUDIOES_H
