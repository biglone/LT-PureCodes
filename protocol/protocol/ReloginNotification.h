#ifndef _RELOGINNOTIFICATION_H_
#define _RELOGINNOTIFICATION_H_

#include <list>
#include "SpecificNotification.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT ReloginNotification : public SpecificNotification
	{
	public:
		ReloginNotification();

		bool Parse(iks* pnIks);

		int getNotificationType();


		std::list<std::string> getPsgs() const;

	private:
		std::list<std::string> m_psgs;

	};
}
#endif //_RELOGINNOTIFICATION_H_