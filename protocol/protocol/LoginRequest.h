#ifndef _LOGINREQUEST_H_
#define _LOGINREQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT LoginRequest : public net::Request
	{
	public:
		LoginRequest(const std::string& rsUser, const std::string& rsPasswd, 
			const std::string& rsResource, const std::string& rsPlatform, 
			bool bViolent, bool bBalance);

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		std::string         m_sUser;
		std::string         m_sPasswd;
		std::string         m_sResource;
		std::string         m_sPlatform;
		bool                m_bViolent;
		bool                m_bBalance;
	};
}
#endif