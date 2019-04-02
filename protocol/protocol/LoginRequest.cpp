#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "LoginResponse.h"
#include "LoginRequest.h"

namespace protocol
{

	LoginRequest::LoginRequest(const std::string& rsUser, 
		const std::string& rsPasswd,
		const std::string& rsResource,
		const std::string& rsPlatform,
		bool bViolent,
		bool bBalance)
	{
		m_sUser = rsUser;
		m_sPasswd = rsPasswd;
		m_sResource = rsResource;
		m_sPlatform = rsPlatform;
		m_bViolent = bViolent;
		m_bBalance = bBalance;
	}

	int LoginRequest::getType()
	{
		return protocol::Request_Base_Login;
	}

	std::string LoginRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_BASE, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnLogin = iks_insert(pnMessage, protocol::TAG_LOGIN);
			if (!pnLogin) break;

			iks_insert_attrib(pnLogin, protocol::ATTRIBUTE_NAME_ID, m_sUser.c_str());
			iks_insert_attrib(pnLogin, protocol::ATTRIBUTE_NAME_PASSWD, m_sPasswd.c_str());
			iks_insert_attrib(pnLogin, protocol::ATTRIBUTE_NAME_RESOURCE, m_sResource.c_str());
			iks_insert_attrib(pnLogin, protocol::ATTRIBUTE_NAME_PLATFORM, m_sPlatform.c_str());
			iks_insert_attrib(pnLogin, protocol::ATTRIBUTE_NAME_VIOLENT, m_bViolent ? "true" : "false");
			iks_insert_attrib(pnLogin, protocol::ATTRIBUTE_NAME_BALANCE, m_bBalance ? "true" : "false");

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while (0);

		return sRet;
	}

	void LoginRequest::onResponse(net::RemoteResponse* response)
	{
		LoginResponse* lr = new LoginResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}
}