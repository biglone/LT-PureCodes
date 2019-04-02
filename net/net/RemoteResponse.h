#ifndef _REMOTERESPONSE_H_
#define _REMOTERESPONSE_H_

#include "RemoteXmlMsg.h"
#include "net_global.h"

namespace net
{

class NET_EXPORT RemoteResponse : public RemoteXmlMsg
{
public:
	RemoteResponse(iks* pElement);
	~RemoteResponse();
};

}
#endif

