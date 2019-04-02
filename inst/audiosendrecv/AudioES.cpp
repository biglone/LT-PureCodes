#include <QDataStream>
#include <QAudioInput>
#include <QDateTime>
#include <QMutexLocker>
#include <QThread>
#include <QList>
#include <QApplication>
#include <QDebug>

#include "util/NetUtil.h"
#include "util/ThreadUtil.h"

#include "common/datetime.h"

#include "audioudpsender.h"

#include "AudioES.h"

#include "PmApp.h"
#include "logger/logger.h"

//////////////////////////////////////////////////////////////////////////
// class AudioESThread
class AudioESThread : public QThread
{
	Q_OBJECT

public:
	explicit AudioESThread(AudioES *pAes, QObject *parent = 0);
	virtual ~AudioESThread();

	bool startRun();
	bool stopRun();

protected:
	void run();

private:
	AudioES     *m_pAes;
	QAudioInput *m_pAudioInput;
};

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AudioESThread
AudioESThread::AudioESThread(AudioES *pAes, QObject *parent /*= 0*/)
    : QThread(parent)
	, m_pAes(pAes)
    , m_pAudioInput(0)
{
}

AudioESThread::~AudioESThread()
{
	stopRun();
	m_pAes = 0;
}

bool AudioESThread::startRun()
{
	if (isRunning())
		return true;
	
	start();

	return true;
}

bool AudioESThread::stopRun()
{
	qDebug() << Q_FUNC_INFO << "begin";

	if (isRunning())
	{
		quit();
		wait();
	}

	qDebug() << Q_FUNC_INFO << "end";

	return true;
}

void AudioESThread::run()
{
    bool main_thread = QThread::currentThread() == qApp->thread();
	qDebug("AudioES task running in thread %p [main thread=%d]: frame count per package: %d", 
		QThread::currentThreadId(), main_thread, m_pAes->m_packageFrameCount);

    m_pAudioInput = new QAudioInput(QAudioDeviceInfo::defaultInputDevice(), m_pAes->m_audioFormat);
    QIODevice *io = m_pAudioInput->start();

    int periodSize = m_pAudioInput->periodSize();
	/*
    int frameMs = periodSize * 8000 / m_pAes->m_toADesc.rate() / m_pAes->m_toADesc.bit() / m_pAes->m_toADesc.chan();
    m_pAes->m_toADesc.setFrame(frameMs);
	*/
	
	/*
	qDebug("AudioES begin: ------------------- periodSize: %d, rate: %d, bit: %d, channel: %d, buffer size: %d",
		periodSize, m_pAes->m_toADesc.rate(), m_pAes->m_toADesc.bit(), m_pAes->m_toADesc.chan(), m_pAudioInput->bufferSize());
	*/

    AudioUdpSender *sender = m_pAes->createSender(m_pAes, periodSize);
    connect(io, SIGNAL(readyRead()), sender, SLOT(onReadyRead()));

    QThread::exec();

	/*
	qDebug("AudioES end: ------------------- periodSize: %d, rate: %d, bit: %d, channel: %d, buffer size: %d",
		periodSize, m_pAes->m_toADesc.rate(), m_pAes->m_toADesc.bit(), m_pAes->m_toADesc.chan(), m_pAudioInput->bufferSize());
	*/

	m_pAudioInput->stop();
    delete m_pAudioInput;
	m_pAudioInput = 0;

	delete sender;
	sender = 0;

    qDebug("AudioESThread task stop in thread %p", QThread::currentThreadId());
}

////////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AudioES
AudioES::AudioES()
    : m_audioFormat()
	, m_audioDeviceOK(true)
	, m_packageFrameCount(1)
{
	m_audioESThread.reset(new AudioESThread(this));
}

AudioES::~AudioES()
{
    release();
}

bool AudioES::init( const QString &aid, const bean::AudioDesc &adesc, int packageFrameCount /*= 1*/ )
{
    if (m_audioESThread->isRunning())
	{
        return true;
	}

	m_packageFrameCount = packageFrameCount;
	m_audioDeviceOK = true;
	bool inited = false;
	do 
	{
		m_toADesc = adesc;
		m_toADesc.setId(aid);

		// init audio format
		QAudioFormat format;
		format.setSampleRate(m_toADesc.rate());
		format.setChannelCount(m_toADesc.chan());
		format.setSampleSize(m_toADesc.bit());
		format.setCodec("audio/pcm");
		format.setByteOrder(QAudioFormat::LittleEndian);
		format.setSampleType(QAudioFormat::UnSignedInt);

		QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
		if (!info.isFormatSupported(format)) 
		{
			qWarning() << "default format not supported try to use nearest";
			qPmApp->getLogger()->debug(QString("!!!!!!!!!!!! AudioES default format not supported try to use nearest."));
			qDebug() << "supported byte order: " << info.supportedByteOrders()
			<< "\n	supported channel counts: " << info.supportedChannelCounts()
			<< "\n	supported codecs: " << info.supportedCodecs()
			<< "\n	supported sample rates: " << info.supportedSampleRates()
			<< "\n	supported sample sizes: " << info.supportedSampleSizes()
			<< "\n	supported sample types: " << info.supportedSampleTypes();

			format = info.nearestFormat(format);
		}

		if (!format.isValid())
		{
			qPmApp->getLogger()->debug(QString("!!!!!!!!!!!! AudioES can't find the right format"));
			break;
		}

		// init audio desc
		m_toADesc.setRate(format.sampleRate());
		m_toADesc.setChan(format.channelCount());
		m_toADesc.setBit(format.sampleSize());
		
		m_audioFormat = format;

		inited = true;

	} while (0);

	m_seq = 1;
	m_toADesc.setBeginseq(m_seq);
	m_toADesc.setBegintime(CDateTime::currentMSecsSinceEpoch());

	if (!inited)
	{
		m_audioFormat = QAudioFormat();

		m_audioDeviceOK = false; // audio device is not ok
	}

	return inited;
}

void AudioES::release()
{
    m_audioESThread->stopRun();

    m_toADesc = bean::AudioDesc();
	m_audioFormat = QAudioFormat();
    m_seq = 0;
}

bool AudioES::isInit() const
{
    return m_audioESThread->isRunning();
}

bool AudioES::isAudioDeviceOK() const
{
	return m_audioDeviceOK;
}

void AudioES::setPackageFrameCount(int count)
{
	m_packageFrameCount = count;
}

int AudioES::packageFrameCount() const
{
	return m_packageFrameCount;
}

QString AudioES::id() const
{
    return m_toADesc.id();
}

bean::AudioDesc AudioES::audioDesc() const
{
    return m_toADesc;
}

int AudioES::curSeq() const
{
    return m_seq;
}

void AudioES::append( const QString &sid, const QString &addrs )
{
	bool start = false;
	{
		QMutexLocker lock(&m_lock);

		if (m_mapSid2Addr.contains(sid))
		{
			return;
		}

		start = m_mapSid2Addr.isEmpty();

		bean::HostInfo addr = NetUtil::getRightAddress(addrs);
		m_mapSid2Addr.insert(sid, addr);
		if (!m_hashAddr2Sids.contains(addr, sid))
		{
			m_hashAddr2Sids.insertMulti(addr, sid);
		}
	}

	if (start)
	{
		startAudio();
	}
}

void AudioES::remove( const QString &sid )
{
	bool stop = false;
	{
		QMutexLocker lock(&m_lock);

		if (m_mapSid2Addr.contains(sid))
		{
			bean::HostInfo addr = m_mapSid2Addr.take(sid);
			m_hashAddr2Sids.remove(addr, sid);
		}

		stop = m_mapSid2Addr.isEmpty();
	}

	if (stop)
	{
		stopAudio();
	}
}

void AudioES::startAudio()
{
	if (m_audioDeviceOK)
	{
		m_audioESThread->startRun();
	}
}

void AudioES::stopAudio()
{
	m_audioESThread->stopRun();
}

#include "AudioES.moc"
