#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "GroupResponse.h"
#include "GroupRequest.h"

namespace protocol
{
	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS GroupRequest
	GroupRequest::GroupRequest(const std::string& rsId /* = "" */)
		: m_sId(rsId)
	{
	}

	int GroupRequest::getType()
	{
		return protocol::Request_IM_Group;
	}

	std::string GroupRequest::getGroupId() const
	{
		return m_sId;
	}

	std::string GroupRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnGroup = iks_insert(pnMessage, protocol::TAG_GROUP);
			if (!pnGroup) break;

			iks_insert_attrib(pnGroup, protocol::ATTRIBUTE_NAME_TYPE, "sync");

			if (!m_sId.empty())
			{
				iks_insert_attrib(pnGroup, protocol::ATTRIBUTE_NAME_ID, m_sId.c_str());
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void GroupRequest::onResponse(net::RemoteResponse* response)
	{
		GroupResponse* gr = new GroupResponse(response);
		gr->Parse();
		getProtocolCallback()->onResponse(this, gr);
	}

	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS GroupCardNameRequest
	GroupCardNameRequest::GroupCardNameRequest(const std::string &rsId, const std::string &rsUid, const std::string &rsCardName)
		: m_sId(rsId), m_sUid(rsUid), m_sCardName(rsCardName)
	{
	}

	int GroupCardNameRequest::getType()
	{
		return protocol::Request_Group_CardName;
	}

	std::string GroupCardNameRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks *pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks *pnGroup = iks_insert(pnMessage, protocol::TAG_GROUP);
			if (!pnGroup) break;

			iks_insert_attrib(pnGroup, protocol::ATTRIBUTE_NAME_TYPE, "changecardname");
			iks_insert_attrib(pnGroup, protocol::ATTRIBUTE_NAME_ID, m_sId.c_str());

			iks *pnItem = iks_insert(pnGroup, protocol::TAG_ITEM);
			iks_insert_attrib(pnItem, protocol::ATTRIBUTE_NAME_ID, m_sUid.c_str());
			iks_insert_attrib(pnItem, protocol::ATTRIBUTE_NAME_CARDNAME, m_sCardName.c_str());

			const char *pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	std::string GroupCardNameRequest::getGroupId()
	{
		return m_sId;
	}

	void GroupCardNameRequest::onResponse(net::RemoteResponse *response)
	{
		GroupCardNameResponse *res = new GroupCardNameResponse(response);
		res->Parse();
		getProtocolCallback()->onResponse(this, res);
	}
}
