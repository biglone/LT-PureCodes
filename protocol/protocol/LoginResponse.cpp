#include "net/RemoteResponse.h"
#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ErrorResponse.h"
#include "LoginResponse.h"
#include "AddressHelper.h"

namespace protocol
{
	LoginResponse::LoginResponse(net::RemoteResponse* pRr)
		: Response(pRr)
	{
	}

	bool LoginResponse::Parse()
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

			// 找到 login
			iks* pnLogin = iks_find(pnResponse, protocol::TAG_LOGIN);
			if (!pnLogin)
			{
				break;
			}

			do 
			{
				// psg or sg, need to check both
				iks* pnPsgs = iks_find(pnLogin, protocol::TAG_PSGS);
				if (!pnPsgs)
				{
					pnPsgs = iks_find(pnLogin, protocol::TAG_SGS);
					if (!pnPsgs)
						break;
				}

				// psg or sg
				iks* pnTag = iks_first_tag(pnPsgs);
				while (pnTag)
				{
					if (strcmp(protocol::TAG_PSG, iks_name(pnTag)) != 0 ||
						strcmp(protocol::TAG_SG, iks_name(pnTag)) != 0)
					{
						// id
						char* pszId = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_ID);
						if (!pszId || strlen(pszId) <= 0)
							break;

						// main
						char* pszMain = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_MAIN);
						if (pszMain && strlen(pszMain) > 0 && atoi(pszMain) > 0)
						{
							m_psgs.insert(m_psgs.begin(), pszId);
						}
						else
						{
							m_psgs.push_back(pszId);
						}
					}

					pnTag = iks_next_tag(pnTag);
				}
			} while(0);

			// 判断是否为错误应答，放在sg解析后面，因为有登录错误的sg的这种情况，必须要把sg解析出来
			m_pER = ErrorResponse::Parse(pnLogin);
			if (m_pER)
			{
				ErrorResponse::Log("LoginResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			/*
			// parameters
			iks* pnParameters = iks_find(pnLogin, protocol::TAG_PARAMETERS);
			if (!pnParameters)
			{
				break;
			}

			// parameter
			// iterate through child elements of params,这里如果有节点名字不是parameter,直接忽略,不认为是不合法的信令
			iks* pnParameter = iks_first_tag(pnParameters);
			while (pnParameter)
			{
				do 
				{
					const char* pszTag = iks_name(pnParameter);
					if (!pszTag || strcmp(pszTag, protocol::TAG_PARAMETER) != 0)
					{
						break;
					}

					const char* pszId = iks_find_attrib(pnParameter, "id");
					const char* pszValue = iks_find_attrib(pnParameter, "value");

					if (!pszId || !pszValue)
					{
						break;
					}

					addParam(pszId, pszValue);

				} while(0);

				pnParameter = iks_next_tag(pnParameter);
			}
			*/
			
			// modules
			iks* pnModules = iks_find(pnLogin, protocol::TAG_MODULES);
			if (!pnModules)
			{
				break;
			}

			// module
			iks* pnModule = iks_first_tag(pnModules);
			while (pnModule)
			{
				do 
				{
					const char* pszTag = iks_name(pnModule);
					if (!pszTag || strcmp(pszTag, protocol::TAG_MODULE) != 0)
					{
						break;
					}

					const char* pszId = iks_find_attrib(pnModule, protocol::ATTRIBUTE_NAME_ID);
					if (!pszId || strlen(pszId) <= 0)
					{
						break; 
					}

					addModule(pszId);
				} while(0);

				pnModule = iks_next_tag(pnModule);
			}

			//services
			iks* pnServices = iks_find(pnLogin, protocol::TAG_SERVICES);
			if (!pnServices)
			{
				break;
			}

			//service
			iks* pnService = iks_first_tag(pnServices);
			while (pnService)
			{
				do 
				{
					const char* pszTag = iks_name(pnService);
					if (!pszTag || strcmp(pszTag, protocol::TAG_SERVICE) != 0)
					{
						break;
					}

					const char* pszId = iks_find_attrib(pnService, "id");
					if (!pszId || strlen(pszId) <= 0)
					{
						break; 
					}

					iks* pnParameter = iks_first_tag(pnService);
					if (!pnParameter)
					{
						break;
					}

					if (0 == strcmp(pszId, "rtc"))
					{
						addRtcParam(pnParameter);
					}
					else
					{
						addService(pszId, pnParameter);
					}
				} while(0);

				pnService = iks_next_tag(pnService);
			}
			
			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	/*
	std::map<std::string, std::string> LoginResponse::getPamams()
	{
		return m_mapParm;
	}
	*/

	std::list<std::string> LoginResponse::getModule()
	{
		return m_listModule;
	}

	std::map<std::string, base::AddressMap> LoginResponse::getMapService()
	{
		return m_mapServices;
	}

	std::list<std::string> LoginResponse::getPsgs() const
	{
		return m_psgs;
	}

	LoginResponse::RtcParam LoginResponse::getRtcParam() const
	{
		return m_rtcParam;
	}

	/*
	bool LoginResponse::addParam(const std::string& rsId, const std::string& rsValue)
	{
		bool bRet = false;

		do 
		{
			std::map<std::string, std::string>::iterator itr = m_mapParm.find(rsId);
			if (itr != m_mapParm.end()) break;

			m_mapParm[rsId] = rsValue;

			bRet = true;
		} while(0);

		return bRet;
	}
	*/

	void LoginResponse::addModule(const std::string& rsId)
	{
		m_listModule.push_back(rsId);
		m_listModule.sort();
		m_listModule.unique();
	}

	bool LoginResponse::addService(const std::string& rsId, iks* pnIks)
	{
		bool bRet = false;

		do 
		{
			if (!pnIks)
				break;

			iks *pnTag = iks_first_tag(pnIks);
			if (!pnTag)
				break;

			std::map<std::string, base::Address> mapAddress = AddressHelper::ParseAddresses(pnTag);

			if (mapAddress.empty())
			{
				break;
			}

			m_mapServices[rsId] = mapAddress;

			bRet = true;
		} while(0);

		return bRet;
	}

	bool LoginResponse::addRtcParam(iks* pnIks)
	{
		if (!pnIks)
			return false;

		iks *pnTag = iks_first_tag(pnIks);
		while (pnTag)
		{
			char *tagName = iks_name(pnTag);
			if (0 == strcmp(tagName, "urls"))
			{
				iks *urlTag = iks_first_tag(pnTag);
				while (urlTag)
				{
					char *url = iks_cdata(iks_child(urlTag));
					if (url)
						m_rtcParam.urls.push_back(url);
					urlTag = iks_next_tag(urlTag);
				}
			}
			else if (0 == strcmp(tagName, "username"))
			{
				char *username = iks_cdata(iks_child(pnTag));
				if (username)
					m_rtcParam.username = username;
			}
			else if (0 == strcmp(tagName, "credential"))
			{
				char *credential = iks_cdata(iks_child(pnTag));
				if (credential)
					m_rtcParam.credential = credential;
			}

			pnTag = iks_next_tag(pnTag);
		}

		return true;
	}

}