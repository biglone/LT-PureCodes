#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"
#include "cttk/base.h"

#include "net/IProtocolCallback.h"
#include "net/RemoteResponse.h"

#include "protocol/ErrorResponse.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "WithdrawMessage.h"

namespace protocol
{
	namespace WithdrawMessage
	{
		WithdrawSyncRequest::WithdrawSyncRequest(const char *szUid, const char *szWithdrawId)
		{
			if (szUid)
				m_uid = szUid;
			else
				m_uid = "";

			if (szWithdrawId)
				m_withdrawId = szWithdrawId;
			else
				m_withdrawId = "";
		}

		int WithdrawSyncRequest::getType()
		{
			return protocol::Request_Withdraw_Sync;
		}

		std::string WithdrawSyncRequest::getBuffer()
		{
			/*
			<message seq="20160509184925_88" type="request" module="MID_IM">
				<withdraws type="sync" uid="lixx" withdrawId="1234"/>
			</message>
			*/
			std::string sRet = "";

			do 
			{
				iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
				if (!pnMessage) break;

				CAutoIks aMessage(pnMessage);

				iks* pnWithdraws = iks_insert(pnMessage, protocol::TAG_WITHDRAWS);
				if (!pnWithdraws) break;

				// type
				iks_insert_attrib(pnWithdraws, protocol::ATTRIBUTE_NAME_TYPE, "sync");

				// uid
				iks_insert_attrib(pnWithdraws, protocol::ATTRIBUTE_NAME_UID, m_uid.c_str());

				// withdraw id
				iks_insert_attrib(pnWithdraws, protocol::ATTRIBUTE_NAME_WITHDRAWID, m_withdrawId.c_str());

				const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
				if (!pszBuffer) break;

				sRet = pszBuffer;
			} while(0);

			return sRet;
		}

		void WithdrawSyncRequest::onResponse(net::RemoteResponse* response)
		{
			WithdrawSyncResponse *withdrawSyncResponse  = new WithdrawSyncResponse(response);
			withdrawSyncResponse->Parse();
			getProtocolCallback()->onResponse(this, withdrawSyncResponse);
		}

		WithdrawSyncResponse::WithdrawSyncResponse(net::RemoteResponse *pRr)
			: Response(pRr)
		{

		}

		bool WithdrawSyncResponse::Parse()
		{
			/*
			<message seq="20160509184925_88" type="response" from="im:spim_83_1/" to="sung1/computer" module="MID_IM">
				<withdraws type="sync" uid="lixx" withdrawId="1234">
					<withdraw type="chat" to="lixx" from="sung1" timestamp="00000000001462781987322139399" id="2345"/>
					<withdraw type="groupchat" to="2" from="sung1" timestamp="00000000001462781987322139400" id="3456"/>
					<withdraw type="discuss" to="3" from="sung1" timestamp="00000000001462781987322139401" id="4567"/>
				</withdraws>
			</message>
			*/

			bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(a.协议信令本身错误,b.errcode+errmsg类型的应答)
			bool bPError = true;         // 是否协议信令本身的错误

			do
			{
				if (!m_pRR)
					break;

				iks* pnResponse = m_pRR->getMessage();
				if (!pnResponse)
					break;

				iks* pnWithdraws = iks_find(pnResponse, protocol::TAG_WITHDRAWS);
				if (!pnWithdraws)
					break;

				// 判断是否为错误应答
				m_pER = ErrorResponse::Parse(pnWithdraws);
				if (m_pER)
				{
					bPError = false;
					break;
				}

				iks* pnWithdraw = iks_first_tag(pnWithdraws);
				while (pnWithdraw)
				{
					const char* withdrawName = iks_name(pnWithdraw);
					if (!strcmp(withdrawName, TAG_WITHDRAW))
					{
						// type
						const char* pszType = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TYPE);

						// to
						const char* pszTo = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TO);

						// from
						const char* pszFrom = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_FROM);

						// timestamp
						const char* pszTimeStamp = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TIMESTAMP);

						// id
						const char* pszWithdrawId = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_ID);

						WithdrawItem item;
						item.m_chatType = (pszType ? pszType : "");
						item.m_to = (pszTo ? pszTo : "");
						item.m_from = (pszFrom ? pszFrom : "");
						item.m_timeStamp = (pszTimeStamp ? pszTimeStamp : "");
						item.m_withdrawId = (pszWithdrawId ? pszWithdrawId : "");
						m_withdrawItems.push_back(item);
					}

					pnWithdraw = iks_next_tag(pnWithdraw);
				}

				bOk = true;
			}while(false);

			if (!bOk)
			{
				setPError(bPError);
			}

			return bOk;
		}

		WithdrawRequest::WithdrawRequest(const char *szChatType, 
			                             const char *szTo, 
										 const char *szFrom, 
										 const char *szTimeStamp)
		{
			if (szChatType)
				m_chatType = szChatType;
			if (szTo)
				m_to = szTo;
			if (szFrom)
				m_from = szFrom;
			if (szTimeStamp)
				m_timeStamp = szTimeStamp;
		}

		int WithdrawRequest::getType()
		{
			return protocol::Request_Message_Withdraw;
		}

		std::string WithdrawRequest::getBuffer()
		{
			/*
			<message seq="20160509163923_14" type="request" module="MID_IM">
				<withdraw type="chat" to="lixx" from="sung1" timestamp="00000000001462781987322139374"/>
			</message>
			*/
			std::string sRet = "";

			do 
			{
				iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
				if (!pnMessage) break;

				CAutoIks aMessage(pnMessage);

				iks* pnWithdraw = iks_insert(pnMessage, protocol::TAG_WITHDRAW);
				if (!pnWithdraw) break;

				// type
				iks_insert_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TYPE, m_chatType.c_str());

				// to
				iks_insert_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TO, m_to.c_str());

				// from
				iks_insert_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_FROM, m_from.c_str());

				// timestamp
				iks_insert_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TIMESTAMP, m_timeStamp.c_str());

				const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
				if (!pszBuffer) break;

				sRet = pszBuffer;
			} while(0);

			return sRet;
		}

		void WithdrawRequest::onResponse(net::RemoteResponse* response)
		{
			WithdrawResponse *withdrawResponse  = new WithdrawResponse(response);
			withdrawResponse->Parse();
			getProtocolCallback()->onResponse(this, withdrawResponse);
		}

		WithdrawResponse::WithdrawResponse(net::RemoteResponse *pRr)
			: Response(pRr)
		{

		}

		bool WithdrawResponse::Parse()
		{
			bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(a.协议信令本身错误,b.errcode+errmsg类型的应答)
			bool bPError = true;         // 是否协议信令本身的错误

			do
			{
				if (!m_pRR)
					break;

				iks* pnResponse = m_pRR->getMessage();
				if (!pnResponse)
					break;

				iks* pnWithdraw = iks_find(pnResponse, protocol::TAG_WITHDRAW);
				if (!pnWithdraw)
					break;

				// 判断是否为错误应答
				m_pER = ErrorResponse::Parse(pnWithdraw);
				if (m_pER)
				{
					bPError = false;
					break;
				}

				// type
				const char* pszType = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TYPE);
				
				// to
				const char* pszTo = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TO);

				// from
				const char* pszFrom = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_FROM);

				// timestamp
				const char* pszTimeStamp = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TIMESTAMP);

				// id
				const char* pszWithdrawId = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_ID);
				
				m_withdrawItem.m_chatType = (pszType ? pszType : "");
				m_withdrawItem.m_to = (pszTo ? pszTo : "");
				m_withdrawItem.m_from = (pszFrom ? pszFrom : "");
				m_withdrawItem.m_timeStamp = (pszTimeStamp ? pszTimeStamp : "");
				m_withdrawItem.m_withdrawId = (pszWithdrawId ? pszWithdrawId : "");

				bOk = true;
			}while(false);

			if (!bOk)
			{
				setPError(bPError);
			}

			return bOk;
		}

		WithdrawNotification::WithdrawNotification()
		{

		}

		int WithdrawNotification::getNotificationType()
		{
			return protocol::MESSAGE_WITHDRAW;
		}

		bool WithdrawNotification::Parse(iks* pnIks)
		{
			bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)

			do
			{
				if (!pnIks)
					break;

				iks* pnWithdraw = pnIks;

				// type
				const char* pszType = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TYPE);

				// to
				const char* pszTo = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TO);

				// from
				const char* pszFrom = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_FROM);

				// timestamp
				const char* pszTimeStamp = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_TIMESTAMP);

				// id
				const char* pszWithdrawId = iks_find_attrib(pnWithdraw, protocol::ATTRIBUTE_NAME_ID);

				m_withdrawItem.m_chatType = (pszType ? pszType : "");
				m_withdrawItem.m_to = (pszTo ? pszTo : "");
				m_withdrawItem.m_from = (pszFrom ? pszFrom : "");
				m_withdrawItem.m_timeStamp = (pszTimeStamp ? pszTimeStamp : "");
				m_withdrawItem.m_withdrawId = (pszWithdrawId ? pszWithdrawId : "");

				bOk = true;
			}while(false);

			return bOk;
		}
	}
}