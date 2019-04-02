#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "HistoryRequest.h"
#include "HistoryResponse.h"

namespace protocol
{

	HistoryRequest::HistoryRequest(protocol::OfflineResponse::ItemType fType, const char* from, const char* ts, 
		Direction direction, int number)
	{
		m_fType = fType;
		if (from && strlen(from) > 0)
			m_from = from;
		else
			m_from = "";
		m_from = from;
		if (ts && strlen(ts) > 0)
			m_ts = ts;
		else
			m_ts = "";
		m_direction = direction;
		m_number = number;
	}

	int HistoryRequest::getType()
	{
		return protocol::Request_Msg_History;
	}

	std::string HistoryRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_MSG, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnMsg = iks_insert(pnMessage, protocol::TAG_MSG);
			if (!pnMsg) break;

			// type
			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TYPE, "history");
			
			// ftype
			if (m_fType == protocol::OfflineResponse::Group)
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_FTYPE, "group");
			else if (m_fType == protocol::OfflineResponse::Discuss)
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_FTYPE, "discuss");
			else
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_FTYPE, "user");
			
			// from
			if (m_from.length() > 0)
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_FROM, m_from.c_str());

			// ts
			if (m_ts.length() > 0)
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS, m_ts.c_str());

			// sd
			if (m_direction == Backward)
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_SD, "backward");
			else
				iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_SD, "forward");

			// number
			char pszNumber[128];
			itoa(m_number, pszNumber, 10);
			iks_insert_attrib(pnMsg, protocol::ATTRIBUTE_NAME_NUMBER, pszNumber);

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while (0);

		return sRet;
	}

	void HistoryRequest::onResponse(net::RemoteResponse* response)
	{
		HistoryResponse* lr = new HistoryResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}
}