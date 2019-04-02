#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "SessionByeNotification.h"

namespace protocol
{
	int SessionByeNotification::In::getNotificationType()
	{
		return protocol::SESSION_BYE;
	}

	bool SessionByeNotification::In::Parse( iks* pnIks )
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

			char *reasonType = 0;
			char *reasonDesc = 0;
			do {
				iks* pnReason = iks_find(pnIks, protocol::TAG_REASON);
				if (!pnReason)
				{
					break;
				}

				reasonType = iks_find_attrib(pnReason, protocol::ATTRIBUTE_NAME_TYPE);
				reasonDesc = iks_find_attrib(pnReason, protocol::ATTRIBUTE_NAME_DESC);

			} while(0);


			param.sp = getSpFrom();
			param.id = pszId;
			param.from = pszFrom;
			param.to = pszTo;
			param.reasonType = reasonType ? reasonType : "";
			param.reasonDesc = reasonDesc ? reasonDesc : "";

			// validate
			if (!param.isValid())
			{
				break;
			}

			isOk = true;
		}while(0);

		return isOk;
	}

	std::string SessionByeNotification::Out::getBuffer()
	{
		string sRet = "";

		do
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, param.sp.empty() ? 0 : param.sp.c_str(), protocol::ATTRIBUTE_MODULE_RTC, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnBye = iks_insert(pnMessage, protocol::TAG_BYE);
			if (!pnBye) break;

			iks_insert_attrib(pnBye, protocol::ATTRIBUTE_NAME_ID, param.id.c_str());
			iks_insert_attrib(pnBye, protocol::ATTRIBUTE_NAME_FROM, param.from.c_str());
			iks_insert_attrib(pnBye, protocol::ATTRIBUTE_NAME_TO, param.to.c_str());

			iks *pnReason = iks_insert(pnBye, protocol::TAG_REASON);
			if (!pnReason) break;

			iks_insert_attrib(pnReason, protocol::ATTRIBUTE_NAME_TYPE, param.reasonType.c_str());
			iks_insert_attrib(pnReason, protocol::ATTRIBUTE_NAME_DESC, param.reasonDesc.c_str());

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;

		} while(0);

		return sRet;
	}

}
