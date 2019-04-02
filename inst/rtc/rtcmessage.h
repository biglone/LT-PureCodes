#ifndef __RTC_MESSAGE_H__
#define __RTC_MESSAGE_H__

#include <QString>
#include <QStringList>

class RtcMessage
{
public:
	enum MessageType
	{
		NoType,
		Init,
		InitResult,
		Offer,
		Data,
		State,
		Statistics,
		Cmd,
		CmdResult,
		Bye
	};

public:
	RtcMessage(MessageType type, const QString &id);

	MessageType type() const { return m_type; }
	QString id() const { return m_id; }

	virtual QString toJson() const { return ""; }
	virtual bool fromJson(const QString &json) { Q_UNUSED(json); return false; }
	QString toXml() const;

	static QString type2Str(MessageType type);
	static MessageType str2Type(const QString &str);

protected:
	MessageType m_type;
	QString     m_id;
};

class RtcInit : public RtcMessage
{
public:
	RtcInit(const QString &id);

	bool isAudioSend() const;
	void setAudioSend(bool send);
	bool isAudioRecv() const;
	void setAudioRecv(bool recv);

	bool isVideoSend() const;
	void setVideoSend(bool send);
	bool isVideoRecv() const;
	void setVideoRecv(bool recv);

	bool isInitial() const;
	void setInitial(bool initial);

	void getIceServers(QStringList &urls, QString &username, QString &credential);
	void setIceServers(const QStringList &urls, const QString &username, const QString &credential);

	virtual QString toJson() const;
	virtual bool fromJson(const QString &json);

private:
	bool m_audioSend;
	bool m_audioRecv;
	bool m_videoSend;
	bool m_videoRecv;
	bool m_initial;
	QStringList m_iceUrls;
	QString     m_username;
	QString     m_credential;
};

class RtcInitResult : public RtcMessage
{
public:
	enum ErrType
	{
		kErrNoError,
		kErrAudio,
		kErrVideo,
		kErrServer
	};

	static const char *kErrAudioDesc;
	static const char *kErrVideoDesc;
	static const char *kErrServerDesc;

public:
	RtcInitResult(const QString &id);

	bool isOK() const;
	void setOK(bool ok);

	QString desc() const;
	void setDesc(const QString &desc);

	static ErrType errTypeFromDesc(const QString &desc);

	virtual QString toJson() const;
	virtual bool fromJson(const QString &json);

private:
	bool    m_ok;
	QString m_desc;
};

class RtcOffer : public RtcMessage
{
public:
	RtcOffer(const QString &id);
};

class RtcData : public RtcMessage
{
public:
	RtcData(const QString &id);

	QString data() const;
	void setData(const QString &data);

	virtual QString toJson() const;
	virtual bool fromJson(const QString &json);

private:
	QString m_data;
};

class RtcState : public RtcMessage
{
public:
	enum State
	{
		Uninited,
		Inited,
		Connecting,
		Connected,
		Error
	};

public:
	RtcState(const QString &id);

	State state() const;
	void setState(State s);

	QString desc() const;
	void setDesc(const QString &desc);

	static QString state2Str(State s);
	static State str2State(const QString &str);

	virtual QString toJson() const;
	virtual bool fromJson(const QString &json);

private:
	State   m_state;
	QString m_desc;
};

class RtcStatistics : public RtcMessage
{
public:
	RtcStatistics(const QString &id);

	bool hasAudio() const;
	bool hasVideo() const;

	void setHasAudio(bool audio);
	void setHasVideo(bool video);

	void audioStatistics(int &packagesLost, int &packagesReceived);
	void videoStatistics(int &packagesLost, int &packagesReceived);

	void setAudioStatistics(int packagesLost, int packagesReceived);
	void setVideoStatistics(int packagesLost, int packagesReceived);

	virtual QString toJson() const;
	virtual bool fromJson(const QString &json);

private:
	bool m_hasAudio;
	bool m_hasVideo;
	int  m_audioPackagesLost;
	int  m_audioPackagesReceived;
	int  m_videoPackagesLost;
	int  m_videoPackagesReceived;
};

class RtcCmd : public RtcMessage
{
public:
	enum ChangeType
	{
		ChangeNone,
		AudioSend,
		VideoSend,
		PlayVolume,
	};
public:
	RtcCmd(const QString &id);

	ChangeType changeType() const;
	void setChangeType(ChangeType type);

	QString changeValue() const;
	void setChangeValue(const QString &v);

	static ChangeType str2Type(const QString &str);
	static QString type2Str(ChangeType type);

	virtual QString toJson() const;
	virtual bool fromJson(const QString &json);

private:
	ChangeType m_changeType;
	QString    m_changeValue;
};

class RtcCmdResult : public RtcCmd
{
public:
	RtcCmdResult(const QString &id);
};

class RtcBye : public RtcMessage
{
public:
	RtcBye(const QString &id);
};

#endif // __RTC_MESSAGE_H__
