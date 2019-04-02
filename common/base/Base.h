#ifndef _BASE_BASE_H_
#define _BASE_BASE_H_

#include <map>
#include <string>

#include "common_global.h"

namespace base
{
	class COMMON_EXPORT Address
	{
	public:
		std::string tag;
		std::string name;
		std::string ip;
		int port;

		Address(): tag(""), name(""), ip(""), port(0) {}
		Address(const Address& other);

		bool operator < (const Address& rAddr) const;
		
		Address& operator = (const Address& rAddr);

		bool isValid();
	};

	typedef std::map<std::string, base::Address> AddressMap;

}
#endif
