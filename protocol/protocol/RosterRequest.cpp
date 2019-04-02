#include "cttk/base.h"

#include "psmscommon/PSMSUtility.h"
#include "iks/iksemel.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "ErrorResponse.h"
#include "net/RemoteResponse.h"
#include "net/IProtocolCallback.h"

#include "RosterRequest.h"
#include "RosterResponse.h"

namespace protocol
{
	RosterRequest::RosterRequest(ActionType eType /*= Action_Sync*/, const char *syncVersion /*= 0*/)
		: m_actionType(eType), m_clientType(ClientSyncRoster)
	{
		m_syncVersion = "";
		if (syncVersion)
			m_syncVersion = syncVersion;
	}

	RosterRequest::~RosterRequest()
	{
	}

	int RosterRequest::getType()
	{
		return protocol::Request_IM_Roster;
	}

	void RosterRequest::setClientType(ClientType clientType)
	{
		m_clientType = clientType;
	}

	RosterRequest::ClientType RosterRequest::clientType() const
	{
		return m_clientType;
	}

	RosterRequest::ActionType RosterRequest::getActionType() const
	{
		return m_actionType;
	}

	std::string RosterRequest::getSyncVersion() const
	{
		return m_syncVersion;
	}

	void RosterRequest::addRosterItem(ModifyType modifyType, const char *id, const char *name, const char *group)
	{
		RosterItem rosterItem(id, name, group);
		rosterItem.m_modifyType = modifyType;
		m_rosterItems.push_back(rosterItem);
	}

	void RosterRequest::clearRosterItems()
	{
		m_rosterItems.clear();
	}

	std::vector<RosterRequest::RosterItem> RosterRequest::getRosterItems() const
	{
		return m_rosterItems;
	}

	void RosterRequest::onResponse(net::RemoteResponse* response)
	{
		RosterResponse* pRr = new RosterResponse(response);
		pRr->Parse();
		getProtocolCallback()->onResponse(this, pRr);
	}

	std::string RosterRequest::getBuffer()
	{
		string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnRoster = iks_insert(pnMessage, protocol::TAG_ROSTER);
			if (!pnRoster) break;

			switch (m_actionType)
			{
			case Action_Sync:
				{
					// type
					iks_insert_attrib(pnRoster, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_SYNC);

					// version
					if (!m_syncVersion.empty())
						iks_insert_attrib(pnRoster, protocol::ATTRIBUTE_NAME_VERSION, m_syncVersion.c_str());
				}
				break;
			case Action_Modify:
				{
					// type
					iks_insert_attrib(pnRoster, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_MODIFY);

					// modify items
					for (size_t j = 0; j < m_rosterItems.size(); j++)
					{
						RosterItem rosterItem = m_rosterItems[j];
						iks* pnItem = iks_insert(pnRoster, TAG_ITEM);
						if (pnItem)
						{
							if (rosterItem.m_id.empty() || rosterItem.m_group.empty())
								continue;

							// type
							if (rosterItem.m_modifyType == ModifyAdd)
								iks_insert_attrib(pnItem, ATTRIBUTE_NAME_TYPE, "add");
							else if (rosterItem.m_modifyType == ModifyDelete)
								iks_insert_attrib(pnItem, ATTRIBUTE_NAME_TYPE, "delete");

							// id
							iks_insert_attrib(pnItem, ATTRIBUTE_NAME_ID, rosterItem.m_id.c_str());

							// name
							iks_insert_attrib(pnItem, ATTRIBUTE_NAME_NAME, rosterItem.m_name.c_str());

							// group
							iks_insert_attrib(pnItem, ATTRIBUTE_NAME_GROUP, rosterItem.m_group.c_str());
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
}