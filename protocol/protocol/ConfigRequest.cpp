#include "cttk/base.h"

#include "psmscommon/PSMSUtility.h"
#include "iks/iksemel.h"
#include "iks/AutoIks.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"

#include "ErrorResponse.h"
#include "net/RemoteResponse.h"
#include "net/IProtocolCallback.h"

#include "ConfigRequest.h"
#include "ConfigResponse.h"

namespace protocol
{

	ConfigRequest::ConfigRequest(ActionType type /*= Action_Get*/)
		: m_eType(type)
	{
	}

	int ConfigRequest::getType()
	{
		return protocol::Request_Config_Config;
	}

	void ConfigRequest::setConfigData(const std::string &name, const std::string &content)
	{
		m_configData[name] = content;
	}

	int ConfigRequest::actionType() const
	{
		return m_eType;
	}

	std::string ConfigRequest::getBuffer()
	{
		string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_MODULE_CONFIG, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnConfig = iks_insert(pnMessage, protocol::TAG_CONFIG);
			if (!pnConfig) break;

			switch (m_eType)
			{
			case Action_Get:
				{
					// type
					iks_insert_attrib(pnConfig, protocol::ATTRIBUTE_NAME_TYPE, "get");

					// conf1
					std::map<std::string, std::string>::iterator iter = m_configData.find(protocol::TAG_GROUPS);
					if (iter != m_configData.end())
					{
						iks* pnConf1 = iks_insert(pnConfig, protocol::TAG_CONF1);
						if (!pnConf1) break;
					}

					// conf2
					iter = m_configData.find(protocol::TAG_IDS);
					if (iter != m_configData.end())
					{
						iks* pnConf2 = iks_insert(pnConfig, protocol::TAG_CONF2);
						if (!pnConf2) break;
					}
					
					// conf3
					iter = m_configData.find(protocol::TAG_SILENCE);
					if (iter != m_configData.end())
					{
						iks* pnConf3 = iks_insert(pnConfig, protocol::TAG_CONF3);
						if (!pnConf3) break;
					}
				}
				break;
			case Action_Set:
				{
					// type
					iks_insert_attrib(pnConfig, protocol::ATTRIBUTE_NAME_TYPE, "set");

					// conf1
					std::map<std::string, std::string>::iterator iter = m_configData.find(protocol::TAG_GROUPS);
					if (iter != m_configData.end())
					{
						// conf1
						iks* pnConf1 = iks_insert(pnConfig, protocol::TAG_CONF1);
						if (!pnConf1) break;

						// add groups
						iks* pnGroups = iks_insert(pnConf1, protocol::TAG_GROUPS);
						if (!pnGroups) break;

						std::string groupNames = m_configData[protocol::TAG_GROUPS];
						iks_insert_cdata(pnGroups, groupNames.c_str(), 0);
					}

					// conf2
					iter = m_configData.find(protocol::TAG_IDS);
					if (iter != m_configData.end())
					{
						// conf2
						iks* pnConf2 = iks_insert(pnConfig, protocol::TAG_CONF2);
						if (!pnConf2) break;

						// add ids
						iks* pnIds = iks_insert(pnConf2, protocol::TAG_IDS);
						if (!pnIds) break;

						std::string ids = m_configData[protocol::TAG_IDS];
						iks_insert_cdata(pnIds, ids.c_str(), 0);
					}

					// conf3
					iter = m_configData.find(protocol::TAG_SILENCE);
					if (iter != m_configData.end())
					{
						// conf3
						iks* pnConf3 = iks_insert(pnConfig, protocol::TAG_CONF3);
						if (!pnConf3) break;

						// add silence
						iks* pnSilence = iks_insert(pnConf3, protocol::TAG_SILENCE);
						if (!pnSilence) break;

						std::string silence = m_configData[protocol::TAG_SILENCE];
						iks_insert_cdata(pnSilence, silence.c_str(), 0);
					}
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

	void ConfigRequest::onResponse(net::RemoteResponse* response)
	{
		ConfigResponse* pRr = new ConfigResponse(response);
		pRr->Parse();
		getProtocolCallback()->onResponse(this, pRr);
	}

}