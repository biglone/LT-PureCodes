#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "SessionIceCandidateNotification.h"

// session
const char          VALUE_TRUE[]         = "1";

namespace protocol
{
    int SessionIceCandidateNotification::In::getNotificationType()
    {
        return protocol::SESSION_ICE;
    }

    bool SessionIceCandidateNotification::In::Parse( iks* pnIks )
    {
        bool isOk = false;
        do
        {
            if (!pnIks)
            {
                break;
            }

            const char* pszId = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ID);
            if (!pszId || strlen(pszId) <= 0)
            {
                break;
            }

            const char* pszFrom = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROM);
            if (!pszFrom || strlen(pszFrom) <= 0)
            {
                break;
            }

            const char* pszTo = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TO);
            if (!pszTo || strlen(pszTo) <= 0)
            {
                break;
            }

			char* pszIceCandidate = iks_cdata(iks_child(pnIks));
			if (!pszIceCandidate || strlen(pszIceCandidate) <= 0)
			{
				break;
			}

            param.sp = getSpFrom();
            param.id = pszId;
            param.from = pszFrom;
            param.to = pszTo;
			param.iceCandidate = pszIceCandidate;

            // validate
            if (!param.isValid())
            {
                break;
            }

            isOk = true;
        } while(0);

        return isOk;
    }


    std::string SessionIceCandidateNotification::Out::getBuffer()
    {
        string sRet = "";

        do
        {
            iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, param.sp.empty() ? 0 : param.sp.c_str(), protocol::ATTRIBUTE_MODULE_RTC, 0);
            if (!pnMessage) break;

            CAutoIks aMessage(pnMessage);

            iks* pnIceCandidate = iks_insert(pnMessage, protocol::TAG_ICE_CANDIDATE);
            if (!pnIceCandidate) break;

            iks_insert_attrib(pnIceCandidate, protocol::ATTRIBUTE_NAME_ID, param.id.c_str());
            iks_insert_attrib(pnIceCandidate, protocol::ATTRIBUTE_NAME_FROM, param.from.c_str());
            iks_insert_attrib(pnIceCandidate, protocol::ATTRIBUTE_NAME_TO, param.to.c_str());
			iks_insert_cdata(pnIceCandidate, param.iceCandidate.c_str(), 0);

            const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
            if (!pszBuffer) break;

            sRet = pszBuffer;
        } while(0);

        return sRet;
    }

}