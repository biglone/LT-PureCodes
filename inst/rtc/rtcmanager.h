#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include <QObject>
#include "pmclient/PmClientInterface.h"
#include "rtcsessionglobal.h"

class PmClient;

namespace rtcsession
{

	class Session;

	class RtcManager : public QObject, public IPmClientNotificationHandler
	{
		Q_OBJECT
		Q_INTERFACES(IPmClientNotificationHandler);

	public:
		RtcManager(QObject *parent);
		~RtcManager();

		QString myUid() const;

		bool sendInvite(Session *s);
		bool sendRinging(Session *s);
		bool sendOk(Session *s);
		bool sendReject(Session *s, const QString &reasonType, const QString &reasonDesc);
		bool sendAck(Session *s);
		bool sendBye(Session *s, const QString &reasonType, const QString &reasonDesc);
		bool sendModify(Session *s);
		bool sendReport();
		bool sendSdp(Session *s, const QString &sdp);
		bool sendIceCandidate(Session *s, const QString &iceCandidate);

	Q_SIGNALS:
		void inviteRecved(const SessionParam &param);
		void ringingRecved(const SessionParam &param);
		void okRecved(const SessionParam &param);
		void rejectRecved(const SessionParam &param, const QString &reasonType, const QString &reasonDesc);
		void ackRecved(const SessionParam &param);
		void byeRecved(const SessionParam &param, const QString &reasonType, const QString &reasonDesc);
		void modifyRecved(const SessionParam &param);
		void notifyRecved(const QString &sid, const QString &toFullId, const QString &action);
		void sdpRecved(const SessionParam &param, const QString &sdp);
		void iceCandidateRecved(const SessionParam &param, const QString &iceCandidate);

	public:
		virtual bool initObject();
		virtual void removeObject();
		virtual QObject* instance();
		virtual int handledId() const;
		virtual QList<int> types() const;
		virtual bool onNotication(int handleId, protocol::SpecificNotification *sn);

	private Q_SLOTS:
		void processInvite(void *pValue);
		void processRinging(void *pValue);
		void processOk(void *pValue);
		void processReject(void *pValue);
		void processAck(void *pValue);
		void processBye(void *pValue);
		void processModify(void *pValue);
		void processReport(void *pValue);
		void processNotify(const QString &sId, const QString &toFullId, const QString &action);
		void processSdp(void *pValue);
		void processIceCandidate(void *pValue);

	private:
		PmClient *m_pmClient;
		int       m_nHandleId;
	};

}

#endif // RTCMANAGER_H
