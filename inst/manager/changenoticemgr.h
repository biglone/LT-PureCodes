#ifndef CHANGENOTICEMGR_H
#define CHANGENOTICEMGR_H

#include <QObject>
#include <QList>
#include "pmclient/PmClientInterface.h"
#include "protocol/ChangeNoticeNotification.h"

class ChangeNoticeMgr : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT

	Q_INTERFACES(IPmClientNotificationHandler);

	struct EventParam {
		QString v;
	};

	struct Event {
		QString             name;
		EventParam          param;

		Event() {}
	};

public:
	ChangeNoticeMgr(QObject *parent = 0);
	~ChangeNoticeMgr();

	void postGroupChangeNotice(const QString &groupId, const QString &changeType);
	void postDiscussChangeNotice(const QString &discussId, const QString &changeType);

Q_SIGNALS:
	void groupChangeNotice(const QString &param);
	void discussChangeNotice(const QString &param);
	void rosterAddNotice(int action, const QString &param);
	void rosterAddResponded(const QString &param);
	void hasSubscriptionMsg();
	void subscriptionSubscribed(const QString &param);
	void subscriptionUnsubscribed(const QString &param);
	void deleteFriend(const QString &id);
	void secretAckRecved(const QString &fromUid, const QString &stamp, int readState);
	void secretAcked(const QString &toUid, const QString &stamp);
	void configChanged(const QString &param);
	void passwdModified();
	void userDeleted(const QString &id);
	void userFrozen(const QString &id);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private slots:
	void processChangeNotice(const QList<ChangeNoticeMgr::Event> &changeEvents);

private:
	void processDeleteFriend(const QString &param);
	void processSecretAck(const QString &param);
	void processSecretAcked(const QString &param);
	void processDeleteUser(const QString &param);
	void processFreezeUser(const QString &param);

private:
	int m_nHandleId;
};

#endif // CHANGENOTICEMGR_H
