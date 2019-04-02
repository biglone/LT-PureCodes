#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "protocol/ProtocolConst.h"

#include "SessionModifyNotification.h"

// session
const char          VALUE_TRUE[]         = "1";
const char          VALUE_FALSE[]        = "0";

namespace protocol
{
	int SessionModifyNotification::In::getNotificationType()
	{
		return protocol::SESSION_MODIFY;
	}

	bool SessionModifyNotification::In::Parse( iks* pnIks )
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

            bool bAudioRecv = false;
			bool bAudioSend = false;
            do 
            {
                // audio
                iks* pnAudio = iks_find(pnIks, protocol::TAG_AUDIO);
                if (!pnAudio)
                {
                    break;
                }

                const char* pszAudioRecv = iks_find_attrib(pnAudio, protocol::ATTRIBUTE_NAME_RECV);
                if (pszAudioRecv && strcmp(pszAudioRecv, VALUE_TRUE) == 0)
                {
                    bAudioRecv = true;
                }

                const char* pszAudioSend = iks_find_attrib(pnAudio, protocol::ATTRIBUTE_NAME_SEND);
                if (pszAudioSend && strcmp(pszAudioSend, VALUE_TRUE) == 0)
                {
                    bAudioSend = true;
                }
            } while (0);

            bool bHasVideo = false;
			bool bVideRecv = false;
			bool bVideoSend = false;
            do 
            {
                iks* pnVideo = iks_find(pnIks, protocol::TAG_VIDEO);
                if (!pnVideo)
                    break;
				
				const char* pszVideoRecv = iks_find_attrib(pnVideo, protocol::ATTRIBUTE_NAME_RECV);
                if (pszVideoRecv && strcmp(pszVideoRecv, VALUE_TRUE) == 0)
                {
                    bVideRecv = true;
                }

                const char* pszVideoSend = iks_find_attrib(pnVideo, protocol::ATTRIBUTE_NAME_SEND);
                if (pszVideoSend && strcmp(pszVideoSend, VALUE_TRUE) == 0)
                {
                    bVideoSend = true;
                }
            } while (0);

			param.sp = getSpFrom();
			param.id = pszId;
			param.from = pszFrom;
			param.to = pszTo;
			param.audioRecv = bAudioRecv;
			param.audioSend = bAudioSend;
			param.hasVideo = bHasVideo;
			param.videoRecv = bVideRecv;
			param.videoSend = bVideoSend;

			// validate
			if (!param.isValid())
			{
				break;
			}

			isOk = true;
		}while(0);

		return isOk;
	}

	std::string SessionModifyNotification::Out::getBuffer()
	{
		string sRet = "";

		do
		{
            iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, param.sp.empty() ? 0 : param.sp.c_str(), protocol::ATTRIBUTE_MODULE_RTC, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnModify = iks_insert(pnMessage, protocol::TAG_MODIFY);
			if (!pnModify) break;

			iks_insert_attrib(pnModify, protocol::ATTRIBUTE_NAME_ID, param.id.c_str());
			iks_insert_attrib(pnModify, protocol::ATTRIBUTE_NAME_FROM, param.from.c_str());
			iks_insert_attrib(pnModify, protocol::ATTRIBUTE_NAME_TO, param.to.c_str());

            if (param.hasVideo) 
            {
                // video
                iks* pnVideo = iks_insert(pnModify, protocol::TAG_VIDEO);
                if (!pnVideo)
                    break;

                iks_insert_attrib(pnVideo, protocol::ATTRIBUTE_NAME_RECV, (param.videoRecv ? VALUE_TRUE : VALUE_FALSE));
                iks_insert_attrib(pnVideo, protocol::ATTRIBUTE_NAME_SEND, (param.videoSend ? VALUE_TRUE : VALUE_FALSE));
            }

            // audio
            iks* pnAudio = iks_insert(pnModify, protocol::TAG_AUDIO);
            if (!pnAudio)
                break;
			
			iks_insert_attrib(pnAudio, protocol::ATTRIBUTE_NAME_RECV, (param.audioRecv ? VALUE_TRUE : VALUE_FALSE));
            iks_insert_attrib(pnAudio, protocol::ATTRIBUTE_NAME_SEND, (param.audioSend ? VALUE_TRUE : VALUE_FALSE));

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

}