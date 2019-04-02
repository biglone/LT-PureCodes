#include "AddressHelper.h"
#include "protocol/ProtocolConst.h"

namespace protocol
{
	base::AddressMap AddressHelper::ParseAddresses(iks* pnIks)
	{
		iks* pnTag = pnIks;
		base::AddressMap mapRet;
		std::string sKey;

		while(pnTag != 0)
		{
			const char* pszTagName = iks_name(pnTag);
			const char* pszName = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_NAME);
			const char* pszIp = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_IP);
			const char* pszPort = iks_find_attrib(pnTag, protocol::ATTRIBUTE_NAME_PORT);

			do 
			{
				base::Address addr;
				addr.tag = pszTagName ? pszTagName : "";
				addr.name = pszName ? pszName : "";
				addr.ip = pszIp ? pszIp : "";
				addr.port = pszPort ? atoi(pszPort) : -1;

				if (!addr.isValid())
				{
					break;
				}

				sKey = addr.name;
				if (sKey.empty())
					sKey = addr.tag;

				if (sKey.empty())
					sKey = addr.ip + pszPort;

				mapRet[sKey] = addr;
			} while (0);

			pnTag = iks_next_tag(pnTag);
		}

		return mapRet;
	}

	base::Address AddressHelper::parsePsgAddress(const std::string& str, char sep)
	{
		base::Address addr;

		do 
		{
			if (str.empty())
				break;

			int nColon = str.find_first_of(sep);
			if (nColon <= 0)
				break;

			std::string ip = str.substr(0, nColon);
			std::string port = str.substr(nColon+1);
			if (ip.empty() || port.empty())
				break;

			addr.ip = ip;
			addr.port = atoi(port.c_str());
		} while (0);

		return addr;
	}
}