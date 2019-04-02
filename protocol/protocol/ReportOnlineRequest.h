#ifndef _REPORT_ONLINE_REQUEST_H_
#define _REPORT_ONLINE_REQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ReportOnlineRequest : public net::Request
	{
	public:
		ReportOnlineRequest(const char *pszId, const char *pszState);

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		std::string m_id;
		std::string m_state;
	};
}
#endif // _REPORT_TS_REQUEST_H_