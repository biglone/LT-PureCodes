#include "cttk/base.h"

#include "psmscommon/PSMSUtility.h"
#include "iks/iksemel.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "ErrorResponse.h"
#include "net/RemoteResponse.h"
#include "net/IProtocolCallback.h"

#include "DiscussResponse.h"
#include "DiscussRequest.h"

namespace protocol
{

	DiscussRequest::DiscussRequest( ActionType type /*= Action_Sync*/, const std::string& rsIdorName/*= ""*/ )
		: m_eType(type)
		, m_sIdorName(rsIdorName)
	{

	}

	void DiscussRequest::addDiscussItem( const std::string &rsId, const std::string &rsName /*= ""*/ )
	{
		DiscussItem item(rsId.c_str(), rsName.c_str());
		m_lstItems.push_back(item);
	}

	void DiscussRequest::clearDiscussItems()
	{
		m_lstItems.clear();
	}

	std::string DiscussRequest::getIdorName() const
	{
		return m_sIdorName;
	}

	int DiscussRequest::getType()
	{
		return protocol::Request_Discuss_Discuss;
	}

	int DiscussRequest::actionType() const
	{
		return m_eType;
	}

	std::string DiscussRequest::getBuffer()
	{
		string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_MODULE_DISCUSS, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnDiscuss = iks_insert(pnMessage, protocol::TAG_DISCUSS);
			if (!pnDiscuss) break;

			switch (m_eType)
			{
			case Action_Create:
				{
					// type
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_CREATE);

					// name 
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_NAME, m_sIdorName.c_str());
				}
				break;
			case Action_Add:
				{
					// type
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_ADD);

					// id
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_ID, m_sIdorName.c_str());

					// create items
					for (size_t j = 0; j < m_lstItems.size(); j++)
					{
						DiscussItem discussItem = m_lstItems[j];
						iks* pnItem = iks_insert(pnDiscuss, TAG_ITEM);
						if (pnItem)
						{
							iks_insert_attrib(pnItem, protocol::ATTRIBUTE_NAME_ID, discussItem.id.c_str());
						}
					}
				}
				break;
			case Action_Quit:
				{
					// type
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_QUIT);

					// id
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_ID, m_sIdorName.c_str());

					// create items
					for (size_t j = 0; j < m_lstItems.size(); j++)
					{
						DiscussItem discussItem = m_lstItems[j];
						iks* pnItem = iks_insert(pnDiscuss, TAG_ITEM);
						if (pnItem)
						{
							iks_insert_attrib(pnItem, protocol::ATTRIBUTE_NAME_ID, discussItem.id.c_str());
						}
					}

				}
				break;
			case Action_Sync:
				{
					// type
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_SYNC);

					// version
					if (!m_sIdorName.empty())
						iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_ID, m_sIdorName.c_str());
				}
				break;
			case Action_ChangeName:
				{
					// type
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_CHANGENAME);

					// items
					if (!m_lstItems.empty())
					{
						DiscussItem discussItem = m_lstItems[0];
						iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_ID, discussItem.id.c_str());
						iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_NAME, discussItem.name.c_str());
					}
				}
				break;
			case Action_ChangeCardName:
				{
					// type
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_CHANGECARDNAME);

					// id
					iks_insert_attrib(pnDiscuss, protocol::ATTRIBUTE_NAME_ID, m_sIdorName.c_str());

					// item
					if (!m_lstItems.empty())
					{
						DiscussItem discussItem = m_lstItems[0];
						iks* pnItem = iks_insert(pnDiscuss, TAG_ITEM);
						if (pnItem)
						{
							iks_insert_attrib(pnItem, protocol::ATTRIBUTE_NAME_ID, discussItem.id.c_str());
							iks_insert_attrib(pnItem, protocol::ATTRIBUTE_NAME_CARDNAME, discussItem.name.c_str());
						}
					}
				}
				break;
			default:
				break;
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void DiscussRequest::onResponse( net::RemoteResponse* response )
	{
		DiscussResponse* pRr = new DiscussResponse(response);
		pRr->Parse();
		getProtocolCallback()->onResponse(this, pRr);
	}

}