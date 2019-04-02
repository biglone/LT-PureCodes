#include "cttk/base.h"

#include "psmscommon/PSMSUtility.h"
#include "iks/iksemel.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "ErrorResponse.h"
#include "net/RemoteResponse.h"
#include "net/IProtocolCallback.h"

#include "InterphoneRequest.h"
#include "InterphoneResponse.h"

namespace protocol
{
	InterphoneRequest::InterphoneRequest(ActionType type, const std::string &iid, const std::vector<std::string> &params)
		: m_type(type), m_iid(iid), m_params(params)
	{

	}

	int InterphoneRequest::getType()
	{
		return protocol::Request_Interphone_Interphone;
	}

	int InterphoneRequest::actionType() const
	{
		return m_type;
	}

	std::string InterphoneRequest::interphoneId() const
	{
		return m_iid;
	}

	std::vector<std::string> InterphoneRequest::params() const
	{
		return m_params;
	}

	std::string InterphoneRequest::getBuffer()
	{
		string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_MODULE_INTERPHONE, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			switch (m_type)
			{
			case Action_Sync:
				{
					iks *pnInterphones = iks_insert(pnMessage, protocol::TAG_INTERPHONES);
					if (!pnInterphones) break;

					// type
					iks_insert_attrib(pnInterphones, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_SYNC);

					// all ids
					for (int i = 0; i < (int)m_params.size(); i++)
					{
						std::string param = m_params[i];
						iks *pnInterphone = iks_insert(pnInterphones, protocol::TAG_INTERPHONE);
						iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_ID, param.c_str());
					}
				}
				break;
			case Action_Member:
				{
					iks *pnInterphone = iks_insert(pnMessage, protocol::TAG_INTERPHONE);
					if (!pnInterphone) break;

					// type
					iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_SYNC);

					// id
					iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_ID, m_iid.c_str());
				}
				break;
			case Action_Add:
				{
					iks *pnInterphone = iks_insert(pnMessage, protocol::TAG_INTERPHONE);
					if (!pnInterphone) break;

					if (m_params.empty()) break;

					// type
					iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_ADD);

					// id
					iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_ID, m_iid.c_str());

					// member
					std::string param = m_params[0];
					iks *pnMember = iks_insert(pnInterphone, protocol::TAG_MEMBER);
					iks_insert_attrib(pnMember, protocol::ATTRIBUTE_NAME_ID, param.c_str());
				}
				break;
			case Action_Quit:
				{
					iks *pnInterphone = iks_insert(pnMessage, protocol::TAG_INTERPHONE);
					if (!pnInterphone) break;

					if (m_params.empty()) break;

					// type
					iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_TYPE, protocol::ATTRIBUTE_NAME_QUIT);

					// id
					iks_insert_attrib(pnInterphone, protocol::ATTRIBUTE_NAME_ID, m_iid.c_str());

					// member
					std::string param = m_params[0];
					iks *pnMember = iks_insert(pnInterphone, protocol::TAG_MEMBER);
					iks_insert_attrib(pnMember, protocol::ATTRIBUTE_NAME_ID, param.c_str());
				}
				break;
			case Action_Speak:
				{
					iks *pnSpeak = iks_insert(pnMessage, protocol::TAG_SPEAK);
					if (!pnSpeak) break;

					if (m_params.empty()) break;
					
					// type
					std::string param = m_params[0];
					iks_insert_attrib(pnSpeak, protocol::ATTRIBUTE_NAME_TYPE, param.c_str());

					// id
					iks_insert_attrib(pnSpeak, protocol::ATTRIBUTE_NAME_ID, m_iid.c_str());
				}
				break;
			default:
				break;
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void InterphoneRequest::onResponse(net::RemoteResponse* response)
	{
		InterphoneResponse *pRr = new InterphoneResponse(response);
		pRr->setActionType(m_type);
		pRr->Parse();
		getProtocolCallback()->onResponse(this, pRr);
	}

}