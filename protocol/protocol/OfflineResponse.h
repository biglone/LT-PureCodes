#ifndef _OFFLINERESPONSE_H_
#define _OFFLINERESPONSE_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include "iks/iksemel.h"
#include "base/Base.h"

#include "Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT OfflineResponse : public Response
	{
	public:
		enum ItemType
		{
			// do not change the value !!! -- same to OfflineMsgManager::FromType
			User,
			Group,
			Discuss
		};

		class Item 
		{
		public:
			Item()
			{
				m_type = User;
				m_count = 0;
			}

			Item(ItemType itemType, const char *from, int count)
			{
				m_type = itemType;
				m_from = from;
				m_count = count;
			}

			ItemType    m_type;
			std::string m_from;
			int         m_count;
		};

	public:
		OfflineResponse(net::RemoteResponse* rr);

		bool Parse();

		std::list<Item> getItems() const;
		std::string getTs() const;
		std::string getTs2() const;
		std::string getTsNow() const;

	private:
		std::string       m_ts;
		std::string       m_tsNow;
		std::string       m_ts2;
		std::list<Item>   m_items;
	};
}

#endif // _OFFLINERESPONSE_H_