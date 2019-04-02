#ifndef _PRESENCENOTIFICATION_H_
#define _PRESENCENOTIFICATION_H_
#include <string>

#include "SpecificNotification.h"
#include "net/XmlMsg.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT PresenceNotification
	{
	private:
		PresenceNotification() {}
		~PresenceNotification() {}

	public:

		enum PresenceType
		{
			Presence_None         = 0x00000000,
			Presence_Unavailable  = 0x00000001,
			Presence_Subscribe    = 0x00000002,
			Presence_Subscribed   = 0x00000003,
			Presence_Unsubscribed = 0x00000004,
			Presence_Unsubscribe  = 0x00000005
		};

		enum Ttype
		{
			Terminal_None         = 0x00000000,
			Terminal_PC           = 0x00000001,
			Terminal_Android      = 0x00000002,
			Terminal_IPhone       = 0x00000003,
			Terminal_Web          = 0x00000004
		};

		enum Show
		{
			Show_None             = 0x00000000,
			Show_Away             = 0x00000001,
			Show_Chat             = 0x00000002,
			Show_DND              = 0x00000003,
			Show_XA               = 0x00000004
		};

		static PresenceType getPresenceType(const std::string& rsType);
		static std::string presenceTypeString(PresenceNotification::PresenceType eType);

		static PresenceNotification::Ttype getTerminalType(const std::string& rsTtype);
		static std::string terminalTypeString(PresenceNotification::Ttype eTtype);

		static PresenceNotification::Show getShow(const std::string& rsShow);
		static std::string showString(PresenceNotification::Show eShow);

		struct PROTOCOL_EXPORT PresenceInfo
		{
			PresenceInfo() : type(Presence_None), ttype(Terminal_None), show(Show_None) {}

			std::string  from;
			std::string  to;

			PresenceType type;
			Ttype        ttype;
			Show         show;

			std::string  status;
		};

		class PROTOCOL_EXPORT In : public SpecificNotification
		{
		public:
			int getNotificationType();
			
			bool Parse(iks* pnIks);

		public:
			PresenceInfo presenceInfo;
		};

		class PROTOCOL_EXPORT Out : public net::XmlMsg
		{
		public:
			Out(PresenceType eType, Ttype eTtype = Terminal_None, Show eShow = Show_None, const std::string& rsStatus = "");

			std::string getBuffer();

		public:
			PresenceInfo m_PresenceInfo;
		};	
	};
}
#endif