#ifndef _NOTIFICATIONFACTORY_H_
#define _NOTIFICATIONFACTORY_H_

#include "protocol_global.h"

namespace net
{
	class RemoteNotification;
}

namespace protocol
{
	class SpecificNotification;
	class PROTOCOL_EXPORT NotificationFactory
	{
	private:
		NotificationFactory() {}
		~NotificationFactory() {}

	public:
		static SpecificNotification* Create(net::RemoteNotification* pRn);

	};
}
#endif
