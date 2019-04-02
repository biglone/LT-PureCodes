#include "ProtocolConst.h"
#include "ProtocolType.h"
#include "net/RemoteResponse.h"
#include "ErrorResponse.h"

#include "DiscussResponse.h"

namespace protocol
{


	DiscussResponse::DiscussResponse( net::RemoteResponse* pRR )
		: Response(pRR)
		, m_actionType(DiscussRequest::Action_None)
		, m_discussId()
		, m_discussName()
		, m_discussTime()
		, m_discussCreator()
	{
	}

	DiscussResponse::~DiscussResponse()
	{

	}

	bool DiscussResponse::Parse()
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

			// 找到 Discuss
			iks* pnDiscuss = iks_find(pnResponse, protocol::TAG_DISCUSS);
			if (!pnDiscuss)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnDiscuss);
			if (m_pER)
			{
				ErrorResponse::Log("DiscussResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			// check action type
			const char *actionType = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_TYPE);
			if (!actionType || !strlen(actionType))
			{
				break;
			}

			if (strcmp(actionType, ATTRIBUTE_NAME_CREATE) == 0)
			{
				m_actionType = DiscussRequest::Action_Create;
			}
			else if (strcmp(actionType, ATTRIBUTE_NAME_ADD) == 0)
			{
				m_actionType = DiscussRequest::Action_Add;
			}
			else if (strcmp(actionType, ATTRIBUTE_NAME_QUIT) == 0)
			{
				m_actionType = DiscussRequest::Action_Quit;
			}
			else if (strcmp(actionType, ATTRIBUTE_NAME_SYNC) == 0)
			{
				m_actionType = DiscussRequest::Action_Sync;
			}
			else if (strcmp(actionType, ATTRIBUTE_NAME_CHANGENAME) == 0)
			{
				m_actionType = DiscussRequest::Action_ChangeName;
			}
			else if (strcmp(actionType, ATTRIBUTE_NAME_CHANGECARDNAME) == 0)
			{
				m_actionType = DiscussRequest::Action_ChangeCardName;
			}

			if (m_actionType == DiscussRequest::Action_None)
			{
				break;
			}

			if (m_actionType == DiscussRequest::Action_Sync)
			{
				// id
				const char *pszId = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_ID);
				if (pszId)
				{
					m_discussId = pszId;
				}
				else
				{
					m_discussId = "";
				}

				// name
				const char *pszName = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_NAME);
				if (pszName)
				{
					m_discussName = pszName;
				}
				else
				{
					m_discussName = "";
				}

				// time
				const char *pszTime = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_TIME);
				if (pszTime)
				{
					m_discussTime = pszTime;
				}
				else
				{
					m_discussTime = "";
				}

				// creator
				const char *pszCreator = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_CREATOR);
				if (pszCreator)
				{
					m_discussCreator = pszCreator;
				}
				else
				{
					m_discussCreator = "";
				}

				// version
				const char *pszVersion = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_VERSION);
				if (pszVersion)
				{
					m_discussVersion = pszVersion;
				}
				else
				{
					m_discussVersion = "";
				}

				if (m_discussId.empty())
				{
					// discuss items
					iks *item = iks_first_tag(pnDiscuss);
					while (item != 0)
					{
						char *itemName = iks_name(item);
						if (strcmp(itemName, "item") == 0)
						{
							pszId = iks_find_attrib(item, ATTRIBUTE_NAME_ID);
							pszName = iks_find_attrib(item, ATTRIBUTE_NAME_NAME);
							pszCreator = iks_find_attrib(item, ATTRIBUTE_NAME_CREATOR);
							pszTime = iks_find_attrib(item, ATTRIBUTE_NAME_TIME);
							pszVersion = iks_find_attrib(item, ATTRIBUTE_NAME_VERSION);

							DiscussRequest::DiscussItem discussItem(pszId, pszName);
							if (pszCreator)
							{
								discussItem.creator = pszCreator;
							}
							if (pszTime)
							{
								discussItem.time = pszTime;
							}
							if (pszVersion)
							{
								discussItem.version = pszVersion;
							}
							m_discussItems.push_back(discussItem);
						}

						item = iks_next_tag(item);
					}
				}
				else
				{
					// member items
					iks *item = iks_first_tag(pnDiscuss);
					while (item != 0)
					{
						char *itemName = iks_name(item);
						if (strcmp(itemName, "item") == 0)
						{
							const char *pszId = iks_find_attrib(item, ATTRIBUTE_NAME_ID);
							const char *pszName = iks_find_attrib(item, ATTRIBUTE_NAME_NAME);
							const char *pszAdded = iks_find_attrib(item, ATTRIBUTE_NAME_ADDED);
							const char *pszTime = iks_find_attrib(item, ATTRIBUTE_NAME_TIME);
							const char *pszCardName = iks_find_attrib(item, ATTRIBUTE_NAME_CARDNAME);

							DiscussRequest::DiscussItem discussItem(pszId, pszName);
							if (pszAdded)
							{
								discussItem.creator = pszAdded;
							}
							if (pszTime)
							{
								discussItem.time = pszTime;
							}
							if (pszCardName)
							{
								discussItem.cardName = pszCardName;
							}
							m_discussItems.push_back(discussItem);
						}

						item = iks_next_tag(item);
					}
				}
			}
			else
			{
				// id
				const char *pszId = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_ID);
				if (!pszId || !strlen(pszId))
				{
					break;
				}

				m_discussId = pszId;

				// name
				const char *pszName = iks_find_attrib(pnDiscuss, ATTRIBUTE_NAME_NAME);
				if (pszName)
				{
					m_discussName = pszName;
				}
				else
				{
					m_discussName = "";
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

	DiscussRequest::ActionType DiscussResponse::getActionType() const
	{
		return m_actionType;
	}

	std::vector<DiscussRequest::DiscussItem> DiscussResponse::getDiscussItems() const
	{
		return m_discussItems;
	}

	std::string DiscussResponse::getDiscussId() const
	{
		return m_discussId;
	}

	std::string DiscussResponse::getDiscussName() const
	{
		return m_discussName;
	}

	std::string DiscussResponse::getDiscussTime() const
	{
		return m_discussTime;
	}

	std::string DiscussResponse::getDiscussCreator() const
	{
		return m_discussCreator;
	}

	std::string DiscussResponse::getDiscussVersion() const
	{
		return m_discussVersion;
	}

}

