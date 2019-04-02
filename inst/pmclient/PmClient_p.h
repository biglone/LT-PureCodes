#ifndef _PMCLIENT_P_H_
#define _PMCLIENT_P_H_

#include <QtGlobal>
#include <QMap>
#include <QMultiMap>
#include <QNetworkConfigurationManager>
#include "base/Base.h"
#include "net/ICallback.h"
#include "net/TcpClient.h"
#include "PmClient.h"
#include "logger/loggable.h"
#include "Constants.h"

namespace net
{
	class IProtocolCallback;
	class Request;
	class RemoteRequest;
	class RemoteNotification;
}

class IPmClientResponseHandler;
class IPmClientNotificationHandler;
class PmClient;
class RequestHandle;

class PmClientPrivate : net::ICallback, ILoggable
{
	Q_DECLARE_PUBLIC(PmClient)
public:
	PmClientPrivate();
	virtual ~PmClientPrivate();

public:
	// ICallback  --------------------------------------------------------
	// 得到协议回调器
	virtual net::IProtocolCallback* getProtocolCallback();

	// TCP 连接动作结束的结果报告
	virtual void onConnect(bool bConnect);

	// 加密动作结束的结果报告,如果没有要求加密,可以不回调
	virtual void onEncrypt(bool bEncrypt);

	// TCP 连接断开时间提示
	virtual void onBroken();

	// TCP 连接是否断开
	virtual bool isBroken() const;

	// XML 解析错误
	virtual void onDataParseError();

	// request 信令结果报告,超时,应答
	virtual void onRequestResult(net::Request* request);

	// notification 信令报告
	virtual void onNotification(net::RemoteNotification* notification);

	// request 信令报告
	virtual void onRequest(net::RemoteRequest* remoteRequest);

	// 内部错误,主要是线程停止等
	virtual void onInternalError();

	// ILoggable
	virtual void debug(const char *message);
	virtual void info(const char *message);
	virtual void warning(const char *message);
	virtual void logReceived(const char *message);
	virtual void logSent(const char *message);

public:
	void _q_onNotification(void* sn);
	void processNotification(protocol::SpecificNotification* sn);

	void _q_onResHandlerDestroyed(QObject* obj);
	void _q_onNtfHandlerDestroyed(QObject* obj);

public:
	// 主线程处理TcpClient内部错误函数
	void _q_onInternalError();

	void _q_aboutClose();

	void _q_openError();

	void _q_onTcpClientError(const QString& err);

	void _q_onOnlineStateChanged(bool online);

public:
	base::Address                   m_ServAddress;
	EncryptType                     m_EncryptType;
	QString                         m_caFile;
	QString                         m_certFile;
	QString                         m_keyFile;

	bool                            m_bOpened;
	net::TcpClient                  m_TcpClient;

	QString                         m_sId;
	QString                         m_sName;

	PmClient*                       q_ptr;

	QMap<int, IPmClientResponseHandler*>       m_mapResHandlers;         // Response
	QMap<int, IPmClientNotificationHandler*>   m_mapNtfHandlers;         // Notification

	QMultiMap<int, int>                        m_mapResTypesId;          // Req Type -> handle id
	QMultiMap<int, int>                        m_mapNtfTypesId;          // Ntf type -> handle id;



	bool                            m_bBroken;

	QNetworkConfigurationManager    m_networkMgr;
	bool                            m_networkOnline;
};
#endif