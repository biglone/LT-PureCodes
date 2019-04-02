#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "LogoutResponse.h"
#include "LogoutRequest.h"

namespace protocol
{
	int LogoutRequest::getType()
	{
		return protocol::Request_Base_Logout;
	}

	std::string LogoutRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_BASE, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnLogout = iks_insert(pnMessage, protocol::TAG_LOGOUT);
			if (!pnLogout) break;

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void LogoutRequest::onResponse(net::RemoteResponse* response)
	{
		LogoutResponse* lr = new LogoutResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}
}