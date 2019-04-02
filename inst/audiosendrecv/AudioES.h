#ifndef _AUDIOES_H_
#define _AUDIOES_H_

#include <QObject>
#include <QMap>
#include <QMultiHash>
#include <QList>
#include <QScopedPointer>
#include <QByteArray>
#include <QtMultimedia/QAudio>
#include <QMutex>
#include <QAudioFormat>

#include "bean/HostInfo.h"
#include "bean/AudioDesc.h"

class AudioUdpSender;
class AudioESThread;

//////////////////////////////////////////////////////////////////////////
// class AudioES
class AudioES : public QObject
{
    Q_OBJECT

public:
    explicit AudioES();
    virtual ~AudioES();

public:
    bool init(const QString &aid, const bean::AudioDesc &adesc, int packageFrameCount = 1);
    void release();

    bool isInit() const;

	bool isAudioDeviceOK() const;

	void setPackageFrameCount(int count);
	int packageFrameCount() const;

    QString id() const;
    bean::AudioDesc audioDesc() const;

    int curSeq() const;

    void append(const QString &sid, const QString &addrs);
    void remove(const QString &sid);

protected:
	virtual AudioUdpSender *createSender(AudioES *audioES, int periodSize) = 0;

private:
	void startAudio();
	void stopAudio();

private:
    QMap<QString, bean::HostInfo>       m_mapSid2Addr;
    QMultiHash<bean::HostInfo, QString> m_hashAddr2Sids;

    QMutex          m_lock;
    bean::AudioDesc m_toADesc;
    int             m_seq;
    QScopedPointer<AudioESThread>   m_audioESThread;
	QAudioFormat    m_audioFormat;

	bool            m_audioDeviceOK;

	int             m_packageFrameCount; // how many frames in one package

    friend class AudioESThread;
    friend class AudioUdpSender;
};

#endif //_AUDIOES_H_
