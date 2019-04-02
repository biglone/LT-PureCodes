#include "audiorecvtask.h"

AudioRecvTask::AudioRecvTask(AudioRecv *ar, QObject *parent /*= 0*/)
	: QThread(parent), m_pAR(ar), m_bExit(true)
{
	Q_ASSERT(m_pAR != NULL);
}

AudioRecvTask::~AudioRecvTask()
{
	stopRun();
}

bool AudioRecvTask::startRun()
{
	if (isRunning())
		return true;

	m_bExit = false;
	start();

	return true;
}

bool AudioRecvTask::stopRun()
{
	if (isRunning())
	{
		m_bExit = true;
		wait();
	}

	return true;
}

void AudioRecvTask::run()
{
	doRecvAndPlay();
}
