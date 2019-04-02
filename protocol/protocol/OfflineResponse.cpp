#include "net/RemoteResponse.h"
#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ErrorResponse.h"
#include "OfflineResponse.h"

namespace protocol
{
	OfflineResponse::OfflineResponse(net::RemoteResponse* pRr)
		: Response(pRr)
	{
		m_ts = "";
		m_tsNow = "";
		m_ts2 = "";
		m_items.clear();
	}

	bool OfflineResponse::Parse()
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(a.协议信令本身错误,b.errcode+errmsg类型的应答)
		bool bPError = true;         // 是否协议信令本身的错误

		do
		{
			// TODO: 解析message头的信息 包括data等；

			if (!m_pRR)
			{
				break;
			}

			iks* pnResponse = m_pRR->getMessage();

			if (!pnResponse)
			{
				break;
			}

			// 找到 msg
			iks* pnMsg = iks_find(pnResponse, protocol::TAG_MSG);
			if (pnMsg)
			{
				char* pszTs = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS);
				if (!pszTs || strlen(pszTs) <= 0)
					break;
				m_ts = pszTs;

				char* pszTsNow = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TSNOW);
				if (!pszTsNow)
					pszTsNow = "";
				m_tsNow = pszTsNow;

				char* pszTs2 = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS2);
				if (!pszTs2)
					pszTs2 = "";
				m_ts2 = pszTs2;
			}

			// items
			do 
			{
				// item
				iks* pnItem = iks_first_tag(pnMsg);
				while (pnItem)
				{
					if (strcmp(protocol::TAG_ITEM, iks_name(pnItem)) != 0)
					{
						continue;
					}

					// ftype
					char* pszFType = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_FTYPE);
					if (!pszFType || strlen(pszFType) <= 0)
						continue;
					ItemType itemType = User;
					if (iks_strcasecmp(pszFType, "group") == 0)
						itemType = Group;
					else if (iks_strcasecmp(pszFType, "discuss") == 0)
						itemType = Discuss;
					else
						itemType = User;

					// from
					char* pszFrom = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_FROM);
					if (!pszFrom || strlen(pszFrom) <= 0)
						continue;
					
					// count
					char* pszCount = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_COUNT);
					if (!pszCount || strlen(pszCount) <= 0)
						continue;
					int count = atoi(pszCount);
					
					// create item
					Item item(itemType, pszFrom, count);
					m_items.push_back(item);

					pnItem = iks_next_tag(pnItem);
				}
			} while (0);

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnMsg);
			if (m_pER)
			{
				ErrorResponse::Log("OfflineResponse.Parse()", m_pER);
				bPError = false;
				break;
			}
			
			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	std::list<protocol::OfflineResponse::Item> OfflineResponse::getItems() const
	{
		return m_items;
	}

	std::string OfflineResponse::getTs() const
	{
		return m_ts;
	}

	std::string OfflineResponse::getTs2() const
	{
		return m_ts2;
	}

	std::string OfflineResponse::getTsNow() const
	{
		return m_tsNow;
	}
}