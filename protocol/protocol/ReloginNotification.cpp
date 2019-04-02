#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ReloginNotification.h"

namespace protocol
{
	ReloginNotification::ReloginNotification()
	{}

	bool ReloginNotification::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)

		do
		{
			if (!pnIks)
				break;

			//<psgs>
			//   <psg id='58.211.187.150:8801/10.35.128.64:8801' main='1'/>
			// 　<psg id='58.211.187.150:8803/10.35.128.62:8801'/>
			//</psgs>

			iks* pnPsgs = iks_find(pnIks, protocol::TAG_PSGS);
			if (!pnPsgs)
			{
				break;
			}

			// PSG
			// psg
			iks* pnTag = iks_first_tag(pnPsgs);
			while (pnTag)
			{
				if (strcmp(protocol::TAG_PSG, iks_name(pnTag)) != 0)
				{
					continue;
				}

				// id
				char* pszId = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_ID);
				if (!pszId || strlen(pszId) <= 0)
					continue;

				// main
				char* pszMain = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_MAIN);
				if (pszMain && strlen(pszMain) > 0 && atoi(pszMain) > 0)
				{
					m_psgs.insert(m_psgs.begin(), pszId);
				}
				else
				{
					m_psgs.push_back(pszId);
				}

				pnTag = iks_next_tag(pnTag);
			}

			bOk = true;
		}while(false);

		return bOk;
	}

	int ReloginNotification::getNotificationType()
	{
		return protocol::RELOGIN;
	}

	std::list<std::string> ReloginNotification::getPsgs() const
	{
		return m_psgs;
	}
}