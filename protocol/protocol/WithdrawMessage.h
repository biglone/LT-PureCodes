#ifndef __WITHDRAW_MESSAGE_H__
#define __WITHDRAW_MESSAGE_H__

#include <string>
#include <map>
#include <list>

#include "net/Request.h"
#include "protocol/Response.h"
#include "protocol/SpecificNotification.h"

#include "protocol_global.h"

namespace protocol
{
	namespace WithdrawMessage
	{
		struct PROTOCOL_EXPORT WithdrawItem
		{
			WithdrawItem() 
			{
				m_chatType = "";
				m_to = "";
				m_from = "";
				m_timeStamp = "";
				m_withdrawId = "";
			}

			std::string m_chatType;
			std::string m_to;
			std::string m_from;
			std::string m_timeStamp;
			std::string m_withdrawId;
		};

		class PROTOCOL_EXPORT WithdrawSyncRequest : public net::Request
		{
		public:
			WithdrawSyncRequest(const char *szUid, const char *szWithdrawId);

			virtual int getType();

			std::string getBuffer();

			std::string uid() {return m_uid;}
			std::string withdrawId() {return m_withdrawId;}

		protected:
			virtual void onResponse(net::RemoteResponse* response);

		private:
			std::string             m_uid;
			std::string             m_withdrawId;
		};

		class PROTOCOL_EXPORT WithdrawSyncResponse : public Response
		{
		public:
			WithdrawSyncResponse(net::RemoteResponse *pRr);

			bool Parse();

			std::string uid() {return m_uid;}
			std::string withdrawId() {return m_withdrawId;}
			std::list<protocol::WithdrawMessage::WithdrawItem> withdrawItems() {return m_withdrawItems;}

		private:
			std::string             m_uid;
			std::string             m_withdrawId;
			std::list<WithdrawItem> m_withdrawItems;
		};

		class PROTOCOL_EXPORT WithdrawRequest : public net::Request
		{
		public:
			WithdrawRequest(const char *szChatType, const char *szTo, const char *szFrom, const char *szTimeStamp);

			virtual int getType();

			std::string getBuffer();

			std::string chatType() const {return m_chatType;}
			std::string chatTo() const {return m_to;}
			std::string chatFrom() const {return m_from;}
			std::string chatTimeStamp() const {return m_timeStamp;}

		protected:
			virtual void onResponse(net::RemoteResponse* response);
			
		private:
			std::string m_chatType;
			std::string m_to;
			std::string m_from;
			std::string m_timeStamp;
		};

		class PROTOCOL_EXPORT WithdrawResponse : public Response
		{
		public:
			WithdrawResponse(net::RemoteResponse *pRr);
			
			bool Parse();

			protocol::WithdrawMessage::WithdrawItem withdrawItem() {return m_withdrawItem;}

		private:
			WithdrawItem m_withdrawItem;
		};

		class PROTOCOL_EXPORT WithdrawNotification : public protocol::SpecificNotification
		{
		public:
			WithdrawNotification();

			int getNotificationType();

			bool Parse(iks* pnIks);

			protocol::WithdrawMessage::WithdrawItem withdrawItem() {return m_withdrawItem;}

		private:
			WithdrawItem m_withdrawItem;
		};
	}
}

#endif // __WITHDRAW_MESSAGE_H__