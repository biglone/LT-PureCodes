#include <QThread>
#include "pmaudiocodec.h"
#include <QFile>
#include "common/datetime.h"
#include <QAudioInput>
#include <QApplication>
#include <QDebug>
#include "wave/amrrecord.h"

//////////////////////////////////////////////////////////////////////////
// class AmrRecordThread
class AmrRecordThread : public QThread
{
	Q_OBJECT

public:
	explicit AmrRecordThread(QObject *parent = 0);
	virtual ~AmrRecordThread();

	bool startRun();
	bool stopRun();

	void setAmrFile(QFile *file);

signals:
	void startError();
	void timeElapsed(int timeInMs);

protected:
	void run();

private slots:
	void onStateChanged(QAudio::State state);
	void onReadyRead();

private:
	QAudioInput    *m_audioInput;
	int             m_timeInMs;
	PmAudioEncoder *m_pAmrEncoder;
	QFile          *m_pAmrFile; 
};

AmrRecordThread::AmrRecordThread(QObject *parent /*= 0*/)
        : QThread(parent)
        , m_audioInput(0)
		, m_timeInMs(0)
		, m_pAmrFile(0)
{
	m_pAmrEncoder = PmAudioEncoderFactory::createAudioEncoder(g_amr_name, 8000, 1);
}

AmrRecordThread::~AmrRecordThread()
{
	stopRun();

	if (m_pAmrEncoder)
		delete m_pAmrEncoder;
	m_pAmrEncoder = 0;
}

bool AmrRecordThread::startRun()
{
	if (isRunning())
		return true;
	
	start();

	return true;
}

bool AmrRecordThread::stopRun()
{
	if (isRunning())
	{
		quit();
		wait();
	}

	return true;
}

void AmrRecordThread::setAmrFile(QFile *file)
{
	m_pAmrFile = file;
}

void AmrRecordThread::run()
{
    bool main_thread = QThread::currentThread() == qApp->thread();
	qDebug("AmrRecord task running in thread %p [main thread=%d]", QThread::currentThreadId(), main_thread);

	do 
	{
		QAudioFormat format;
		format.setSampleRate(8000);
		format.setChannelCount(1);
		format.setSampleSize(16);
		format.setCodec("audio/pcm");
		format.setByteOrder(QAudioFormat::LittleEndian);
		format.setSampleType(QAudioFormat::UnSignedInt);

		QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
		if (!info.isFormatSupported(format)) 
		{
			qWarning() << Q_FUNC_INFO << "default format not supported try to use nearest";
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
			emit startError();
			break;
		}

		m_audioInput = new QAudioInput(QAudioDeviceInfo::defaultInputDevice(), format);
		connect(m_audioInput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChanged(QAudio::State)));

		m_timeInMs = 0;
		QIODevice *io = m_audioInput->start();
		connect(io, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

		QThread::exec();

		m_audioInput->stop();
		delete m_audioInput;
		m_audioInput = 0;

	} while (0);

    qDebug("AmrRecord task stop in thread %p", QThread::currentThreadId());
}

void AmrRecordThread::onStateChanged(QAudio::State state)
{
	switch (state) 
	{
		 case QAudio::StoppedState:
			 if (m_audioInput)
			 {
				 if (m_audioInput->error() != QAudio::NoError) {
					 // start error
					 emit startError();
				 }
			 }
			 break;
		 default:
			 break;
	}
}

void AmrRecordThread::onReadyRead()
{
	QIODevice *io = qobject_cast<QIODevice*>(sender());
	if (!io)
		return;

	int periodSize = 320;
	QByteArray allData = io->readAll();
	unsigned char *pAmr = new unsigned char[periodSize];
	for (int i = 0; i < allData.length()/periodSize; i++)
	{
		QByteArray frame = allData.mid(i*periodSize, periodSize);

		memset(pAmr, 0, periodSize);

		int nAmr = m_pAmrEncoder->encode((short *)frame.constData(), periodSize/2, pAmr, periodSize);
		if (nAmr <= 0)
		{
			qDebug() << Q_FUNC_INFO << "amr encode failed!!! encode size: " << nAmr;
		}
		else
		{
			// write to amr file
			m_pAmrFile->write((const char *)pAmr, nAmr);

			// notify time elapsed
			m_timeInMs += 20; // encode an other 20ms
			emit timeElapsed(m_timeInMs);
		}
	}

	delete[] pAmr;
	pAmr = 0;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AmrRecord
int AmrRecord::s_idGenerator = 0;

AmrRecord::AmrRecord(QObject *parent)
	: QObject(parent), m_maxRecordTime(60*1000), m_currentRecordId(0)
{
	m_recordThread.reset(new AmrRecordThread());
	
	bool connectOK = false;
	connectOK = connect(m_recordThread.data(), SIGNAL(startError()), this, SLOT(onStartError()), Qt::QueuedConnection);
	Q_ASSERT(connectOK);

	connectOK = connect(m_recordThread.data(), SIGNAL(timeElapsed(int)), this, SLOT(onTimeElapsed(int)), Qt::QueuedConnection);
	Q_ASSERT(connectOK);
}

AmrRecord::~AmrRecord()
{

}

void AmrRecord::setMaxRecordTime(int ms)
{
	m_maxRecordTime = ms;
}

int AmrRecord::maxRecordTime() const
{
	return m_maxRecordTime;
}

bool AmrRecord::isRecording() const
{
	return m_recordThread.data()->isRunning();
}

int AmrRecord::currentRecordId() const
{
	return m_currentRecordId;
}

bool AmrRecord::start(const QString &fileDir, int &recordId, QString &desc)
{
	if (isRecording())
	{
		desc = tr("recording");
		return false;
	}

	QDateTime dateTime = CDateTime::currentDateTime();
	QString fileName = dateTime.toString("yyyyMMddhhmmsszzz");
	fileName += ".amr";
	QString filePath = fileDir + "/" + fileName;
	m_amrFile = new QFile(filePath);
	if (!m_amrFile->open(QIODevice::WriteOnly))
	{
		delete m_amrFile;
		m_amrFile = 0;

		desc = tr("Open record file failed");
		return false;
	}

	// write file header
	QByteArray amrFileHeader("#!AMR\n");
	m_amrFile->write(amrFileHeader);

	m_recordThread.data()->setAmrFile(m_amrFile);
	m_recordThread.data()->startRun();

	recordId = ++s_idGenerator;
	m_currentRecordId = recordId;

	return true;
}

void AmrRecord::stop()
{
	m_recordThread.data()->stopRun();

	if (m_amrFile)
	{
		m_amrFile->close();

		emit finished(m_currentRecordId, m_amrFile->fileName());
	}
}

void AmrRecord::cancel()
{
	m_recordThread.data()->stopRun();

	if (m_amrFile)
	{
		m_amrFile->close();

		QFile::remove(m_amrFile->fileName());

		emit canceled(m_currentRecordId);
	}
}

void AmrRecord::onStartError()
{
	m_recordThread.data()->stopRun();

	if (m_amrFile)
	{
		m_amrFile->close();

		QFile::remove(m_amrFile->fileName());

		emit startError(m_currentRecordId, tr("No record device found"));
	}
}

void AmrRecord::onTimeElapsed(int timeInMs)
{
	emit timeElapsed(m_currentRecordId, timeInMs);

	if (timeInMs >= m_maxRecordTime)
	{
		if (isRecording()) // 需要判断一下，有可能 onTimeElapsed 会回调多次
		{
			stop();
		}
	}
}

#include "amrrecord.moc"
