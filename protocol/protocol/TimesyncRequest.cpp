#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "TimesyncResponse.h"
#include "TimesyncRequest.h"

namespace protocol
{
	int TimesyncRequest::getType()
	{
		return protocol::Request_Base_Timesync;
	}

	std::string TimesyncRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_BASE, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnTimeSync = iks_insert(pnMessage, protocol::TAG_TIMESYNC);
			if (!pnTimeSync) break;

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void TimesyncRequest::onResponse(net::RemoteResponse* response)
	{
		TimesyncResponse* tr = new TimesyncResponse(response);
		tr->Parse();
		getProtocolCallback()->onResponse(this, tr);
	}
}