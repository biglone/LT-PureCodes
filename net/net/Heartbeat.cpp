#include "iks/AutoIks.h"
#include "Heartbeat.h"
#include <string>
using namespace std;

namespace net
{
	SeqGenerator Heartbeat::seqGenerator;

	Heartbeat::Heartbeat()
	{
		m_sSeq = seqGenerator.getSeq();
	}

	std::string Heartbeat::getSeq()
	{
		return m_sSeq;
	}

	std::string Heartbeat::getBuffer()
	{
		string sRet = "";

		do 
		{
			iks* pnHeartbeat = iks_new("hbreq");
			if (!pnHeartbeat) break;

			CAutoIks aMessage(pnHeartbeat);

			const char* pszBuffer = iks_string(iks_stack(pnHeartbeat), pnHeartbeat);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}
}