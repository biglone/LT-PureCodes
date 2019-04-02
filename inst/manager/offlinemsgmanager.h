#ifndef OFFLINEMSGMANAGER_H
#define OFFLINEMSGMANAGER_H

#include <QObject>
#include "pmclient/PmClientInterface.h"
#include <QString>
#include <QList>
#include <bean/MessageBody.h>

class OfflineMsgManager : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	enum FromType
	{
		// do not change the value !!! -- same to protocol::OfflineResponse::ItemType
		User,
		Group,
		Discuss
	};

	enum MsgDirection
	{
		// do not change the value !!! -- same to protocol::HistoryRequest::Direction
		Backward,
		Forward
	};

	struct OfflineItem
	{
		FromType m_type;
		QString  m_from;
		int      m_count;         // all count, including sync-message count 
		int      m_offlineCount;  // off-line message count
		QString  m_ts;
	};

	static const int PAGE_MSG_COUNT = 20;

public:
	OfflineMsgManager(QObject *parent = 0);
	~OfflineMsgManager();

	void clear();
	QList<OfflineItem> offlineItems() const;
	bool containOfflineItem(FromType fType, const QString &fromId) const;
	int offlineMsgCount(FromType fType, const QString &fromId) const;
	void clearOfflineMsgCount(FromType fType, const QString &fromId);
	int syncMsgCount(FromType fType, const QString &fromId) const;
	bool clearOfflineItem(FromType fType, const QString &fromId);

	static bean::MessageType offlineFromType2MessageType(FromType fromType);
	static FromType messageType2OfflineFromType(bean::MessageType msgType);

public slots:
	void requestOfflineMsg(const QString &ts);
	void requestOfflineSyncMsg();
	bool requestHistoryMsg(FromType fType, const QString &fromId);
	void reportMaxTs(const QString &maxTs);

signals:
	void offlineRecvOK();
	void historyMsgRecvOK(int fType, const QString &id, const bean::MessageBodyList &messages, bool offline);
	void historyMsgRecvFailed(int fType, const QString &id);
	void offlineChanged();
	void reportTsOk();
	void reportTsFailed();

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private slots:
	void offlineFailed();
	void offlineFinished();
	void offlineSyncFinished();
	void historyMsgReceived(int fType, const QString &bareFromId, int number, const bean::MessageBodyList &messages);
	void historyMsgFailed(int fType, const QString &bareFromId);

private:
	void processOfflineMsg(net::Request* req, protocol::Response* res);
	void processHistoryMsg(net::Request* req, protocol::Response* res);
	void processReportTs(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

private:
	int                m_nHandleId;
	QString            m_ts;
	QString            m_tsNow;
	QString            m_ts2;
	QList<OfflineItem> m_offlineItems;
};

#endif // OFFLINEMSGMANAGER_H
