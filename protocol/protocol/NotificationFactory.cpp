#include "net/RemoteNotification.h"
#include "ProtocolConst.h"
#include "SpecificNotification.h"
#include "KickNotification.h"
#include "ReloginNotification.h"

#include "MessageNotification.h"
#include "PresenceNotification.h"
#include "HeartbeatMessage.h"

/*
#include "TalkBeginNotification.h"
#include "TalkEndNotification.h"
#include "TalkRecvendNotification.h"
#include "TalkResendNotification.h"

#include "VideoStartNotification.h"
#include "VideoOkNotification.h"
#include "VideoRejectNotification.h"
#include "VideoReportNotification.h"
#include "VideoStopNotification.h"
*/

#include "SessionInviteNotification.h"
#include "SessionRingingNotification.h"
#include "SessionAckNotification.h"
#include "SessionByeNotification.h"
#include "SessionOkNotification.h"
#include "SessionRejectNotification.h"
#include "SessionReportNotification.h"
#include "SessionModifyNotification.h"
#include "SessionNotifyNotification.h"
#include "SessionSdpNotification.h"
#include "SessionIceCandidateNotification.h"

#include "DiscussNotification.h"

#include "ChangeNoticeNotification.h"

#include "TipNotification.h"

#include "InterphoneNotification.h"

#include "RosterNotification.h"

#include "WithdrawMessage.h"

#include "GroupNotification.h"

#include "NotificationFactory.h"


namespace protocol
{
	SpecificNotification* NotificationFactory::Create(net::RemoteNotification* pRn)
	{
		SpecificNotification* pRet = 0;
		do 
		{
			iks* pnIks = pRn->getMessage();
			if (!pnIks)
			{
				break;
			}

			const char* pszModel = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_MODULE);
			const char* pszSpFrom = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROM);
			if (!pszSpFrom)
				pszSpFrom = "";

			if (!pszModel || strlen(pszModel) <= 0 /*|| !pszSpFrom || strlen(pszSpFrom) <= 0*/) // notice has no from attribute
			{
				break;
			}

			iks* pnTag = iks_first_tag(pnIks);
			if (!pnTag)
			{
				break;
			}

			const char* pszTag = iks_name(pnTag);
			if (!pszTag || strlen(pszTag) <= 0)
			{
				break;
			}

			if (!strcmp(pszModel, protocol::ATTRIBUTE_BASE))
			{
				if (!strcmp(pszTag, protocol::TAG_KICK))
				{
					// kick
					pRet = new KickNotification;
				}
				else if (!strcmp(pszTag, protocol::TAG_RELOGIN))
				{
					// relogin
					pRet = new ReloginNotification;
				}
				else if (!strcmp(pszTag, protocol::TAG_NOTICE))
				{
					// notice
					pRet = new ChangeNoticeNotification;
				}
				else
				{
					break;
				}
			} 
			else if (!strcmp(pszModel, protocol::ATTRIBUTE_IM))
			{
				if (!strcmp(pszTag, protocol::TAG_PRESENCE))
				{
					// presence
					pRet = new PresenceNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_MESSAGE))
				{
					// message
					pRet = new MessageNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_HEARTBEAT))
				{
					// heartbeat
					pRet = new HeartbeatMessage::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_TIP))
				{
					// tip
					pRet = new TipNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_ROSTER))
				{
					// roster
					pRet = new RosterNotification;
				}
				else if (!strcmp(pszTag, protocol::TAG_WITHDRAW))
				{
					// withdraw
					pRet = new WithdrawMessage::WithdrawNotification;
				}
				else if (!strcmp(pszTag, protocol::TAG_GROUP))
				{
					// group
					pRet = new GroupNotification;
				}
				else
				{
					break;
				}
			}
			/*
			else if (!strcmp(pszModel, protocol::ATTRIBUTE_MODULE_VIDEO))
			{
				if (!strcmp(pszTag, protocol::TAG_START))
				{
					pRet = new VideoStartNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_OK))
				{
					pRet = new VideoOkNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_REJECT))
				{
					pRet = new VideoRejectNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_REPORT))
				{
					pRet = new VideoReportNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_STOP))
				{
					pRet = new VideoStopNotification::In;
				}
				else
				{
					break;
				}
			}
			else if (!strcmp(pszModel, protocol::ATTRIBUTE_MODULE_TALK))
			{
				if (!strcmp(pszTag, protocol::TAG_TALKBEGIN))
				{
					pRet = new TalkbeginNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_TALKEND))
				{
					pRet = new TalkendNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_RECVEND))
				{
					pRet = new TalkRecvendNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_RESEND))
				{
					pRet = new TalkResendNotification::In;
				}
				else
				{
					break;
				}
			}
			*/
            else if (!strcmp(pszModel, protocol::ATTRIBUTE_MODULE_RTC))
            {
                if (!strcmp(pszTag, protocol::TAG_INVITE))
                {
                    pRet = new SessionInviteNotification::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_RINGING))
                {
                    pRet = new SessionRingingNotifiction::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_OK))
                {
                    pRet = new SessionOkNotification::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_REJECT))
                {
                    pRet = new SessionRejectNotification::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_ACK))
                {
                    pRet = new SessionAckNotification::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_BYE))
                {
                    pRet = new SessionByeNotification::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_MODIFY))
                {
                    pRet = new SessionModifyNotification::In;
                }
                else if (!strcmp(pszTag, protocol::TAG_REPORT))
                {
                    pRet = new SessionReportNotifiction::In;
                }
				else if (!strcmp(pszTag, protocol::TAG_NOTIFY))
				{
					pRet = new SessionNotifyNotifiction;
				}
				else if (!strcmp(pszTag, protocol::TAG_SDP))
				{
					pRet = new SessionSdpNotification::In;
				}
				else if (!strcmp(pszTag, protocol::TAG_ICE_CANDIDATE))
				{
					pRet = new SessionIceCandidateNotification::In;
				}
                else
                {
                    break;
                }
            }
			else if (!strcmp(pszModel, protocol::ATTRIBUTE_MODULE_DISCUSS))
			{
				if (!strcmp(pszTag, protocol::TAG_DISCUSS))
				{
					pRet = new DiscussNotification;
				}
				else
				{
					break;
				}
			}
			else if (!strcmp(pszModel, protocol::ATTRIBUTE_MODULE_INTERPHONE))
			{
				if (!strcmp(pszTag, protocol::TAG_INTERPHONE))
				{
					pRet = new InterphoneNotification;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			if (!pRet)
			{
				break;
			}

			pRet->setModel(pszModel);
			pRet->setSpFrom(pszSpFrom);
			pRet->setTagName(pszTag);

			pRet->Parse(pnTag);
		} while (0);

		return pRet;
	}
}