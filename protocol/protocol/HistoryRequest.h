#ifndef _HISTORYREQUEST_H_
#define _HISTORYREQUEST_H_

#include "net/Request.h"
#include "OfflineResponse.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT HistoryRequest : public net::Request
	{
	public:
		enum Direction 
		{
			// do not change the value !!! -- same to OfflineMsgManager::MsgDirection
			Backward,
			Forward
		};

	public:
		HistoryRequest(protocol::OfflineResponse::ItemType fType, const char* from, const char* ts, Direction direction, int number);

		int getType();

		protocol::OfflineResponse::ItemType fromType() const {return m_fType;}
		std::string fromId() const {return m_from;}

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		protocol::OfflineResponse::ItemType m_fType;
		std::string                         m_from;
		std::string                         m_ts;
		Direction                           m_direction;
		int                                 m_number;
	};
}
#endif // _HISTORYREQUEST_H_