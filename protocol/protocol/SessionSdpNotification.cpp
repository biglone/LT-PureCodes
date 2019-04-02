#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "SessionSdpNotification.h"

// session
const char          VALUE_TRUE[]         = "1";

namespace protocol
{
    int SessionSdpNotification::In::getNotificationType()
    {
        return protocol::SESSION_SDP;
    }

    bool SessionSdpNotification::In::Parse( iks* pnIks )
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

			char* pszSdp = iks_cdata(iks_child(pnIks));
			if (!pszSdp || strlen(pszSdp) <= 0)
			{
				break;
			}

            param.sp = getSpFrom();
            param.id = pszId;
            param.from = pszFrom;
            param.to = pszTo;
			param.sdp = pszSdp;

            // validate
            if (!param.isValid())
            {
                break;
            }

            isOk = true;
        } while(0);

        return isOk;
    }


    std::string SessionSdpNotification::Out::getBuffer()
    {
        string sRet = "";

        do
        {
            iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, param.sp.empty() ? 0 : param.sp.c_str(), protocol::ATTRIBUTE_MODULE_RTC, 0);
            if (!pnMessage) break;

            CAutoIks aMessage(pnMessage);

            iks* pnSdp = iks_insert(pnMessage, protocol::TAG_SDP);
            if (!pnSdp) break;

            iks_insert_attrib(pnSdp, protocol::ATTRIBUTE_NAME_ID, param.id.c_str());
            iks_insert_attrib(pnSdp, protocol::ATTRIBUTE_NAME_FROM, param.from.c_str());
            iks_insert_attrib(pnSdp, protocol::ATTRIBUTE_NAME_TO, param.to.c_str());
			iks_insert_cdata(pnSdp, param.sdp.c_str(), 0);

            const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
            if (!pszBuffer) break;

            sRet = pszBuffer;
        } while(0);

        return sRet;
    }

}