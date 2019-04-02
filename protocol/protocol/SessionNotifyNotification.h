#ifndef _SESSIONNOTIFYNOTIFICATION_H_
#define _SESSIONNOTIFYNOTIFICATION_H_

#include "protocol/SpecificNotification.h"
#include "protocol_global.h"
#include <string>

namespace protocol
{
	class PROTOCOL_EXPORT SessionNotifyNotifiction : public protocol::SpecificNotification
	{
	public:
		SessionNotifyNotifiction() {}
		~SessionNotifyNotifiction() {}

		int getNotificationType();

		bool Parse(iks* pnIks);

		std::string sessionId() const {return m_sId;}
		std::string fromId() const {return m_fromId;}
		std::string toId() const {return m_toId;}
		std::string toFullId() const {return m_toFullId;}
		std::string action() const {return m_action;}
	
	public:
		std::string m_sId;
		std::string m_fromId;
		std::string m_toId;
		std::string m_toFullId;
		std::string m_action;
	};
}

#endif // _SESSIONRINGINGNOTIFICATION_H_
