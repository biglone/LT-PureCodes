#include "cttk/base.h"
#include "TcpClient.h"
#include "ICallback.h"
#include "Recver.h"
#include <QTcpSocket>

namespace net
{

Recver::Recver(TcpClient* pTcpClient) : QObject(), m_pTcpClient(pTcpClient)
{
	assert(m_pTcpClient != NULL);
}

Recver::~Recver()
{

}

void Recver::onReadyRead()
{
	bool needRead = true;
	char szBuffer[cttk::BUFFER_LEN + 1] = {0};
	int nRecv = 0;

	while (needRead)
	{
		// 接收数据
		nRecv = m_pTcpClient->m_pTcpSocket->read(szBuffer, cttk::BUFFER_LEN);
		if (nRecv < 0)
		{
			// 网络断开
			m_pTcpClient->onSocketBroken();
			return;
		}

		needRead = false;
		szBuffer[nRecv] = 0;

		if (nRecv > 0)
		{
			// 解析
			const char* pszAppend = szBuffer;
			int nAppend = nRecv;
			if (!Append(pszAppend, nAppend))
			{
				QString errMsg = QString("XML parse error: %1").arg(szBuffer);
				m_pTcpClient->loggable()->debug(errMsg.toUtf8());

				qWarning() << Q_FUNC_INFO << errMsg;
				m_pTcpClient->onDataParseError();
				return;
			}

			// 判断是否还有数据需要读取
			if (nRecv >= cttk::BUFFER_LEN)
				needRead = true;
		}
	}
}

void Recver::OnNode(iks* pnNode)
{
	m_pTcpClient->onElementPath(pnNode);
}

}