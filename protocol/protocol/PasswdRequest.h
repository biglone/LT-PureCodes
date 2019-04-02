#ifndef _PASSWDREQUEST_H_
#define _PASSWDREQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT PasswdRequest : public net::Request
	{
	public:
		PasswdRequest(const std::string& rsOldPasswd, const std::string& rsNewPasswd);

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		std::string m_sOldPasswd;
		std::string m_sNewPasswd;
	};
}
#endif