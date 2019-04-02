#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "InterphoneNotification.h"

namespace protocol
{
	int InterphoneNotification::getNotificationType()
	{
		return protocol::INTERPHONE;
	}

	bool InterphoneNotification::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)
		do
		{
			if (!pnIks)
			{
				break;
			}

			char *pszId = 0;
			char *pszAttachType = 0;
			char *pszAttachId = 0;
			char *pszMemberCount = 0;
			char *pszSpeaker = 0;

			pszId = iks_find_attrib(pnIks, ATTRIBUTE_NAME_ID);
			pszAttachType = iks_find_attrib(pnIks, ATTRIBUTE_NAME_ATTACHTYPE);
			pszAttachId = iks_find_attrib(pnIks, ATTRIBUTE_NAME_ATTACHID);
			pszMemberCount = iks_find_attrib(pnIks, ATTRIBUTE_NAME_MEMBERCOUNT);
			pszSpeaker = iks_find_attrib(pnIks, ATTRIBUTE_NAME_SPEAKER);

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

			bOk = true;

		}while(false);

		return bOk;
	}

	std::string InterphoneNotification::interphoneId() const
	{
		return m_iid;
	}

	std::string InterphoneNotification::attachType() const
	{
		return m_attachType;
	}

	std::string InterphoneNotification::attachId() const
	{
		return m_attachId;
	}

	std::string InterphoneNotification::speakId() const
	{
		return m_speakId;
	}

	int InterphoneNotification::memberCount() const
	{
		return m_memberCount;
	}

}