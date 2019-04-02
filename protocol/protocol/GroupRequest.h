#ifndef _GROUPREQUEST_H_
#define _GROUPREQUEST_H_

#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT GroupRequest : public net::Request
	{
	public:
		GroupRequest(const std::string& rsId = "");

		std::string getGroupId() const;

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		std::string m_sId;
	};

	class PROTOCOL_EXPORT GroupCardNameRequest : public net::Request
	{
	public:
		GroupCardNameRequest(const std::string &rsId, const std::string &rsUid, const std::string &rsCardName);

		int getType();

		std::string getBuffer();

		std::string getGroupId();

	protected:
		void onResponse(net::RemoteResponse *response);

	private:
		std::string m_sId;
		std::string m_sUid;
		std::string m_sCardName;
	};
}
#endif