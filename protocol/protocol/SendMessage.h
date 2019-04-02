#ifndef _SEND_MESSAGE_H_
#define _SEND_MESSAGE_H_

#include "net/Request.h"
#include "Response.h"
#include "protocol/MessageNotification.h"
#include <string>

#include "protocol_global.h"

namespace protocol 
{
	class PROTOCOL_EXPORT SendMessageRequest : public net::Request
	{
	public:
		SendMessageRequest();

		int getType();

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	public:
		MessageNotification::Message m_Message;
	};

	class PROTOCOL_EXPORT SendMessageResponse : public Response
	{
	public:
		SendMessageResponse(net::RemoteResponse* rr);

		std::string getTimeStamp();

		bool Parse();

	private:
		std::string m_timestamp;
	};
}
#endif
