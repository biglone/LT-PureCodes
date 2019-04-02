#include "RemoteNotification.h"

namespace net
{

RemoteNotification::RemoteNotification(iks* pElement)
: RemoteXmlMsg(pElement)
{
}

RemoteNotification::~RemoteNotification()
{

}

IProtocolCallback* RemoteNotification::getProtocol()
{
	return XmlMsg::getProtocolCallback();
}

}