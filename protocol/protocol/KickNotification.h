#ifndef _KICKNOTIFICATION_H_
#define _KICKNOTIFICATION_H_

#include "SpecificNotification.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT KickNotification : public SpecificNotification
	{
	public:
		KickNotification();

		bool Parse(iks* pnIks);

		int getNotificationType();

		std::string getContent();

	private:
		void setContent(const std::string& rsContent);

	private:
		std::string m_sContent;	
	};
}
#endif