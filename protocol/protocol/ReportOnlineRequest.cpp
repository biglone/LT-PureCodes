#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ReportOnlineRequest.h"
#include "ReportOnlineResponse.h"
#include "assert.h"

namespace protocol
{
	ReportOnlineRequest::ReportOnlineRequest(const char *pszId, const char *pszState)
	{
		if (pszId)
			m_id = pszId;
		else
			m_id = "";

		if (pszState)
			m_state = pszState;
		else
			m_state = "";

		assert(m_id.length() > 0);
		assert(m_state.length() > 0);
	}

	int ReportOnlineRequest::getType()
	{
		return protocol::Request_Report_Online;
	}

	std::string ReportOnlineRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_ONLINE, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnMsg = iks_insert(pnMessage, m_state.c_str());
			if (!pnMsg) break;

			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_ID, m_id.c_str());

			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_PLATFORM, "pc");

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while (0);

		return sRet;
	}

	void ReportOnlineRequest::onResponse(net::RemoteResponse* response)
	{
		ReportOnlineResponse* lr = new ReportOnlineResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}
}