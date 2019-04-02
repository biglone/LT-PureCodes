#ifndef _INTERPHONENOTIFICATION_H_
#define _INTERPHONENOTIFICATION_H_

#include <vector>
#include <string>

#include "SpecificNotification.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT InterphoneNotification : public SpecificNotification
	{

	public:
		int getNotificationType();

		bool Parse(iks* pnIks);

		std::string interphoneId() const;
		std::string attachType() const;
		std::string attachId() const;
		std::string speakId() const;
		int memberCount() const;

	private:
		std::string                              m_iid;
		std::string                              m_attachType;
		std::string                              m_attachId;
		std::string                              m_speakId;
		int                                      m_memberCount;
	};

}
#endif //_INTERPHONENOTIFICATION_H_
