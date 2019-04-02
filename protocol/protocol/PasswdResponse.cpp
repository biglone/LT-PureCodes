#include "net/RemoteResponse.h"
#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ErrorResponse.h"
#include "PasswdResponse.h"

namespace protocol
{
	PasswdResponse::PasswdResponse(net::RemoteResponse* pRr)
		: Response(pRr)
	{
	}

	bool PasswdResponse::Parse()
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

			// 找到 passwd
			iks* pnPasswd = iks_find(pnResponse, protocol::TAG_PASSWD);
			if (!pnPasswd)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnPasswd);
			if (m_pER)
			{
				ErrorResponse::Log("PasswdResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}
}