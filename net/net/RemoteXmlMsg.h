#ifndef _REMOTEXMLMSG_H_
#define _REMOTEXMLMSG_H_

#include <string>
#include "iks/iksemel.h"
#include "XmlMsg.h"
#include "net_global.h"

namespace net
{

class NET_EXPORT RemoteXmlMsg : public XmlMsg
{
public:
	RemoteXmlMsg(iks* pElement);
	virtual ~RemoteXmlMsg();

	iks* getMessage();

	std::string getBuffer();

private:
	iks* m_pElement;
};

}
#endif
