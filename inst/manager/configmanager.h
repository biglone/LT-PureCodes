#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include "pmclient/PmClientInterface.h"
#include <QByteArray>
#include <QStringList>

class ConfigManager : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	ConfigManager(QObject *parent = 0);
	~ConfigManager();

public:
	void getConfig(const QList<int> &configNums);
	void setConfig1(const QStringList &groupNames);
	void setConfig2(const QStringList &ids);
	void setConfig3(const QStringList &silenceList);

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

Q_SIGNALS:
	void config1GotOk(const QStringList &groupNames);
	void config2GotOk(const QStringList &ids);
	void config3GotOk(const QStringList &silentList);
	void configError();

private:
	void processConfig(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

	QByteArray encodeGroupNames(const QStringList &groupNames);
	QStringList decodeGroupNames(const QByteArray &groupNamesText);

private:
	int m_nHandleId;
};

#endif // CONFIGMANAGER_H
