#ifndef _INTERPHONERESPONSE_H_
#define _INTERPHONERESPONSE_H_

#include <string>
#include <vector>

#include "iks/iksemel.h"
#include "net/Request.h"
#include "Response.h"
#include "InterphoneRequest.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT InterphoneResponse : public Response
	{
	public:
		struct Item
		{
			std::string                              m_iid;
			std::string                              m_attachType;
			std::string                              m_attachId;
			std::string                              m_speakId;
			int                                      m_memberCount;

			Item() : m_memberCount(0) {}
		};

	public:
		InterphoneResponse(net::RemoteResponse* pRR);
		~InterphoneResponse();

		bool Parse();

		void setActionType(InterphoneRequest::ActionType type);
		InterphoneRequest::ActionType getActionType() const;
		std::string interphoneId() const;
		std::string attachType() const;
		std::string attachId() const;
		std::string speakId() const;
		int memberCount() const;
		std::vector<std::string> memberIds() const;
		std::string audioAddr() const;

		std::vector<InterphoneResponse::Item> items() const;

	private:
		InterphoneRequest::ActionType            m_actionType;
		std::string                              m_iid;
		std::string                              m_attachType;
		std::string                              m_attachId;
		std::string                              m_speakId;
		int                                      m_memberCount;
		std::vector<std::string>                 m_memberIds;
		std::string                              m_audioAddr;

		std::vector<InterphoneResponse::Item>    m_items;
	};
}
#endif //_INTERPHONERESPONSE_H_
