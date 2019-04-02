//#pragma once
#ifndef _XMLMSG_H_
#define _XMLMSG_H_

#include <string>
#include "net_global.h"

namespace net
{
class IProtocolCallback;
class NET_EXPORT XmlMsg
{
public:
	static const int MAX_BUFFER_SIZE;

public:
	XmlMsg();
	virtual ~XmlMsg();

	virtual std::string getBuffer() = 0;
	virtual bool isRequest() { return false; }

public:
	static void setProtocolCallback(IProtocolCallback* protocolCallback);

protected:
	static IProtocolCallback* getProtocolCallback();

private:
	static IProtocolCallback* s_pProtocolCallback;
};

}

#endif

