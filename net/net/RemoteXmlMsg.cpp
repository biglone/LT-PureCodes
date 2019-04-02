#include <assert.h>
#include "RemoteXmlMsg.h"
namespace net
{

RemoteXmlMsg::RemoteXmlMsg(iks* pElement)
: XmlMsg()
, m_pElement(0)
{
	assert(pElement != NULL);

	m_pElement = iks_copy(pElement);
}

RemoteXmlMsg::~RemoteXmlMsg(void)
{
	if (m_pElement)
	{
		iks_delete(m_pElement);
		m_pElement = 0;
	}
}

iks* RemoteXmlMsg::getMessage()
{
	return m_pElement;
}

std::string RemoteXmlMsg::getBuffer()
{
	std::string sRet = "";
	do 
	{
		if (!m_pElement)
			break;

		const char* pszBuffer = iks_string(iks_stack(m_pElement), m_pElement);
		if (!pszBuffer) break;

		sRet = pszBuffer;
	} while (0);

	return sRet;
}
}
