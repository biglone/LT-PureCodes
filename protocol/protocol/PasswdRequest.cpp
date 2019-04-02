#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "PasswdResponse.h"
#include "PasswdRequest.h"

namespace protocol
{
	PasswdRequest::PasswdRequest(const std::string& rsOldPasswd, const std::string& rsNewPasswd)
		: m_sOldPasswd(rsOldPasswd)
		, m_sNewPasswd(rsNewPasswd)
	{

	}

	int PasswdRequest::getType()
	{
		return protocol::Request_Base_Password;
	}

	std::string PasswdRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_BASE, 0);
			if (!pnMessage)
			{
				break;
			}

			CAutoIks aMessage(pnMessage);

			iks* pnPasswd = iks_insert(pnMessage, protocol::TAG_PASSWD);
			if (!pnPasswd)
			{
				break;
			}

			iks_insert_attrib(pnPasswd, "old", m_sOldPasswd.c_str());
			iks_insert_attrib(pnPasswd, "new", m_sNewPasswd.c_str());

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void PasswdRequest::onResponse(net::RemoteResponse* response)
	{
		PasswdResponse* pr  = new PasswdResponse(response);
		pr->Parse();
		getProtocolCallback()->onResponse(this, pr);
	}
}