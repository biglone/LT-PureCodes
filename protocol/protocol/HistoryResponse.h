#ifndef _HISTORYRESPONSE_H_
#define _HISTORYRESPONSE_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include "iks/iksemel.h"
#include "base/Base.h"

#include "Response.h"
#include "HistoryRequest.h"
#include "MessageNotification.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT HistoryResponse : public Response
	{

	public:
		HistoryResponse(net::RemoteResponse* rr);

		bool Parse();

		std::list<protocol::MessageNotification::Message> getMessages() const;
		protocol::OfflineResponse::ItemType fType() const;
		std::string from() const;
		std::string ts() const;
		protocol::HistoryRequest::Direction direction() const;
		int number() const;

	private:
		protocol::OfflineResponse::ItemType m_fType;
		std::string                         m_from;
		std::string                         m_ts;
		protocol::HistoryRequest::Direction m_direction;
		int                                 m_number;
		std::list<protocol::MessageNotification::Message> m_messages;
	};
}

#endif // _HISTORYRESPONSE_H_