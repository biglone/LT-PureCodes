#ifndef _ICALLBACK_H_
#define _ICALLBACK_H_

#include "net_global.h"

namespace net
{
class IProtocolCallback;
class Request;
class RemoteNotification;
class RemoteRequest;

// 所有回调函数都需要立即返回,不能阻塞,不然很可能会影响网络通信
class NET_EXPORT ICallback
{
public:
	// 得到协议回调器
	virtual IProtocolCallback* getProtocolCallback() = 0;

	// TCP 连接动作结束的结果报告
	virtual void onConnect(bool bConnect) = 0;

	// 加密动作结束的结果报告,如果没有要求加密,可以不回调
	virtual void onEncrypt(bool bEncrypt) = 0;

	// TCP 连接断开时间提示
	virtual void onBroken() = 0;

	// TCP 是否断开连接
	virtual bool isBroken() const = 0;

	// XML 解析错误
	virtual void onDataParseError() = 0;

	// request 信令结果报告,超时,应答
	virtual void onRequestResult(Request* request) = 0;

	// notification 信令报告
	virtual void onNotification(RemoteNotification* notification) = 0;

	// request 信令报告
	virtual void onRequest(RemoteRequest* remoteRequest) = 0;

	// 内部错误,主要是线程停止等
	virtual void onInternalError() = 0;
};

}
#endif
