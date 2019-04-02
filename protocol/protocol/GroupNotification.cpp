#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "cttk/base.h"
#include "GroupNotification.h"

namespace protocol
{
	int GroupNotification::getNotificationType()
	{
		return protocol::GROUP;
	}

	bool GroupNotification::Parse(iks *pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)
		do
		{
			if (!pnIks)
			{
				break;
			}

			const char *pszId = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ID);
			if (!pszId)
			{
				break;
			}


			const char* pszName = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_NAME);
			if (!pszName)
			{
				break;
			}

			const char* pszVersion = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_VERSION);
			const char* pszDesc = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_DESC);

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

						char *pszIndex = iks_find_attrib(pItem, ATTRIBUTE_NAME_INDEX);
						int index = 0;
						if (pszIndex)
						{
							cttk::str::toint(pszIndex, index);
						}
						indice.push_back(index);

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
			name = pszName;
			if (pszDesc)
				desc = pszDesc;
			if (pszVersion)
				version = pszVersion;

			bOk = true;

		}while(false);

		return bOk;
	}

}