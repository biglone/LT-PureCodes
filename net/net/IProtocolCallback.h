#ifndef _IPROTOCOLCALLBACK_H_
#define _IPROTOCOLCALLBACK_H_

#include "net_global.h"

namespace protocol
{
	class Response;
	class SpecificNotification;
}

namespace net
{

class Request;

class NET_EXPORT IProtocolCallback
{
public:
	// 请求有应答,1.res==null(请求超时)，2.res!=null(a.可能是正常应答,b.可能是类似errcode+errmsg形式应答,c.甚至有可能协议错误)
	virtual void onResponse(net::Request* req, protocol::Response* res) = 0;

	// 通知消息
	virtual void onNotification(protocol::SpecificNotification* sn) = 0;

	// 服务器请求
};

}

#endif