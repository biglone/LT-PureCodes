#include "rtcmessage.h"
#include "qt-json/json.h"
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcMessage
RtcMessage::RtcMessage(MessageType type, const QString &id)
	: m_type(type), m_id(id)
{
}

QString RtcMessage::toXml() const
{
	QString head = QString("<message id='%1' type='%2'>").arg(m_id).arg(type2Str(m_type));
	QString end = QString("</message>");
	return head + toJson() + end;
}

QString RtcMessage::type2Str(MessageType type)
{
	QString str;
	switch (type)
	{
	case RtcMessage::NoType:
		str = "";
		break;
	case RtcMessage::Init:
		str = "init";
		break;
	case RtcMessage::InitResult:
		str = "init_result";
		break;
	case RtcMessage::Offer:
		str = "offer";
		break;
	case RtcMessage::Data:
		str = "data";
		break;
	case RtcMessage::Statistics:
		str = "statistics";
		break;
	case RtcMessage::State:
		str = "state";
		break;
	case RtcMessage::Cmd:
		str = "cmd";
		break;
	case RtcMessage::CmdResult:
		str = "cmd_result";
		break;
	case RtcMessage::Bye:
		str = "bye";
		break;
	default:
		break;
	}
	return str;
}

RtcMessage::MessageType RtcMessage::str2Type(const QString &str)
{
	RtcMessage::MessageType type = NoType;
	if (str == QString::fromLatin1("init"))
	{
		type = RtcMessage::Init;
	}
	else if (str == QString::fromLatin1("init_result"))
	{
		type = RtcMessage::InitResult;
	}
	else if (str == QString::fromLatin1("offer"))
	{
		type = RtcMessage::Offer;
	}
	else if (str == QString::fromLatin1("data"))
	{
		type = RtcMessage::Data;
	}
	else if (str == QString::fromLatin1("statistics"))
	{
		type = RtcMessage::Statistics;
	}
	else if (str == QString::fromLatin1("state"))
	{
		type = RtcMessage::State;
	}
	else if (str == QString::fromLatin1("cmd"))
	{
		type = RtcMessage::Cmd;
	}
	else if (str == QString::fromLatin1("cmd_result"))
	{
		type = RtcMessage::CmdResult;
	}
	else if (str == QString::fromLatin1("bye"))
	{
		type = RtcMessage::Bye;
	}

	return type;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcInit
RtcInit::RtcInit(const QString &id)
	: RtcMessage(RtcMessage::Init, id), 
	m_audioSend(false),
	m_audioRecv(false),
	m_videoSend(false),
	m_videoRecv(false),
	m_initial(false)
{
}

bool RtcInit::isAudioSend() const
{
	return m_audioSend;
}

void RtcInit::setAudioSend(bool send)
{
	m_audioSend = send;
}

bool RtcInit::isAudioRecv() const
{
	return m_audioRecv;
}

void RtcInit::setAudioRecv(bool recv)
{
	m_audioRecv = recv;
}

bool RtcInit::isVideoSend() const
{
	return m_videoSend;
}

void RtcInit::setVideoSend(bool send)
{
	m_videoSend = send;
}

bool RtcInit::isVideoRecv() const
{
	return m_videoRecv;
}

void RtcInit::setVideoRecv(bool recv)
{
	m_videoRecv = recv;
}

bool RtcInit::isInitial() const
{
	return m_initial;
}

void RtcInit::setInitial(bool initial)
{
	m_initial = initial;
}

void RtcInit::getIceServers(QStringList &urls, QString &username, QString &credential)
{
	urls = m_iceUrls;
	username = m_username;
	credential = m_credential;
}

void RtcInit::setIceServers(const QStringList &urls, const QString &username, const QString &credential)
{
	m_iceUrls = urls;
	m_username = username;
	m_credential = credential;
}

QString RtcInit::toJson() const
{
	QVariantMap vm;

	QVariantMap audioVm;
	audioVm["recv"] = m_audioRecv ? 1 : 0;
	audioVm["send"] = m_audioSend ? 1 : 0;
	vm["audio"] = audioVm;
	
	QVariantMap videoVm;
	videoVm["recv"] = m_videoRecv ? 1 : 0;
	videoVm["send"] = m_videoSend ? 1 : 0;
	vm["video"] = videoVm;

	vm["isInitial"] = m_initial ? 1 : 0;
	
	QVariantMap iceServers;
	QVariantList urls;
	foreach (QString url, m_iceUrls)
	{
		urls.append(url);
	}
	iceServers["urls"] = urls;
	iceServers["username"] = m_username;
	iceServers["credential"] = m_credential;
	vm["ice_servers"] = iceServers;

	return QString::fromUtf8(QtJson::serialize(vm));
}

bool RtcInit::fromJson(const QString &json)
{
	bool ok = false;
	QVariant v = QtJson::parse(json, ok);
	if (ok)
	{
		QVariantMap vm = v.toMap();
		
		QVariantMap audioVm = vm["audio"].toMap();
		m_audioRecv = (audioVm["recv"].toInt() == 1);
		m_audioSend = (audioVm["send"].toInt() == 1);

		QVariantMap videoVm = vm["video"].toMap();
		m_videoRecv = (videoVm["recv"].toInt() == 1);
		m_videoSend = (videoVm["send"].toInt() == 1);

		m_initial = (vm["isInitial"].toInt() == 1);

		QVariantMap iceServers = vm["ice_servers"].toMap();
		QVariantList urls = iceServers["urls"].toList();
		foreach (QVariant url, urls)
		{
			m_iceUrls.append(url.toString());
		}
		m_username = iceServers["username"].toString();
		m_credential = iceServers["credential"].toString();
	}
	return ok;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcInitResult
const char * RtcInitResult::kErrAudioDesc = "audio";
const char * RtcInitResult::kErrVideoDesc = "video";
const char * RtcInitResult::kErrServerDesc = "server";

RtcInitResult::RtcInitResult(const QString &id)
	: RtcMessage(RtcMessage::InitResult, id),
	m_ok(false)
{
}

bool RtcInitResult::isOK() const
{
	return m_ok;
}

void RtcInitResult::setOK(bool ok)
{
	m_ok = ok;
}

QString RtcInitResult::desc() const
{
	return m_desc;
}

void RtcInitResult::setDesc(const QString &desc)
{
	m_desc = desc;
}

RtcInitResult::ErrType RtcInitResult::errTypeFromDesc(const QString &desc)
{
	RtcInitResult::ErrType errType = kErrNoError;
	if (desc == kErrAudioDesc)
		errType = kErrAudio;
	else if (desc == kErrVideoDesc)
		errType = kErrVideo;
	else if (desc == kErrServerDesc)
		errType = kErrServer;
	return errType;
}

QString RtcInitResult::toJson() const
{
	QVariantMap vm;
	vm["result"] = m_ok ? "ok" : "error";
	vm["desc"] = m_desc;
	return QString::fromUtf8(QtJson::serialize(vm));
}

bool RtcInitResult::fromJson(const QString &json)
{
	bool ok = false;
	QVariant v = QtJson::parse(json, ok);
	if (ok)
	{
		QVariantMap vm = v.toMap();
		if (vm["result"].toString() == QString("ok"))
			m_ok = true;
		else
			m_ok = false;
		m_desc = vm["desc"].toString();
	}
	return ok;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcOffer
RtcOffer::RtcOffer(const QString &id)
	: RtcMessage(RtcMessage::Offer, id)
{
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcData
RtcData::RtcData(const QString &id)
	: RtcMessage(RtcMessage::Data, id)
{
}

QString RtcData::data() const
{
	return m_data;
}

void RtcData::setData(const QString &data)
{
	m_data = data;
}

QString RtcData::toJson() const
{
	return m_data;
}

bool RtcData::fromJson(const QString &json)
{
	m_data = json;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcState
RtcState::RtcState(const QString &id)
	: RtcMessage(RtcMessage::State, id)
{
	m_state = RtcState::Error;
}

RtcState::State RtcState::state() const
{
	return m_state;
}

void RtcState::setState(State s)
{
	m_state = s;
}

QString RtcState::desc() const
{
	return m_desc;
}

void RtcState::setDesc(const QString &desc)
{
	m_desc = desc;
}

QString RtcState::state2Str(RtcState::State s)
{
	QString str;
	switch (s)
	{
	case RtcState::Uninited:
		str = "uninited";
		break;
	case RtcState::Inited:
		str = "inited";
		break;
	case RtcState::Connecting:
		str = "connecting";
		break;
	case RtcState::Connected:
		str = "connected";
		break;
	case RtcState::Error:
		str = "error";
		break;
	default:
		break;
	}
	return str;
}

RtcState::State RtcState::str2State(const QString &str)
{
	RtcState::State state = RtcState::Error;
	if (str == QString::fromLatin1("uninited"))
		state = RtcState::Uninited;
	else if (str == QString::fromLatin1("inited"))
		state = RtcState::Inited;
	else if (str == QString::fromLatin1("connecting"))
		state = RtcState::Connecting;
	else if (str == QString::fromLatin1("connected"))
		state = RtcState::Connected;
	else if (str == QString::fromLatin1("error"))
		state = RtcState::Error;
	return state;
}

QString RtcState::toJson() const
{
	QVariantMap vm;
	vm["state"] = state2Str(m_state);
	vm["desc"] = m_desc;
	return QString::fromUtf8(QtJson::serialize(vm));
}

bool RtcState::fromJson(const QString &json)
{
	bool ok = false;
	QVariant v = QtJson::parse(json, ok);
	if (ok)
	{
		QVariantMap vm = v.toMap();
		QString str = vm["state"].toString();
		m_state = str2State(str);
		m_desc = vm["desc"].toString();
	}
	return ok;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcStatistics
RtcStatistics::RtcStatistics(const QString &id)
	: RtcMessage(RtcMessage::Statistics, id), m_hasAudio(false), m_hasVideo(false),
	m_audioPackagesLost(0), m_audioPackagesReceived(0), m_videoPackagesLost(0), m_videoPackagesReceived(0)
{
}

bool RtcStatistics::hasAudio() const
{
	return m_hasAudio;
}

bool RtcStatistics::hasVideo() const
{
	return m_hasVideo;
}

void RtcStatistics::setHasAudio(bool audio)
{
	m_hasAudio = audio;
}

void RtcStatistics::setHasVideo(bool video)
{
	m_hasVideo = video;
}

void RtcStatistics::audioStatistics(int &packagesLost, int &packagesReceived)
{
	packagesLost = m_audioPackagesLost;
	packagesReceived = m_audioPackagesReceived;
}

void RtcStatistics::videoStatistics(int &packagesSent, int &packagesReceived)
{
	packagesSent = m_videoPackagesLost;
	packagesReceived = m_videoPackagesReceived;
}

void RtcStatistics::setAudioStatistics(int packagesLost, int packagesReceived)
{
	m_audioPackagesLost = packagesLost;
	m_audioPackagesReceived = packagesReceived;
}

void RtcStatistics::setVideoStatistics(int packagesLost, int packagesReceived)
{
	m_videoPackagesLost = packagesLost;
	m_videoPackagesReceived = packagesReceived;
}

QString RtcStatistics::toJson() const
{
	QVariantMap vm;
	if (m_hasAudio)
	{
		QVariantMap audioVm;
		audioVm["packetsLost"] = m_audioPackagesLost;
		audioVm["packetsReceived"] = m_audioPackagesReceived;
		vm["audio"] = audioVm;
	}
	if (m_hasVideo)
	{
		QVariantMap videoVm;
		videoVm["packetsLost"] = m_videoPackagesLost;
		videoVm["packetsReceived"] = m_videoPackagesReceived;
		vm["video"] = videoVm;
	}
	return QString::fromUtf8(QtJson::serialize(vm));
}

bool RtcStatistics::fromJson(const QString &json)
{
	bool ok = false;
	QVariant v = QtJson::parse(json, ok);
	if (ok)
	{
		QVariantMap vm = v.toMap();
		if (vm.contains("audio"))
		{
			m_hasAudio = true;
			QVariantMap audioVm = vm["audio"].toMap();
			m_audioPackagesLost = audioVm["packetsLost"].toInt();
			m_audioPackagesReceived = audioVm["packetsReceived"].toInt();
		}
		if (vm.contains("video"))
		{
			m_hasVideo = true;
			QVariantMap videoVm = vm["video"].toMap();
			m_videoPackagesLost = videoVm["packetsLost"].toInt();
			m_videoPackagesReceived = videoVm["packetsReceived"].toInt();
		}
	}
	return ok;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcCmd
RtcCmd::RtcCmd(const QString &id)
	: RtcMessage(RtcMessage::Cmd, id), m_changeType(ChangeNone)
{
}

RtcCmd::ChangeType RtcCmd::changeType() const
{
	return m_changeType;
}

void RtcCmd::setChangeType(ChangeType type)
{
	m_changeType = type;
}

QString RtcCmd::changeValue() const
{
	return m_changeValue;
}

void RtcCmd::setChangeValue(const QString &v)
{
	m_changeValue = v;
}

RtcCmd::ChangeType RtcCmd::str2Type(const QString &str)
{
	ChangeType changeType = ChangeNone;
	if (str == QString::fromLatin1("audio_send"))
	{
		changeType = AudioSend;
	}
	else if (str == QString::fromLatin1("video_send"))
	{
		changeType = VideoSend;
	}
	else if (str == QString::fromLatin1("play_volume"))
	{
		changeType = PlayVolume;
	}
	return changeType;
}

QString RtcCmd::type2Str(ChangeType type)
{
	QString str;
	if (type == AudioSend)
	{
		str = QString::fromLatin1("audio_send");
	}
	else if (type == VideoSend)
	{
		str = QString::fromLatin1("video_send");
	}
	else if (type == PlayVolume)
	{
		str = QString::fromLatin1("play_volume");
	}
	return str;
}

QString RtcCmd::toJson() const
{
	QVariantMap vm;
	vm["type"] = type2Str(m_changeType);
	vm["value"] = m_changeValue;

	return QString::fromUtf8(QtJson::serialize(vm));
}

bool RtcCmd::fromJson(const QString &json)
{
	bool ok = false;
	QVariant v = QtJson::parse(json, ok);
	if (ok)
	{
		QVariantMap vm = v.toMap();
		m_changeType = str2Type(vm["type"].toString());
		m_changeValue = vm["value"].toString();
	}
	return ok;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcCmdResult
RtcCmdResult::RtcCmdResult(const QString &id)
	: RtcCmd(id)
{
	m_type = CmdResult;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RtcBye
RtcBye::RtcBye(const QString &id)
	: RtcMessage(RtcMessage::Bye, id)
{
}
