#ifndef _REPORT_TS_RESPONSE_H_
#define _REPORT_TS_RESPONSE_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include "iks/iksemel.h"
#include "base/Base.h"

#include "Response.h"

namespace protocol
{
	class ReportTsResponse : public Response
	{
	public:
		ReportTsResponse(net::RemoteResponse* rr);

		bool Parse();
	};
}

#endif // _REPORT_TS_RESPONSE_H_