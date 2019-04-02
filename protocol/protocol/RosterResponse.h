#ifndef _ROSTERRESPONSE_H_
#define _ROSTERRESPONSE_H_

#include <string>
#include <map>
#include <vector>

#include "iks/iksemel.h"
#include "net/Request.h"
#include "Response.h"
#include "RosterRequest.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT RosterResponse : public Response
	{
	public:
		RosterResponse(net::RemoteResponse* pRR);
		~RosterResponse();

		bool Parse();

		RosterRequest::ActionType getActionType() const;
		std::vector<RosterRequest::RosterItem> getRosterItems() const;
		std::string getSyncVersion() const;

	private:
		std::vector<RosterRequest::RosterItem> m_rosterItems;
		RosterRequest::ActionType              m_actionType;
		std::string                            m_syncVersion;
	};
}

#endif // _ROSTERRESPONSE_H_