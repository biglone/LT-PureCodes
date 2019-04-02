#include <QDebug>
#include "protocol/ProtocolType.h"
#include "pmclient/PmClient.h"

#include "protocol/SessionAckNotification.h"
#include "protocol/SessionByeNotification.h"
#include "protocol/SessionInviteNotification.h"
#include "protocol/SessionModifyNotification.h"
#include "protocol/SessionOkNotification.h"
#include "protocol/SessionRejectNotification.h"
#include "protocol/SessionReportNotification.h"
#include "protocol/SessionRingingNotification.h"
#include "protocol/SessionNotifyNotification.h"
#include "protocol/SessionSdpNotification.h"
#include "protocol/SessionIceCandidateNotification.h"

#include "rtcsessionglobal.h"
#include "rtcsession.h"

#include "rtcmanager.h"

namespace rtcsession
{
	RtcManager::RtcManager(QObject *parent)
		: QObject(parent)
	{

	}

	RtcManager::~RtcManager()
	{

	}

	bool RtcManager::initObject()
	{
		m_pmClient = PmClient::instance();
		Q_ASSERT_X(m_pmClient, "PmClient", "PmClient is NULL");

		m_nHandleId = m_pmClient->insertNotificationHandler(this);
		qDebug() << Q_FUNC_INFO << " handle: " << m_nHandleId;

		return true;
	}

	void RtcManager::removeObject()
	{
		PmClient::instance()->removeNotificationHandler(m_nHandleId);
		m_nHandleId = -1;
	}

	QObject* RtcManager::instance()
	{
		return this;
	}

	int RtcManager::handledId() const
	{
		return m_nHandleId;
	}

	QList<int> RtcManager::types() const
	{
		return QList<int>() << protocol::SESSION_ACK
			<< protocol::SESSION_BYE
			<< protocol::SESSION_INVITE
			<< protocol::SESSION_MODIFY
			<< protocol::SESSION_OK
			<< protocol::SESSION_REJECT
			<< protocol::SESSION_REPORT
			<< protocol::SESSION_RINGING
			<< protocol::SESSION_NOTIFY
			<< protocol::SESSION_SDP
			<< protocol::SESSION_ICE;
	}

	bool RtcManager::onNotication(int handleId, protocol::SpecificNotification *sn)
	{
		if (handleId != m_nHandleId)
			return false;

		switch (sn->getNotificationType())
		{
		case protocol::SESSION_INVITE:
			{
				protocol::SessionInviteNotification::In *pIn = static_cast<protocol::SessionInviteNotification::In*>(sn);
				if (pIn)
				{
					protocol::SessionInviteNotification::Param *pParam = new protocol::SessionInviteNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processInvite", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_RINGING:
			{
				protocol::SessionRingingNotifiction::In *pIn = static_cast<protocol::SessionRingingNotifiction::In *>(sn);
				if (pIn)
				{
					protocol::SessionRingingNotifiction::Param *pParam = new protocol::SessionRingingNotifiction::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processRinging", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_OK:
			{
				protocol::SessionOkNotification::In *pIn = static_cast<protocol::SessionOkNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionOkNotification::Param *pParam = new protocol::SessionOkNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processOk", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_REJECT:
			{
				protocol::SessionRejectNotification::In *pIn = static_cast<protocol::SessionRejectNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionRejectNotification::Param *pParam = new protocol::SessionRejectNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processReject", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_MODIFY:
			{
				protocol::SessionModifyNotification::In *pIn = static_cast<protocol::SessionModifyNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionModifyNotification::Param *pParam = new protocol::SessionModifyNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processModify", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_ACK:
			{
				protocol::SessionAckNotification::In *pIn = static_cast<protocol::SessionAckNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionAckNotification::Param *pParam = new protocol::SessionAckNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processAck", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_BYE:
			{
				protocol::SessionByeNotification::In *pIn = static_cast<protocol::SessionByeNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionByeNotification::Param *pParam = new protocol::SessionByeNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processBye", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_REPORT:
			{
				protocol::SessionReportNotifiction::In *pIn = static_cast<protocol::SessionReportNotifiction::In *>(sn);
				if (pIn)
				{
					protocol::SessionReportNotifiction::Param *pParam = new protocol::SessionReportNotifiction::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processReport", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_NOTIFY:
			{
				protocol::SessionNotifyNotifiction *pIn = static_cast<protocol::SessionNotifyNotifiction *>(sn);
				if (pIn)
				{
					QString sessionId = QString::fromLocal8Bit(pIn->sessionId().c_str());
					QString toFullId = QString::fromLocal8Bit(pIn->toFullId().c_str());
					QString action = QString::fromLocal8Bit(pIn->action().c_str());
					QMetaObject::invokeMethod(this, "processNotify", Qt::QueuedConnection, 
						Q_ARG(QString, sessionId), Q_ARG(QString, toFullId), Q_ARG(QString, action));
				}
			}
			break;
		case protocol::SESSION_SDP:
			{
				protocol::SessionSdpNotification::In *pIn = static_cast<protocol::SessionSdpNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionSdpNotification::Param *pParam = new protocol::SessionSdpNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processSdp", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;
		case protocol::SESSION_ICE:
			{
				protocol::SessionIceCandidateNotification::In *pIn = static_cast<protocol::SessionIceCandidateNotification::In *>(sn);
				if (pIn)
				{
					protocol::SessionIceCandidateNotification::Param *pParam = new protocol::SessionIceCandidateNotification::Param(pIn->param);
					QMetaObject::invokeMethod(this, "processIceCandidate", Qt::QueuedConnection, Q_ARG(void*, pParam));
				}
			}
			break;

		default:
			break;
		}

		return true;
	}

	void RtcManager::processInvite(void *pValue)
	{
		protocol::SessionInviteNotification::Param *pParam = static_cast<protocol::SessionInviteNotification::Param *>(pValue);
		if (!pParam)
			return;
		
		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		sessionParam.hasVideo = pParam->hasVideo;
		sessionParam.audioSend = pParam->audioSend;
		sessionParam.audioRecv = pParam->audioRecv;
		sessionParam.videoSend = pParam->videoSend;
		sessionParam.videoRecv = pParam->videoRecv;
		emit inviteRecved(sessionParam);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processRinging(void *pValue)
	{
		protocol::SessionRingingNotifiction::Param *pParam = static_cast<protocol::SessionRingingNotifiction::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		emit ringingRecved(sessionParam);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processOk(void *pValue)
	{
		protocol::SessionOkNotification::Param *pParam = static_cast<protocol::SessionOkNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		sessionParam.hasVideo = pParam->hasVideo;
		sessionParam.audioSend = pParam->audioSend;
		sessionParam.audioRecv = pParam->audioRecv;
		sessionParam.videoSend = pParam->videoSend;
		sessionParam.videoRecv = pParam->videoRecv;
		emit okRecved(sessionParam);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processReject(void *pValue)
	{
		protocol::SessionRejectNotification::Param *pParam = static_cast<protocol::SessionRejectNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		QString reasonType = QString::fromLocal8Bit(pParam->reasonType.c_str());
		QString reasonDesc = QString::fromLocal8Bit(pParam->reasonDesc.c_str());
		emit rejectRecved(sessionParam, reasonType, reasonDesc);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processAck(void *pValue)
	{
		protocol::SessionAckNotification::Param *pParam = static_cast<protocol::SessionAckNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		emit ackRecved(sessionParam);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processBye(void *pValue)
	{
		protocol::SessionByeNotification::Param *pParam = static_cast<protocol::SessionByeNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		QString reasonType = QString::fromLocal8Bit(pParam->reasonType.c_str());
		QString reasonDesc = QString::fromLocal8Bit(pParam->reasonDesc.c_str());
		emit byeRecved(sessionParam, reasonType, reasonDesc);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processModify(void *pValue)
	{
		protocol::SessionModifyNotification::Param *pParam = static_cast<protocol::SessionModifyNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		sessionParam.hasVideo = pParam->hasVideo;
		sessionParam.audioSend = pParam->audioSend;
		sessionParam.audioRecv = pParam->audioRecv;
		sessionParam.videoSend = pParam->videoSend;
		sessionParam.videoRecv = pParam->videoRecv;
		emit modifyRecved(sessionParam);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processReport(void *pValue)
	{
		protocol::SessionReportNotifiction::Param *pParam = static_cast<protocol::SessionReportNotifiction::Param *>(pValue);
		if (pParam)
		{
			// currently we don't use this notification
		}

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processNotify(const QString &sId, const QString &toFullId, const QString &action)
	{
		emit notifyRecved(sId, toFullId, action);
	}

	void RtcManager::processSdp(void *pValue)
	{
		protocol::SessionSdpNotification::Param *pParam = static_cast<protocol::SessionSdpNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		QString sdp = QString::fromLocal8Bit(pParam->sdp.c_str());
		emit sdpRecved(sessionParam, sdp);

		delete pParam;
		pParam = 0;
	}

	void RtcManager::processIceCandidate(void *pValue)
	{
		protocol::SessionIceCandidateNotification::Param *pParam = static_cast<protocol::SessionIceCandidateNotification::Param *>(pValue);
		if (!pParam)
			return;

		SessionParam sessionParam;
		sessionParam.sid = QString::fromLocal8Bit(pParam->id.c_str());
		sessionParam.from = QString::fromLocal8Bit(pParam->from.c_str());
		sessionParam.to = QString::fromLocal8Bit(pParam->to.c_str());
		sessionParam.sp = QString::fromLocal8Bit(pParam->sp.c_str());
		QString iceCandidate = QString::fromLocal8Bit(pParam->iceCandidate.c_str());
		emit iceCandidateRecved(sessionParam, iceCandidate);

		delete pParam;
		pParam = 0;
	}

	QString RtcManager::myUid() const
	{
		QString ret = "";
		if (m_pmClient)
		{
			ret = m_pmClient->id();
		}
		return ret;
	}

	bool RtcManager::sendInvite(Session *s)
	{
		protocol::SessionInviteNotification::Out *pOut = new protocol::SessionInviteNotification::Out();
		if (!pOut)
			return false;

		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		if (s->type() == ST_AudioVideo)
		{
			pOut->param.hasVideo = true;
		}
		else
		{
			pOut->param.hasVideo = false;
		}
		SessionMediaParam mediaParam = s->selfMediaParam();
		pOut->param.audioSend = mediaParam.audioSend;
		pOut->param.audioRecv = mediaParam.audioRecv;
		pOut->param.videoSend = mediaParam.videoSend;
		pOut->param.videoRecv = mediaParam.videoRecv;

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendRinging(Session *s)
	{
		protocol::SessionRingingNotifiction::Out *pOut = new protocol::SessionRingingNotifiction::Out();
		if (!pOut)
			return false;

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendOk(Session *s)
	{
		protocol::SessionOkNotification::Out *pOut = new protocol::SessionOkNotification::Out;
		if (!pOut)
			return false;

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		if (s->type() == ST_AudioVideo)
		{
			pOut->param.hasVideo = true;
		}
		else
		{
			pOut->param.hasVideo = false;
		}
		SessionMediaParam mediaParam = s->selfMediaParam();
		pOut->param.audioSend = mediaParam.audioSend;
		pOut->param.audioRecv = mediaParam.audioRecv;
		pOut->param.videoSend = mediaParam.videoSend;
		pOut->param.videoRecv = mediaParam.videoRecv;

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendReject(Session *s, const QString &reasonType, const QString &reasonDesc)
	{
		protocol::SessionRejectNotification::Out *pOut = new protocol::SessionRejectNotification::Out;
		if (!pOut)
		{
			return false;
		}

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		pOut->param.reasonType = qPrintable(reasonType);
		pOut->param.reasonDesc = qPrintable(reasonDesc);

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendAck(Session *s)
	{
		protocol::SessionAckNotification::Out *pOut = new protocol::SessionAckNotification::Out;
		if (!pOut)
		{
			return false;
		}

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendBye(Session *s, const QString &reasonType, const QString &reasonDesc)
	{
		protocol::SessionByeNotification::Out *pOut = new protocol::SessionByeNotification::Out;
		if (!pOut)
		{
			return false;
		}

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		pOut->param.reasonType = qPrintable(reasonType);
		pOut->param.reasonDesc = qPrintable(reasonDesc);

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendModify(Session *s)
	{
		protocol::SessionModifyNotification::Out *pOut = new protocol::SessionModifyNotification::Out;
		if (!pOut)
			return false;

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		if (s->type() == ST_AudioVideo)
		{
			pOut->param.hasVideo = true;
		}
		else
		{
			pOut->param.hasVideo = false;
		}
		SessionMediaParam mediaParam = s->selfMediaParam();
		pOut->param.audioSend = mediaParam.audioSend;
		pOut->param.audioRecv = mediaParam.audioRecv;
		pOut->param.videoSend = mediaParam.videoSend;
		pOut->param.videoRecv = mediaParam.videoRecv;

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendReport()
	{
		return false;
	}

	bool RtcManager::sendSdp(Session *s, const QString &sdp)
	{
		protocol::SessionSdpNotification::Out *pOut = new protocol::SessionSdpNotification::Out;
		if (!pOut)
			return false;

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		pOut->param.sdp = qPrintable(sdp);

		return m_pmClient->send(pOut);
	}

	bool RtcManager::sendIceCandidate(Session *s, const QString &iceCandidate)
	{
		protocol::SessionIceCandidateNotification::Out *pOut = new protocol::SessionIceCandidateNotification::Out;
		if (!pOut)
			return false;

		pOut->param.sp = qPrintable(s->sp());
		pOut->param.id = qPrintable(s->sid());
		pOut->param.from = qPrintable(s->from());
		pOut->param.to = qPrintable(s->to());
		pOut->param.iceCandidate = qPrintable(iceCandidate);

		return m_pmClient->send(pOut);
	}
}
