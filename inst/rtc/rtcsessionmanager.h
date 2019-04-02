#ifndef RTCSESSIONMANAGER_H
#define RTCSESSIONMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QMap>
#include <QStringList>
#include "rtcsessionglobal.h"
#include "rtcsession.h"
#include "messageparser.h"
#include <QTimer>

class RtcLocalClient;
class SharedBuffer;
class RtcMessage;

namespace session
{
	class VideoFrameCB;
}

namespace rtcsession
{
	class RtcManager;

	class RtcSessionManager : public QObject, public MessageParserObserver
	{
		Q_OBJECT

	public:
		RtcSessionManager(QObject *parent = 0);
		~RtcSessionManager();

		void initObject();
		void removeObject();

		void setIceServerInfo(const QStringList &iceUrls, const QString &iceUsername, const QString &iceCredential);
		void init();
		void release();

		Session * createSession(Session::PEER peer, const QString &from, const QString &to, 
			const QString &sessionId = QString());
		Session * session(const QString &sid);
		void prepareInvite(const Session *s);
		void prepareOk(const Session *s);

		bool hasVideoSession() const;
		bool hasSession() const;

		bool hasSessionWithUid(const QString &uid) const;
		Session *sessionWithUid(const QString &uid) const;

		RtcManager *rtcManager() const;
		session::VideoFrameCB *selfVideoFrameCB() const;

		void sendInitMsg(const Session *s);
		void sendOfferMsg(const Session *s);
		void sendDataMsg(const Session *s, const QString &data);
		void sendCmdAudioMsg(const Session *s, bool audioSend);
		void sendCmdVideoMsg(const Session *s, bool videoSend);
		void sendCmdVolumeMsg(const Session *s, int volume);
		void sendByeMsg(const Session *s);

	public: // From MessageParserObserver
		void onMessage(const QString &parserId, const QString &id, const QString &typeStr, const QString &json);
		void onError(const QString &parserId, const QString &desc);

	Q_SIGNALS:
		void recvInvite(const QString &sid, const QString &from);

	private Q_SLOTS:
		void onInviteRecved(const SessionParam &sessionParam);
		void onRingingRecved(const SessionParam &sessionParam);
		void onOkRecved(const SessionParam &sessionParam);
		void onRejectRecved(const SessionParam &sessionParam, const QString &reasonType, const QString &reasonDesc);
		void onAckRecved(const SessionParam &sessionParam);
		void onByeRecved(const SessionParam &sessionParam, const QString &reasonType, const QString &reasonDesc);
		void onModifyRecved(const SessionParam &sessionParam);
		void onNotifyRecved(const QString &sid, const QString &toFullId, const QString &action);
		void onSdpRecved(const SessionParam &sessionParam, const QString &sdp);
		void onIceCandidateRecved(const SessionParam &sessionParam, const QString &iceCandidate);

		void onRtcInitFinished(const QString &sid, bool result, int sessionErr);
		
		void deleteSession(const QString &sid);
		void closeAllSessions();

		// slots to local client
		void onDataRecved(const QByteArray &msg);
		void onClientConnected();
		void onClientDisconnected();
		void startClientConnect();
		void connectClientSync();

		void selfVideoFrame();

	private:
		QString createId() const;
		bool checkConflict(const QString &from);
		QString getOtherSideUid(const QString &from, const QString &to);

		void processInitResultMsg(RtcMessage *msg);
		void processStateMsg(RtcMessage *msg);
		void processDataMsg(RtcMessage *msg);
		void processStatisticsMsg(RtcMessage *msg);
		void processCmdResultMsg(RtcMessage *msg);

		void startRecvSelfVideo(const QString &sid);
		void stopRecvSelfVideo(const QString &sid);

	private:
		QScopedPointer<RtcManager> m_rtcManager;
		
		QScopedPointer<RtcLocalClient>        m_rtcLocalClient;
		QScopedPointer<MessageParser>         m_messageParser;
		QScopedPointer<SharedBuffer>          m_selfBuffer;
		QScopedPointer<session::VideoFrameCB> m_selfFrameCB;
		int                                   m_connectCount;

		QMap<QString, Session*> m_mapSession; // sid <-> session
		QMap<QString, QString>  m_mapUid2Sid; // uid <-> sid

		QStringList m_iceUrls;
		QString     m_iceUsername;
		QString     m_iceCredential;

		QStringList m_selfVideoSids;
		QTimer      m_selfVideoTimer;
	};

}

#endif // RTCSESSIONMANAGER_H
