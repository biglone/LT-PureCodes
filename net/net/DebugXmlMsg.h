#ifndef _DEBUGXMLMSG_H_
#define _DEBUGXMLMSG_H_

#include "XmlMsg.h"
#include "net_global.h"

namespace net
{
	class NET_EXPORT DebugXmlmsg : public XmlMsg
	{
	public:
		void setBuffer(const std::string& buf);

		virtual std::string getBuffer();

	private:
		std::string m_sBuf;
	};
}
#endif