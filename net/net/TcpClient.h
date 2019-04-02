#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <map>
#include <list>
#include <string>
using namespace std;

#include <QThread>
#include "cttk/Mutex.h"
#include "iks/iksemel.h"
#include "base/Base.h"
#include "Request.h"
#include "logger/loggable.h"
#include "net_global.h"

class QTcpSocket;

namespace net
{

class Connecter;
class Sender;
class Recver;
class Keeper;

class ICallback;

// TCP 客户端,支持连接,加密,XML协议的实现,但不负责具体协议的定义分析
// 除了uninitialize函数,所有函数都不阻塞,函数的结果都通过回调来报告
class NET_EXPORT TcpClient
{
public:
	enum TypeEncrypt
	{
		TypeEncrypt_None = 0,
		// TypeEncrypt_RC4, -- omit
		TypeEncrypt_TLSV12 = 2,
		TypeEncrypt_TLSV11,
		TypeEncrypt_TLSV1,
		TypeEncrypt_SSLV23,
		TypeEncrypt_SSLV3,
		TypeEncrypt_SSLV2
	};

	struct InitParam
	{
		InitParam() : encryptType(TypeEncrypt_None), pICallback(0) {}

		base::Address                address;
		TypeEncrypt                  encryptType;
		std::string                  caFile;   // openssl ca file
		std::string                  certFile; // openssl client cert file
		std::string                  keyFile;  // openssl client key file
		ICallback*                   pICallback;
	};

public:
	TcpClient();
	~TcpClient();

	// 初始化 TcpClient,进行 TCP 连接,支持加密
	// remark:请调用者保证在有结果后才进行下一次调用,暂时不考虑这种错误的使用方法
	void initialize(InitParam initParam);

	// 阻塞,保证正确释放
	void uninitialize();

public:
	// 设置日志接口
	void setLoggable(ILoggable *loggable);

	// 获取日志接口
	ILoggable* loggable() const;

	// 发送一个协议信令 request
	bool send(Request* request, bool bEmergency);

	// 发送一个协议信令 notification response
	bool send(XmlMsg* xmlMsg, bool bEmergency);

	// 撤销一个 request 信令的请求,不再要求该 request 的结果被报告,不能确保 request 不被发送
	// return: true,操作成功; false,操作失败,还是会被报告
	bool cancel(Request* request);
	
	bool cancel(const std::string& rsSeq);

	/// 设置心跳
	bool setHeartbeat(bool bOpen);

private:
	bool sendHeartbeat();

	void checkHeartbeat();

	void cacheRequest(Request* request);

	Request* removeRequest(string seq);

	void checkTimeout();

	bool onElementPath(iks* ep);

	void onMessage(iks* message);

	void onHeartbeat(iks* heartbeat);

	void onSocketBroken();

	void onDataParseError();

	void clearNoResultRequest();

public:
	static const int                TIME_CHECK_INTERVAL; // 1 s
	static const int                HEARTBEAT_INTERVAL;  // 60 s
	static const int                HEARTBEAT_TIMEOUT;   // 20 s

private:
	InitParam                       m_Param;

	QThread                         m_thread; // Connecter, Sender and Recver running in this thread 
	
	QTcpSocket*                     m_pTcpSocket;

	Connecter*                      m_pConnecter;
	Sender*                         m_pSender;
	Recver*                         m_pRecver;
	Keeper*                         m_pKeeper;

	map<string, Request*>           m_mapRequest;
	cttk::CMutex                    m_mtxRequest;

	list<Request*>                  m_listTimeout;

	bool                            m_bCallback;
	
	enum Heartbeat_State
	{
		Heartbeat_Invalid,
		Heartbeat_Send,
		Heartbeat_Recv
	};
	Heartbeat_State                 m_eHeartbeatState;
	bool                            m_bStartHeartbeat;

	ILoggable*                      m_loggable;

private:
	friend class Connecter;
	friend class Sender;
	friend class Recver;
	friend class Keeper;
};

}

#endif
