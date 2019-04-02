#ifndef _REPORT_ONLINE_RESPONSE_H_
#define _REPORT_ONLINE_RESPONSE_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include "iks/iksemel.h"
#include "base/Base.h"

#include "Response.h"

namespace protocol
{
	class ReportOnlineResponse : public Response
	{
	public:
		ReportOnlineResponse(net::RemoteResponse* rr);

		bool Parse();
	};
}

#endif // _REPORT_TS_RESPONSE_H_