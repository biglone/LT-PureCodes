#include "cttk/base.h"
#include "Keeper.h"
#include "TcpClient.h"

namespace net
{

Keeper::Keeper(TcpClient* pTcpClient)
: m_pTcpClient(pTcpClient)
{
	assert(m_pTcpClient != NULL);
}

Keeper::~Keeper()
{
}

cttk::CThreadAdaptor::ReturnType Keeper::OnRun()
{
	int times = 0;

	while (!IsExitSet())
	{
		++times;
		cttk::sys::sleep(TcpClient::TIME_CHECK_INTERVAL);

		if (!m_pTcpClient->m_pTcpSocket)
		{
			break;
		}

		m_pTcpClient->checkTimeout();

		if (m_pTcpClient->m_bStartHeartbeat)
		{
			if (times == TcpClient::HEARTBEAT_TIMEOUT)
			{
				m_pTcpClient->checkHeartbeat();
			}

			if (times >= TcpClient::HEARTBEAT_INTERVAL)
			{
				if (m_pTcpClient->sendHeartbeat())
				{
					times = 0;
				}
			}
		}
	}

	return cttk::CThreadAdaptor::success;
}

}
