#ifndef RTCSESSION_H
#define RTCSESSION_H

#include <QObject>
#include <QPointer>
#include <QTimer>
#include "rtcsessionglobal.h"

class SharedBuffer;
namespace session
{
	class VideoFrameCB;
}

namespace rtcsession
{
	class RtcSessionManager;

	class Session : public QObject
	{
		Q_OBJECT
		Q_DISABLE_COPY(Session);

	public:
		enum PEER
		{
			Peer_A,
			Peer_B
		};

		enum SessionState
		{
			State_Init,
			State_Invite,
			State_Ring,
			State_Connect,
			State_Setup,
			State_End
		};

		enum SessionError
		{
			Error_NoError,
			Error_InviteTimeout,
			Error_RingingTimeout,
			Error_InviteConflict,
			Error_NoCamera,
			Error_CameraOpened,
			Error_AckTimeout,
			Error_DataTimeout,
			Error_AudioDeviceError,
			Error_PhoneHandle,
			Error_RtcServiceError
		};

		enum CloseType
		{
			CloseNormal,
			CloseError,
			CloseBusy,
			CloseUnsupport,
			CloseOther
		};

	public:
		explicit Session(PEER peer, const QString &from, const QString &to, const QString &sid, RtcSessionManager *sessionManager, QObject *parent = 0);
		virtual ~Session();

		static CloseType str2CloseType(const QString &str);
		static QString closeType2Str(CloseType closeType);

		QString sid() const;
		QString from() const;
		QString to() const;
		void setSp(const QString &sp);
		QString sp() const;
		SessionType type() const;
		Session::PEER peer() const;
		SessionError sessionError() const;
		SessionMediaParam selfMediaParam() const;
		SessionMediaParam otherMediaParam() const;

		void setAudioVolume(int vol);

		void setAudioSendEnable(bool enable);
		void setVideoSendEnable(bool enable);

		void onAudioSendChanged(bool send);
		void onVideoSendChanged(bool send);

		session::VideoFrameCB * otherVideoFrameCB() const;

		bool invite(SessionType type);
		bool ringing();
		void ok();
		void reject(CloseType closeType, const QString &desc = "");
		void bye(CloseType closeType, const QString &desc = "");
		void close(CloseType closeType, const QString &desc = "");
		void conflict();
		void serviceError();

		CloseType otherCloseType() const;
		QString otherCloseDesc() const;

		void setAudioStatistics(int packageLost, int packageReceived);
		void setVideoStatistics(int packageLost, int packageReceived);

	Q_SIGNALS:
		void onOk();
		void onRejected(const QString &reasonType, const QString &reasonDesc);
		
		void aboutClose(const QString &sid);
		void error(int errCode);

		void stateChanged(int ctrlPeer, int curState, int oldState);

		void audioSendChanged(bool send);
		void videoSendChanged(bool send);

		void otherAudioChanged(bool before, bool now);
		void otherVideoChanged(bool before, bool now);

		void packageStatistics(int packageLost, int packageReceived);

	public:
		void recvInvite(bool hasVideo, bool audioSend, bool audioRecv, bool videoSend, bool videoRecv);
		void recvRinging();
		void recvOk(bool audioSend, bool audioRecv, bool videoSend, bool videoRecv);
		void recvReject(const QString &reasonType, const QString &reasonDesc);
		void recvAck();
		void recvModify(bool audioSend, bool audioRecv, bool videoSend, bool videoRecv);
		void recvBye(const QString &reasonType, const QString &reasonDesc);
		void recvNotify(const QString &toFullId, const QString &action);
		
		void inviteFinished(bool ok, SessionError err);
		void okFinished(bool ok, SessionError err);
		void startRecvOtherVideo();

	private Q_SLOTS:
		void onInviteTimeout();
		void onRingingTimeout();
		void onPlayInviteBeep();
		void onAckTimeout();
		void doAboutClose();
		void onOtherVideoFrame();
		
	private:
		void setState(PEER ctrlPeer, SessionState s);
		void setError(SessionError err);

		void startOtherVideo();
		void stopOtherVideo();

	private:
		SessionType                 m_type;
		SessionState                m_state;
		PEER                        m_peer;
		QString                     m_to;
		QString                     m_from;
		QString                     m_sid;
		QString                     m_sp;
		
		SessionMediaParam           m_selfMedia;
		SessionMediaParam           m_otherMedia;

		QPointer<RtcSessionManager> m_rtcSessionManager;

		// invite timer: PEER_A
		QScopedPointer<QTimer> m_pInviteTimer;
		// ringing timer: PEER_B
		QScopedPointer<QTimer> m_pRingingTimer;
		// invite beep timer:  PEER_A / PEER_B
		QScopedPointer<QTimer> m_pInviteBeepTimer;
		// ack timer: PEER_B
		QScopedPointer<QTimer> m_pAckTimer;

		SessionError           m_errCode;

		QScopedPointer<session::VideoFrameCB> m_otherVideoFrameCB;
		QScopedPointer<SharedBuffer>          m_otherBuffer;
		QTimer                                m_otherVideoTimer;
		bool                                  m_otherVideoStarted;

		CloseType m_otherCloseType;
		QString   m_otherCloseDesc;

		int m_audioPackageLost;
		int m_audioPackageReceived;
		int m_videoPackageLost;
		int m_videoPackageReceived;
	};

}

#endif // RTCSESSION_H
