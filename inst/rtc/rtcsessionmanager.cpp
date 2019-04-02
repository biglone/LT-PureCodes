#include "rtcsessionmanager.h"
#include "rtcmanager.h"
#include <QUuid>
#include <QDebug>
#include "rtclocalclient.h"
#include "sharedbuffer.h"
#include "session/VideoFrameCB.h"
#include <QProcess>
#include <QApplication>
#include "rtcmessage.h"
#include "qt-json/json.h"
#include <QVariantMap>

namespace rtcsession
{

RtcSessionManager::RtcSessionManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
	m_rtcManager.reset(new RtcManager(this));
	
	bool connectOK = false;
	connectOK = connect(m_rtcManager.data(), SIGNAL(inviteRecved(SessionParam)),
		this, SLOT(onInviteRecved(SessionParam)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(ringingRecved(SessionParam)),
		this, SLOT(onRingingRecved(SessionParam)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(okRecved(SessionParam)),
		this, SLOT(onOkRecved(SessionParam)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(rejectRecved(SessionParam, QString, QString)),
		this, SLOT(onRejectRecved(SessionParam, QString, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(ackRecved(SessionParam)),
		this, SLOT(onAckRecved(SessionParam)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(byeRecved(SessionParam, QString, QString)),
		this, SLOT(onByeRecved(SessionParam, QString, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(modifyRecved(SessionParam)),
		this, SLOT(onModifyRecved(SessionParam)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(notifyRecved(QString, QString, QString)),
		this, SLOT(onNotifyRecved(QString, QString, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(sdpRecved(SessionParam, QString)),
		this, SLOT(onSdpRecved(SessionParam, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcManager.data(), SIGNAL(iceCandidateRecved(SessionParam, QString)),
		this, SLOT(onIceCandidateRecved(SessionParam, QString)));
	Q_ASSERT(connectOK);

	m_rtcLocalClient.reset(new RtcLocalClient());
	m_messageParser.reset(new MessageParser(QString::fromLatin1("rtc_parser"), *this));
	m_selfBuffer.reset(new SharedBuffer());
	m_selfFrameCB.reset(new session::VideoFrameCB());

	connectOK = connect(m_rtcLocalClient.data(), SIGNAL(dataRecved(QByteArray)), 
		this, SLOT(onDataRecved(QByteArray)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcLocalClient.data(), SIGNAL(sessionConnected()),
		this, SLOT(onClientConnected()), Qt::UniqueConnection);
	Q_ASSERT(connectOK);

	connectOK = connect(m_rtcLocalClient.data(), SIGNAL(sessionDisconnected()),
		this, SLOT(onClientDisconnected()), Qt::UniqueConnection);
	Q_ASSERT(connectOK);

	m_connectCount = 0;
	QTimer::singleShot(0, this, SLOT(startClientConnect()));
}

RtcSessionManager::~RtcSessionManager()
{
	foreach (QString sid, m_mapSession.keys())
	{
		qDebug() << Q_FUNC_INFO << sid;
		Session *pS = m_mapSession.value(sid, 0);
		if (pS)
		{
			delete pS;
		}
	}
}

void RtcSessionManager::initObject()
{
	m_rtcManager->initObject();
}

void RtcSessionManager::removeObject()
{
	m_rtcManager->removeObject();
}

void RtcSessionManager::setIceServerInfo(const QStringList &iceUrls, const QString &iceUsername, const QString &iceCredential)
{
	m_iceUrls = iceUrls;
	m_iceUsername = iceUsername;
	m_iceCredential = iceCredential;
}

void RtcSessionManager::init()
{
}

void RtcSessionManager::release()
{
	m_selfVideoTimer.stop();
	m_selfBuffer->closeBuffer();

	closeAllSessions();
}

Session * RtcSessionManager::createSession(Session::PEER peer, 
	                                       const QString &from, 
										   const QString &to,
										   const QString &sessionId /*= QString()*/)
{
	Session *s = 0;
	do 
	{
		if (from.isEmpty() || to.isEmpty())
			break;

		QString sid = sessionId;
		if (sid.isEmpty())
			sid = createId();
		s = session(sid);
		if (s)
		{
			if (s->to().compare(to) == 0)
			{
				return s;
			}
			else
			{
				qDebug() << QString("Same sid with different sid: %1 uid: %2 old uid: %3").arg(sid).arg(to).arg(s->to());
				return 0;
			}
		}

		s = new Session(peer, from, to, sid, this);
		bool connectOK = connect(s, SIGNAL(aboutClose(QString)), this, SLOT(deleteSession(QString)));
		Q_ASSERT(connectOK);

		m_mapSession[sid] = s;

	} while (0);

	return s;
}

Session * RtcSessionManager::session(const QString &sid)
{
	 return m_mapSession.value(sid, 0);
}

void RtcSessionManager::prepareInvite(const Session *s)
{
	if (!m_rtcLocalClient->connected())
	{
		qWarning() << Q_FUNC_INFO << "still do not connect to rtc server yet";

		connectClientSync();
		if (!m_rtcLocalClient->connected())
		{
			qWarning() << Q_FUNC_INFO << "failed to connect to rtc server";
			QMetaObject::invokeMethod(this, "onRtcInitFinished", Qt::QueuedConnection,
				Q_ARG(QString, s->sid()), Q_ARG(bool, false), Q_ARG(int, (int)Session::Error_RtcServiceError));
			return;
		}
	}

	sendInitMsg(s);
}

void RtcSessionManager::prepareOk(const Session *s)
{
	if (!m_rtcLocalClient->connected())
	{
		qWarning() << Q_FUNC_INFO << "still do not connect to rtc server yet";

		connectClientSync();
		if (!m_rtcLocalClient->connected())
		{
			qWarning() << Q_FUNC_INFO << "failed to connect to rtc server";
			QMetaObject::invokeMethod(this, "onRtcInitFinished", Qt::QueuedConnection,
				Q_ARG(QString, s->sid()), Q_ARG(bool, false), Q_ARG(int, (int)Session::Error_RtcServiceError));
			return;
		}
	}

	sendInitMsg(s);
}

bool RtcSessionManager::hasVideoSession() const
{
	bool hasVideo = false;
	QList<Session *> sessions = m_mapSession.values();
	foreach (Session *s, sessions)
	{
		if (s && s->type() == ST_AudioVideo)
		{
			hasVideo = true;
			break;
		}
	}
	return hasVideo;
}

bool RtcSessionManager::hasSession() const
{
	QList<Session *> sessions = m_mapSession.values();
	return !sessions.isEmpty();
}

bool RtcSessionManager::hasSessionWithUid(const QString &uid) const
{
	if (uid.isEmpty())
		return false;

	QList<Session *> sessions = m_mapSession.values();
	foreach (Session *s, sessions)
	{
		if (s && (s->from() == uid || s->to() == uid))
		{
			return true;
		}
	}
	return false;
}

Session *RtcSessionManager::sessionWithUid(const QString &uid) const
{
	Session *ret = 0;

	if (uid.isEmpty())
		return ret;

	QList<Session *> sessions = m_mapSession.values();
	foreach (Session *s, sessions)
	{
		if (s && (s->from() == uid || s->to() == uid))
		{
			ret = s;
			break;
		}
	}

	return ret;
}

RtcManager *RtcSessionManager::rtcManager() const
{
	return m_rtcManager.data();
}

session::VideoFrameCB *RtcSessionManager::selfVideoFrameCB() const
{
	return m_selfFrameCB.data();
}

void RtcSessionManager::sendInitMsg(const Session *s)
{
	RtcInit initMsg(s->sid());
	if (s->peer() == Session::Peer_A)
		initMsg.setInitial(true);
	else
		initMsg.setInitial(false);
	initMsg.setAudioSend(true);
	initMsg.setAudioRecv(true);
	if (s->type() == ST_Audio)
	{
		initMsg.setVideoSend(false);
		initMsg.setVideoRecv(false);
	}
	else
	{
		initMsg.setVideoSend(true);
		initMsg.setVideoRecv(true);
	}
	initMsg.setIceServers(m_iceUrls, m_iceUsername, m_iceCredential);
	QByteArray xml = initMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::sendOfferMsg(const Session *s)
{
	RtcOffer offerMsg(s->sid());
	QByteArray xml = offerMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::sendDataMsg(const Session *s, const QString &data)
{
	RtcData dataMsg(s->sid());
	dataMsg.setData(data);
	QByteArray xml = dataMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::sendCmdAudioMsg(const Session *s, bool audioSend)
{
	RtcCmd cmdMsg(s->sid());
	cmdMsg.setChangeType(RtcCmd::AudioSend);
	if (audioSend)
		cmdMsg.setChangeValue("1");
	else
		cmdMsg.setChangeValue("0");
	QByteArray xml = cmdMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::sendCmdVideoMsg(const Session *s, bool videoSend)
{
	RtcCmd cmdMsg(s->sid());
	cmdMsg.setChangeType(RtcCmd::VideoSend);
	if (videoSend)
		cmdMsg.setChangeValue("1");
	else
		cmdMsg.setChangeValue("0");
	QByteArray xml = cmdMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::sendCmdVolumeMsg(const Session *s, int volume)
{
	RtcCmd cmdMsg(s->sid());
	cmdMsg.setChangeType(RtcCmd::PlayVolume);
	cmdMsg.setChangeValue(QString::number(volume));
	QByteArray xml = cmdMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::sendByeMsg(const Session *s)
{
	RtcBye byeMsg(s->sid());
	QByteArray xml = byeMsg.toXml().toUtf8();
	m_rtcLocalClient->sendMessage(xml);

	qDebug() << "===> rtcmodule:" << xml;
}

void RtcSessionManager::onMessage(const QString &parserId, const QString &id, const QString &typeStr, const QString &json)
{
	Q_UNUSED(parserId);
	if (id.isEmpty() || typeStr.isEmpty())
		return;

	if (typeStr != RtcMessage::type2Str(RtcMessage::Statistics))
	{
		qDebug() << "rtc module ==>" << id << typeStr << json;
	}

	RtcMessage::MessageType type = RtcMessage::str2Type(typeStr);
	RtcMessage *rtcMessage = 0;
	switch (type)
	{
	case RtcMessage::InitResult:
		rtcMessage = new RtcInitResult(id);
		rtcMessage->fromJson(json);
		processInitResultMsg(rtcMessage);
		break;
	case RtcMessage::Data:
		rtcMessage = new RtcData(id);
		rtcMessage->fromJson(json);
		processDataMsg(rtcMessage);
		break;
	case RtcMessage::Statistics:
		rtcMessage = new RtcStatistics(id);
		rtcMessage->fromJson(json);
		processStatisticsMsg(rtcMessage);
		break;
	case RtcMessage::State:
		rtcMessage = new RtcState(id);
		rtcMessage->fromJson(json);
		processStateMsg(rtcMessage);
		break;
	case RtcMessage::CmdResult:
		rtcMessage = new RtcCmdResult(id);
		rtcMessage->fromJson(json);
		processCmdResultMsg(rtcMessage);
		break;
	default:
		break;
	}
}

void RtcSessionManager::onError(const QString &parserId, const QString &desc)
{
	Q_UNUSED(parserId);
	qDebug() << Q_FUNC_INFO << "parse message error: " << desc;
}

void RtcSessionManager::onInviteRecved(const SessionParam &sessionParam)
{
	QString from = sessionParam.from;
	QString to = sessionParam.to;
	QString sid = sessionParam.sid;
	QString sp = sessionParam.sp;

	Session *s = createSession(Session::Peer_B, from, to, sid);
	if (!s)
	{
		qDebug() << Q_FUNC_INFO << "create session failed";
		return;
	}

	s->setSp(sp);
	s->recvInvite(sessionParam.hasVideo, sessionParam.audioSend, sessionParam.audioRecv,
		sessionParam.videoSend, sessionParam.videoRecv);

	if (checkConflict(s->from()))
	{
		qDebug() << Q_FUNC_INFO << "conflict";
		deleteSession(sid);
		return;
	}

	m_mapUid2Sid[s->from()] = s->sid();

	emit recvInvite(s->from(), s->sid());
}

void RtcSessionManager::onRingingRecved(const SessionParam &sessionParam)
{
	QString sid = sessionParam.sid;
	QString from = sessionParam.from;
	QString sp = sessionParam.sp;

	Session *s = session(sid);
	if (s)
	{
		s->setSp(sp);
		s->recvRinging();
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when ringing";
	}
}

void RtcSessionManager::onOkRecved(const SessionParam &sessionParam)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		s->recvOk(sessionParam.audioSend, sessionParam.audioRecv, sessionParam.videoSend, sessionParam.videoRecv);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when ok";
	}
}

void RtcSessionManager::onRejectRecved(const SessionParam &sessionParam, const QString &reasonType, const QString &reasonDesc)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		s->recvReject(reasonType, reasonDesc);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when reject";
	}
}

void RtcSessionManager::onAckRecved(const SessionParam &sessionParam)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		s->recvAck();
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when ack.";
	}
}

void RtcSessionManager::onByeRecved(const SessionParam &sessionParam, const QString &reasonType, const QString &reasonDesc)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		s->recvBye(reasonType, reasonDesc);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when bye";
	}
}

void RtcSessionManager::onModifyRecved(const SessionParam &sessionParam)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		s->recvModify(sessionParam.audioSend, sessionParam.audioRecv, sessionParam.videoSend, sessionParam.videoRecv);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when modify";
	}
}

void RtcSessionManager::onNotifyRecved(const QString &sid, const QString &toFullId, const QString &action)
{
	Session *s = session(sid);
	if (s)
	{
		s->recvNotify(toFullId, action);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when notify";
	}
}

void RtcSessionManager::onSdpRecved(const SessionParam &sessionParam, const QString &sdp)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		sendDataMsg(s, sdp);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when sdp";
	}
}

void RtcSessionManager::onIceCandidateRecved(const SessionParam &sessionParam, const QString &iceCandidate)
{
	QString sid = sessionParam.sid;
	Session *s = session(sid);
	if (s)
	{
		sendDataMsg(s, iceCandidate);
	}
	else
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when ice candidate";
	}
}

void RtcSessionManager::onRtcInitFinished(const QString &sid, bool result, int sessionErr)
{
	Session::SessionError err = (Session::SessionError)sessionErr;
	Session *s = session(sid);
	if (!s)
	{
		// error
		qWarning() << Q_FUNC_INFO << "do not have a session when rtc init finished";
		return;
	}

	if (s->peer() == Session::Peer_A)
	{
		// start invite
		if (result)
		{
			m_mapUid2Sid[s->to()] = s->sid();
		}

		s->inviteFinished(result, err);
	}
	else
	{
		// start ok
		s->okFinished(result, err);
	}
}

void RtcSessionManager::deleteSession(const QString &sid)
{
	if (m_mapSession.contains(sid))
	{
		qDebug() << Q_FUNC_INFO << sid;
		Session *pS = m_mapSession.take(sid);

		sendByeMsg(pS);
		stopRecvSelfVideo(sid);

		QString otherSide = getOtherSideUid(pS->from(), pS->to());
		m_mapUid2Sid.remove(otherSide);
		delete pS;
		pS = 0;
	}
}

void RtcSessionManager::closeAllSessions()
{
	foreach (QString sid, m_mapSession.keys())
	{
		qDebug() << Q_FUNC_INFO << sid;
		Session *pS = m_mapSession.value(sid, 0);
		if (pS)
		{
			pS->close(rtcsession::Session::CloseNormal);
		}
	}
}

void RtcSessionManager::onDataRecved(const QByteArray &msg)
{
	m_messageParser->addData(msg);
}

void RtcSessionManager::onClientConnected()
{
	qDebug() << "connected to rtc module";
}

void RtcSessionManager::onClientDisconnected()
{
	qDebug() << "disconnect from rtc module";
}

void RtcSessionManager::startClientConnect()
{
	if (m_rtcLocalClient->connected())
	{
		m_connectCount = 0;
		return;
	}

	if (m_connectCount >= 3)
	{
		qWarning() << Q_FUNC_INFO << "can not connect to rtc module";
		return;
	}

	m_rtcLocalClient->connectToServer(QString::fromLatin1(kLocalServerName));
	++m_connectCount;
	QTimer::singleShot(3000, this, SLOT(startClientConnect()));
}

void RtcSessionManager::connectClientSync()
{
	if (m_rtcLocalClient->connected())
		return;

	m_rtcLocalClient->connectToServer(QString::fromLatin1(kLocalServerName), 1500);
}

void RtcSessionManager::selfVideoFrame()
{
	if (m_selfBuffer.isNull())
		return;

	if (m_selfFrameCB.isNull())
		return;

	QByteArray buffer = m_selfBuffer->readAll();
	if (!buffer.isEmpty())
	{
		QImage frame = QImage::fromData(buffer, "bmp");
		if (!frame.isNull())
		{
			QSize frameSz = frame.size();
			m_selfFrameCB->frameChanged(frame);
		}
	}
}

QString RtcSessionManager::createId() const
{
	QString id = QUuid::createUuid().toString();
	id = id.mid(1, id.length()-2);
	return id;
}

bool RtcSessionManager::checkConflict(const QString &from)
{
	bool bRet = false;
	do 
	{
		// conflict
		QString oldSid = m_mapUid2Sid.value(from, "");
		if (oldSid.isEmpty())
		{
			break;
		}

		Session *oldS = m_mapSession.value(oldSid, 0);
		if (!oldS)
		{
			m_mapUid2Sid.remove(from);
			m_mapSession.remove(oldSid);
			break;
		}

		qDebug() << Q_FUNC_INFO << from << oldSid;

		// oldS set error
		oldS->conflict(); // bye

		bRet = true;
	} while (0);

	return bRet;
}

QString RtcSessionManager::getOtherSideUid(const QString &from, const QString &to)
{
	if (from != m_rtcManager->myUid())
	{
		return from;
	}
	else
	{
		return to;
	}
}

void RtcSessionManager::processInitResultMsg(RtcMessage *msg)
{
	if (!msg)
		return;

	QScopedPointer<RtcInitResult> initResultMsg(static_cast<RtcInitResult *>(msg));
	QString sid = initResultMsg->id();
	bool ok = initResultMsg->isOK();
	QString desc = initResultMsg->desc();
	RtcInitResult::ErrType initErr = RtcInitResult::errTypeFromDesc(desc);
	Session::SessionError err = Session::Error_NoError;
	switch (initErr)
	{
	case RtcInitResult::kErrNoError:
		err = Session::Error_NoError;
		break;
	case RtcInitResult::kErrAudio:
		err = Session::Error_AudioDeviceError;
		break;
	case RtcInitResult::kErrVideo:
		err = Session::Error_CameraOpened;
		break;
	case RtcInitResult::kErrServer:
		err = Session::Error_RtcServiceError;
		break;
	default:
		break;
	}
	onRtcInitFinished(sid, ok, (int)err);
}

void RtcSessionManager::processStateMsg(RtcMessage *msg)
{
	if (!msg)
		return;

	QScopedPointer<RtcState> stateMsg(static_cast<RtcState *>(msg));
	QString sid = stateMsg->id();
	Session *s = session(sid);
	if (!s)
	{
		qWarning() << Q_FUNC_INFO << "no session for state message";
		return;
	}

	qDebug() << Q_FUNC_INFO << RtcState::state2Str(stateMsg->state()) << stateMsg->desc();

	RtcState::State state = stateMsg->state();
	switch (state)
	{
	case RtcState::Uninited:
		break;
	case RtcState::Inited:
		break;
	case RtcState::Connecting:
		break;
	case RtcState::Connected:
		if (s->type() == ST_AudioVideo)
		{
			SessionMediaParam selfParam = s->selfMediaParam();
			if (selfParam.videoSend)
			{
				startRecvSelfVideo(sid);
			}
			SessionMediaParam otherParam = s->otherMediaParam();
			if (otherParam.videoSend)
			{
				s->startRecvOtherVideo();
			}
		}
		break;
	case RtcState::Error:
		s->serviceError();
		break;
	default:
		break;
	}
}

void RtcSessionManager::processDataMsg(RtcMessage *msg)
{
	if (!msg)
		return;

	QScopedPointer<RtcData> dataMsg(static_cast<RtcData *>(msg));
	QString sid = dataMsg->id();
	Session *s = session(sid);
	if (!s)
	{
		qWarning() << Q_FUNC_INFO << "no session for data message";
		return;
	}

	QString json = dataMsg->toJson();
	QVariantMap vm = QtJson::parse(json).toMap();
	if (!vm.contains("type"))
	{
		qWarning() << Q_FUNC_INFO << "error json: " << json;
		return;
	}

	QString type = vm["type"].toString();
	if (type == "candidate")
	{
		m_rtcManager->sendIceCandidate(s, json);
	}
	else if (type == "offer" || type == "answer")
	{
		m_rtcManager->sendSdp(s, json);
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "error json type: " << json;
	}
}

void RtcSessionManager::processStatisticsMsg(RtcMessage *msg)
{
	if (!msg)
		return;

	QScopedPointer<RtcStatistics> statisticMsg(static_cast<RtcStatistics *>(msg));
	QString sid = statisticMsg->id();
	Session *s = session(sid);
	if (!s)
	{
		qWarning() << Q_FUNC_INFO << "no session for statistic message";
		return;
	}

	if (statisticMsg->hasAudio())
	{
		int packageLost = 0;
		int packageReceived = 0;
		statisticMsg->audioStatistics(packageLost, packageReceived);
		s->setAudioStatistics(packageLost, packageReceived);
	}

	if (statisticMsg->hasVideo())
	{
		int packageLost = 0;
		int packageReceived = 0;
		statisticMsg->videoStatistics(packageLost, packageReceived);
		s->setVideoStatistics(packageLost, packageReceived);
	}
}

void RtcSessionManager::processCmdResultMsg(RtcMessage *msg)
{
	if (!msg)
		return;

	QScopedPointer<RtcCmdResult> cmdResultMsg(static_cast<RtcCmdResult *>(msg));
	QString sid = cmdResultMsg->id();
	Session *s = session(sid);
	if (!s)
	{
		qWarning() << Q_FUNC_INFO << "no session for cmd result message";
		return;
	}

	if (cmdResultMsg->changeType() == RtcCmdResult::AudioSend)
	{
		bool audioSendRet = (cmdResultMsg->changeValue() == QString("1")) ? true : false;
		SessionMediaParam selfMedia = s->selfMediaParam();
		if (selfMedia.audioSend == audioSendRet)
		{
			// modify OK
			m_rtcManager->sendModify(s);
		}
		else
		{
			// recover
			s->onAudioSendChanged(audioSendRet);
		}
	}
	else if (cmdResultMsg->changeType() == RtcCmdResult::VideoSend)
	{
		bool videoSendRet = (cmdResultMsg->changeValue() == QString("1")) ? true : false;
		SessionMediaParam selfMedia = s->selfMediaParam();
		if (selfMedia.videoSend == videoSendRet)
		{
			// modify OK
			m_rtcManager->sendModify(s);

			if (videoSendRet)
			{
				startRecvSelfVideo(sid);
			}
			else
			{
				stopRecvSelfVideo(sid);
			}
		}
		else
		{
			// recover
			s->onVideoSendChanged(videoSendRet);
		}
	}
	else if (cmdResultMsg->changeType() == RtcCmdResult::PlayVolume)
	{
		// do nothing
	}
}

void RtcSessionManager::startRecvSelfVideo(const QString &sid)
{
	if (!m_selfVideoSids.isEmpty())
	{
		if (!m_selfVideoSids.contains(sid))
			m_selfVideoSids << sid;
		return;
	}

	wchar_t selfBufferName[128] = {0};
	QString(kLocalVideoRendererName).toWCharArray(selfBufferName);
	if (!m_selfBuffer->openBuffer(selfBufferName, kVideoRenderSize))
	{
		qWarning() << Q_FUNC_INFO << "open self video buffer failed";
		return;
	}

	m_selfVideoTimer.stop();
	m_selfVideoTimer.setInterval(100);
	m_selfVideoTimer.setSingleShot(false);
	m_selfVideoTimer.start();
	connect(&m_selfVideoTimer, SIGNAL(timeout()), this, SLOT(selfVideoFrame()), Qt::UniqueConnection);

	m_selfVideoSids << sid;
}

void RtcSessionManager::stopRecvSelfVideo(const QString &sid)
{
	m_selfVideoSids.removeOne(sid);
	if (!m_selfVideoSids.isEmpty())
		return;

	m_selfVideoTimer.stop();
	m_selfBuffer->closeBuffer();
}

}