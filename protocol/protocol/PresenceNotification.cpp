#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "PresenceNotification.h"

static const char UNAVAILABLE[]      = "unavailable";
static const char SUBSCRIBE[]        = "subscribe";
static const char SUBSCRIBED[]       = "subscribed";
static const char UNSUBSCRIBED[]     = "unsubscribed";
static const char UNSUBSCRIBE[]      = "unsubscribe";

static const char PC[]               = "pc";
static const char MOBILE[]           = "mobile";
static const char ANDROID[]          = "android";
static const char IPHONE[]           = "iphone";
static const char WEB[]              = "web";

static const char AWAY[]            = "away";
static const char CHAT[]            = "chat";
static const char DND[]             = "dnd";
static const char XA[]              = "xa";

namespace protocol
{
	PresenceNotification::PresenceType PresenceNotification::getPresenceType(const std::string& rsType)
	{
		PresenceNotification::PresenceType eRet = PresenceNotification::Presence_None;

		if (!rsType.compare(UNAVAILABLE))
		{
			eRet = PresenceNotification::Presence_Unavailable;
		}
		else if (!rsType.compare(SUBSCRIBE))
		{
			eRet = PresenceNotification::Presence_Subscribe;
		}
		else if (!rsType.compare(SUBSCRIBED))
		{
			eRet = PresenceNotification::Presence_Subscribed;
		}
		else if (!rsType.compare(UNSUBSCRIBE))
		{
			eRet = PresenceNotification::Presence_Unavailable;
		}
		else if (!rsType.compare(UNSUBSCRIBED))
		{
			eRet = PresenceNotification::Presence_Unsubscribed;
		}

		return eRet;
	}

	std::string PresenceNotification::presenceTypeString(PresenceNotification::PresenceType eType)
	{
		std::string sRet = "";

		switch (eType)
		{
		case PresenceNotification::Presence_Unavailable:
			sRet = UNAVAILABLE;
			break;
		case PresenceNotification::Presence_Subscribe:
			sRet = SUBSCRIBE;
			break;
		case PresenceNotification::Presence_Subscribed:
			sRet = SUBSCRIBED;
			break;
		case PresenceNotification::Presence_Unsubscribe:
			sRet = UNSUBSCRIBE;
			break;
		case PresenceNotification::Presence_Unsubscribed:
			sRet = UNSUBSCRIBED;
			break;				
		}

		return sRet;
	}

	PresenceNotification::Ttype PresenceNotification::getTerminalType(const std::string& rsTtype)
	{
		PresenceNotification::Ttype eTtype = PresenceNotification::Terminal_None;
		
		if (!rsTtype.compare(PC))
		{
			eTtype = PresenceNotification::Terminal_PC;
		}
		else if (!rsTtype.compare(ANDROID))
		{
			eTtype = PresenceNotification::Terminal_Android;
		}
		else if (!rsTtype.compare(IPHONE))
		{
			eTtype = PresenceNotification::Terminal_IPhone;
		}
		else if (!rsTtype.compare(WEB))
		{
			eTtype = PresenceNotification::Terminal_Web;
		}
		else
		{
			eTtype = PresenceNotification::Terminal_PC;
		}

		return eTtype;
	}

	std::string PresenceNotification::terminalTypeString(PresenceNotification::Ttype eTtype)
	{
		std::string sRet = "";

		switch (eTtype)
		{
		case PresenceNotification::Terminal_PC:
			sRet = PC;
			break;
		case PresenceNotification::Terminal_Android:
			sRet = ANDROID;
			break;
		case PresenceNotification::Terminal_IPhone:
			sRet = IPHONE;
			break;
		case PresenceNotification::Terminal_Web:
			sRet = WEB;
			break;
		default:
			break;
		}

		return sRet;
	}

	PresenceNotification::Show PresenceNotification::getShow(const std::string& rsShow)
	{
		PresenceNotification::Show eRet = PresenceNotification::Show_None;

		if (!rsShow.compare(AWAY))
		{
			eRet = PresenceNotification::Show_Away;
		}
		else if (!rsShow.compare(CHAT))
		{
			eRet = PresenceNotification::Show_Chat;
		}
		else if (!rsShow.compare(DND))
		{
			eRet = PresenceNotification::Show_DND;
		}
		else if (!rsShow.compare(XA))
		{
			eRet = PresenceNotification::Show_XA;
		}

		return eRet;
	}

	std::string PresenceNotification::showString(PresenceNotification::Show eShow)
	{
		std::string sRet = "";

		switch (eShow)
		{
		case PresenceNotification::Show_Away:
			sRet = AWAY;
			break;
		case PresenceNotification::Show_Chat:
			sRet = CHAT;
			break;
		case PresenceNotification::Show_DND:
			sRet = DND;
			break;
		case PresenceNotification::Show_XA:
			sRet = XA;
			break;
		}

		return sRet;
	}

	int PresenceNotification::In::getNotificationType()
	{
		return protocol::PRESENCE;
	}

	bool PresenceNotification::In::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)
		do
		{
			if (!pnIks)
			{
				break;
			}

			const char* pszFrom = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROM);
			if (!pszFrom)
			{
				break;
			}

			const char* pszTo = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TO);
			if (!pszTo)
			{
				break;
			}

			const char* pszType = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TYPE);

			const char* pszTtype = 0;
			do 
			{
				iks* pnTType = iks_find(pnIks, TAG_TTYPE);

				if (!pnTType) break;

				iks* pnTTypeCdata = iks_child(pnTType);
				if (!pnTTypeCdata) break;

				pszTtype = iks_cdata(pnTTypeCdata);
			} while (0);

			const char* pszShow = 0;
			do 
			{
				iks* pnShow = iks_find(pnIks, TAG_SHOW);
				if (!pnShow) break;

				iks* pnShowCdata = iks_child(pnShow);
				if (!pnShowCdata) break;

				pszShow = iks_cdata(pnShowCdata);
			} while (0);

			const char* pszStatus = 0;
			do 
			{
				iks* pnStatus = iks_find(pnIks, TAG_STATUS);
				if (!pnStatus) break;

				iks* pnStatusCdata = iks_child(pnStatus);
				if (!pnStatusCdata) break;

				pszStatus = iks_cdata(pnStatusCdata);
			} while (0);

			presenceInfo.from = pszFrom;
			presenceInfo.to = pszTo;
			presenceInfo.type = protocol::PresenceNotification::getPresenceType(pszType ? pszType : "");
			presenceInfo.ttype = protocol::PresenceNotification::getTerminalType(pszTtype ? pszTtype : "");
			presenceInfo.show = protocol::PresenceNotification::getShow(pszShow ? pszShow : "");
			presenceInfo.status = pszStatus ? pszStatus : "";

			bOk = true;
		}while(false);

		return bOk;
	}

	PresenceNotification::Out::Out(PresenceType eType, Ttype eTtype/* = Terminal_Invaild*/, Show eShow/* = Show_Invaild*/, const std::string& rsStatus/* = ""*/)
	{
		m_PresenceInfo.type = eType;
		m_PresenceInfo.ttype = eTtype;
		m_PresenceInfo.show = eShow;
		m_PresenceInfo.status = rsStatus;
	}

	std::string PresenceNotification::Out::getBuffer()
	{
		std::string sRet = "";
		
		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, "", 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnPresence = iks_insert(pnMessage, protocol::TAG_PRESENCE);
			if (!pnPresence) break;

			std::string sType = presenceTypeString(m_PresenceInfo.type);
			if (!sType.empty())
			{
				iks_insert_attrib(pnPresence, protocol::ATTRIBUTE_NAME_TYPE, sType.c_str());
			}
			
			std::string sTtype = terminalTypeString(m_PresenceInfo.ttype);
			if (!sTtype.empty())
			{
				iks* pnTType = iks_insert(pnPresence, protocol::TAG_TTYPE);
				iks_insert_cdata(pnTType, sTtype.c_str(), 0);
			}

			std::string sShow = showString(m_PresenceInfo.show);
			if (!sShow.empty())
			{
				iks* pnShow = iks_insert(pnPresence, protocol::TAG_SHOW);
				iks_insert_cdata(pnShow, sShow.c_str(), 0);

			}

			if (!m_PresenceInfo.status.empty())
			{
				iks* pnStatus = iks_insert(pnPresence, protocol::TAG_STATUS);
				iks_insert_cdata(pnStatus, m_PresenceInfo.status.c_str(), 0);
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}
}