#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "OfflineRequest.h"
#include "OfflineResponse.h"

namespace protocol
{

	OfflineRequest::OfflineRequest(const char *pszTs, const char *pszTs2 /*= 0*/)
	{
		if (pszTs && strlen(pszTs) > 0)
			m_ts = pszTs;
		else
			m_ts = "";

		if (pszTs2 && strlen(pszTs2) > 0)
			m_ts2 = pszTs2;
		else
			m_ts2 = "";
	}

	int OfflineRequest::getType()
	{
		return protocol::Request_Msg_Offline;
	}

	std::string OfflineRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_MSG, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnMsg = iks_insert(pnMessage, protocol::TAG_MSG);
			if (!pnMsg) break;

			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TYPE, "offline");

			if (m_ts.length() > 0)
			{
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS, m_ts.c_str());
			}

			if (m_ts2.length() > 0)
			{
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS2, m_ts2.c_str());
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while (0);

		return sRet;
	}

	void OfflineRequest::onResponse(net::RemoteResponse* response)
	{
		OfflineResponse* lr = new OfflineResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}
}