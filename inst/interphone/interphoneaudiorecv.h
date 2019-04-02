#ifndef INTERPHONEAUDIORECV_H
#define INTERPHONEAUDIORECV_H

#include "AudioRecv.h"

namespace interphone
{

	class InterphoneAudioRecv : public AudioRecv
	{
		Q_OBJECT

	public:
		InterphoneAudioRecv(RecvBeginGenerator &recvBeginGenerator, QObject *parent = 0);
		~InterphoneAudioRecv();

	protected: // from AudioRecv
		AudioRecvTask * createTask();
		
	};

}

#endif // INTERPHONEAUDIORECV_H
