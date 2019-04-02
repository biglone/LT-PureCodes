#ifndef _TIMESYNCREQUEST_H_
#define _TIMESYNCREQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT TimesyncRequest : public net::Request
	{
	public:
		TimesyncRequest() {}

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	};
}
#endif
