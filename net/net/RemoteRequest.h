#ifndef _REMOTEREQUEST_H_
#define _REMOTEREQUEST_H_

#include "RemoteXmlMsg.h"
#include "net_global.h"

namespace net
{

class NET_EXPORT RemoteRequest : public RemoteXmlMsg
{
public:
	RemoteRequest(iks* pElement);
	~RemoteRequest();
};

}
#endif
