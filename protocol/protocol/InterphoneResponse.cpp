#include "ProtocolConst.h"
#include "ProtocolType.h"
#include "net/RemoteResponse.h"
#include "ErrorResponse.h"
#include <set>

#include "InterphoneResponse.h"

namespace protocol
{
	InterphoneResponse::InterphoneResponse(net::RemoteResponse* pRR)
		: Response(pRR)
		, m_actionType(InterphoneRequest::Action_None)
		, m_memberCount(0)
	{
	}

	InterphoneResponse::~InterphoneResponse()
	{

	}

	bool InterphoneResponse::Parse()
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

			char *pszId = 0;
			char *pszAttachType = 0;
			char *pszAttachId = 0;
			char *pszMemberCount = 0;
			char *pszSpeaker = 0;

			if (m_actionType == InterphoneRequest::Action_Sync)
			{
				iks *pnInterphones = iks_find(pnResponse, protocol::TAG_INTERPHONES);
				if (!pnInterphones) break;

				// 判断是否为错误应答
				m_pER = ErrorResponse::Parse(pnInterphones);
				if (m_pER)
				{
					ErrorResponse::Log("Interphones sync error", m_pER);
					bPError = false;
					break;
				}

				m_items.clear();

				// interphone items
				iks *item = iks_first_tag(pnInterphones);
				while (item != 0)
				{
					char *itemName = iks_name(item);
					if (strcmp(itemName, protocol::TAG_INTERPHONE) == 0)
					{
						pszId = iks_find_attrib(item, ATTRIBUTE_NAME_ID);
						pszAttachType = iks_find_attrib(item, ATTRIBUTE_NAME_ATTACHTYPE);
						pszAttachId = iks_find_attrib(item, ATTRIBUTE_NAME_ATTACHID);
						pszMemberCount = iks_find_attrib(item, ATTRIBUTE_NAME_MEMBERCOUNT);

						if (!pszId || !pszMemberCount)
							continue;

						Item item;
						item.m_iid = pszId;
						if (pszAttachType)
							item.m_attachType = pszAttachType;
						if (pszAttachId)
							item.m_attachId = pszAttachId;
						item.m_memberCount = atoi(pszMemberCount);
						m_items.push_back(item);
					}

					item = iks_next_tag(item);
				}
			}
			else if (m_actionType == InterphoneRequest::Action_Member ||
				     m_actionType == InterphoneRequest::Action_Add ||
					 m_actionType == InterphoneRequest::Action_Quit)
			{
				iks *pnInterphone = iks_find(pnResponse, protocol::TAG_INTERPHONE);
				if (!pnInterphone) break;

				// 判断是否为错误应答
				m_pER = ErrorResponse::Parse(pnInterphone);
				if (m_pER)
				{
					ErrorResponse::Log("Interphone member/add/quit error", m_pER);
					bPError = false;
					break;
				}

				pszId = iks_find_attrib(pnInterphone, ATTRIBUTE_NAME_ID);
				pszAttachType = iks_find_attrib(pnInterphone, ATTRIBUTE_NAME_ATTACHTYPE);
				pszAttachId = iks_find_attrib(pnInterphone, ATTRIBUTE_NAME_ATTACHID);
				pszMemberCount = iks_find_attrib(pnInterphone, ATTRIBUTE_NAME_MEMBERCOUNT);
				pszSpeaker = iks_find_attrib(pnInterphone, ATTRIBUTE_NAME_SPEAKER);

				if (!pszId || !pszMemberCount)
					break;

				m_iid = pszId;
				if (pszAttachType)
					m_attachType = pszAttachType;
				if (pszAttachId)
					m_attachId = pszAttachId;
				m_memberCount = atoi(pszMemberCount);
				if (pszSpeaker)
					m_speakId = pszSpeaker;

				// members && parameter
				m_memberIds.clear();
				std::set<std::string> memberIds;
				iks *item = iks_first_tag(pnInterphone);
				while (item != 0)
				{
					char *itemName = iks_name(item);
					if (strcmp(itemName, protocol::TAG_MEMBER) == 0)
					{
						pszId = iks_find_attrib(item, ATTRIBUTE_NAME_ID);
						if (pszId)
						{
							memberIds.insert(pszId);
						}
					}
					else if (strcmp(itemName, protocol::TAG_PARAMETER) == 0)
					{
						iks *audio = iks_first_tag(item);
						if (audio && (strcmp(iks_name(audio), protocol::TAG_AUDIO) == 0))
						{
							char *pszAudioAddr = iks_find_attrib(audio, protocol::ATTRIBUTE_NAME_ADDR);
							if (pszAudioAddr)
								m_audioAddr = pszAudioAddr;
							else
								m_audioAddr = "";
						}
					}

					item = iks_next_tag(item);
				}

				for (std::set<std::string>::iterator it=memberIds.begin(); it!=memberIds.end(); ++it)
				{
					m_memberIds.push_back(*it);
				}
			}
			else if (m_actionType == InterphoneRequest::Action_Speak)
			{
				iks *pnSpeak = iks_find(pnResponse, protocol::TAG_SPEAK);
				if (!pnSpeak) break;

				// 判断是否为错误应答
				m_pER = ErrorResponse::Parse(pnSpeak);
				if (m_pER)
				{
					ErrorResponse::Log("Interphone speak error", m_pER);
					bPError = false;
					break;
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

	void InterphoneResponse::setActionType(InterphoneRequest::ActionType type)
	{
		m_actionType = type;
	}

	InterphoneRequest::ActionType InterphoneResponse::getActionType() const
	{
		return m_actionType;
	}

	std::string InterphoneResponse::interphoneId() const
	{
		return m_iid;
	}

	std::string InterphoneResponse::attachType() const
	{
		return m_attachType;
	}

	std::string InterphoneResponse::attachId() const
	{
		return m_attachId;
	}

	std::string InterphoneResponse::speakId() const
	{
		return m_speakId;
	}

	int InterphoneResponse::memberCount() const
	{
		return m_memberCount;
	}

	std::vector<std::string> InterphoneResponse::memberIds() const
	{
		return m_memberIds;
	}

	std::string InterphoneResponse::audioAddr() const
	{
		return m_audioAddr;
	}

	std::vector<InterphoneResponse::Item> InterphoneResponse::items() const
	{
		return m_items;
	}

}

