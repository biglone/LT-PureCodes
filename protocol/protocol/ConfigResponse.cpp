#include "ProtocolConst.h"
#include "ProtocolType.h"
#include "net/RemoteResponse.h"
#include "ErrorResponse.h"
#include "ConfigResponse.h"

namespace protocol
{

	ConfigResponse::ConfigResponse( net::RemoteResponse* pRR )
		: Response(pRR), m_actionType(ConfigRequest::Action_None)
	{
		memset(m_configs, 0, sizeof(m_configs));
	}

	ConfigResponse::~ConfigResponse()
	{

	}

	bool ConfigResponse::Parse()
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(a.协议信令本身错误,b.errcode+errmsg类型的应答)
		bool bPError = true;         // 是否协议信令本身的错误

		do
		{
			// TODO: 解析message头的信息 包括data等；

			if (!m_pRR)
			{
				break;
			}

			iks* pnResponse = m_pRR->getMessage();

			if (!pnResponse)
			{
				break;
			}

			// 找到 Config
			iks* pnConfig = iks_find(pnResponse, protocol::TAG_CONFIG);
			if (!pnConfig)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnConfig);
			if (m_pER)
			{
				ErrorResponse::Log("ConfigResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			// check action type
			const char *szActionType = iks_find_attrib(pnConfig, ATTRIBUTE_NAME_TYPE);
			if (!szActionType || !strlen(szActionType))
			{
				break;
			}

			m_actionType = ConfigRequest::Action_None;
			if (strcmp(szActionType, "set") == 0)
			{
				m_actionType = ConfigRequest::Action_Set;
			}
			else if (strcmp(szActionType, "get") == 0)
			{
				m_actionType = ConfigRequest::Action_Get;
			}
			
			if (m_actionType == ConfigRequest::Action_None)
			{
				break;
			}

			if (m_actionType == ConfigRequest::Action_Get)
			{
				// get every config
				iks* pnConfigN = iks_first_tag(pnConfig);
				while (pnConfigN)
				{
					char* szTagName = iks_name(pnConfigN);
					if (strcmp(szTagName, TAG_CONF1) == 0)
					{
						m_configs[1] = 1;

						iks* pnGroups = iks_first_tag(pnConfigN);
						if (pnGroups && (strcmp(iks_name(pnGroups), TAG_GROUPS) == 0))
						{
							char* szGroups = iks_cdata(iks_child(pnGroups));
							if (szGroups)
							{
								m_configData[TAG_GROUPS] = szGroups;
							}
						}
					}
					else if (strcmp(szTagName, TAG_CONF2) == 0)
					{
						m_configs[2] = 1;

						iks* pnIds = iks_first_tag(pnConfigN);
						if (pnIds && (strcmp(iks_name(pnIds), TAG_IDS) == 0))
						{
							char* szIds = iks_cdata(iks_child(pnIds));
							if (szIds)
							{
								m_configData[TAG_IDS] = szIds;
							}
						}
					}
					else if (strcmp(szTagName, TAG_CONF3) == 0)
					{
						m_configs[3] = 1;

						iks* pnSilence = iks_first_tag(pnConfigN);
						if (pnSilence && (strcmp(iks_name(pnSilence), TAG_SILENCE) == 0))
						{
							char* szSilence = iks_cdata(iks_child(pnSilence));
							if (szSilence)
							{
								m_configData[TAG_SILENCE] = szSilence;
							}
						}
					}

					pnConfigN = iks_next_tag(pnConfigN);
				}
			}

			bOk = true;
		} while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	bool ConfigResponse::hasConfig(int num) const
	{
		bool ret = false;
		if (num >= 1 && num <= 9)
		{
			ret = (m_configs[num] == 1) ? true : false;
		}
		return ret;
	}

	std::string ConfigResponse::configData(const std::string &name) const
	{
		std::string config;
		std::map<std::string, std::string>::const_iterator iter = m_configData.find(name);
		if (iter != m_configData.end())
		{
			config = iter->second;
		}
		return config;
	}

	ConfigRequest::ActionType ConfigResponse::actionType() const
	{
		return m_actionType;
	}

}

