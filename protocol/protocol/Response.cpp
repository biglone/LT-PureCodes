#include <assert.h>
#include "ErrorResponse.h"
#include "Response.h"
#include "net/RemoteResponse.h"

namespace protocol
{
Response::Response(net::RemoteResponse* pRr)
	: m_pRR(pRr)
	, m_pER(0)
	, m_bPError(false)
	, m_sFrom("")
	, m_sModule("")
{
}

Response::~Response()
{
}

bool Response::isError()
{
	return m_pER != NULL;
}

std::string Response::getErrcode()
{
	if (isError())
	{
		return m_pER->getErrcode();
	}
	else
	{
		const char *kClientDefineProtocolError = "55555555";
		return std::string(kClientDefineProtocolError);
	}
}

std::string Response::getErrmsg()
{
	if (isError())
	{
		return m_pER->getErrmsg();
	}
	else
	{
		std::string protocolErrMsg("Protocol error, no error response.");
		if (m_pRR)
		{
			protocolErrMsg = protocolErrMsg + std::string("\n") + m_pRR->getBuffer();
		}
		return protocolErrMsg;
	}
}

bool Response::getPError()
{
	return m_bPError;
}

std::string Response::getModule()
{
	return m_sModule;
}

std::string Response::getFrom()
{
	return m_sFrom;
}

void Response::setPError(bool bPError)
{
	m_bPError = bPError;
}

void Response::setModule(const std::string& rsModel)
{
	m_sModule = rsModel;
}

void Response::setFrom(const std::string& rsFrom)
{
	m_sFrom = rsFrom;
}

}
