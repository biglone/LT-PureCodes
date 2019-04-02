#ifndef PMCLIENT_H
#define PMCLIENT_H

#include <QObject>
#include <QScopedPointer>
#include "base/Base.h"
#include "net/IProtocolCallback.h"
#include "Constants.h"

namespace net
{
	class Request;
	class XmlMsg;
}

class IPmClientResponseHandler;
class IPmClientNotificationHandler;

class PmClientPrivate;
class PmClient : public QObject, net::IProtocolCallback
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(PmClient)
	Q_DISABLE_COPY(PmClient)

public:
	explicit PmClient(QObject *parent = 0);
	~PmClient();

	static PmClient* instance();

public:
	bool isOpened() const;

	QString id() const;
	QString name() const;
	void setId(const QString& rsId);
	void setName(const QString& rsName);

	void setEncryptType(EncryptType encryptType);
	void setOpensslCertFiles(const QString &caFile, const QString &certFile = QString(), const QString &keyFile = QString());

	void setAddress(base::Address address);
	base::Address address();

	bool setHeartbeat(bool bOpen);

public:
	bool send(net::Request* request, bool bEmergency = true);
	bool send(net::XmlMsg* xmlMsg, bool bEmergency = true);

	bool cancel(net::Request* request);
	bool cancel(const std::string& rsSeq);

	int insertResponseHandler(IPmClientResponseHandler* handler);
	void removeResponseHandler(int handleId);

	int insertNotificationHandler(IPmClientNotificationHandler* handler);
	void removeNotificationHandler(int handleId);

Q_SIGNALS:
	void opened();
	void closed();
	void aboutOpen();
	void aboutClose();

	void error(const QString& rsError);

	void responseHandlerInserted(int handleId, IPmClientResponseHandler* handler);
	void responseHandlerRemoved(int handleId, IPmClientResponseHandler* handler);

	void notificationHandlerInserted(int handleId, IPmClientNotificationHandler* handler);
	void notificationHandlerRemoved(int handleId, IPmClientNotificationHandler* handler);

public slots:
	void open();
	void close();

	void startHeartbeat();

private:
	// net::IProtocolCallback  work thread --------------------------------------------------------
	// 请求有应答,1.res==null(请求超时)，
	// 2.res!=null(a.可能是正常应答,b.可能是类似errcode+errmsg形式应答,c.甚至有可能协议错误)
	virtual void onResponse(net::Request* req, protocol::Response* res);
	// 通知消息
	virtual void onNotification(protocol::SpecificNotification* sn);

private:
	static PmClient* self;
	QScopedPointer<PmClientPrivate> d_ptr;

	Q_PRIVATE_SLOT(d_func(), void _q_onInternalError())
	Q_PRIVATE_SLOT(d_func(), void _q_aboutClose())
	Q_PRIVATE_SLOT(d_func(), void _q_openError())
	Q_PRIVATE_SLOT(d_func(), void _q_onTcpClientError(QString))
	//Q_PRIVATE_SLOT(d_func(), void _q_onResponse(void*, void*))
	Q_PRIVATE_SLOT(d_func(), void _q_onNotification(void*))
	Q_PRIVATE_SLOT(d_func(), void _q_onResHandlerDestroyed(QObject* obj))
	Q_PRIVATE_SLOT(d_func(), void _q_onNtfHandlerDestroyed(QObject* obj))
	Q_PRIVATE_SLOT(d_func(), void _q_onOnlineStateChanged(bool));
};

#endif // PMCLIENT_H
