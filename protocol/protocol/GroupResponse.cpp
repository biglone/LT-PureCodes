#include "cttk/base.h"

#include "net/RemoteResponse.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ErrorResponse.h"
#include "GroupResponse.h"

namespace protocol
{
	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS GroupResponse
	GroupResponse::GroupResponse(net::RemoteResponse* pRr)
		: Response(pRr)
		, m_sGroupId("")
		, m_bIsChild(false)
		, m_sVersion("")
	{
		m_listItems.clear();
	}

	bool GroupResponse::Parse()
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

			// 找到 group
			iks* pnGroup = iks_find(pnResponse, protocol::TAG_GROUP);
			if (!pnGroup)
			{
				break;
			}

			const char* pszId = iks_find_attrib(pnGroup, protocol::ATTRIBUTE_NAME_ID);
			if (!pszId)
			{
				setChild(false);
			}
			else
			{
				setChild(true);
				setGroupId(pszId);

				const char* pszDesc = iks_find_attrib(pnGroup, protocol::ATTRIBUTE_NAME_DESC);
				m_sDesc = pszDesc ? pszDesc : "";

				const char* pszVersion = iks_find_attrib(pnGroup, protocol::ATTRIBUTE_NAME_VERSION);
				m_sVersion = pszVersion ? pszVersion : "";
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnGroup);
			if (m_pER)
			{
				ErrorResponse::Log("GroupResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			std::list<Item> listItems;
			// 解析group内容
			iks* pnItem = iks_first_tag(pnGroup);
			while (pnItem)
			{
				do 
				{
					const char* pszTag = iks_name(pnItem);
					if (strcmp(pszTag, protocol::TAG_ITEM) != 0) 
					{
						break;
					}

					const char* pszId = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_ID);
					const char* pszName = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_NAME);
					const char* pszIndex = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_INDEX);
					const char* pszVersion = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_VERSION);
					const char *pszCardName = iks_find_attrib(pnItem, protocol::ATTRIBUTE_NAME_CARDNAME);

					if (!pszId || !pszName || !pszIndex)
					{
						break;
					}

					if (strlen(pszId) <= 0)
					{
						break;
					}

					int index = 0;
					if (!cttk::str::toint(pszIndex, index))
					{
						// index error
					}

					char* pszLogoVersion = 0;
					char* pszAnnt = 0;
					iks* pnItemChild = iks_first_tag(pnItem);
					while (pnItemChild)
					{
						pszTag = iks_name(pnItemChild);
						if (!strcmp(pszTag, protocol::TAG_LOGO_VERSION)) 
						{
							pszLogoVersion = iks_cdata(iks_child(pnItemChild));
						}
						else if (!strcmp(pszTag, protocol::TAG_ANNT))
						{
							pszAnnt = iks_cdata(iks_child(pnItemChild));
						}
						pnItemChild = iks_next_tag(pnItemChild);
					}

					Item item;
					item.id = pszId;
					item.name = pszName;
					item.index = index;
					if (pszLogoVersion && strlen(pszLogoVersion) > 0)
						item.logoVersion = atoi(pszLogoVersion);
					else
						item.logoVersion = kInvalidLogoVersion;
					if (pszAnnt)
						item.annt = pszAnnt;
					if (pszVersion)
						item.version = pszVersion;
					if (pszCardName)
						item.cardName = pszCardName;

					listItems.push_back(item);

				} while(0);

				pnItem = iks_next_tag(pnItem);
			}

			m_listItems = listItems;

			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	std::list<GroupResponse::Item> GroupResponse::getItems()
	{
		return m_listItems;
	}

	std::string GroupResponse::getGroupId()
	{
		return m_sGroupId;
	}

	std::string GroupResponse::getGroupDesc()
	{
		return m_sDesc;
	}

	std::string GroupResponse::getGroupVersion()
	{
		return m_sVersion;
	}

	bool GroupResponse::isChild()
	{
		return m_bIsChild;
	}

	void GroupResponse::setChild(bool bIsChild)
	{
		m_bIsChild = bIsChild;
	}

	void GroupResponse::setGroupId(const std::string& rsGroupId)
	{
		m_sGroupId = rsGroupId;
	}

	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLAS GroupCardNameResponse
	GroupCardNameResponse::GroupCardNameResponse(net::RemoteResponse *pRr)
		: Response(pRr)
	{
		m_sGroupId = "";
	}

	bool GroupCardNameResponse::Parse()
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

			iks *pnResponse = m_pRR->getMessage();

			if (!pnResponse)
			{
				break;
			}

			// 找到 group
			iks *pnGroup = iks_find(pnResponse, protocol::TAG_GROUP);
			if (!pnGroup)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnGroup);
			if (m_pER)
			{
				ErrorResponse::Log("GroupCardNameResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			const char *pszId = iks_find_attrib(pnGroup, protocol::ATTRIBUTE_NAME_ID);
			if (!pszId)
			{
				break;
			}

			m_sGroupId = pszId;

			bOk = true;
		} while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	std::string GroupCardNameResponse::getGroupId()
	{
		return m_sGroupId;
	}
}
