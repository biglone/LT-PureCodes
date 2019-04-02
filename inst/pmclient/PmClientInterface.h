#ifndef _PMCLIENTINTERFACE_H_
#define _PMCLIENTINTERFACE_H_

#include <QList>

namespace net
{
	class Request;
};

namespace protocol
{
	class Response;
	class SpecificNotification;
};

class QObject;
// Request的返回结果的回调
class IPmClientResponseHandler
{
public:
	virtual bool initObject() = 0;
	virtual void removeObject() = 0;
	virtual QObject* instance() = 0;
	virtual QList<int> types() const = 0;
	virtual int handledId() const = 0;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res) = 0;
};

// Notification的回调
class IPmClientNotificationHandler
{
public:
	virtual bool initObject() = 0;
	virtual void removeObject() = 0;
	virtual QObject* instance() = 0;
	virtual QList<int> types() const = 0;
	virtual int handledId() const = 0;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn) = 0;
};

Q_DECLARE_INTERFACE(IPmClientResponseHandler,"Pm.Pmclient.IPmclientResponseHander/1.0");
Q_DECLARE_INTERFACE(IPmClientNotificationHandler,"Pm.Pmclient.IPmClientNotificationHandler/1.0");
#endif
