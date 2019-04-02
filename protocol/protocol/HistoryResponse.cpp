#include "net/RemoteResponse.h"
#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ErrorResponse.h"
#include "HistoryResponse.h"

namespace protocol
{
	HistoryResponse::HistoryResponse(net::RemoteResponse* pRr)
		: Response(pRr)
	{
		m_fType = protocol::OfflineResponse::User;
		m_from = "";
		m_ts = "";
		m_direction = protocol::HistoryRequest::Backward;
		m_number = 0;
		m_messages.clear();
	}

	bool HistoryResponse::Parse()
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

			// 找到 msg
			iks* pnMsg = iks_find(pnResponse, protocol::TAG_MSG);
			
			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnMsg);
			if (m_pER)
			{
				ErrorResponse::Log("HistoryResponse.Parse()", m_pER);
				bPError = false;
				break;
			}

			if (pnMsg)
			{
				// type
				char* pszType = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TYPE);
				if (!pszType || strlen(pszType) <= 0)
					break;
				if (iks_strcasecmp(pszType, "history") != 0)
					break;

				// ftype
				char* pszFType = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_FTYPE);
				if (!pszFType || strlen(pszFType) <= 0)
					break;
				if (iks_strcasecmp(pszFType, "group") == 0)
					m_fType = protocol::OfflineResponse::Group;
				else if (iks_strcasecmp(pszFType, "discuss") == 0)
					m_fType = protocol::OfflineResponse::Discuss;
				else
					m_fType = protocol::OfflineResponse::User;

				// from
				char* pszFrom = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_FROM);
				if (!pszFrom || strlen(pszFrom) <= 0)
					break;
				m_from = pszFrom;

				// ts
				char* pszTs = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_TS);
				if (!pszTs || strlen(pszTs) <= 0)
					break;
				m_ts = pszTs;

				// sd
				char* pszSd = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_SD);
				if (!pszSd || strlen(pszSd) <= 0)
					break;
				if (iks_strcasecmp(pszSd, "backward") == 0)
					m_direction = protocol::HistoryRequest::Backward;
				else
					m_direction = protocol::HistoryRequest::Forward;
				
				// number
				char* pszNumber = iks_find_attrib(pnMsg, protocol::ATTRIBUTE_NAME_NUMBER);
				if (!pszNumber || strlen(pszNumber) <= 0)
					break;
				m_number = atoi(pszNumber);
			}

			// items
			do 
			{
				// message
				iks* pnItem = iks_first_tag(pnMsg);
				while (pnItem)
				{
					if (strcmp(protocol::TAG_MESSAGE, iks_name(pnItem)) != 0)
					{
						continue;
					}

					// parse message
					protocol::MessageNotification::In msgIn;
					msgIn.Parse(pnItem);
					m_messages.push_back(msgIn.m_Message);

					pnItem = iks_next_tag(pnItem);
				}
			} while (0);
			
			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

	std::list<protocol::MessageNotification::Message> HistoryResponse::getMessages() const
	{
		return m_messages;
	}

	protocol::OfflineResponse::ItemType HistoryResponse::fType() const
	{
		return m_fType;
	}

	std::string HistoryResponse::from() const
	{
		return m_from;
	}

	std::string HistoryResponse::ts() const
	{
		return m_ts;
	}

	protocol::HistoryRequest::Direction HistoryResponse::direction() const
	{
		return m_direction;
	}

	int HistoryResponse::number() const
	{
		return m_number;
	}
}