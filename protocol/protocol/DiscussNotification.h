#ifndef _DISCUSSNOTIFICATION_H_
#define _DISCUSSNOTIFICATION_H_

#include <vector>
#include <string>

#include "SpecificNotification.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT DiscussNotification : public SpecificNotification
	{

	public:
		int getNotificationType();

		bool Parse(iks* pnIks);

	public:
		std::string              id;
		std::string              type;
		std::string              name;
		std::string              creator;
		std::string              time;
		std::string              version;
		std::vector<std::string> members;
		std::vector<std::string> memberNames;
		std::vector<std::string> addedIds;
		std::vector<std::string> cardNames;
	};

}
#endif //_DISCUSSNOTIFICATION_H_
