#ifndef _REPORT_TS_REQUEST_H_
#define _REPORT_TS_REQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ReportTsRequest : public net::Request
	{
	public:
		ReportTsRequest(const char *pszTs);

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		std::string m_ts;
	};
}
#endif // _REPORT_TS_REQUEST_H_