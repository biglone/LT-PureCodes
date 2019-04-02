#ifndef _SESSIONRINGINGNOTIFICATION_H_
#define _SESSIONRINGINGNOTIFICATION_H_

#include "protocol/SpecificNotification.h"
#include "net/XmlMsg.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT SessionRingingNotifiction
	{
	public:
		struct Param
		{
			std::string                          sp;
			std::string                          id;
			std::string                          from;
			std::string                          to;

			Param() {}

			bool isValid()
			{
				if (sp.empty() || from.empty() || to.empty() || id.empty())
					return false;

				return true;
			}
		};

		class PROTOCOL_EXPORT In : public protocol::SpecificNotification
		{
		public:
			In() {}
			~In() {}

			int getNotificationType();

			bool Parse(iks* pnIks);

		public:
			Param param;
		};

		class PROTOCOL_EXPORT Out : public net::XmlMsg
		{
		public:
			std::string getBuffer();

		public:
			Param param;
		};	
	};

}

#endif // _SESSIONRINGINGNOTIFICATION_H_
