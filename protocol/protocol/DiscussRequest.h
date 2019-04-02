#ifndef _DISCUSSREQUEST_H_
#define _DISCUSSREQUEST_H_

#include <string>
#include <vector>
#include "net/Request.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT DiscussRequest : public net::Request
	{
	public:
		enum ActionType 
		{
			Action_None,
			Action_Create,
			Action_Add,
			Action_Quit,
			Action_Sync,
			Action_ChangeName,
			Action_ChangeCardName
		};

		class DiscussItem 
		{
		public:
			std::string id;
			std::string name;
			std::string creator;
			std::string time;
			std::string version;
			std::string cardName;

			DiscussItem() {}
			DiscussItem(const char *pszId, const char *pszName)
			{
				if (pszId)
				{
					id = pszId;
				}

				if (pszName)
				{
					name = pszName;
				}
			}
		};
	public:
		DiscussRequest(ActionType type = Action_Sync, const std::string& rsIdorName= "");

		void addDiscussItem(const std::string &rsId, const std::string &rsName = "");
		void clearDiscussItems();

		std::string getIdorName() const;

		int getType();

		int actionType() const;

		std::string getBuffer();

	protected:
		void onResponse(net::RemoteResponse* response);

	private:
		ActionType  m_eType;
		std::string m_sIdorName;
		std::vector<DiscussItem> m_lstItems;
	};
}
#endif //_DISCUSSREQUEST_H_
