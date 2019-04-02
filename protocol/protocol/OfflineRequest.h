#ifndef _OFFLINEREQUEST_H_
#define _OFFLINEREQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT OfflineRequest : public net::Request
	{
	public:
		OfflineRequest(const char *pszTs, const char *pszTs2 = 0);

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		std::string m_ts;
		std::string m_ts2;
	};
}
#endif // _OFFLINEREQUEST_H_