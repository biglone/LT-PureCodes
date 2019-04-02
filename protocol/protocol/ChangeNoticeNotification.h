#ifndef _CHANGE_NOTICE_NOTIFICATION_H_
#define _CHANGE_NOTICE_NOTIFICATION_H_

#include "SpecificNotification.h"
#include <vector>
#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ChangeNoticeNotification : public SpecificNotification
	{
	public:
		struct EventParam {
			std::string v;
		};

		struct Event {
			std::string             name;
			EventParam              param;
			
			Event() {name = "";}
		};

	public:
		ChangeNoticeNotification();

		bool Parse(iks* pnIks);

		int getNotificationType();

		std::vector<Event> getEvents() const;

	private:
		std::vector<Event> m_events;
	};
}
#endif