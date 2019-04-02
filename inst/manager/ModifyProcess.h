#ifndef MODIFYPROCESS_H
#define MODIFYPROCESS_H

#include <QObject>
#include <QMap>
#include "pmclient/PmClientInterface.h"

class ModifyProcess : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);
public:
	explicit ModifyProcess(QObject* parent = 0);
	~ModifyProcess();

	bool sendModify(const QMap<int, QVariant>& vals);

public:
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

Q_SIGNALS:
	void finish();
	void error(const QString& errMsg);

private:
	int  m_handleId;
};

#endif // MODIFYPROCESS_H
