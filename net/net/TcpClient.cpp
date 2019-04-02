#include <assert.h>
#include "cttk/base.h"

#include "ICallback.h"

#include "Connecter.h"
#include "Sender.h"
#include "Recver.h"
#include "Keeper.h"
#include <QTcpSocket>

#include "RemoteRequest.h"
#include "RemoteResponse.h"
#include "RemoteNotification.h"

#include "TcpClient.h"

namespace net
{
const int           TcpClient::TIME_CHECK_INTERVAL = 1;     // 1 s
const int           TcpClient::HEARTBEAT_INTERVAL  = 180;   // 180 s
const int           TcpClient::HEARTBEAT_TIMEOUT   = 60;    // 60 s

const char          ATTRIBUTE_NAME_TYPE[]            = "type";         
const char          ATTRIBUTE_NAME_SEQ[]             = "seq";   
const char          TAG_MESSAGE[]                    = "message";       
const char          ATTRIBUTE_REQUEST[]              = "request";      
const char          ATTRIBUTE_RESPONSE[]             = "response";     
const char          ATTRIBUTE_NOTIFICATION[]         = "notification"; 

TcpClient::TcpClient()
: m_pTcpSocket(0)
, m_pConnecter(0)
, m_pRecver(0)
, m_pSender(0)
, m_pKeeper(0)
, m_bCallback(false)
, m_eHeartbeatState(Heartbeat_Invalid)
, m_bStartHeartbeat(false)
{
}

TcpClient::~TcpClient()
{
	uninitialize();
}

void TcpClient::initialize(InitParam initParam)
{
	m_Param = initParam;

	XmlMsg::setProtocolCallback(m_Param.pICallback->getProtocolCallback());

	m_bCallback = true;

	m_pConnecter = new Connecter(this);
	m_pConnecter->moveToThread(&m_thread);

	m_pSender = new Sender(this);
	m_pSender->moveToThread(&m_thread);

	m_pRecver = new Recver(this);
	m_pRecver->moveToThread(&m_thread);

	bool connectOK = false;
	connectOK = QObject::connect(&m_thread, SIGNAL(finished()), m_pConnecter, SLOT(deleteLater()));
	Q_ASSERT(connectOK);
	connectOK = QObject::connect(&m_thread, SIGNAL(finished()), m_pSender, SLOT(deleteLater()));
	Q_ASSERT(connectOK);
	connectOK = QObject::connect(&m_thread, SIGNAL(finished()), m_pRecver, SLOT(deleteLater()));
	Q_ASSERT(connectOK);
	connectOK = QObject::connect(&m_thread, SIGNAL(started()), m_pConnecter, SLOT(connect()));
	Q_ASSERT(connectOK);
	
	m_thread.start();
}

void TcpClient::uninitialize()
{
	// 不在回调
	m_bCallback = false;

	// 停掉heartbeat
	m_bStartHeartbeat = false;

	// 结束 keeper
	if (m_pKeeper)
	{
		m_pKeeper->Stop();
		SAFE_DELETE(m_pKeeper);
	}

	// 结束线程
	m_thread.quit();
	m_thread.wait(3000);
	m_pConnecter = 0;
	m_pSender = 0;
	m_pRecver = 0;
	m_pTcpSocket = 0;

	// clear all no result requests
	clearNoResultRequest();
}

// 设置日志接口
void TcpClient::setLoggable(ILoggable *loggable)
{
	m_loggable = loggable;
}

// 获取日志接口
ILoggable* TcpClient::loggable() const
{
	return m_loggable;
}

// 发送一个协议信令 request
bool TcpClient::send(Request* request, bool bEmergency)
{
	assert(m_pSender != NULL);
	
	cacheRequest(request);

	return m_pSender->post(request, bEmergency);
}

// 发送一个协议信令 notification response
bool TcpClient::send(XmlMsg* xmlMsg, bool bEmergency)
{
	assert(m_pSender != NULL);

	return m_pSender->post(xmlMsg, bEmergency);
}

// 撤销一个 request 信令的请求,不再要求该 request 的结果被报告,不能确保 request 不被发送
// return:
// true,操作成功; false,操作失败,还是会被报告
bool TcpClient::cancel(Request* request)
{
	bool bCanceled = false;
	
	assert(m_pSender != NULL);

	m_pSender->cancel(request);

	if (removeRequest(request->getSeq()))
	{
		bCanceled = true;
	}

	return bCanceled;
}

bool TcpClient::cancel(const std::string& rsSeq)
{
	bool bCanceled = false;

	assert(m_pSender != NULL);

	Request* pRequest = removeRequest(rsSeq);

	if (pRequest)
	{
		bCanceled = true;
		m_pSender->cancel(pRequest);
	}

	return bCanceled;
}


bool TcpClient::setHeartbeat(bool bOpen)
{
	m_bStartHeartbeat = bOpen;
	if (!m_bStartHeartbeat)
	{
		m_eHeartbeatState = Heartbeat_Invalid;
	}
	return true;
}

bool TcpClient::sendHeartbeat()
{
	std::string sSeq = m_pSender->postHeartbeat();
	if (sSeq.empty())
		return false;

	m_eHeartbeatState = Heartbeat_Send;
	return true;
}

void TcpClient::checkHeartbeat()
{
	if (Heartbeat_Send == m_eHeartbeatState)
	{
		loggable()->warning("TcpClient onSocketBroken");

		// broken
		onSocketBroken();
	}
}

void TcpClient::cacheRequest(Request* request)
{
	m_mtxRequest.Lock();
	m_mapRequest[request->getSeq()] = request;
	m_mtxRequest.Unlock();
}

Request* TcpClient::removeRequest(string seq)
{
	Request* request = NULL;
	m_mtxRequest.Lock();

	map<string, Request*>::iterator itr = m_mapRequest.find(seq);
	if (itr != m_mapRequest.end())
	{
		request = itr->second;
		m_mapRequest.erase(itr);
	}

	m_mtxRequest.Unlock();

	return request;
}

void TcpClient::checkTimeout()
{
	m_listTimeout.clear();
	
	m_mtxRequest.Lock();
	map<string, Request*>::iterator itr = m_mapRequest.begin();
	for (; itr != m_mapRequest.end();)
	{
		Request* request = itr->second;
		if (request->checkTimeout())
		{
			m_listTimeout.push_back(request);
			m_mapRequest.erase(itr++);
		}
		else
		{
			itr++;
		}
	}
	m_mtxRequest.Unlock();
	
	while (!m_listTimeout.empty())
	{
		Request* request = m_listTimeout.front();
		request->setIsTimeout(); // set this request is timeout
		m_listTimeout.pop_front();
		m_pSender->cancel(request);
		if (m_Param.pICallback)
		{
			m_Param.pICallback->onRequestResult(request);
		}
	}
}

bool TcpClient::onElementPath(iks* ep)
{
	bool done = false;

	do 
	{
		if (!ep)
			break;

		// log
		loggable()->logReceived(iks_string(iks_stack(ep), ep));

		// heart beat check
		onHeartbeat(ep);

		// process
		const char* pTag = iks_name(ep);
		string sTag = pTag;
		if (sTag.compare(TAG_MESSAGE) == 0)
		{
			// message
			onMessage(ep);
		}
		else
		{
			// TODO..
		}

		done = true;
	} while (0);

	return done;
}

void TcpClient::onMessage(iks* message)
{
	const char* pszType = iks_find_attrib(message, ATTRIBUTE_NAME_TYPE);
	
	if (!pszType || strlen(pszType) <= 0)
	{
		// TODO...
		return;
	}

	ICallback* pICallback = m_Param.pICallback;
	
	if (!strcmp(pszType, ATTRIBUTE_REQUEST))
	{		
		// remote Request
		RemoteRequest* pRemoteRequst = new RemoteRequest(message);
		if (m_bCallback && pICallback)
		{
			pICallback->onRequest(pRemoteRequst);
		}
		// delete
		SAFE_DELETE(pRemoteRequst);
	}
	else if (!strcmp(pszType, ATTRIBUTE_RESPONSE))
	{
		const char* pszSeq = iks_find_attrib(message, ATTRIBUTE_NAME_SEQ);
		if (!pszSeq || strlen(pszSeq) <= 0)
		{
			// TODO ...
			return;
		}
		
		Request* request = removeRequest(pszSeq);
		if (!request)
		{
			// TODO ...
			return;
		}

		RemoteResponse* pRemoteResponse = new RemoteResponse(message);
		request->setResponse(pRemoteResponse);

		if (m_bCallback && pICallback)
		{
			pICallback->onRequestResult(request);
		}

		// delete with response, not here
		// SAFE_DELETE(request);
	}
	else if (!strcmp(pszType, ATTRIBUTE_NOTIFICATION))
	{
		// 
		RemoteNotification* pNotification = new RemoteNotification(message);
		if (m_bCallback && pICallback)
		{
			pICallback->onNotification(pNotification);
		}
		// delete
		SAFE_DELETE(pNotification);
	}
	else
	{
		// TODO...
	}
	
}

void TcpClient::onHeartbeat(iks* /*heartbeat*/)
{
	m_eHeartbeatState = Heartbeat_Recv;
}

void TcpClient::onSocketBroken()
{
	if (m_bCallback)
	{
		loggable()->warning("TcpClient onSocketBroken");
	}

	if (m_bCallback && m_Param.pICallback)
	{
		m_Param.pICallback->onBroken();
	}
}

void TcpClient::onDataParseError()
{
	if (m_bCallback)
	{
		loggable()->warning("TcpClient onDataParseError");
	}

	if (m_bCallback && m_Param.pICallback)
	{
		m_Param.pICallback->onDataParseError();
	}
}

void TcpClient::clearNoResultRequest()
{
	map<string, Request*>::iterator itr = m_mapRequest.begin();
	for (; itr != m_mapRequest.end();)
	{
		Request* request = itr->second;
		request->setTcpError();

		m_mapRequest.erase(itr++);

		m_Param.pICallback->onRequestResult(request);
	}

	m_mapRequest.clear();
}

} // namespace



