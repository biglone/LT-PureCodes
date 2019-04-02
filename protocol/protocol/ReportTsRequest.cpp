#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ReportTsRequest.h"
#include "ReportTsResponse.h"

namespace protocol
{

	ReportTsRequest::ReportTsRequest(const char *pszTs)
	{
		if (pszTs && strlen(pszTs) > 0)
			m_ts = pszTs;
		else
			m_ts = "";
	}

	int ReportTsRequest::getType()
	{
		return protocol::Request_Report_Ts;
	}

	std::string ReportTsRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_MSG, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnMsg = iks_insert(pnMessage, protocol::TAG_MSG);
			if (!pnMsg) break;

			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TYPE, "report");

			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS, m_ts.c_str());

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while (0);

		return sRet;
	}

	void ReportTsRequest::onResponse(net::RemoteResponse* response)
	{
		ReportTsResponse* lr = new ReportTsResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}
}