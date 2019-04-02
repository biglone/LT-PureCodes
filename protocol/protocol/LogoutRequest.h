#ifndef _LOGOUTREQUEST_H_
#define _LOGOUTREQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT LogoutRequest : public net::Request
	{
	public:
		LogoutRequest() {}

		int getType();
		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);
		
	};
}
#endif