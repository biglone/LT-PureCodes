#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "SessionRingingNotification.h"

namespace protocol
{
	int SessionRingingNotifiction::In::getNotificationType()
	{
		return protocol::SESSION_RINGING;
	}

	bool SessionRingingNotifiction::In::Parse( iks* pnIks )
	{
		bool isOk = false;
		do
		{
			if (!pnIks)
			{
				break;
			}

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

			param.sp = getSpFrom();
			param.id = pszId;
			param.from = pszFrom;
			param.to    = pszTo;

			// validate
			if (!param.isValid())
			{
				break;
			}

			isOk = true;
		}while(0);

		return isOk;
	}

	std::string SessionRingingNotifiction::Out::getBuffer()
	{
		string sRet = "";

		do
		{
            iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, param.sp.empty() ? 0 : param.sp.c_str(), protocol::ATTRIBUTE_MODULE_RTC, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnRinging = iks_insert(pnMessage, protocol::TAG_RINGING);
			if (!pnRinging) break;

			iks_insert_attrib(pnRinging, protocol::ATTRIBUTE_NAME_ID, param.id.c_str());
			iks_insert_attrib(pnRinging, protocol::ATTRIBUTE_NAME_FROM, param.from.c_str());
			iks_insert_attrib(pnRinging, protocol::ATTRIBUTE_NAME_TO, param.to.c_str());

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

}
