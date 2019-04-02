#ifndef _DISCUSSRESPONSE_H_
#define _DISCUSSRESPONSE_H_

#include <string>
#include <map>

#include "iks/iksemel.h"
#include "net/Request.h"
#include "Response.h"
#include "ConfigRequest.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ConfigResponse : public Response
	{
	public:
		ConfigResponse(net::RemoteResponse* pRR);
		~ConfigResponse();

		bool Parse();

		bool hasConfig(int num) const;
		std::string configData(const std::string &name) const;
		ConfigRequest::ActionType actionType() const;

	private:
		ConfigRequest::ActionType               m_actionType;
		std::map<std::string, std::string>      m_configData;
		int                                     m_configs[10];
	};
}
#endif //_DISCUSSRESPONSE_H_
