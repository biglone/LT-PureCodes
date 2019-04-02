#ifndef _MODIFYMESSAGE_H_
#define _MODIFYMESSAGE_H_
#include <string>
#include <map>

#include "net/Request.h"
#include "protocol/Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ModifyMessage
	{
	public:

		enum ModifyType
		{
			Modify_User_Detail      = 0x00000000
		};

		static std::string ModifyTypeToString(ModifyType type);
		static ModifyType ModifyTypeFromString(const std::string& type);
		

		typedef std::map<std::string, std::string> ModifyContent;

		class PROTOCOL_EXPORT ModifyRequest : public net::Request
		{
		public:
			ModifyRequest(ModifyType type = Modify_User_Detail);

			void addContent(const std::string& key, const std::string& sValue);

			virtual int getType();

			std::string getBuffer();

		protected:
			// 有结果
			virtual void onResponse(net::RemoteResponse* response);

		private:
			ModifyType               m_eType;
			ModifyContent            m_Content;
		};

		class PROTOCOL_EXPORT ModifyResponse : public Response
		{
		public:
			ModifyResponse(net::RemoteResponse* pRr);
			bool Parse();

		private:
		};

	private:
		ModifyMessage() {}

	};
}
#endif