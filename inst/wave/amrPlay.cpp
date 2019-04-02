#include <QDebug>
#include <QString>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QDateTime>
#include <QBuffer>

#include "pmaudiocodec.h"
#include "amrPlay.h"

//////////////////////////////////////////////////////////////////////////
/// CLASS CAmrDecoderTask
class CAmrDecoderTask : public QThread
{
	Q_OBJECT

public:
	CAmrDecoderTask();
	virtual ~CAmrDecoderTask();

	void setFrameSize(int frameSize) {m_frameSize = frameSize;}
	int frameSize() const {return m_frameSize;}

	void setDecodeData(const QByteArray &decodeData) {m_decodeData = decodeData;}

	void startDecode(int frameSize, const QByteArray &decodeData);
	void stopDecode();

Q_SIGNALS:
	void dataDecoded(const QByteArray &data);

private:
	void run();

private:
	int           m_frameSize;
	QByteArray    m_decodeData;
	volatile bool m_exit;
};

//////////////////////////////////////////////////////////////////////////
/// CLASS CAmrPlayPrivate
class CAmrPlayPrivate : public QObject
{
    Q_OBJECT

public:
    explicit CAmrPlayPrivate(QObject *parent = 0);
    virtual ~CAmrPlayPrivate();

    bool play(const QString& filename);
    void stop();

private slots:
	void onReadyRead();
    void onStateChanged(QAudio::State newState);
    void onNotify();
	void onDataDecoded(const QByteArray &data);

private:
	bool initAudioOutput();
	void startAudioOutput();
	bool checkAmrFile(const QString &filename);

public:
    CAmrPlay                    *q_ptr;
    CAmrPlayMonitor             *m_pMonitor;

    QString                      m_fileName;

    QScopedPointer<QAudioOutput> m_pAO;
    bool                         m_isPlaying;

    QScopedPointer<QBuffer>      m_pWriteBuffer;
    QScopedPointer<QBuffer>      m_pReadBuffer;
    QByteArray                   m_audioData; // audio data to play
	int                          m_audioMs;
	int                          m_frameSize;

	CAmrDecoderTask              m_decodeTask;
	QByteArray                   m_decodeData; // data to decode
};

//////////////////////////////////////////////////////////////////////////
/// CLASS Thread
class Thread : public QThread
{
public:
    static void usleep(unsigned long usec)
    {
        QThread::usleep(usec);
    }

    static void msleep(unsigned long msec)
    {
        QThread::msleep(msec);
    }

    static void sleep(unsigned long sec)
    {
        QThread::sleep(sec);
    }
};

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CAmrPlayPrivate
CAmrPlayPrivate::CAmrPlayPrivate(QObject *parent)
: QObject(parent)
, m_isPlaying(false)
, m_audioMs(0)
, m_frameSize(0)
, m_decodeTask()
{
	connect(&m_decodeTask, SIGNAL(dataDecoded(QByteArray)), this, SLOT(onDataDecoded(QByteArray)));
}

CAmrPlayPrivate::~CAmrPlayPrivate()
{
	m_pMonitor = 0;

    stop();
}

bool CAmrPlayPrivate::play(const QString& filename)
{
    bool ret = false;
    do 
    {
		if (!QFile::exists(filename))
        {
			qWarning() << Q_FUNC_INFO << "file does not exist.";
            break;
        }

		m_fileName = filename;
		m_audioMs = 0;

		if (!checkAmrFile(m_fileName))
		{
			qWarning() << Q_FUNC_INFO << "check amr file failed.";
			break;
		}

		if (!initAudioOutput())
		{
			qWarning() << Q_FUNC_INFO << "init audio output failed.";
			break;
		}

		// decode write buffer
        m_audioData.clear();
        m_pWriteBuffer.reset(new QBuffer(&m_audioData));
		connect(m_pWriteBuffer.data(), SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        m_pWriteBuffer->open(QIODevice::WriteOnly);

		// start decode thread
        m_decodeTask.startDecode(m_frameSize, m_decodeData);

        ret = true;
    } while (0);

    return ret;
}

void CAmrPlayPrivate::stop()
{
	// stop decode task
	m_decodeTask.stopDecode();
	m_decodeData.clear();

	// stop audio output
    if (m_pAO && m_isPlaying)
    {
        m_isPlaying = false;
        m_pAO->stop();
    }

	// close write buffer
	if (!m_pWriteBuffer.isNull())
	{
		m_pWriteBuffer->close();
		m_pWriteBuffer.reset();
	}

	// close read buffer
	if (!m_pReadBuffer.isNull())
	{
		m_pReadBuffer->close();
		m_pReadBuffer.reset();
	}

	// clear audio data
	m_audioData.clear();

	// notify outside
	if (m_pMonitor)
	{
		m_pMonitor->onPlayOver();
	}
}

bool CAmrPlayPrivate::initAudioOutput()
{
	bool inited = false;
    do 
    {
        // must be mono channel, 8000Hz rate and sample size 16
        QAudioFormat format;
        format.setSampleRate(8000);
        format.setChannelCount(1);
        format.setSampleSize(16);
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

        m_pAO.reset(new QAudioOutput(format));
        connect(m_pAO.data(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChanged(QAudio::State)));
        connect(m_pAO.data(), SIGNAL(notify()), this, SLOT(onNotify()));

		inited = true;

    } while (0);

    return inited;
}

void CAmrPlayPrivate::startAudioOutput()
{
	if (m_pAO.isNull())
		return;

	if (m_isPlaying)
		return;

	// start output
	m_pReadBuffer.reset(new QBuffer(&m_audioData));
	m_pReadBuffer.data()->open(QIODevice::ReadOnly);
	m_pAO->start(m_pReadBuffer.data());
	m_isPlaying = true;
}

bool CAmrPlayPrivate::checkAmrFile(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << Q_FUNC_INFO << "open file " << filename << " failed.";
		return false;
	}

	// check file header
	const QByteArray AMR_FILE_HEADER("#!AMR\n", 6);
	QByteArray header = file.read(AMR_FILE_HEADER.length());
	if (header != AMR_FILE_HEADER)
	{
		qDebug() << Q_FUNC_INFO << filename << " is not an amr file.";
		file.close();
		return false;
	}

	int read_size;
	unsigned char analysis[32];
	int dec_mode;
	const short block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };

	// calculate audio time in ms
	if (file.read((char*)analysis, 1) <= 0)
	{
		qDebug() << Q_FUNC_INFO << filename << " no audio data.";
		file.close();
		return false;
	}

	dec_mode = ((analysis[0] >> 3) & 0x000F);
	read_size = block_size[dec_mode];
	m_frameSize = 1 + read_size;
	m_audioMs = 20 * (file.size() - AMR_FILE_HEADER.length())/(read_size+1);
	file.seek(file.pos() - 1);

	// get all decode data from file
	m_decodeData = file.read(file.size() - AMR_FILE_HEADER.length());

	file.close();
	
	return true;
}

void CAmrPlayPrivate::onReadyRead()
{
	startAudioOutput();
}

void CAmrPlayPrivate::onStateChanged(QAudio::State newState)
{
    if (newState == QAudio::IdleState) 
	{
		qDebug() << Q_FUNC_INFO << " QAudio::IdleState";

		stop();
    }
}

void CAmrPlayPrivate::onNotify()
{
    if (m_pAO && m_pMonitor)
    {
		qint64 bytesInBuffer = m_pAO->bufferSize() - m_pAO->bytesFree();
		qint64 usInBuffer = (qint64)(1000000) * bytesInBuffer / (  1/*channels*/ * 16/*sampleSize*/ / 8 ) / 8000 /*frequency*/;
		qint64 usPlayed = m_pAO->processedUSecs() - usInBuffer;
		int msPlayed = (int)(usPlayed/1000);
		m_pMonitor->onPlayProgress(msPlayed, m_audioMs);
    }
}

void CAmrPlayPrivate::onDataDecoded(const QByteArray &data)
{
	if (m_pWriteBuffer)
	{
		m_pWriteBuffer->write(data);
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CAmrDecoderTask
CAmrDecoderTask::CAmrDecoderTask()
: m_frameSize(0), m_exit(true)
{
}

CAmrDecoderTask::~CAmrDecoderTask()
{
	stopDecode();
}

void CAmrDecoderTask::startDecode(int frameSize, const QByteArray &decodeData)
{
	if (isRunning())
		stopDecode();

	setFrameSize(frameSize);
	setDecodeData(decodeData);
	m_exit = false;
	start();
}

void CAmrDecoderTask::stopDecode()
{
	if (!isRunning())
		return;

	m_exit = true;
	wait();
}

void CAmrDecoderTask::run()
{
    qDebug() << Q_FUNC_INFO << "start";

    short synth[160];   // 20ms pcm data size
    int read_size = m_frameSize; // frame size of amr MR102 mode
    unsigned char analysis[32];
    qint64 len = 0;

	QBuffer file(&m_decodeData);
	file.open(QIODevice::ReadOnly);

    // decode every 20ms frame
	PmAudioDecoder *pAmrDecoder = PmAudioDecoderFactory::createAudioDecoder(g_amr_name, 8000, 1);

    while (!m_exit)
    {
		memset(analysis, 0, 32);
        len = file.read((char*)analysis, read_size);
        if (len <= 0)
        {
            break;
        }

		// call decoder
		pAmrDecoder->decode(analysis, read_size, synth, 160, 0);

        // play one frame
		QByteArray synthBuffer((char*)synth, 160*2);
		emit dataDecoded(synthBuffer);

        Thread::msleep(5);
    }

    if (pAmrDecoder)
		delete pAmrDecoder;
	pAmrDecoder = 0;

    file.close();

    qDebug() << Q_FUNC_INFO << "end";
}

//////////////////////////////////////////////////////////////////////////
//CAmrPlay
CAmrPlay::CAmrPlay(CAmrPlayMonitor *pMonitor/* = 0*/)
: d_ptr(new CAmrPlayPrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->m_pMonitor = pMonitor;
}

CAmrPlay::~CAmrPlay()
{
    delete d_ptr;
    d_ptr = 0;
}

bool CAmrPlay::Play(const QString &filePath)
{
    return d_ptr->play(filePath);
}

void CAmrPlay::Stop()
{
    d_ptr->stop();
}

#include "amrPlay.moc"
