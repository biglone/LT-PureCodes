#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "KickNotification.h"

namespace protocol
{
	KickNotification::KickNotification()
		: m_sContent("")
	{}

	bool KickNotification::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)

		do
		{
			if (!pnIks)
				break;

			const char* pszContent = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_CONTENT);

			if (!pszContent)
			{
				break;
			}
			setContent(pszContent);

			bOk = true;
		}while(false);

		return bOk;
	}

	int KickNotification::getNotificationType()
	{
		return protocol::KICK;
	}

	std::string KickNotification::getContent()
	{
		return m_sContent;
	}

	void KickNotification::setContent(const std::string& rsContent)
	{
		m_sContent = rsContent;
	}
}