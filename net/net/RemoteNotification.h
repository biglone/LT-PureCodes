#ifndef _REMOTENOTIFICATION_H_
#define _REMOTENOTIFICATION_H_

#include "RemoteXmlMsg.h"
#include "net_global.h"

namespace net
{

class NET_EXPORT RemoteNotification : public RemoteXmlMsg
{
public:
	RemoteNotification(iks* pElement);
	~RemoteNotification();

public:
	IProtocolCallback* getProtocol();
	
};

}
#endif