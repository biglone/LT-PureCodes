#include "AudioRecv.h"
#include "util/NetUtil.h"
#include "settings/GlobalSettings.h"
#include "recvbegingenerator.h"
#include <QAudioFormat>
#include "audiorecvtask.h"
#include <QAudioDeviceInfo>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AudioRecv
AudioRecv::AudioRecv(RecvBeginGenerator &recvBeginGenerator, QObject *parent /*= 0*/)
    : QObject(parent)
	, m_recvBeginGenerator(recvBeginGenerator)
	, m_bufferedFrameInMs(200) // default 200ms to buffer
	, m_logToFile(false)
{
}

AudioRecv::~AudioRecv()
{
    stop();
}

RecvBeginGenerator & AudioRecv::recvBeginGenerator() const
{
	return m_recvBeginGenerator;
}

int AudioRecv::start()
{
    if (m_ADesc.id().isEmpty())
	{
        return -2;
	}

    if (!m_ADesc.send())
    {
        return -3;
    }

    if (!initAudioOutput())
    {
        return -4;
    }

	if (m_audioRecvTask.isNull())
	{
		m_audioRecvTask.reset(createTask());
	}

	stop();

	m_logToFile = GlobalSettings::audioLogToFile();

    m_audioRecvTask->startRun();

    return 0;
}

void AudioRecv::stop()
{
	if (m_audioRecvTask)
		m_audioRecvTask->stopRun();
}

bool AudioRecv::isActive() const
{
	if (m_audioRecvTask)
		return m_audioRecvTask->isRunning();
	else
		return false;
}

bean::AudioDesc AudioRecv::desc() const
{
    return m_ADesc;
}

void AudioRecv::setDesc( const bean::AudioDesc &desc )
{
    m_ADesc = desc;
}

void AudioRecv::setAddr( const QString &addrs )
{
    m_addr = NetUtil::getRightAddress(addrs);
}

bean::HostInfo AudioRecv::addr() const
{
	return m_addr;
}

void AudioRecv::setBeginSeq( int seq )
{
    m_ADesc.setBeginseq(seq);
}

void AudioRecv::setBeginTime( qint64 time )
{
    m_ADesc.setBegintime(time);
}

void AudioRecv::setBufferedFrameInMs(int ms)
{
	m_bufferedFrameInMs = ms;
}

int AudioRecv::bufferedFrameInMs() const
{
	return m_bufferedFrameInMs;
}

bool AudioRecv::logToFile() const
{
	return m_logToFile;
}

void AudioRecv::setAudioRecvBegin()
{
	emit audioRecvBegin();
}

void AudioRecv::setAudioPlayed()
{
	emit audioPlayed();
}

bool AudioRecv::initAudioOutput()
{
    bool bRet = false;
    do 
    {
        QAudioFormat format;
        // Set up the format
        format.setSampleRate(m_ADesc.rate());
        format.setChannelCount(m_ADesc.chan());
        format.setSampleSize(m_ADesc.bit());
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format)) 
		{
            qWarning() << "Raw audio format not supported by back-end, cannot play audio.";
			qDebug() << "supported byte order: " << info.supportedByteOrders()
			<< "\n	supported channel counts: " << info.supportedChannelCounts()
			<< "\n	supported codecs: " << info.supportedCodecs()
			<< "\n	supported sample rates: " << info.supportedSampleRates()
			<< "\n	supported sample sizes: " << info.supportedSampleSizes()
			<< "\n	supported sample types: " << info.supportedSampleTypes();
            break;
        }

        bRet = true;
    } while (0);

    return bRet;
}

