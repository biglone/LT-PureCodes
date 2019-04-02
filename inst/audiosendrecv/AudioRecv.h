#ifndef _AUDIORECV_H_
#define _AUDIORECV_H_

#include "bean/AudioDesc.h"
#include "bean/HostInfo.h"
#include <QScopedPointer>

class RecvBeginGenerator;
class AudioRecvTask;

//////////////////////////////////////////////////////////////////////////
// class AudioRecv
class AudioRecv : public QObject
{
	Q_OBJECT

public:
    explicit AudioRecv(RecvBeginGenerator &recvBeginGenerator, QObject *parent = 0);
    virtual ~AudioRecv();

public:
	RecvBeginGenerator & recvBeginGenerator() const;

    bean::AudioDesc desc() const;
    void setDesc(const bean::AudioDesc &desc);

    void setAddr(const QString &addrs);
	bean::HostInfo addr() const;

    void setBeginSeq(int seq);
    void setBeginTime(qint64 time);

	void setBufferedFrameInMs(int ms);
	int bufferedFrameInMs() const;

	bool logToFile() const;

	void setAudioRecvBegin();

	void setAudioPlayed();

    int start();
    void stop();
	bool isActive() const;

protected:
	virtual AudioRecvTask * createTask() = 0;

signals:
	void audioRecvBegin();
	void audioPlayed();

private:
    bool initAudioOutput();

private:
    RecvBeginGenerator  &m_recvBeginGenerator;

    bean::AudioDesc m_ADesc;

	int             m_bufferedFrameInMs; // how many milliseconds to buffer before the first frame to play, default 200ms

    bean::HostInfo  m_addr;

    QScopedPointer<AudioRecvTask> m_audioRecvTask;

	bool            m_logToFile; // log received PCM data to file
};

#endif //_AUDIORECV_H_
