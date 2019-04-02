#ifndef __TIP_NOTIFICATION_H__
#define __TIP_NOTIFICATION_H__

#include <string>
#include "SpecificNotification.h"
#include "net/XmlMsg.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT TipNotification
	{
	private:
		TipNotification() {}
		~TipNotification() {}

	public:

		class PROTOCOL_EXPORT In : public SpecificNotification
		{
		public:
			In() {}

			std::string from() const {return m_from;}
			std::string to() const {return m_to;}
			std::string type() const {return m_type;}
			std::string action() const {return m_action;}

			int getNotificationType();
			
			bool Parse(iks* pnIks);

		public:
			std::string m_from;
			std::string m_to;
			std::string m_type;
			std::string m_action;
		};

		class PROTOCOL_EXPORT Out : public net::XmlMsg
		{
		public:
			Out(const char *from, const char *to, const char *type, const char *action);

			std::string getBuffer();

		public:
			std::string m_from;
			std::string m_to;
			std::string m_type;
			std::string m_action;
		};	
	};
}

#endif // __TIP_NOTIFICATION_H__