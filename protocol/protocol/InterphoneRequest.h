#ifndef _INTERPHONEREQUEST_H_
#define _INTERPHONEREQUEST_H_

#include <string>
#include <vector>
#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT InterphoneRequest : public net::Request
	{
	public:
		enum ActionType 
		{
			Action_None,
			Action_Sync,
			Action_Member,
			Action_Add,
			Action_Quit,
			Action_Speak,
		};

	public:
		InterphoneRequest(ActionType type, const std::string &iid, const std::vector<std::string> &params);

		int getType();

		int actionType() const;

		std::string interphoneId() const;

		std::vector<std::string> params() const;

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		ActionType  m_type;
		std::string m_iid;
		std::vector<std::string> m_params;
	};
}
#endif //_INTERPHONEREQUEST_H_
