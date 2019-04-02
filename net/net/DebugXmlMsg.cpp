#include "DebugXmlMsg.h"

namespace net
{
	void DebugXmlmsg::setBuffer(const std::string& buf)
	{
		m_sBuf = buf;
	}

	std::string DebugXmlmsg::getBuffer()
	{
		return m_sBuf;
	}
}
