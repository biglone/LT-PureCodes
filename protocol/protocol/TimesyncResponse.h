#ifndef _TIMESYNCRESPONSE_H_
#define _TIMESYNCRESPONSE_H_

#include "Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT TimesyncResponse : public Response
	{
	public:
		TimesyncResponse(net::RemoteResponse* pRr);

		bool Parse();

		std::string getTime();

	private:
		void setTime(const std::string& rsTime);

	private:
		std::string m_sTime;
	};
}
#endif