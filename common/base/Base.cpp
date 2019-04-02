#include "Base.h"

namespace base
{

	Address::Address(const Address& other)
	{
		tag = other.tag;
		name = other.name;
		ip = other.ip;
		port = other.port;
	}

	bool Address::operator < (const Address& rAddr) const
	{
		int ret = ip.compare(rAddr.ip);
		if (ret == 0)
		{
			return port < rAddr.port;
		}

		return ret < 0;
	}

	Address& Address::operator = (const Address& rAddr)
	{
		tag = rAddr.tag;
		name = rAddr.name;
		ip = rAddr.ip;
		port = rAddr.port;

		return *this;
	}

	bool Address::isValid() 
	{ 
		return (!ip.empty() && port > 0);
	}

}