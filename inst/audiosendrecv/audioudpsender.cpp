#include "audioudpsender.h"
#include "pmaudiocodec.h"
#include "AudioES.h"
#include "settings/GlobalSettings.h"
#include "PmApp.h"
#include "logger/logger.h"

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AudioUdpSender
AudioUdpSender::AudioUdpSender( AudioES *pAes, int periodSize )
    : m_pAes(pAes)
    , m_periodSize(periodSize)
	, m_frameCount(0)
	, m_lossIndex(0)
{
	m_audioEncoder = PmAudioEncoderFactory::createAudioEncoder(pAes->m_toADesc.type().toLatin1(), 
		m_pAes->m_toADesc.rate(), m_pAes->m_toADesc.chan());
	if (pAes->m_toADesc.type() == "amr")
	{
		m_periodSize = 320; // 20ms PCM data
	}
	m_lossCount = GlobalSettings::audioPackageLossCount()/10;
}

AudioUdpSender::~AudioUdpSender()
{
	if (m_audioEncoder)
		delete m_audioEncoder;
	m_audioEncoder = 0;
}

void AudioUdpSender::onReadyRead()
{
    QIODevice *io = qobject_cast<QIODevice*>(sender());
    if (!io)
    {
        return;
    }

    processAudioFrame(io->readAll());
}

void AudioUdpSender::processAudioFrame( const QByteArray &ba )
{
	/* 
	// check thread
	bool main_thread = QThread::currentThread() == qApp->thread();
	qDebug("AudioES send audio frame running in thread %p [main thread=%d]", QThread::currentThreadId(), main_thread);
	*/

    QList<bean::HostInfo> addrs;
    {
        QMutexLocker lock(&m_pAes->m_lock);
        addrs = m_pAes->m_hashAddr2Sids.uniqueKeys();
    }

    if (addrs.isEmpty())
    {
        return;
    }

    for (int i = 0; i < ba.length()/m_periodSize; ++i)
    {
		// encode frame to specific encode type
		++m_frameCount;
		QByteArray rawFrame = ba.mid(i*m_periodSize, m_periodSize);

		/*
		quint64 startPoint = QDateTime::currentMSecsSinceEpoch();
		*/

		QByteArray encodedFrame = encodeFrame(rawFrame);

		/* // encode using about 1ms
		quint64 stopPoint = QDateTime::currentMSecsSinceEpoch();
		qPmApp->getLogger()->debug(QString("encode packet using: %1 ms").arg(stopPoint-startPoint));
		*/

		m_frames.append(encodedFrame);

		// check if need to send these frames
		if (m_frameCount >= m_pAes->m_packageFrameCount)
		{
			// make package and send
			QByteArray data = makePacket(m_pAes->m_seq, m_pAes->m_toADesc, m_frames);

			//////////////////////////////////////////////////////////////////////////
			if (m_pAes->m_seq == m_pAes->m_toADesc.beginseq())
			{
				qPmApp->getLogger()->debug(QString("AudioSend -------------- first packet number: %1, should be %2")
					.arg(m_pAes->m_seq).arg(m_pAes->m_toADesc.beginseq()));
			}
			//////////////////////////////////////////////////////////////////////////
			
			if ((m_pAes->m_seq%1000) == 1)
			{
				qDebug("AudioUdpSender: -- need process audio frame count: %d", ba.length()/m_periodSize);
				qPmApp->getLogger()->debug(QString("AudioUdpSender: -- need process audio frame count: %1/%2=%3").arg(ba.length()).arg(m_periodSize).arg(ba.length()/m_periodSize));

				qDebug("AudioUdpSender - audio package [%d] size: %d", m_pAes->m_seq, data.length());
				qPmApp->getLogger()->debug(QString("AudioUdpSender: - audio package[%1]: frame count: %2, size: %3")
					.arg(m_pAes->m_seq).arg(m_frameCount).arg(data.length()));
			}
			
			++(m_pAes->m_seq);
			
			if (m_pAes->m_seq > 0 && ((m_pAes->m_seq%10) != 0) && ((m_pAes->m_seq%5) == 0))
			{
				m_lossIndex = 0;
			}
			
			if (m_lossIndex < m_lossCount)
			{
				// do packet loss by not sending package
				++m_lossIndex;
			}
			else
			{
				// send the package
				foreach (bean::HostInfo addr, addrs)
				{
					socket.writeDatagram(data, addr.ip(), addr.port());
				}
			}

			// clear send packages
			m_frames.clear();
			m_frameCount = 0;
		}
    }
}

/*
QByteArray AudioUdpSender::makePacket( const QList<QByteArray> &bas )
{
    QByteArray data;
    do 
    {
		AudioPacket *audioPacket = m_pAes->m_audioPacketDelegate.getAudioPacket();
		audioPacket->setAudioId(m_pAes->m_toADesc.id());
		audioPacket->setSeq(m_pAes->m_seq);
		audioPacket->setBeginFrame(m_pAes->m_seq - m_pAes->m_toADesc.beginseq());
		audioPacket->setFrameCount(bas.count());
		audioPacket->setAFList(bas);

		data = audioPacket->make();

		delete audioPacket;
		audioPacket = 0;

    } while (0);

    return data;
}
*/

QByteArray AudioUdpSender::encodeFrame(const QByteArray &ba)
{
	QString type = m_pAes->m_toADesc.type();
	QByteArray frame;
	frame.resize(m_periodSize/2);
	short *pOrid = (short *)(ba.data());
	int sampleSize = m_periodSize/2;
	int encodeSize = m_audioEncoder->encode(pOrid, sampleSize, (unsigned char*)frame.data(), frame.length());
	if (encodeSize > 0)
	{
		frame = frame.left(encodeSize);
	}
	else
	{
		frame.clear();

		qDebug("AudioUdpSender -- encode failed!!! sample size: %d encode size: %d", sampleSize, encodeSize);
		qPmApp->getLogger()->debug(QString("AudioUdpSender -- encode failed!!! sample size: %1 encode size: %2")
			.arg(sampleSize).arg(encodeSize));
	}

	return frame;
}

