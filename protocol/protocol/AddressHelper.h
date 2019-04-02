#ifndef __ADDRESS_HELPER_H__
#define __ADDRESS_HELPER_H__

#include "base/Base.h"
#include "iks/iksemel.h"

namespace protocol 
{

class AddressHelper
{
public:
	static base::AddressMap ParseAddresses(iks* pnIks);

	base::Address parsePsgAddress(const std::string& str, char sep = ':');
};

};

#endif // __ADDRESS_HELPER_H__