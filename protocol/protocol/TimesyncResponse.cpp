#include "net/RemoteResponse.h"
#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ErrorResponse.h"
#include "TimesyncResponse.h"


namespace protocol
{
	TimesyncResponse::TimesyncResponse(net::RemoteResponse* pRr)
		: Response(pRr)
		, m_sTime("")
	{

	}

	bool TimesyncResponse::Parse()
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(a.协议信令本身错误,b.errcode+errmsg类型的应答)
		bool bPError = true;         // 是否协议信令本身的错误

		do
		{
			// TODO: 解析message头的信息 包括data等；

			if (!m_pRR)
			{
				break;
			}

			iks* pnResponse = m_pRR->getMessage();

			if (!pnResponse)
			{
				break;
			}

			// 找到 logout
			iks* pnTimesync = iks_find(pnResponse, protocol::TAG_TIMESYNC);
			if (!pnTimesync)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnTimesync);
			if (m_pER)
			{
				ErrorResponse::Log("TimesyncResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			char* pszTime = iks_find_attrib(pnTimesync, "to");
			if (!pszTime)
			{
				break;
			}

			if(strlen(pszTime) <= 0)
			{
				break;
			}

			setTime(pszTime);

			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	std::string TimesyncResponse::getTime()
	{
		return m_sTime;
	}

	void TimesyncResponse::setTime(const std::string& rsTime)
	{
		m_sTime = rsTime;
	}
}