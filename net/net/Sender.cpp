#include <assert.h>

#include "cttk/base.h"

#include "TcpClient.h"
#include "Sender.h"
#include "XmlMsg.h"
#include "Request.h"
#include "ICallback.h"
#include "Heartbeat.h"
#include <QTcpSocket>

namespace net 
{

Sender::Sender(TcpClient* pTcpClient) : QObject(), m_pTcpClient(pTcpClient)
{
	assert(m_pTcpClient != NULL);
	m_listEmergency.clear();
	m_listNormal.clear();
}

Sender::~Sender()
{
	{
		std::deque<XmlMsg*>::iterator itr = m_listEmergency.begin();
		for (; itr != m_listEmergency.end(); ++itr)
		{
			XmlMsg* pXmlMsg = (*itr);
			if (!pXmlMsg->isRequest())
				SAFE_DELETE(pXmlMsg);
		}
		m_listEmergency.clear();
	}

	{
		std::deque<XmlMsg*>::iterator itr = m_listNormal.begin();
		for (; itr != m_listNormal.end(); ++itr)
		{
			XmlMsg* pXmlMsg = (*itr);
			if (!pXmlMsg->isRequest())
				SAFE_DELETE(pXmlMsg);
		}
		m_listNormal.clear();
	}
}

bool Sender::post(XmlMsg* xmlMsg, bool bEmergency)
{
	if (!xmlMsg)
		return false;

	if (bEmergency)
	{
		m_mtxEmergency.Lock();
		m_listEmergency.push_back(xmlMsg);
		m_mtxEmergency.Unlock();
	}
	else
	{
		m_mtxNormal.Lock();
		m_listNormal.push_back(xmlMsg);
		m_mtxNormal.Unlock();
	}

	QMetaObject::invokeMethod(this, "onSend", Qt::QueuedConnection);

	return true;
}

std::string Sender::postHeartbeat()
{
	std::string sRet = "";
	do 
	{
		bool bHB = true;

		m_mtxEmergency.Lock();
		if (m_listEmergency.size() > 0)
		{
			bHB = false;
		}
		m_mtxEmergency.Unlock();
		if (!bHB)
		{
			break;
		}

		bHB = true;
		m_mtxNormal.Lock();
		if (m_listNormal.size() > 0)
		{
			bHB = false;
		}
		m_mtxNormal.Unlock();

		if (!bHB)
		{
			break;
		}

		net::Heartbeat* pHeartbeat = new net::Heartbeat();

		sRet = pHeartbeat->getSeq();
		
		post(pHeartbeat, false);
		
	} while (0);

	return sRet;
}

bool Sender::cancel(Request* request)
{
	bool bRemoved = false;
	
	m_mtxEmergency.Lock();
	{
		deque<XmlMsg*>::iterator itr = m_listEmergency.begin();
		for (; itr != m_listEmergency.end();)
		{
			if (*itr == request)
			{
				m_listEmergency.erase(itr++);
				bRemoved = true;
			}
			else
			{
				++itr;
			}
		}
	}
	m_mtxEmergency.Unlock();


	if (!bRemoved)
	{
		m_mtxNormal.Lock();
		{
			std::deque<XmlMsg*>::iterator itr = m_listNormal.begin();
			for (; itr != m_listNormal.end();)
			{
				if (*itr == request)
				{
					m_listNormal.erase(itr++);
					bRemoved = true;
				}
				else
				{
					++itr;
				}
			}
		}
		m_mtxNormal.Unlock();
	}

	return bRemoved;
}

void Sender::onSend()
{
	do 
	{
		// send all emergency
		if (!Send(m_listEmergency, m_mtxEmergency, 0))
			break;

		// send one normal
		Send(m_listNormal, m_mtxNormal, 1);

	} while (0);
}

bool Sender::Send(std::deque<XmlMsg*>& listMsg, cttk::CMutex& rMutex, int n)
{
	int sent = 0;

	while (1)
	{
		if ((n > 0) && (sent >= n))
		{
			break;
		}

		XmlMsg* pXmlMsg = 0;

		// get
		rMutex.Lock();
		if (!listMsg.empty())
		{
			pXmlMsg = listMsg.front();
			listMsg.pop_front();
		}
		rMutex.Unlock();

		sent++;

		if (!pXmlMsg)
			break;

		string sBuffer = pXmlMsg->getBuffer();
		assert(sBuffer.length() > 0);

		if (!pXmlMsg->isRequest())
		{
			SAFE_DELETE(pXmlMsg);
		}

		int nBufLen = sBuffer.length();
		char *pBuf = (char*)sBuffer.c_str();
		char *pSend = 0;

		while (nBufLen > 0)
		{
			int nSendLen = nBufLen;
			if (nSendLen > cttk::BUFFER_LEN)
			{
				nSendLen = cttk::BUFFER_LEN;
			}
			pSend = pBuf;

			pBuf += nSendLen;
			nBufLen -= nSendLen;

			// send
			int nSent = m_pTcpClient->m_pTcpSocket->write(pSend, nSendLen);
			if (nSent < 0)
			{
				// socket error
				m_pTcpClient->onSocketBroken();
				return false;
			}
		}

		// log
		m_pTcpClient->loggable()->logSent(sBuffer.c_str());
	}

	return true;
}

}
