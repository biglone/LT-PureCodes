#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "TipNotification.h"

namespace protocol
{
	int TipNotification::In::getNotificationType()
	{
		return protocol::TIP;
	}

	bool TipNotification::In::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)
		do
		{
			if (!pnIks)
			{
				break;
			}

			const char* pszFrom = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROM);
			if (!pszFrom)
			{
				break;
			}

			const char* pszTo = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TO);
			if (!pszTo)
			{
				break;
			}

			const char* pszType = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TYPE);
			if (!pszType)
			{
				break;
			}

			const char* pszAction = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ACTION);

			m_from = pszFrom;
			m_to = pszTo;
			m_type = pszType;
			m_action = (pszAction ? pszAction : "");
			bOk = true;

		} while(false);

		return bOk;
	}

	TipNotification::Out::Out(const char *from, const char *to, const char *type, const char *action)
	{
		m_from = (from ? from : "");
		m_to = (to ? to : "");
		m_type = (type ? type : "");
		m_action = (action ? action : "");
	}

	std::string TipNotification::Out::getBuffer()
	{
		std::string sRet = "";
		
		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, "", 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnTip = iks_insert(pnMessage, protocol::TAG_TIP);
			if (!pnTip) break;

			if (!m_from.empty())
			{
				iks_insert_attrib(pnTip, protocol::ATTRIBUTE_NAME_FROM, m_from.c_str());
			}
			
			if (!m_to.empty())
			{
				iks_insert_attrib(pnTip, protocol::ATTRIBUTE_NAME_TO, m_to.c_str());
			}

			if (!m_type.empty())
			{
				iks_insert_attrib(pnTip, protocol::ATTRIBUTE_NAME_TYPE, m_type.c_str());
			}

			if (!m_action.empty())
			{
				iks_insert_attrib(pnTip, protocol::ATTRIBUTE_NAME_ACTION, m_action.c_str());
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}
}