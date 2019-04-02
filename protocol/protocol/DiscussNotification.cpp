#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "DiscussNotification.h"

namespace protocol
{
	int DiscussNotification::getNotificationType()
	{
		return protocol::DISCUSS;
	}

	bool DiscussNotification::Parse( iks* pnIks )
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)
		do
		{
			if (!pnIks)
			{
				break;
			}

			const char* pszId = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ID);
			if (!pszId)
			{
				break;
			}

			const char* pszType = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TYPE);
			if (pszType && (0 == iks_strcasecmp(pszType, ATTRIBUTE_NAME_QUIT)))
			{
				id = pszId;
				type = pszType;

				bOk = true;
				break;
			}

			const char* pszName = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_NAME);
			if (!pszName)
			{
				break;
			}

			const char* pszCreator = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_CREATOR);
			const char* pszTime = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TIME);
			const char* pszVersion = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_VERSION);

			// items
			iks *pItem = iks_first_tag(pnIks);
			while (pItem != 0)
			{
				char *itemName = iks_name(pItem);
				if (strcmp(itemName, "item") == 0)
				{
					char *pszId = iks_find_attrib(pItem, ATTRIBUTE_NAME_ID);
					if (pszId)
					{
						members.push_back(pszId);

						char *pszMemberName = iks_find_attrib(pItem, ATTRIBUTE_NAME_NAME);
						std::string memberName;
						if (pszMemberName)
						{
							memberName = pszMemberName;
						}
						memberNames.push_back(memberName);

						char *pszAddedId = iks_find_attrib(pItem, ATTRIBUTE_NAME_ADDED);
						std::string addedId;
						if (pszAddedId)
						{
							addedId = pszAddedId;
						}
						addedIds.push_back(addedId);

						char *pszCardName = iks_find_attrib(pItem, ATTRIBUTE_NAME_CARDNAME);
						std::string cardName;
						if (pszCardName)
						{
							cardName = pszCardName;
						}
						cardNames.push_back(cardName);
					}
				}
				pItem = iks_next_tag(pItem);
			}

			id = pszId;
			type = "";
			name = pszName;
			if (pszCreator)
				creator = pszCreator;
			if (pszTime)
				time = pszTime;
			if (pszVersion)
				version = pszVersion;

			bOk = true;

		}while(false);

		return bOk;
	}

}