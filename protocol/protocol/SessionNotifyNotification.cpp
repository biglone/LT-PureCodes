#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "SessionNotifyNotification.h"

namespace protocol
{
	int SessionNotifyNotifiction::getNotificationType()
	{
		return protocol::SESSION_NOTIFY;
	}

	bool SessionNotifyNotifiction::Parse( iks* pnIks )
	{
		bool isOk = false;
		do
		{
			if (!pnIks)
			{
				break;
			}

			//<message type='notification' module='MID_RTC' to='1111' from='sps1'>
			//<notify id='guid' from='1111' to='2222' fullto='2222/phone' action='reject' />
			//</notify>
			//</message>

			const char* pszId = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ID);
			if (!pszId || strlen(pszId) <= 0)
			{
				break;
			}

			const char* pszFrom = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROM);
			if (!pszFrom || strlen(pszFrom) <= 0)
			{
				break;
			}

			const char* pszTo = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TO);
			if (!pszTo || strlen(pszTo) <= 0)
			{
				break;
			}

			const char* pszFullTo = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FULLTO);
			if (!pszFullTo || strlen(pszFullTo) <= 0)
			{
				break;
			}

			const char* pszAction = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ACTION);
			if (!pszAction || strlen(pszAction) <= 0)
			{
				break;
			}

			m_sId = pszId;
			m_fromId = pszFrom;
			m_toId = pszTo;
			m_toFullId = pszFullTo;
			m_action = pszAction;

			isOk = true;
		}while(0);

		return isOk;
	}

}
