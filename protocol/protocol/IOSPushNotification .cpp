#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolConst.h"

#include "IOSPushNotification.h"

namespace protocol
{
	net::SeqGenerator IOSPushNotification::Out::s_seqGenerator;
    
	IOSPushNotification::Out::Out()
	{
		m_sSeq = s_seqGenerator.getSeq();
	}

	std::string IOSPushNotification::Out::getBuffer()
    {
        string sRet = "";

        do
        {
            iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, m_sSeq.c_str(), 0, 0, protocol::ATTRIBUTE_MODLE_IOS_PUSH, 0);
            if (!pnMessage) break;
            iks* pnPushNotification = iks_insert(pnMessage, protocol::ATTRIBUTE_PUSH_NOTIFICATION);
            if (!pnPushNotification) break;

            iks_insert_attrib(pnPushNotification, protocol::ATTRIBUTE_NAME_TYPE, param.type.c_str());
			iks_insert_attrib(pnPushNotification, protocol::ATTRIBUTE_SILENCE, param.silence.c_str());
            iks_insert_attrib(pnPushNotification, protocol::ATTRIBUTE_NAME_FROM, param.from.c_str());
            iks_insert_attrib(pnPushNotification, protocol::ATTRIBUTE_NAME_TO, param.to.c_str());
            const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
            if (!pszBuffer) break;
            sRet = pszBuffer;
        } while(0);

        return sRet;
    }

}