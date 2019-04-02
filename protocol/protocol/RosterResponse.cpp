#include "cttk/base.h"

#include "psmscommon/PSMSUtility.h"
#include "iks/iksemel.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "ErrorResponse.h"
#include "net/RemoteResponse.h"
#include "net/IProtocolCallback.h"

#include "RosterResponse.h"


namespace protocol
{
	RosterResponse::RosterResponse(net::RemoteResponse* pRR)
		: Response(pRR), m_actionType(RosterRequest::Action_None)
	{
	}

	RosterResponse::~RosterResponse()
	{
	}

	bool RosterResponse::Parse()
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

			// 找到 Roster
			iks* pnRoster = iks_find(pnResponse, protocol::TAG_ROSTER);
			if (!pnRoster)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnRoster);
			if (m_pER)
			{
				ErrorResponse::Log("RosterResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			// check action type
			char *actionType = iks_find_attrib(pnRoster, ATTRIBUTE_NAME_TYPE);
			if (!actionType || !strlen(actionType))
			{
				break;
			}

			if (strcmp(actionType, ATTRIBUTE_NAME_SYNC) == 0)
			{
				m_actionType = RosterRequest::Action_Sync;
			}
			else if (strcmp(actionType, ATTRIBUTE_NAME_MODIFY) == 0)
			{
				m_actionType = RosterRequest::Action_Modify;
			}

			if (m_actionType == RosterRequest::Action_None)
			{
				break;
			}

			if (m_actionType == RosterRequest::Action_Sync)
			{
				// version
				char *syncVersion = iks_find_attrib(pnRoster, ATTRIBUTE_NAME_VERSION);
				if (!syncVersion || !strlen(syncVersion))
				{
					m_syncVersion = "";
				}
				else
				{
					m_syncVersion = syncVersion;
				}

				// roster items
				iks *item = iks_first_tag(pnRoster);
				while (item != 0)
				{
					char *itemName = iks_name(item);
					if (strcmp(itemName, "item") == 0)
					{
						std::string id;
						std::string group;
						std::string name;
						
						char *szId = iks_find_attrib(item, ATTRIBUTE_NAME_ID);
						if (szId)
							id = szId;

						char *szGroup = iks_find_attrib(item, ATTRIBUTE_NAME_GROUP);
						if (szGroup)
							group = szGroup;

						char *szName = iks_find_attrib(item, ATTRIBUTE_NAME_NAME);
						if (szName)
							name = szName;

						RosterRequest::RosterItem rosterItem(id.c_str(), name.c_str(), group.c_str());
						m_rosterItems.push_back(rosterItem);
					}
						
					item = iks_next_tag(item);
				}
			}

			bOk = true;
		} while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}


	RosterRequest::ActionType RosterResponse::getActionType() const
	{
		return m_actionType;
	}

	std::vector<RosterRequest::RosterItem> RosterResponse::getRosterItems() const
	{
		return m_rosterItems;
	}

	std::string RosterResponse::getSyncVersion() const
	{
		return m_syncVersion;
	}
}