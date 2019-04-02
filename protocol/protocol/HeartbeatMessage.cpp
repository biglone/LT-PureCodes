#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "HeartbeatMessage.h"

namespace protocol
{
	// in
	int HeartbeatMessage::In::getNotificationType()
	{
		return protocol::HEARTBEAT;
	}

	std::string HeartbeatMessage::In::getSeq() const
	{
		return m_sSeq;
	}

	bool HeartbeatMessage::In::Parse(iks* pnIks)
	{
		bool isOk = false;
		do
		{
			if (!pnIks)
			{
				break;
			}

			const char* pszSeq = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_SEQ);
			if (!pszSeq)
			{
				break;
			}

			m_sSeq = pszSeq;
			isOk = true;
		}while(0);

		return isOk;
	}


	// out
	net::SeqGenerator HeartbeatMessage::Out::s_seqGenerator;

	HeartbeatMessage::Out::Out()
	{
		m_sSeq = s_seqGenerator.getSeq();
	}

	std::string HeartbeatMessage::Out::getBuffer()
	{
		std::string sRet = "";
		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnHeartbeat = iks_insert(pnMessage, protocol::TAG_HEARTBEAT);
			if (!pnHeartbeat) break;

			iks_insert_attrib(pnHeartbeat, protocol::ATTRIBUTE_NAME_TYPE, "request");
			iks_insert_attrib(pnHeartbeat, protocol::ATTRIBUTE_NAME_SEQ, m_sSeq.c_str());

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while (0);

		return sRet;		
	}
}