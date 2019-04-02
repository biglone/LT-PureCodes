#ifndef _PASSWDRESPONSE_H_
#define _PASSWDRESPONSE_H_

#include "Response.h"

namespace protocol
{
	class PasswdResponse : public Response
	{
	public:
		PasswdResponse(net::RemoteResponse* pRr);

		bool Parse();
	};
}
#endif