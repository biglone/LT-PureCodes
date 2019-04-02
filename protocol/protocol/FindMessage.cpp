#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"
#include "cttk/base.h"

#include "net/IProtocolCallback.h"
#include "net/RemoteResponse.h"

#include "protocol/ErrorResponse.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "FindMessage.h"

static const char FIND_USER_INFO[]           = "user-information";
static const char FIND_USER_DETAIL[]         = "user-detail";
static const char FIND_USER_PHOTO[]          = "user-photo";
static const char FIND_USER_DETAILPHOTO[]    = "user-detail-photo";
static const char FIND_USER_DETAIL_VERSION[] = "user-detail-version";
static const char FIND_GROUP_PRESENCE[]      = "group-presence";

namespace protocol
{
	std::string FindMessage::FindTypeToString(FindMessage::FindType type)
	{
		switch (type)
		{
		case FindMessage::Find_User_Info:
			return FIND_USER_INFO;
		case FindMessage::Find_User_Detail:
			return FIND_USER_DETAIL;
		case FindMessage::Find_User_Photo:
			return FIND_USER_PHOTO;
		case FindMessage::Find_User_DetailPhoto:
			return FIND_USER_DETAILPHOTO;
		case FindMessage::Find_User_Version:
			return FIND_USER_DETAIL_VERSION;
		case FindMessage::Find_Group_Presence:
			return FIND_GROUP_PRESENCE;
		}

		return FIND_USER_INFO;
	}

	FindMessage::FindType FindMessage::FindTypeFromString(const std::string& type)
	{
		if (type.compare(FIND_USER_INFO) == 0)
		{
			return FindMessage::Find_User_Info;
		}
		else if (type.compare(FIND_USER_DETAIL) == 0)
		{
			return FindMessage::Find_User_Detail;
		}
		else if (type.compare(FIND_USER_PHOTO) == 0)
		{
			return FindMessage::Find_User_Photo;
		}
		else if (type.compare(FIND_USER_DETAILPHOTO) == 0)
		{
			return FindMessage::Find_User_DetailPhoto;
		}
		else if (type.compare(FIND_USER_DETAIL_VERSION) == 0)
		{
			return FindMessage::Find_User_Version;
		}
		else if (type.compare(FIND_GROUP_PRESENCE) == 0)
		{
			return FindMessage::Find_Group_Presence;
		}

		return FindMessage::Find_User_Info;
	}

	FindMessage::FindRequest::FindRequest(FindType type /* = Find_User_Info */)
		: m_eType(type)
	{
		m_listIds.clear();
	}

	void FindMessage::FindRequest::addId(const std::string& id)
	{
		m_listIds.push_back(id);
	}

	int FindMessage::FindRequest::getType()
	{
		return protocol::Request_IM_Find;
	}

	int FindMessage::FindRequest::getFindType() const
	{
		return m_eType;
	}

	std::string FindMessage::FindRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnFind = iks_insert(pnMessage, protocol::TAG_FIND);
			if (!pnFind) break;

			// type
			iks_insert_attrib(pnFind, protocol::ATTRIBUTE_NAME_TYPE, FindTypeToString(m_eType).c_str());

			const char* pszTag = 0;
			switch (m_eType)
			{
			case FindMessage::Find_User_Info:
			case FindMessage::Find_User_Detail:
			case FindMessage::Find_User_Photo:
			case FindMessage::Find_User_DetailPhoto:
			case FindMessage::Find_User_Version:
				pszTag = protocol::TAG_USER;
				break;
			case FindMessage::Find_Group_Presence:
				pszTag = protocol::TAG_GROUP;
				break;
			}
			
			iks* pnTag = 0;

			std::list<std::string>::iterator itr = m_listIds.begin();
			for (; itr != m_listIds.end(); ++itr)
			{
				pnTag = iks_insert(pnFind, pszTag);
				iks_insert_attrib(pnTag, protocol::ATTRIBUTE_NAME_ID, itr->c_str());
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void FindMessage::FindRequest::onResponse(net::RemoteResponse* response)
	{
		FindMessage::FindRespone* fr = new FindMessage::FindRespone(response);
		fr->Parse();
		getProtocolCallback()->onResponse(this, fr);
	}

	FindMessage::FindRespone::FindRespone(net::RemoteResponse* pRr)
		: Response(pRr)
	{}

	bool FindMessage::FindRespone::Parse()
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

			// 找到 Find
			iks* pnFind = iks_find(pnResponse, protocol::TAG_FIND);
			if (!pnFind)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnFind);
			if (m_pER)
			{
				ErrorResponse::Log("FindRespone.Parse()", m_pER);
				bPError = false;
				break;
			}

			// 类型
			const char* pszType = iks_find_attrib(pnFind, protocol::ATTRIBUTE_NAME_TYPE);
			m_eType = FindMessage::FindTypeFromString(pszType);

			// 
			MapFindResult mapUser;
			std::map<std::string, MapFindResult> mapGroup;

			switch (m_eType)
			{
			case FindMessage::Find_User_Info:
			case FindMessage::Find_User_Detail:
			case FindMessage::Find_User_Photo:
			case FindMessage::Find_User_DetailPhoto:
			case FindMessage::Find_User_Version:
				{
					m_mapUser = parseItem(pnFind);
				}
				break;
			case FindMessage::Find_Group_Presence:
				{
					iks* pnGroup = iks_first_tag(pnFind);
					while (pnGroup)
					{
						const char* pszId = iks_find_attrib(pnFind, protocol::ATTRIBUTE_NAME_ID);
						if (pszId && strlen(pszId) > 0)
						{
							MapFindResult mapResult = parseItem(pnGroup);

							m_mapGroup[pszId] = mapResult;
						}

						pnGroup = iks_next_tag(pnGroup);
					}
				}
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

	int FindMessage::FindRespone::getFindType() const
	{
		return m_eType;
	}

	FindMessage::MapFindResult FindMessage::FindRespone::getFindUserResult() const
	{
		return m_mapUser;
	}

	std::map<std::string, FindMessage::MapFindResult> FindMessage::FindRespone::getFindGroupResult() const
	{
		return m_mapGroup;
	}

	FindMessage::MapFindResult FindMessage::FindRespone::parseItem(void* pIks)
	{
		FindMessage::MapFindResult ret;

		do 
		{
			iks* pnIks = static_cast<iks*>(pIks);
			if (!pnIks)
				break;

			iks* pnTag = iks_first_tag(pnIks);
			while (pnTag)
			{
				FindResult result;

				// attribute
				iks* pnAttr = iks_attrib(pnTag);
				while (pnAttr)
				{
					std::string sKey = iks_name(pnAttr);
					std::string sValue = iks_cdata(pnAttr);
					cttk::str::lowcase(sKey);

					result[sKey] = sValue;

					pnAttr = iks_next(pnAttr);
				}

				// photo
				iks* pnPhoto = iks_find(pnTag, protocol::TAG_PHOTO);
				if (pnPhoto)
				{
					iks* pnChild = iks_child(pnPhoto);
					if (pnChild)
					{
						const char* pszPhoto = iks_cdata(pnChild);
						result[protocol::TAG_PHOTO] = pszPhoto ? pszPhoto : "";
					}
				}

				// id
				FindResult::iterator itr = result.find(protocol::ATTRIBUTE_NAME_ID);
				if (itr != result.end())
				{
					ret[itr->second] = result;
				}

				pnTag = iks_next_tag(pnTag);
			}
		} while (0);

		return ret;
	}
}