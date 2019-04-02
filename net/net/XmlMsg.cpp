#include "XmlMsg.h"
#include "IProtocolCallback.h"

namespace net
{

IProtocolCallback* XmlMsg::s_pProtocolCallback = 0;

const int XmlMsg::MAX_BUFFER_SIZE = 1400;

XmlMsg::XmlMsg()
{
}

XmlMsg::~XmlMsg()
{
}

void XmlMsg::setProtocolCallback(IProtocolCallback* protocolCallback)
{
	s_pProtocolCallback = protocolCallback;
}

IProtocolCallback* XmlMsg::getProtocolCallback()
{
	return s_pProtocolCallback;
}
}