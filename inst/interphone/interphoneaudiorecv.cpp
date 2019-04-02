#include "interphoneaudiorecv.h"
#include "interphoneaudiorecvtask.h"

namespace interphone
{

	InterphoneAudioRecv::InterphoneAudioRecv(RecvBeginGenerator &recvBeginGenerator, QObject *parent /*= 0*/)
		: AudioRecv(recvBeginGenerator, parent)
	{
		setBufferedFrameInMs(100);
	}

	InterphoneAudioRecv::~InterphoneAudioRecv()
	{

	}

	AudioRecvTask * InterphoneAudioRecv::createTask()
	{
		return new InterphoneAudioRecvTask(this);
	}

}