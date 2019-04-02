#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "RosterNotification.h"

namespace protocol
{
	RosterNotification::RosterNotification()
	{
		m_rosterItems.clear();
	}

	bool RosterNotification::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok

		do
		{
			// TODO: 解析message头的信息 包括data等；

			if (!pnIks)
			{
				break;
			}

			iks* pnRoster = pnIks;

			// check action type
			char *actionType = iks_find_attrib(pnRoster, ATTRIBUTE_NAME_TYPE);
			if (!actionType || !strlen(actionType))
			{
				break;
			}

			
			if (strcmp(actionType, ATTRIBUTE_NAME_MODIFY) != 0)
			{
				break;
			}

			// roster items
			m_rosterItems.clear();
			iks *item = iks_first_tag(pnRoster);
			while (item != 0)
			{
				char *itemName = iks_name(item);
				if (strcmp(itemName, "item") == 0)
				{
					std::string id;
					std::string group;
					std::string name;
					RosterRequest::ModifyType modifyType = RosterRequest::ModifyNone;

					char *szId = iks_find_attrib(item, ATTRIBUTE_NAME_ID);
					if (szId)
						id = szId;

					char *szGroup = iks_find_attrib(item, ATTRIBUTE_NAME_GROUP);
					if (szGroup)
						group = szGroup;

					char *szName = iks_find_attrib(item, ATTRIBUTE_NAME_NAME);
					if (szName)
						name = szName;

					char *szType = iks_find_attrib(item, ATTRIBUTE_NAME_TYPE);
					if (szType)
					{
						if (0 == iks_strcasecmp(szType, "add"))
							modifyType = RosterRequest::ModifyAdd;

						if (0 == iks_strcasecmp(szType, "delete"))
							modifyType = RosterRequest::ModifyDelete;
					}

					RosterRequest::RosterItem rosterItem(id.c_str(), name.c_str(), group.c_str());
					rosterItem.m_modifyType = modifyType;
					m_rosterItems.push_back(rosterItem);
				}

				item = iks_next_tag(item);
			}

			bOk = true;
		} while(false);

		return bOk;
	}

	int RosterNotification::getNotificationType()
	{
		return protocol::ROSTER;
	}

	std::vector<RosterRequest::RosterItem> RosterNotification::getRosterItems() const
	{
		return m_rosterItems;
	}
}