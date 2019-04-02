#ifndef _NOTIFICATIONCOMMON_H_
#define _NOTIFICATIONCOMMON_H_

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT NotificationCommon
	{
	public:
		NotificationCommon();

		bool Parse();

		std::string getModel();
		std::string getFrom();
		std::string getName();

	protected:
		void setModel(const std::string& rsModel);
		void setFrom(const std::string& rsFrom);
		void setName(const std::string& rsName);

	private:
		std::string       m_sModel;
		std::string       m_sFrom;
		std::string       m_sName;
	};
}
#endif