#ifndef _DISCUSSRESPONSE_H_
#define _DISCUSSRESPONSE_H_

#include <string>
#include <map>
#include <vector>

#include "iks/iksemel.h"
#include "net/Request.h"
#include "Response.h"
#include "DiscussRequest.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT DiscussResponse : public Response
	{
	public:
		DiscussResponse(net::RemoteResponse* pRR);
		~DiscussResponse();

		bool Parse();

		DiscussRequest::ActionType getActionType() const;
		std::vector<DiscussRequest::DiscussItem> getDiscussItems() const;
		std::string getDiscussId() const;
		std::string getDiscussName() const;
		std::string getDiscussTime() const;
		std::string getDiscussCreator() const;
		std::string getDiscussVersion() const;

	private:
		std::vector<DiscussRequest::DiscussItem> m_discussItems;
		DiscussRequest::ActionType               m_actionType;
		std::string                              m_discussId;
		std::string                              m_discussName;
		std::string                              m_discussTime;
		std::string                              m_discussCreator;
		std::string                              m_discussVersion;
	};
}
#endif //_DISCUSSRESPONSE_H_
