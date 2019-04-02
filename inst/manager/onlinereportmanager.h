#ifndef ONLINEREPORTMANAGER_H
#define ONLINEREPORTMANAGER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include "pmclient/PmClientInterface.h"

class OnlineReportManager : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	OnlineReportManager(QObject *parent = 0);
	~OnlineReportManager();

public slots:
	void start(const QString &id);
	void stop();
	void reportOnline();
	void reportOffline();

Q_SIGNALS:
	void reportOnlineOK();
	void reportOnlineFailed();

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private:
	void processReportOnline(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

private:
	int     m_nHandleId;
	QTimer  m_timer;
	QString m_id;
};

#endif // ONLINEREPORTMANAGER_H
