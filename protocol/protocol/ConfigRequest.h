#ifndef _CONFIGREQUEST_H_
#define _CONFIGREQUEST_H_

#include <string>
#include <map>
#include "net/Request.h"
#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ConfigRequest : public net::Request
	{
	public:
		enum ActionType 
		{
			Action_None,
			Action_Get,
			Action_Set
		};

	public:
		ConfigRequest(ActionType type = Action_Get);

		void setConfigData(const std::string &name, const std::string &content);

		int getType();

		int actionType() const;

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		ActionType                         m_eType;
		std::map<std::string, std::string> m_configData;
	};
}
#endif //_DISCUSSREQUEST_H_
