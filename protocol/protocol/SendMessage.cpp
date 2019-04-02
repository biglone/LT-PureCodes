#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"
#include "SendMessage.h"
#include "protocol/ProtocolType.h"
#include "net/IProtocolCallback.h"
#include "protocol/ProtocolConst.h"
#include "net/RemoteResponse.h"
#include "protocol/ErrorResponse.h"

namespace protocol
{
	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS SendMessageRequest
	SendMessageRequest::SendMessageRequest()
	{
	}

	int SendMessageRequest::getType()
	{
		return protocol::Request_Msg_Send;
	}

	std::string SendMessageRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnImMessage = iks_insert(pnMessage, protocol::TAG_MESSAGE);
			if (!pnImMessage) break;

			if (!MessageNotification::makeImMessage(pnImMessage, m_Message)) break;

			// get buffer
			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;

		} while(0);

		return sRet;
	}

	void SendMessageRequest::onResponse(net::RemoteResponse* response)
	{
		SendMessageResponse* lr = new SendMessageResponse(response);
		lr->Parse();
		getProtocolCallback()->onResponse(this, lr);
	}

	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS SendMessageResponse
	SendMessageResponse::SendMessageResponse(net::RemoteResponse* rr) 
		: Response(rr), m_timestamp("")
	{

	}

	std::string SendMessageResponse::getTimeStamp()
	{
		return m_timestamp;
	}

	bool SendMessageResponse::Parse()
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

			// 找到<message>
			iks* pnMsg = iks_find(pnResponse, protocol::TAG_MESSAGE);
			
			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnMsg);
			if (m_pER)
			{
				ErrorResponse::Log("SendMessageResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			char* szTimestamp = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TIMESTAMP);
			if (szTimestamp)
				m_timestamp = szTimestamp;

			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

}