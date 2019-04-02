#ifndef _GROUPNOTIFICATION_H_
#define _GROUPNOTIFICATION_H_

#include <vector>
#include <string>

#include "SpecificNotification.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT GroupNotification : public SpecificNotification
	{

	public:
		int getNotificationType();

		bool Parse(iks *pnIks);

	public:
		std::string              id;
		std::string              name;
		std::string              version;
		std::string              desc;
		std::vector<std::string> members;
		std::vector<std::string> memberNames;
		std::vector<int>         indice;
		std::vector<std::string> cardNames;
	};

}
#endif //_GROUPNOTIFICATION_H_
