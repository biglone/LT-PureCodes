#ifndef _ROSTER_NOTIFICATION_H_
#define _ROSTER_NOTIFICATION_H_

#include "SpecificNotification.h"
#include "RosterNotification.h"
#include "RosterRequest.h"
#include <vector>
#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT RosterNotification : public SpecificNotification
	{
	public:
		RosterNotification();

		bool Parse(iks* pnIks);

		int getNotificationType();

		std::vector<RosterRequest::RosterItem> getRosterItems() const;

	private:
		std::vector<RosterRequest::RosterItem> m_rosterItems;
	};
}

#endif // _ROSTER_NOTIFICATION_H_