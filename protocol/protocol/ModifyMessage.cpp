#include <assert.h>

#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"
#include "net/RemoteResponse.h"

#include "protocol/ErrorResponse.h"
#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"

#include "ModifyMessage.h"

static const char MODIFY_USER_DETAIL[] = "user-detail";

namespace protocol
{
	std::string ModifyMessage::ModifyTypeToString(ModifyMessage::ModifyType type)
	{
		switch (type)
		{
		case ModifyMessage::Modify_User_Detail:
			return MODIFY_USER_DETAIL;
		}

		return MODIFY_USER_DETAIL;
	}

	ModifyMessage::ModifyType ModifyMessage::ModifyTypeFromString(const std::string& type)
	{
		if (type.compare(MODIFY_USER_DETAIL) == 0)
		{
			return ModifyMessage::Modify_User_Detail;
		}

		return ModifyMessage::Modify_User_Detail;
	}

	ModifyMessage::ModifyRequest::ModifyRequest(ModifyType type /* = Modify_User_Detail */)
		: m_eType(type)
	{
		m_Content.clear();
	}

	void ModifyMessage::ModifyRequest::addContent(const std::string& key, const std::string& sValue)
	{
		m_Content[key] = sValue;
	}

	int ModifyMessage::ModifyRequest::getType()
	{
		return protocol::Request_IM_Modify;
	}

	std::string ModifyMessage::ModifyRequest::getBuffer()
	{
		std::string sRet = "";

		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_REQUEST, getSeq().c_str(), 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnModify = iks_insert(pnMessage, protocol::TAG_MODIFY);
			if (!pnModify) break;

			// type
			iks_insert_attrib(pnModify, protocol::ATTRIBUTE_NAME_TYPE, ModifyTypeToString(m_eType).c_str());

			const char* pszTag = protocol::TAG_USER;
			switch (m_eType)
			{
			case ModifyMessage::Modify_User_Detail:
				pszTag = protocol::TAG_USER;
				break;
			}
			
			iks* pnTag = iks_insert(pnModify, pszTag);

			ModifyMessage::ModifyContent::iterator itr = m_Content.begin();
			for (; itr != m_Content.end(); ++itr)
			{
				if (itr->first.compare(protocol::TAG_PHOTO) != 0)
				{
					iks_insert_attrib(pnTag, itr->first.c_str(), itr->second.c_str());
				}
				else
				{
					iks* pnPhoto = iks_insert(pnTag, protocol::TAG_PHOTO);
					iks_insert_cdata(pnPhoto, itr->second.c_str(), 0);
				}
			}

			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);

		return sRet;
	}

	void ModifyMessage::ModifyRequest::onResponse(net::RemoteResponse* response)
	{
		ModifyMessage::ModifyResponse* pMr = new ModifyMessage::ModifyResponse(response);
		assert(pMr != NULL);

		pMr->Parse();
		getProtocolCallback()->onResponse(this, pMr);
	}

	ModifyMessage::ModifyResponse::ModifyResponse(net::RemoteResponse* pRr)
		: Response(pRr)
	{
	}

	bool ModifyMessage::ModifyResponse::Parse()
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

			// 找到 Modify
			iks* pnModify = iks_find(pnResponse, protocol::TAG_MODIFY);
			if (!pnModify)
			{
				break;
			}

			// 判断是否为错误应答
			m_pER = ErrorResponse::Parse(pnModify);
			if (m_pER)
			{
				ErrorResponse::Log("ModifyRespone.Parse()", m_pER);
				bPError = false;
				break;
			}

			// ok

			bOk = true;
		}while(false);

		if (!bOk)
		{
			setPError(bPError);
		}

		return bOk;
	}

}