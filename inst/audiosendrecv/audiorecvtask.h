#ifndef AUDIORECVTASK_H
#define AUDIORECVTASK_H

#include <QThread>

class AudioRecv;

class AudioRecvTask : public QThread
{
	Q_OBJECT

public:
	explicit AudioRecvTask(AudioRecv *ar, QObject *parent = 0);
	virtual ~AudioRecvTask();

	bool startRun();
	bool stopRun();

	virtual void doRecvAndPlay() = 0;

protected: // from QThread
	void run();

protected:
	volatile bool m_bExit;
	AudioRecv    *m_pAR;
};

#endif // AUDIORECVTASK_H
