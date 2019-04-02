#include "interphonesession.h"
#include <QDataStream>
#include "interphonepacket.h"
#include "interphoneaudioes.h"
#include "interphoneaudiorecv.h"
#include <QDebug>
#include "settings/SettingConstants.h"
#include "common/crc.h"
#include "pmaudiocodec.h"
#include "Account.h"

namespace interphone
{

	InterphoneSession::InterphoneSession(const QString &id, const QString &uid, const QString &addr, QObject *parent /*= 0*/)
		: QObject(parent), m_id(id), m_uid(uid), m_addr(addr), m_startOK(false)
	{
		m_audioES.reset(new InterphoneAudioES(m_uid));
		m_audioRecv.reset(new InterphoneAudioRecv(*this));

		bool connectOK = false;
		connectOK = connect(m_audioRecv.data(), SIGNAL(audioRecvBegin()), this, SLOT(onAudioRecvBegin()), Qt::QueuedConnection);
		Q_ASSERT(connectOK);

		m_recvBeginTimer.setSingleShot(true);
		m_recvBeginTimer.setInterval(10*1000); // 10s
		connectOK = connect(&m_recvBeginTimer, SIGNAL(timeout()), this, SLOT(onRecvBeginTimeout()));
		Q_ASSERT(connectOK);
	}

	InterphoneSession::~InterphoneSession()
	{
		stop();
	}

	InterphoneSession::StartType InterphoneSession::start()
	{
		StartType ret = StartOK;

		do {

			bean::AudioDesc desc;
			desc.setId(m_id);
			desc.setType(g_opus_name);
			desc.setChan(CONFIG_AUDIODESC_CHAN);
			desc.setBit(CONFIG_AUDIODESC_BIT);
			desc.setRate(CONFIG_AUDIODESC_RATE);
			desc.setFrame(CONFIG_AUDIODESC_FRAME);
			desc.setBeginseq(0);
			desc.setSend();
			if (!m_audioES->init(m_id, desc))
			{
				qWarning() << Q_FUNC_INFO << "init audioes failed.";
				ret = InputError;
				break;
			}

			m_audioRecv->setAddr(m_addr);
			m_audioRecv->setDesc(desc);
			if (m_audioRecv->start() != 0)
			{
				qWarning() << Q_FUNC_INFO << "init audiorecv failed.";
				ret = OutputError;
				break;
			}

		} while (0);

		if (ret != StartOK)
		{
			m_audioRecv->stop();
			m_startOK = false;
		}
		else
		{
			m_startOK = true;
		}

		if (m_startOK)
		{
			m_recvBeginTimer.start();
		}

		return ret;
	}

	void InterphoneSession::stop()
	{
		m_audioES->remove(m_id);
		m_audioRecv->stop();
	}

	bool InterphoneSession::startTalk()
	{
		if (!m_startOK)
			return false;

		m_audioES->append(m_id, m_addr);
		return true;
	}

	bool InterphoneSession::stopTalk()
	{
		if (!m_startOK)
			return false;

		m_audioES->remove(m_id);
		return true;
	}

	QByteArray InterphoneSession::generateRecvBegin()
	{
		/*
		crc32   |type   |id     |uid
		4       |4      |4+len  |4+len
		*/
		const int RECV_BEGIN_TYPE = 2;

		QByteArray ba;
		QDataStream s(&ba, QIODevice::WriteOnly);
		s << RECV_BEGIN_TYPE;
		s << id().toLatin1();
		QString fullUid = Account::fullIdFromIdResource(uid(), RESOURCE_COMPUTER);
		s << fullUid.toLatin1();
		
		QByteArray crc;
		QDataStream crcs(&crc, QIODevice::WriteOnly);
		int crcVal = CRC_32((unsigned char*)ba.data(), ba.length());
		crcs << crcVal;

		ba.insert(0, crc);

		return ba;
	}

	void InterphoneSession::onAudioRecvBegin()
	{
		m_recvBeginTimer.stop();
	}

	void InterphoneSession::onRecvBeginTimeout()
	{
		m_recvBeginTimer.stop();

		emit recvChannelFailed();
	}

}