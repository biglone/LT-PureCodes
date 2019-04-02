#include "interphoneaudiorecvtask.h"
#include "AudioRecv.h"
#include <QIODevice>
#include "pmaudiocodec.h"
#include <QAudioOutput>
#include "interphonepacket.h"
#include "PmApp.h"
#include "logger.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "settings/GlobalSettings.h"
#include <QUdpSocket>
#include "recvbegingenerator.h"
#include "common/datetime.h"

namespace interphone
{

	InterphoneAudioRecvTask::InterphoneAudioRecvTask(AudioRecv *ar, QObject *parent /*= 0*/)
		: AudioRecvTask(ar, parent), m_audioDecoder(0), m_pAudioOutput(0), m_pIo(0)
	{

	}

	InterphoneAudioRecvTask::~InterphoneAudioRecvTask()
	{

	}

	void InterphoneAudioRecvTask::doRecvAndPlay()
	{
		bool main_thread = QThread::currentThread() == qApp->thread();
		bean::AudioDesc aDesc = m_pAR->desc();
		QString aid = aDesc.id();
		QString aType = aDesc.type().toLower();
		int bufferedMs = m_pAR->bufferedFrameInMs();
		bool needCache = true;
		// int delay = 0;
		int packetCount = 0;
		QUdpSocket udpSocket;
		m_pcms.clear();
		const int kHeartBeatMs = 20*1000;     // how many ms when there is no data to send a heart beat
		qint64 lastHeartBeatTime = CDateTime::currentMSecsSinceEpoch();
		const int kBufferedResetMs = 500;    // how many ms when there is no data to re-buffer audio data
		int emptyMs = 0;

		const int readTimeout = 10/*aDesc.frame()*/; // in milliseconds

		qDebug("Interphone AudioRecv[%s] task running in thread %p [main thread=%d]: buffered time %dms", 
			qPrintable(aid), QThread::currentThreadId(), main_thread, bufferedMs);

		qPmApp->getLogger()->debug(QString("Interphone AudioRecv[%1] task start running: buffered time %2ms")
			.arg(aid).arg(bufferedMs));

		//////////////////////////////////////////////////////////////////////////
		// create test file
		QFile *pAudioFile = 0;
		if (m_pAR->logToFile())
		{
			QDateTime dtNow = QDateTime::currentDateTime();
			QString audioFileNameTail = dtNow.toString("yyyy-MM-dd_hh_mm_ss_zzz");
			pAudioFile = new QFile(QString("%1\\audio_%2.wav").arg(GlobalSettings::audioLogFileDirPath()).arg(audioFileNameTail));
			pAudioFile->open(QIODevice::WriteOnly);
		}
		//////////////////////////////////////////////////////////////////////////

		// start audio output
		startAudioOutput();

		// create decoder
		m_audioDecoder = PmAudioDecoderFactory::createAudioDecoder(aType.toLatin1(), aDesc.rate(), aDesc.chan());

		// connect
		bean::HostInfo addr = m_pAR->addr();
		Q_ASSERT(addr.isValid());
		udpSocket.connectToHost(addr.ip(), addr.port());

		// send recv begin packet
		QByteArray recvBeginPacket = getRecvBeginPacket();
		bool bRecvAck = false;
		udpSocket.write(recvBeginPacket);

		while (!m_bExit)
		{
			if (!udpSocket.waitForReadyRead(readTimeout))
			{
				if (!bRecvAck)
				{
					udpSocket.write(recvBeginPacket);
					continue;
				}
				else
				{
					emptyMs += readTimeout;
				}
			}

			// reading packet
			while (udpSocket.hasPendingDatagrams()) 
			{
				// re-calculate empty time
				emptyMs = 0;

				if (m_bExit)
				{
					break;
				}

				// read packet
				QByteArray ba;
				ba.resize(udpSocket.pendingDatagramSize());
				QHostAddress sender;
				quint16 senderPort;

				udpSocket.readDatagram(ba.data(), ba.size(), &sender, &senderPort);

				if (!bRecvAck)
				{
					bRecvAck = true;
					m_pAR->setAudioRecvBegin();
				}

				if (ba == recvBeginPacket)
				{
					qPmApp->getLogger()->debug(QString("Interphone AudioRecv[%1] task recved recvbegin").arg(aid));
					continue;
				}

				// parse packet
				InterphonePacket packet;
				int packageTimeInMs = 0;
				if (packet.parse(ba, aDesc.frame(), packageTimeInMs))
				{
					bRecvAck = true;

					// decode packet to pcm format
					decodePacket(packet);

					++packetCount;

					//////////////////////////////////////////////////////////////////////////
					if (pAudioFile && pAudioFile->isOpen())
					{
						for (int i = 0; i < packet.frameCount(); i++)
						{
							int index = m_pcms.length()-packet.frameCount()+i;
							QByteArray pcm = m_pcms[index];
							pAudioFile->write(pcm);
						}
					}
					//////////////////////////////////////////////////////////////////////////
				}
				else
				{
					qDebug("Interphone AudioRecv[%s] packet parse error", qPrintable(aid));
					continue;
				}

				//////////////////////////////////////////////////////////////////////////
				// log audio status
				if ((packetCount%1000) == 1)
				{
					qPmApp->getLogger()->debug(QString("Interphone audio output: buffer size: %1, bytes free: %2, period size: %3, packet size: %4")
						.arg(m_pAudioOutput->bufferSize()).arg(m_pAudioOutput->bytesFree())
						.arg(m_pAudioOutput->periodSize()).arg(ba.length()));
					qDebug("Interphone audio output: buffer size: %d, bytes free: %d, period size: %d, packet size: %d", 
						m_pAudioOutput->bufferSize(), m_pAudioOutput->bytesFree(),
						m_pAudioOutput->periodSize(), ba.length());
				}
				//////////////////////////////////////////////////////////////////////////
			}

			if (m_bExit)
			{
				break;
			}

			// check if need to re-buffer data
			if (emptyMs >= kBufferedResetMs && m_pcms.isEmpty() && !needCache)
			{
				qPmApp->getLogger()->debug(QString("Interphone AudioRecv[%1] task reset buffer time in %2MS").arg(aid).arg(emptyMs));
				needCache = true;
				emptyMs = 0;
			}

			// send heart-beat
			qint64 currentMs = CDateTime::currentMSecsSinceEpoch();
			int hbInterval = (int)(currentMs - lastHeartBeatTime);
			if (hbInterval >= kHeartBeatMs)
			{
				qPmApp->getLogger()->debug(QString("Interphone AudioRecv[%1] task send recvbegin in %2MS").arg(aid).arg(hbInterval));

				udpSocket.write(recvBeginPacket);
				
				lastHeartBeatTime = currentMs;
			}

			// play already received data
			if (needCache) // if not played before
			{
				if (recvedAudioInMs() >= bufferedMs) // if buffered frames are enough to play
				{
					// play as much as pcm
					play();

					needCache = false;
				}
			}
			else
			{
				int bytesFree = m_pAudioOutput->bytesFree();
				int bufferSize = m_pAudioOutput->bufferSize();
				bool isFull = (bytesFree == 0);
				bool isEmpty = (bytesFree == bufferSize);
				if (!isFull)
				{
					/*
					if (isEmpty && m_pcms.isEmpty())
					{
						// play an empty data
						int emptySize = playEmpty();

						// calculate the delay
						delay += (emptySize/2/(aDesc.rate()/1000));
						qPmApp->getLogger()->debug(QString("Interphone audio output: ----------------------- play empty audio, delay %1ms")
							.arg(delay));
					}
					else
					*/
					{
						if (isEmpty)
						{
							qPmApp->getLogger()->debug(QString("Interphone audio output: -------------- buffer is empty."));
						}

						// play as much as pcm
						play();
					}
				}
			}
		}
		
		m_pcms.clear();

		// destroy decoder
		if (m_audioDecoder)
		{
			delete m_audioDecoder;
			m_audioDecoder = 0;
		}

		// close udp
		udpSocket.close();

		// stop audio output
		stopAudioOutput();

		//////////////////////////////////////////////////////////////////////////
		if (pAudioFile)
		{
			pAudioFile->close();
			delete pAudioFile;
			pAudioFile = 0;
		}
		//////////////////////////////////////////////////////////////////////////

		qDebug("Interphone AudioRecv[%s] task stop in thread %p", qPrintable(aid), QThread::currentThreadId());

		qPmApp->getLogger()->debug(QString("Interphone AudioRecv[%1] task finished running").arg(aid));
	}

	void InterphoneAudioRecvTask::onAudioOutputStateChanged(QAudio::State state)
	{
		qPmApp->getLogger()->debug(QString("InterphoneAudioRecvTask -- audio output state changed: %1").arg(state));
	}

	void InterphoneAudioRecvTask::onAudioOutputNotify()
	{
		/*
		qPmApp->getLogger()->debug(QString("InterphoneAudioRecvTask -- audio output notify"));
		*/
	}

	int InterphoneAudioRecvTask::doFecPacket(const AudioPacket &packet, int nextRecvSeq)
	{
		if (packet.seq() <= nextRecvSeq)
			return 0;

		if (packet.AFList().isEmpty())
			return 0;

		int lossPacketCount = packet.seq() - nextRecvSeq;
		int fecCount = lossPacketCount * packet.frameCount();
		int lpcCount = 0;
		int maxFecNum = 3;
		if (maxFecNum > m_audioDecoder->maxFecCount())
			maxFecNum = m_audioDecoder->maxFecCount();
		if (fecCount > maxFecNum)
		{
			lpcCount = fecCount - maxFecNum;
			fecCount = maxFecNum;
		}

		bean::AudioDesc aDesc = m_pAR->desc();
		int sampleSize = aDesc.rate() / 1000 * aDesc.frame();
		int samples = 0;
		if (lpcCount > 0)
		{
			/*
			qPmApp->getLogger()->debug(QString("AudioRecv -------------- have to do LPC %1")
				.arg(lpcCount));

			// do lpc (loss packet concealment), throw it
			samples = sampleSize * lpcCount;
			decode(QByteArray(), samples, 1);
			*/
		}

		// do fec (forward error correction)
		// qPmApp->getLogger()->debug(QString("AudioRecv -------------- have to do FEC %1").arg(fecCount));

		samples = sampleSize * fecCount;
		QByteArray pcm = decode(packet.AFList()[0], samples, 1);
		if (pcm.isEmpty())
			return 0;

		// make pcm data
		for (int i = 0; i < fecCount; i++)
		{
			m_pcms.append(pcm.mid(i*sampleSize*2, sampleSize*2));
		}

		return fecCount;
	}

	QByteArray InterphoneAudioRecvTask::getRecvBeginPacket()
	{
		QByteArray ba = m_pAR->recvBeginGenerator().generateRecvBegin();
		return ba;
	}

	int InterphoneAudioRecvTask::play(const QByteArray &pcm)
	{
		// append pcm to io
		int playedBytes = 0;
		if (m_pIo && !pcm.isEmpty())
		{
			m_pIo->write(pcm);
			playedBytes = pcm.length();
		}

		m_pAR->setAudioPlayed();

		return playedBytes;
	}

	int InterphoneAudioRecvTask::play()
	{
		int bytesFree = m_pAudioOutput->bytesFree();
		if (bytesFree <= 0)
			return 0;

		bean::AudioDesc aDesc = m_pAR->desc();
		int playCount = m_pAudioOutput->periodSize()/(aDesc.rate()/1000*aDesc.frame()*2);
		if (playCount < 1)
			playCount = 1;
		if (m_pcms.count() < playCount)
			return 0;

		int allPlayedBytes = 0;
		int playedBytes = 0;
		int index = 0;
		QByteArray pcm;
		while ((bytesFree > 0) && (m_pcms.count() >= playCount))
		{
			// play one sequence
			for (index = 0; index < playCount; index++)
			{
				pcm.append(m_pcms.takeFirst());
			}
			playedBytes = play(pcm);
			allPlayedBytes += playedBytes;
			bytesFree -= playedBytes;
			pcm.clear();
		}

		return allPlayedBytes;
	}

	int InterphoneAudioRecvTask::playEmpty()
	{
		if (!m_pIo)
			return 0;

		// append an empty data
		int emptyPcmSize = m_pAudioOutput->periodSize(); // 1 frame empty pcm data
		QByteArray emptyPcm;
		emptyPcm.fill(0, emptyPcmSize);
		m_pIo->write(emptyPcm);
		return emptyPcmSize;
	}

	void InterphoneAudioRecvTask::decodePacket(const AudioPacket &ap)
	{
		if (!ap.isValid())
			return;

		// decode all frames
		bean::AudioDesc aDesc = m_pAR->desc();
		int sampleSize = aDesc.rate() / 1000 * aDesc.frame();
		foreach (QByteArray ba, ap.AFList())
		{
			QByteArray pcm = decode(ba, sampleSize, 0);
			m_pcms.append(pcm);
		}
	}

	QByteArray InterphoneAudioRecvTask::decode(const QByteArray &af, int sampleSize, int fec /*= 0*/)
	{
		QByteArray ret;
		ret.resize(sampleSize*2);
		int decodeSample = m_audioDecoder->decode((unsigned char *)af.data(), af.length(), (short *)ret.data(), sampleSize, fec);
		if (decodeSample > 0)
		{
			ret = ret.left(decodeSample*2);
		}
		else
		{
			ret.clear();

			qDebug() << Q_FUNC_INFO << "InterphoneAudioRecvTask -- decode failed!!!";
			qPmApp->getLogger()->debug(QString("InterphoneAudioRecvTask -- decode failed!!!"));
		}

		return ret;
	}

	void InterphoneAudioRecvTask::startAudioOutput()
	{
		bean::AudioDesc aDesc = m_pAR->desc();

		QAudioFormat format;
		// Set up the format
		format.setSampleRate(aDesc.rate());
		format.setChannelCount(aDesc.chan());
		format.setSampleSize(aDesc.bit());
		format.setCodec("audio/pcm");
		format.setByteOrder(QAudioFormat::LittleEndian);
		format.setSampleType(QAudioFormat::UnSignedInt);

		m_pAudioOutput = new QAudioOutput(format);
		bool connected = connect(m_pAudioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onAudioOutputStateChanged(QAudio::State)));
		Q_ASSERT(connected);
		Q_UNUSED(connected);

		/*
		connected = connect(m_pAudioOutput, SIGNAL(notify()), this, SLOT(onAudioOutputNotify()));
		Q_ASSERT(connected);
		*/

		/*
		m_pAudioOutput->setBufferSize(6400); // max 400ms pcm data
		*/

		m_pIo = m_pAudioOutput->start();

		/*
		qPmApp->getLogger()->debug(QString("Interphone audio output: buffer size: %1, bytes free: %2, period size: %3").arg(m_pAudioOutput->bufferSize()).arg(m_pAudioOutput->bytesFree()).arg(m_pAudioOutput->periodSize()));
		*/
	}

	void InterphoneAudioRecvTask::stopAudioOutput()
	{
		m_pAudioOutput->stop();

		delete m_pAudioOutput;
		m_pAudioOutput = 0;
	}

	int InterphoneAudioRecvTask::recvedAudioInMs() const
	{
		if (m_pcms.isEmpty())
			return 0;

		bean::AudioDesc aDesc = m_pAR->desc();
		return m_pcms.count()*aDesc.frame();
	}

}