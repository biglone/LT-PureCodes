#ifndef _LOGOUTRESPONSE_H_
#define _LOGOUTRESPONSE_H_

#include "Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT LogoutResponse : public Response
	{
	public:
		LogoutResponse(net::RemoteResponse* pRr);

		bool Parse();

	};
}
#endif