#ifndef _LOGINRESPONSE_H_
#define _LOGINRESPONSE_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include "iks/iksemel.h"
#include "base/Base.h"

#include "Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT LoginResponse : public Response
	{
	public:
		struct RtcParam
		{
			std::vector<std::string> urls;
			std::string username;
			std::string credential;
		};

	public:
		LoginResponse(net::RemoteResponse* rr);

		bool Parse();

		// std::map<std::string, std::string> getPamams();
		std::list<std::string> getModule();
		std::map<std::string, base::AddressMap > getMapService();
		std::list<std::string> getPsgs() const;
		RtcParam getRtcParam() const;

	private:
		// bool addParam(const std::string& rsId, const std::string& rsValue);
		void addModule(const std::string& rsId);
		bool addService(const std::string& rsId, iks* pnIks);
		bool addRtcParam(iks* pnIks);

	private:
		// std::map<std::string, std::string>       m_mapParm;
		std::list<std::string>                   m_listModule;
		std::map<std::string, base::AddressMap > m_mapServices;
		std::list<std::string>                   m_psgs;
		RtcParam                                 m_rtcParam;
	};
}

#endif