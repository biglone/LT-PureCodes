#include <QDebug>
#include <assert.h>
#include <QMetaObject>
#include <QTimer>
#include <QMapIterator>

#include "cttk/base.h"

#include "net/IProtocolCallback.h"
#include "net/RemoteRequest.h"
#include "net/Request.h"
#include "protocol/ProtocolType.h"
#include "protocol/Response.h"
#include "protocol/SpecificNotification.h"
#include "protocol/NotificationFactory.h"

#include "PmClient_p.h"
#include "PmClient.h"
#include "PmClientInterface.h"

#include "logger/logger.h"

const char *PROMPT_PROTOCAL_ERROR = QT_TRANSLATE_NOOP("PmClient", "protocol error");   //  "数据错误"
const char *PROMPT_REQ_FAIL       = QT_TRANSLATE_NOOP("PmClient", "request failed");   //  "请求失败"
const char *PROMPT_TIMEOUT        = QT_TRANSLATE_NOOP("PmClient", "response timeout"); //  "响应超时"
const char *PROMPT_ERR_MESSAGE    = QT_TRANSLATE_NOOP("PmClient", "server error");     //  "服务错误"

// private
PmClientPrivate::PmClientPrivate()
: m_bOpened(false)
, q_ptr(0)
, m_bBroken(false)
{
	m_networkOnline = m_networkMgr.isOnline();
	m_TcpClient.setLoggable(this);
}

PmClientPrivate::~PmClientPrivate()
{
	if (m_bOpened)
	{
		m_TcpClient.uninitialize();
	}
	m_mapResHandlers.clear();
	m_mapNtfHandlers.clear();
}

net::IProtocolCallback* PmClientPrivate::getProtocolCallback()
{
	return q_func();
}

void PmClientPrivate::onConnect(bool bConnect)
{
	qWarning() << Q_FUNC_INFO << " bConnect " << bConnect;

	m_bBroken = false;
	m_bOpened = bConnect;

	if (m_bOpened)
	{
		QMetaObject::invokeMethod(q_func(), "opened", Qt::QueuedConnection);
	}
	else
	{
		QMetaObject::invokeMethod(q_func(), "_q_openError", Qt::QueuedConnection);
	}
}

void PmClientPrivate::onEncrypt(bool bEncrypt)
{
	Q_UNUSED(bEncrypt);
}

void PmClientPrivate::onBroken()
{
 	m_bBroken = true;
	QString sError = QObject::tr("Connection broken");
	QMetaObject::invokeMethod(q_func(), "_q_onTcpClientError", Qt::QueuedConnection, Q_ARG(QString, sError));
}

bool PmClientPrivate::isBroken() const
{
	return m_bBroken;
}

void PmClientPrivate::onDataParseError()
{
	m_bBroken = true;
	QString sError = QObject::tr("Protocol data parse error");
	QMetaObject::invokeMethod(q_func(), "_q_onTcpClientError", Qt::QueuedConnection, Q_ARG(QString, sError));
}

void PmClientPrivate::onRequestResult(net::Request* request)
{
	request->onResult();
}

void PmClientPrivate::onNotification(net::RemoteNotification* notification)
{
	assert(q_func() != NULL);

	protocol::SpecificNotification* sn = protocol::NotificationFactory::Create(notification);
	if (!sn)
		return;

	q_func()->onNotification(sn);
}

void PmClientPrivate::onRequest(net::RemoteRequest* remoteRequest)
{
	qWarning() << Q_FUNC_INFO << " " << QString::fromUtf8(remoteRequest->getBuffer().c_str());
}

void PmClientPrivate::onInternalError()
{
	QMetaObject::invokeMethod(q_func(), "_q_onInternalError", Qt::QueuedConnection);
}

void PmClientPrivate::debug(const char *message)
{
	QString msg = QString::fromUtf8(message);
	QMetaObject::invokeMethod(Logger::getLogger(), "debug", Qt::QueuedConnection, Q_ARG(QString, msg));
}

void PmClientPrivate::info(const char *message)
{
	QString msg = QString::fromUtf8(message);
	QMetaObject::invokeMethod(Logger::getLogger(), "info", Qt::QueuedConnection, Q_ARG(QString, msg));
}

void PmClientPrivate::warning(const char *message)
{
	QString msg = QString::fromUtf8(message);
	QMetaObject::invokeMethod(Logger::getLogger(), "warning", Qt::QueuedConnection, Q_ARG(QString, msg));
}

void PmClientPrivate::logReceived(const char *message)
{
	QString msg = QString::fromUtf8(message);
	QMetaObject::invokeMethod(Logger::getLogger(), "logReceived", Qt::QueuedConnection, Q_ARG(QString, msg));
}

void PmClientPrivate::logSent(const char *message)
{
	QString msg = QString::fromUtf8(message);
	QMetaObject::invokeMethod(Logger::getLogger(), "logSent", Qt::QueuedConnection, Q_ARG(QString, msg));
}

void PmClientPrivate::_q_onNotification(void* sn)
{
	protocol::SpecificNotification* pSn = reinterpret_cast<protocol::SpecificNotification*>(sn);
	processNotification(pSn);

	SAFE_DELETE(pSn);
}

void PmClientPrivate::processNotification(protocol::SpecificNotification* sn)
{
	bool bHook = false;
	QMapIterator<int, IPmClientNotificationHandler*> it(m_mapNtfHandlers);

	while (!bHook && it.hasNext())
	{
		it.next();
		bHook = it.value()->onNotication(it.key(), sn);
	}
}

void PmClientPrivate::_q_onResHandlerDestroyed(QObject* obj)
{
	foreach(int id, m_mapResHandlers.keys())
	{
		IPmClientResponseHandler *handler = m_mapResHandlers.value(id);
		if (handler->instance() == obj)
		{
			q_func()->removeResponseHandler(id);
			break;
		}
	}
}

void PmClientPrivate::_q_onNtfHandlerDestroyed(QObject* obj)
{
	foreach(int id, m_mapNtfHandlers.keys())
	{
		IPmClientNotificationHandler *handler = m_mapNtfHandlers.value(id);
		if (handler->instance() == obj)
		{
			q_func()->removeNotificationHandler(id);
			break;
		}
	}
}

void PmClientPrivate::_q_onInternalError()
{
	_q_onTcpClientError(QObject::tr("Connect error, please check network"));
}

void PmClientPrivate::_q_aboutClose()
{
	if (m_bOpened)
	{
		m_TcpClient.setHeartbeat(false);
		m_TcpClient.uninitialize();
		m_bOpened = false;
	}

	emit q_func()->closed();
}

void PmClientPrivate::_q_openError()
{
	QString sError = QObject::tr("Disconnect from server");
	_q_onTcpClientError(sError);
	return;
}

void PmClientPrivate::_q_onTcpClientError(const QString& err)
{
	if (m_bOpened)
	{
		m_TcpClient.setHeartbeat(false);
		m_bOpened = false;
	}
	m_TcpClient.uninitialize();
	emit q_func()->error(err);
}

void PmClientPrivate::_q_onOnlineStateChanged(bool online)
{
	if (!online && m_networkOnline != online)
	{
		_q_onInternalError();
	}
	m_networkOnline = online;
}


//  PmClient --------------------------------------------------------------
PmClient* PmClient::self = 0;

PmClient::PmClient(QObject *parent)
	: QObject(parent)
	, d_ptr(new PmClientPrivate)
{
	d_func()->q_ptr = this;
	connect(&(d_func()->m_networkMgr), SIGNAL(onlineStateChanged(bool)), SLOT(_q_onOnlineStateChanged(bool)));
	self = this;
}

PmClient::~PmClient()
{
	qDebug() << Q_FUNC_INFO;

	self = 0;
}

PmClient* PmClient::instance()
{
	Q_ASSERT_X(self != NULL, __FUNCTION__, "self is null");
	return self;
}

bool PmClient::isOpened() const
{
	return d_func()->m_bOpened;
}

QString PmClient::id() const
{
	return d_func()->m_sId;
}

QString PmClient::name() const
{
	return d_func()->m_sName;
}

void PmClient::setId(const QString& rsId)
{
	d_func()->m_sId = rsId;
}

void PmClient::setName(const QString& rsName)
{
	d_func()->m_sName = rsName;
}

void PmClient::setEncryptType(EncryptType encryptType)
{
	Q_D(PmClient);

	if (d->m_bOpened)
		return;

	d->m_EncryptType = encryptType;
}

void PmClient::setOpensslCertFiles(const QString &caFile, const QString &certFile /*= QString()*/, const QString &keyFile /*= QString()*/)
{
	Q_D(PmClient);

	if (d->m_bOpened)
		return;

	d->m_caFile = caFile;
	d->m_certFile = certFile;
	d->m_keyFile = keyFile;
}

void PmClient::setAddress(base::Address address)
{
	Q_D(PmClient);

	if (d->m_bOpened)
		return;
	
	d->m_ServAddress = address;
}

base::Address PmClient::address()
{
	Q_D(PmClient);

	return d->m_ServAddress;
}

bool PmClient::setHeartbeat(bool bOpen)
{
	Q_D(PmClient);
	if (!d->m_bOpened)
		return false;

	return d->m_TcpClient.setHeartbeat(bOpen);
}

bool PmClient::send(net::Request* request, bool bEmergency /* = true */)
{
	Q_D(PmClient);
	if (!d->m_bOpened)
		return false;

	request->setBeginTime();
	return d->m_TcpClient.send(request, bEmergency);
}

bool PmClient::send(net::XmlMsg* xmlMsg, bool bEmergency /* = true */)
{
	Q_D(PmClient);
	if (!d->m_bOpened)
	{
		return false;
	}

	return d->m_TcpClient.send(xmlMsg, bEmergency);
}

bool PmClient::cancel(net::Request* request)
{
	Q_D(PmClient);
	if (!d->m_bOpened)
	{
		return false;
	}
	return d->m_TcpClient.cancel(request);
}

bool PmClient::cancel(const std::string& rsSeq)
{
	Q_D(PmClient);
	if (!d->m_bOpened)
	{
		return false;
	}
	return d->m_TcpClient.cancel(rsSeq);
}

int PmClient::insertResponseHandler(IPmClientResponseHandler* handler)
{
	Q_D(PmClient);
	if (handler && !handler->types().isEmpty())
	{
		static int handleId = 0;

		handleId++;
		while(handleId <= 0 || d->m_mapResHandlers.contains(handleId))
			handleId = (handleId > 0) ? handleId+1 : 1;

		d->m_mapResHandlers.insert(handleId, handler);

		foreach(int type, handler->types())
		{
			d->m_mapResTypesId.insert(type, handleId);
		}
		connect(handler->instance(),SIGNAL(destroyed(QObject *)),SLOT(_q_onResHandlerDestroyed(QObject *)));
		emit responseHandlerInserted(handleId, handler);

		qDebug() << Q_FUNC_INFO << "(" << ((void *)this) << ")";

		return handleId;
	}

	return -1;

}

void PmClient::removeResponseHandler(int handleId)
{
	Q_D(PmClient);

	if (d->m_mapResHandlers.contains(handleId))
	{
		IPmClientResponseHandler* handler = d->m_mapResHandlers.take(handleId);
		foreach(int type, handler->types())
		{
			d->m_mapResTypesId.remove(type, handleId);
		}
		emit responseHandlerRemoved(handleId, handler);

		qDebug() << Q_FUNC_INFO << " handle: " << handleId << " removed." << "(" << ((void *)this) << ")";
	}
}

int PmClient::insertNotificationHandler(IPmClientNotificationHandler* handler)
{
	Q_D(PmClient);

	if (handler && !handler->types().isEmpty())
	{
		static int handleId = 0;

		handleId++;
		while(handleId <= 0 || d->m_mapNtfHandlers.contains(handleId))
			handleId = (handleId > 0) ? handleId+1 : 1;

		d->m_mapNtfHandlers.insert(handleId, handler);

		foreach(int type, handler->types())
		{
			d->m_mapNtfTypesId.insert(type, handleId);
		}
		connect(handler->instance(),SIGNAL(destroyed(QObject *)),SLOT(_q_onResHandlerDestroyed(QObject *)));
		emit notificationHandlerInserted(handleId, handler);

		return handleId;
	}

	return -1;
}

void PmClient::removeNotificationHandler(int handleId)
{
	Q_D(PmClient);

	if (d->m_mapNtfHandlers.contains(handleId))
	{
		IPmClientNotificationHandler* handler = d->m_mapNtfHandlers.take(handleId);
		foreach(int type, handler->types())
		{
			d->m_mapNtfTypesId.remove(type, handleId);
		}
		emit notificationHandlerRemoved(handleId, handler);

		qDebug() << Q_FUNC_INFO << " handle: " << handleId << " removed.";
	}
}

void PmClient::open()
{
	Q_D(PmClient);

	emit aboutOpen();

	net::TcpClient::InitParam param;
	param.address = d->m_ServAddress;
	param.encryptType = (net::TcpClient::TypeEncrypt)(d->m_EncryptType);
	param.caFile = d->m_caFile.toUtf8().constData();
	param.certFile = d->m_certFile.toUtf8().constData();
	param.keyFile = d->m_keyFile.toUtf8().constData();
	param.pICallback = d;

	d->m_TcpClient.initialize(param);
}

void PmClient::close()
{
	emit aboutClose();

	QMetaObject::invokeMethod(this, "_q_aboutClose", Qt::DirectConnection);
}

void PmClient::startHeartbeat()
{
	Q_D(PmClient);
	d->m_TcpClient.setHeartbeat(true);
}

void PmClient::onResponse(net::Request* req, protocol::Response* res)
{
	Q_D(PmClient);

	// process error
	if (!res)
	{
		// 没有response错误
		switch (req->getInnerError())
		{
		case net::Request::NoError:
			break;
		case net::Request::TimeoutError:
			{
				int timeoutSecs = req->diffFromBeginTimeInSeconds();
				QString promptTimeout = QString("%1--%2s").arg(tr(PROMPT_TIMEOUT)).arg(timeoutSecs);
				req->setMessage(promptTimeout.toUtf8().constData());
			}
			break;
		case net::Request::TcpError:
			req->setMessage(tr(PROMPT_REQ_FAIL).toUtf8().constData());
			break;
		}
	}
	else
	{
		// 有response
		if (res->isError())
		{
			// response返回错误
			QString msg = QString("%1--%2").arg(tr(PROMPT_ERR_MESSAGE)).arg(QString::fromUtf8(res->getErrcode().c_str()));
			req->setMessage(msg.toUtf8().constData());
			qWarning() << "response error, type: " 
				       << req->getType()
					   << " code: "
					   << QString::fromUtf8(res->getErrcode().c_str())
					   << " errmsg: "
					   << QString::fromUtf8(res->getErrmsg().c_str());
		}
		else if (res->getPError())
		{
			// response信令解析出错
			req->setMessage(tr(PROMPT_PROTOCAL_ERROR).toUtf8().constData());
		}
		else
		{
			req->setResult(true);
		}
	}

	int type = req->getType();
	QList<int> lstHandles = d->m_mapResTypesId.values(type);

	bool bHandled = false;
	foreach (int handleId, lstHandles)
	{
		IPmClientResponseHandler* handle = d->m_mapResHandlers.value(handleId);
		if (!bHandled && handle)
		{
			bHandled = handle->onRequestResult(handleId, req, res);
		}
	}

	if (!bHandled)
	{
		qWarning() << Q_FUNC_INFO << " req[" << type << "] not handle" << " handles:" << d->m_mapResHandlers.keys();
	}

	SAFE_DELETE(req);
	SAFE_DELETE(res);
}

void PmClient::onNotification(protocol::SpecificNotification* sn)
{
	Q_D(PmClient);

	int type = sn->getNotificationType();

	QList<int> lstHandles = d->m_mapNtfTypesId.values(type);

	bool bHandled = false;
	foreach (int handleId, lstHandles)
	{
		IPmClientNotificationHandler* handle = d->m_mapNtfHandlers.value(handleId);
		if (!bHandled && handle)
		{
			bHandled = handle->onNotication(handleId, sn);
		}
	}

	if (!bHandled)
	{
		qWarning() << Q_FUNC_INFO << " req[" << type << "] not handle" << "handles: " << d->m_mapNtfHandlers.keys();;
	}


	SAFE_DELETE(sn);
}

#include <moc_PmClient.cpp>
