#include <QDebug>
#include <QTimer>
#include "util/PlayBeep.h"
#include "PmApp.h"
#include "logger/logger.h"
#include "Account.h"
#include "rtcsessionmanager.h"
#include "rtcsession.h"
#include "rtcmanager.h"
#include "rtcsessionglobal.h"
#include "sharedbuffer.h"
#include "session/VideoFrameCB.h"

static const char *kCloseNormal    = "normal";
static const char *kCloseError     = "error";
static const char *kCloseBusy      = "busy";
static const char *kCloseUnsupport = "unsupport";
static const char *kCloseOther     = "other";

namespace rtcsession
{
	Session::Session(PEER peer, 
		             const QString &from, 
					 const QString &to, 
					 const QString &sid, 
					 RtcSessionManager *sessionManager, 
					 QObject *parent /*= 0*/)
		: QObject(parent)
		, m_rtcSessionManager(sessionManager)
		, m_type(ST_Audio)
		, m_peer(peer)
		, m_state(State_Init)
		, m_from(from)
		, m_to(to)
		, m_sid(sid)
		, m_errCode(Error_NoError)
		, m_otherVideoStarted(false)
		, m_otherCloseType(CloseNormal)
		, m_audioPackageLost(0)
		, m_audioPackageReceived(0)
		, m_videoPackageLost(0)
		, m_videoPackageReceived(0)
	{
		m_otherVideoFrameCB.reset(new session::VideoFrameCB());
    }

    Session::~Session()
    {
    }

	Session::CloseType Session::str2CloseType(const QString &str)
	{
		if (str == QString::fromLatin1(kCloseError))
			return Session::CloseError;
		else if (str == QString::fromLatin1(kCloseBusy))
			return Session::CloseBusy;
		else if (str == QString::fromLatin1(kCloseUnsupport))
			return Session::CloseUnsupport;
		else if (str == QString::fromLatin1(kCloseOther))
			return Session::CloseOther;
		else
			return Session::CloseNormal;
	}

	QString Session::closeType2Str(CloseType closeType)
	{
		QString str = QString::fromLatin1(kCloseNormal);
		switch (closeType)
		{
		case Session::CloseError:
			str = QString::fromLatin1(kCloseError);
			break;
		case Session::CloseBusy:
			str = QString::fromLatin1(kCloseBusy);
			break;
		case Session::CloseUnsupport:
			str = QString::fromLatin1(kCloseUnsupport);
			break;
		case Session::CloseOther:
			str = QString::fromLatin1(kCloseOther);
			break;
		default:
			str = QString::fromLatin1(kCloseNormal);
			break;
		}
		return str;
	}

    QString Session::sid() const
    {
        return m_sid;
    }

    QString Session::from() const
    {
        return m_from;
    }

    QString Session::to() const
    {
        return m_to;
    }

	void Session::setSp(const QString &sp)
	{
		m_sp = sp;
	}

	QString Session::sp() const
	{
		return m_sp;
	}

    SessionType Session::type() const
    {
        return m_type;
    }

    Session::PEER Session::peer() const
    {
        return m_peer;
    }

	Session::SessionError Session::sessionError() const
	{
		return m_errCode;
	}

	SessionMediaParam Session::selfMediaParam() const
	{
		return m_selfMedia;
	}

	SessionMediaParam Session::otherMediaParam() const
	{
		return m_otherMedia;
	}

	void Session::setAudioVolume(int vol)
	{
		m_rtcSessionManager->sendCmdVolumeMsg(this, vol);
	}

	void Session::setAudioSendEnable(bool enable)
	{
		m_selfMedia.audioSend = enable;
		m_rtcSessionManager->sendCmdAudioMsg(this, enable);
	}

	void Session::setVideoSendEnable(bool enable)
	{
		m_selfMedia.videoSend = enable;
		m_rtcSessionManager->sendCmdVideoMsg(this, enable);
	}

	void Session::onAudioSendChanged(bool send)
	{
		if (m_selfMedia.audioSend != send)
		{
			m_selfMedia.audioSend = send;

			emit audioSendChanged(send);
		}
	}

	void Session::onVideoSendChanged(bool send)
	{
		if (m_selfMedia.videoSend != send)
		{
			m_selfMedia.videoSend = send;
			
			emit videoSendChanged(send);
		}
	}

	session::VideoFrameCB * Session::otherVideoFrameCB() const
	{
		return m_otherVideoFrameCB.data();
	}

    bool Session::invite(SessionType type)
    {
		if (Peer_A != m_peer)
		{
			qWarning("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return false;
		}

		m_errCode = Error_NoError;
        m_type = type;
		m_selfMedia.audioSend = true;
		m_selfMedia.audioRecv = true;
		if (m_type == ST_AudioVideo)
		{
			m_selfMedia.videoSend = true;
			m_selfMedia.videoRecv = true;
		}

		m_rtcSessionManager->prepareInvite(this);

		return true;
    }

    bool Session::ringing()
    {
		if (Peer_B != m_peer)
		{
			qWarning("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return false;
		}

        bool bOk = m_rtcSessionManager->rtcManager()->sendRinging(this);
        if (bOk)
        {
            setState(m_peer, State_Ring);
        }

		// start play beep, PEER_B
		m_pInviteBeepTimer.reset(new QTimer());
		m_pInviteBeepTimer->setInterval(2000); // 2s
		m_pInviteBeepTimer->setSingleShot(false);

		disconnect(m_pInviteBeepTimer.data(), SIGNAL(timeout()), this, SLOT(onPlayInviteBeep()));
		connect(m_pInviteBeepTimer.data(), SIGNAL(timeout()), this, SLOT(onPlayInviteBeep()));

		m_pInviteBeepTimer->start();

		// start ringing timeout, PEER_B
		m_pRingingTimer.reset(new QTimer());
		m_pRingingTimer->setInterval(70*1000); // 70s
		m_pRingingTimer->setSingleShot(true);

		disconnect(m_pRingingTimer.data(), SIGNAL(timeout()), this, SLOT(onRingingTimeout()));
		connect(m_pRingingTimer.data(), SIGNAL(timeout()), this, SLOT(onRingingTimeout()));

		m_pRingingTimer->start();

        return bOk;
    }

    void Session::ok()
    {
		if (Peer_B != m_peer)
		{
			qWarning("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return;
		}

		if (m_pInviteBeepTimer)
		{
			m_pInviteBeepTimer->stop();
			m_pInviteBeepTimer.reset();
		}

		if (m_pRingingTimer)
		{
			m_pRingingTimer->stop();
			m_pRingingTimer.reset();
		}

		m_rtcSessionManager->prepareOk(this);
    }

    void Session::reject(CloseType closeType, const QString &desc /*= ""*/)
    {
		if (Peer_B != m_peer)
		{
			qDebug("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return;
		}

		QString closeStr = closeType2Str(closeType);
        m_rtcSessionManager->rtcManager()->sendReject(this, closeStr, desc);

        setState(m_peer, State_End);
    }

    void Session::bye(CloseType closeType, const QString &desc /*= ""*/)
    {
		// stop video
		stopOtherVideo();

		QString closeStr = closeType2Str(closeType);
        m_rtcSessionManager->rtcManager()->sendBye(this, closeStr, desc);

        setState(m_peer, State_End);
    }

    void Session::close(CloseType closeType, const QString &desc /*= ""*/)
    {
        if (Peer_B == m_peer && State_Ring == m_state)
        {
            reject(closeType, desc);
        }
        else if (m_state != State_End)
        {
            bye(closeType, desc);
        }
    }

	void Session::serviceError()
	{
		if (m_state == State_Setup)
		{
			setError(Error_DataTimeout);
			bye(rtcsession::Session::CloseError, tr("The connection is lost"));
		}
	}

	Session::CloseType Session::otherCloseType() const
	{
		return m_otherCloseType;
	}

	QString Session::otherCloseDesc() const
	{
		return m_otherCloseDesc;
	}

	void Session::setAudioStatistics(int packageLost, int packageReceived)
	{
		m_audioPackageLost = packageLost;
		m_audioPackageReceived = packageReceived;

		emit packageStatistics(m_audioPackageLost+m_videoPackageLost, m_audioPackageReceived+m_videoPackageReceived);
	}

	void Session::setVideoStatistics(int packageLost, int packageReceived)
	{
		m_videoPackageLost = packageLost;
		m_videoPackageReceived = packageReceived;

		emit packageStatistics(m_audioPackageLost+m_videoPackageLost, m_audioPackageReceived+m_videoPackageReceived);
	}

	void Session::conflict()
	{
		setError(Error_InviteConflict);
		close(rtcsession::Session::CloseError, tr("Invite at same time, and conflict")); // bye
	}

    void Session::recvInvite(bool hasVideo, bool audioSend, bool audioRecv, bool videoSend, bool videoRecv)
    {
		if (m_peer != Peer_B)
		{
			qDebug("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			setState(Peer_B, State_End);
			return;
		}

		m_errCode = Error_NoError;
        m_type = ST_AudioVideo;
		m_selfMedia.audioSend = true;
		m_selfMedia.audioRecv = true;
		m_selfMedia.videoSend = true;
		m_selfMedia.videoRecv = true;
        if (!hasVideo)
		{
            m_type = ST_Audio;
			m_selfMedia.videoSend = false;
			m_selfMedia.videoRecv = false;
		}
		m_otherMedia.audioSend = audioSend;
		m_otherMedia.audioRecv = audioRecv;
		m_otherMedia.videoSend = videoSend;
		m_otherMedia.videoRecv = videoRecv;

        setState(m_peer, State_Invite);

        // ringing
        ringing();
    }

    void Session::recvRinging()
    {
		if (Peer_A != m_peer)
		{
			qDebug("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return;
		}

        setState(m_peer, State_Ring);

        // start play beep, Peer_A ringing
        m_pInviteBeepTimer.reset(new QTimer());
        m_pInviteBeepTimer->setInterval(2000); // 2s
        m_pInviteBeepTimer->setSingleShot(false);

		disconnect(m_pInviteBeepTimer.data(), SIGNAL(timeout()), this, SLOT(onPlayInviteBeep()));
        connect(m_pInviteBeepTimer.data(), SIGNAL(timeout()), this, SLOT(onPlayInviteBeep()));

        m_pInviteBeepTimer->start();
    }

    void Session::recvOk(bool audioSend, bool audioRecv, bool videoSend, bool videoRecv)
    {
		if (Peer_A != m_peer)
		{
			qDebug("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return;
		}

		// stop invite time
        if (m_pInviteTimer)
        {
            m_pInviteTimer->stop();
			m_pInviteTimer.reset();
        }

		// stop play beep, Peer_A ringing
        if (m_pInviteBeepTimer)
        {
            m_pInviteBeepTimer->stop();
        }

		m_otherMedia.audioSend = audioSend;
		m_otherMedia.audioRecv = audioRecv;
		m_otherMedia.videoSend = videoSend;
		m_otherMedia.videoRecv = videoRecv;

		// send ack
		m_rtcSessionManager->rtcManager()->sendAck(this);

		// offer
		m_rtcSessionManager->sendOfferMsg(this);

		setState(m_peer, State_Setup);

		emit onOk();
    }

    void Session::recvReject(const QString &reasonType, const QString &reasonDesc)
    {
		if (Peer_A != m_peer)
		{
			qDebug("%s peer(%s) failed.", Q_FUNC_INFO, (m_peer == Peer_A ? "A" : "B"));
			return;
		}

		m_otherCloseType = str2CloseType(reasonType);
		m_otherCloseDesc = reasonDesc;

		emit onRejected(reasonType, reasonDesc);

        setState(Peer_B, State_End);
    }

    void Session::recvAck()
    {
		if (m_state == State_Connect && m_peer == Peer_B)
		{
			// stop ack timer
			if (m_pAckTimer)
			{
				m_pAckTimer->stop();
				m_pAckTimer.reset();
			}

			// peer B recv ack
			setState(m_peer, State_Setup);

			emit onOk();
		}
    }

    void Session::recvModify(bool audioSend, bool audioRecv, bool videoSend, bool videoRecv)
    {
		if (videoSend && !m_otherMedia.videoSend)
		{
			startOtherVideo();
		}
		else if (!videoSend && m_otherMedia.videoSend)
		{
			stopOtherVideo();
		}

		if (videoSend != m_otherMedia.videoSend)
		{
			emit otherVideoChanged(m_otherMedia.videoSend, videoSend);
		}

		if (audioSend != m_otherMedia.audioSend)
		{
			emit otherAudioChanged(m_otherMedia.audioSend, audioSend);
		}

		m_otherMedia.audioSend = audioSend;
		m_otherMedia.audioRecv = audioRecv;
		m_otherMedia.videoSend = videoSend;
		m_otherMedia.videoRecv = videoRecv;

		// ack
		m_rtcSessionManager->rtcManager()->sendAck(this);
    }

    void Session::recvBye(const QString &reasonType, const QString &reasonDesc)
    {
		m_otherCloseType = str2CloseType(reasonType);
		m_otherCloseDesc = reasonDesc;

		// stop video
		stopOtherVideo();

		setState((Peer_A == m_peer ? Peer_B : Peer_A), State_End);
    }

	void Session::recvNotify(const QString &toFullId, const QString &action)
	{
		Q_UNUSED(action);
		if (toFullId == Account::fullIdFromIdResource(to(), RESOURCE_PHONE))
		{
			setError(Error_PhoneHandle);
			setState(m_peer, State_End);
		}
	}

	void Session::inviteFinished(bool ok, SessionError err)
	{
		qDebug() << Q_FUNC_INFO << ok << (int)err;

		if (!ok)
		{
			setError(err);
			close(rtcsession::Session::CloseError);
		}
		else
		{
			if (err == Error_CameraOpened)
			{
				setError(err);
				m_selfMedia.videoSend = false;
			}

			m_rtcSessionManager->rtcManager()->sendInvite(this);

			setState(Peer_A, State_Invite);

			m_pInviteTimer.reset(new QTimer());
			connect(m_pInviteTimer.data(), SIGNAL(timeout()), this, SLOT(onInviteTimeout()));
			m_pInviteTimer->setSingleShot(true);
			m_pInviteTimer->start(60000); // 60s
		}
	}

	void Session::okFinished(bool ok, SessionError err)
	{
		qDebug() << Q_FUNC_INFO << ok << (int)err;

		if (!ok)
		{
			setError(err);
			close(rtcsession::Session::CloseError);
		}
		else
		{
			if (err == Error_CameraOpened)
			{
				setError(err);
				m_selfMedia.videoSend = false;
			}

			m_rtcSessionManager->rtcManager()->sendOk(this);

			setState(m_peer, State_Connect);

			// start timer of ack time
			m_pAckTimer.reset(new QTimer());
			m_pAckTimer->setInterval(20000); // 20s
			m_pAckTimer->setSingleShot(true);

			disconnect(m_pAckTimer.data(), SIGNAL(timeout()), this, SLOT(onAckTimeout()));
			connect(m_pAckTimer.data(), SIGNAL(timeout()), this, SLOT(onAckTimeout()));

			m_pAckTimer->start();
		}
	}

	void Session::startRecvOtherVideo()
	{
		startOtherVideo();
	}

    void Session::setState(PEER ctrlPeer, SessionState s)
    {
		SessionState oldS = m_state;
        m_state = s;
		emit stateChanged(ctrlPeer, m_state, oldS);
		if (State_End == s)
		{
			QTimer::singleShot(0, this, SLOT(doAboutClose()));
		}
    }

    void Session::onInviteTimeout()
    {
        // invite time out, set error
        qDebug() << Q_FUNC_INFO;
		setError(Error_InviteTimeout);
		bye(rtcsession::Session::CloseError, tr("Invite timeout"));
    }

	void Session::onRingingTimeout()
	{
		// ringing time out, set error
		qDebug() << Q_FUNC_INFO;
		setError(Error_RingingTimeout);
		reject(rtcsession::Session::CloseError, tr("Ringing timeout"));
	}

    void Session::onPlayInviteBeep()
    {
        PlayBeep::playAudioBeep();
    }

	void Session::onAckTimeout()
	{
		// ack time out, set error
		qDebug() << Q_FUNC_INFO;
		setError(Error_AckTimeout);
		bye(rtcsession::Session::CloseError, tr("Connect timeout"));
	}

	void Session::doAboutClose()
	{
		if (m_pInviteTimer)
		{
			m_pInviteTimer->stop();
			m_pInviteTimer.reset();
		}

		if (m_pRingingTimer)
		{
			m_pRingingTimer->stop();
			m_pRingingTimer.reset();
		}

		if (m_pInviteBeepTimer)
		{
			m_pInviteBeepTimer->stop();
			m_pInviteBeepTimer.reset();
		}

		if (m_pAckTimer)
		{
			m_pAckTimer->stop();
			m_pAckTimer.reset();
		}

		emit aboutClose(m_sid);
	}

	void Session::onOtherVideoFrame()
	{
		if (m_otherBuffer.isNull())
			return;

		if (m_otherVideoFrameCB.isNull())
			return;

		QByteArray buffer = m_otherBuffer->readAll();
		if (!buffer.isEmpty())
		{
			QImage frame = QImage::fromData(buffer, "bmp");
			if (!frame.isNull())
			{
				QSize frameSz = frame.size();
				m_otherVideoFrameCB->frameChanged(frame);
			}
		}
	}

	void Session::setError(SessionError err)
	{
		m_errCode = err;
		emit error(m_errCode);
	}

	void Session::startOtherVideo()
	{
		if (m_otherVideoStarted)
			return;

		m_otherBuffer.reset(new SharedBuffer());
		wchar_t bufferName[128] = {0};
		m_sid.toWCharArray(bufferName);
		if (!m_otherBuffer->openBuffer(bufferName, kVideoRenderSize))
		{
			qWarning() << Q_FUNC_INFO << "open other video buffer failed: " << m_from << m_to;
			return;
		}

		m_otherVideoTimer.stop();
		m_otherVideoTimer.setInterval(100);
		m_otherVideoTimer.setSingleShot(false);
		connect(&m_otherVideoTimer, SIGNAL(timeout()), this, SLOT(onOtherVideoFrame()), Qt::UniqueConnection);

		m_otherVideoTimer.start();
		m_otherVideoStarted = true;
	}

	void Session::stopOtherVideo()
	{
		if (!m_otherVideoStarted)
			return;

		m_otherVideoTimer.stop();
		m_otherBuffer.reset(0);
		m_otherVideoStarted = false;

		// make black
		if (!m_otherVideoFrameCB.isNull())
		{
			QImage image(m_otherVideoFrameCB->imageSize(), QImage::Format_RGB32);
			image.fill(Qt::black);
			m_otherVideoFrameCB->frameChanged(image);
		}
	}
}
