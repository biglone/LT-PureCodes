#ifndef _ROSTERREQUEST_H_
#define _ROSTERREQUEST_H_

#include <string>
#include <vector>

#include "iks/iksemel.h"
#include "net/Request.h"

#include "protocol_global.h"

class CRosterItem;
namespace protocol
{
	class PROTOCOL_EXPORT RosterRequest : public net::Request
	{
	public:
		enum ActionType
		{
			Action_None,
			Action_Sync,
			Action_Modify
		};

		enum ModifyType
		{
			ModifyNone,
			ModifyAdd,
			ModifyDelete
		};

		enum ClientType
		{
			ClientSyncRoster,   // sync roster
			ClientAddRoster,    // add roster
			ClientRemoveRoster, // remove roster
			ClientChangeGroup,  // move roster to a new group
			ClientChangeName    // modify a roster's name, without changing the group of the roster
		};

		class RosterItem
		{
		public:
			ModifyType  m_modifyType;
			std::string m_id;
			std::string m_name;
			std::string m_group;

			RosterItem() { m_modifyType = ModifyNone; }
			RosterItem(const char *id, const char *name, const char *group) 
			{
				m_modifyType = ModifyNone;
				m_id = "";
				if (id)
					m_id = id;
				m_name = "";
				if (name)
					m_name = name;
				m_group = "";
				if (group)
					m_group = group;
			}
		};

		public:
			RosterRequest(ActionType eType = Action_Sync, const char *syncVersion = 0);
			virtual ~RosterRequest();

		public:
			int getType();
			std::string getBuffer();

			void setClientType(ClientType clientType);
			ClientType clientType() const;
			ActionType getActionType() const;
			std::string getSyncVersion() const;
			void addRosterItem(ModifyType modifyType, const char *id, const char *name, const char *group);
			void clearRosterItems();
			std::vector<RosterItem> getRosterItems() const;

		protected:
			void onResponse(net::RemoteResponse* response);

		private:
			ActionType              m_actionType;
			ClientType              m_clientType;
			std::string             m_syncVersion;
			std::vector<RosterItem> m_rosterItems;
	};
}

#endif // _ROSTERREQUEST_H_