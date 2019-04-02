#ifndef _GROUPRESPONSE_H_
#define _GROUPRESPONSE_H_

#include <list>
#include "Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT GroupResponse : public Response
	{
	public:
		static const int kInvalidLogoVersion = -1;

		struct Item
		{
			Item() : id(""), name(""), index(0), cardName(""), logoVersion(kInvalidLogoVersion), annt(""), version("") {}

			std::string id;
			std::string name;
			int         index;
			std::string cardName;
			int         logoVersion;
			std::string annt;
			std::string version;
		};

	public:
		GroupResponse(net::RemoteResponse* pRr);

		bool Parse();

		std::list<GroupResponse::Item> getItems();
		std::string getGroupId();
		std::string getGroupDesc();
		std::string getGroupVersion();
		bool isChild();

	private:
		void setChild(bool bIsChild);
		void setGroupId(const std::string& rsGroupId);

	private:
		std::list<Item>     m_listItems;
		bool                m_bIsChild;
		std::string         m_sGroupId;
		std::string         m_sDesc;
		std::string         m_sVersion;
	};

	class PROTOCOL_EXPORT GroupCardNameResponse : public Response
	{
	public:
		GroupCardNameResponse(net::RemoteResponse *pRr);

		bool Parse();

		std::string getGroupId();

	private:
		std::string         m_sGroupId;
	};
}
#endif
