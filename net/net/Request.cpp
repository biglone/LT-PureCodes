#include <time.h>
#include <assert.h>
#include "cttk/base.h"

#include "Request.h"
#include "IProtocolCallback.h"
#include "RemoteResponse.h"

namespace net
{

SeqGenerator Request::seqGenerator;
int Request::s_default_timeout = 40;

Request::Request()
: XmlMsg()
, m_pResponse(0)
, m_bTimeout(false)
, m_bTcpError(false)
, m_bResult(false)
, m_eInnerError(NoError)
{
	m_nBeginMs = cttk::datetime::currenttime();
	m_sSeq = seqGenerator.getSeq();
	m_nTimeout = s_default_timeout;
}

Request::~Request()
{
	if (m_pResponse)
	{
		delete m_pResponse;
		m_pResponse = 0;
	}
}

void Request::setDefaultTimeout(int timeout)
{
	s_default_timeout = timeout;
}

void Request::setTcpError()
{
	m_bTcpError = true;
}

void Request::setIsTimeout()
{
	m_bTimeout = true;
}

void Request::setTimeout(int timeout)
{
	m_nTimeout = timeout;
}

void Request::setBeginTime()
{
	m_nBeginMs = cttk::datetime::currenttime();
}

int Request::diffFromBeginTimeInSeconds() const
{
	int diff = cttk::datetime::diff(m_nBeginMs);
	return diff;
}

bool Request::checkTimeout()
{
	int diff = cttk::datetime::diff(m_nBeginMs);

	return diff >= m_nTimeout;
}

void Request::setResponse(RemoteResponse* response)
{
	assert(response != NULL);

	m_pResponse = response;
}

std::string Request::getSeq()
{
	return m_sSeq;
}

void Request::setMessage(const std::string& message)
{
	m_sMessage = message;
}

std::string Request::getMessage() const
{
	return m_sMessage;
}

void Request::setResult(bool result)
{
	m_bResult = result;
}

bool Request::getResult() const
{
	return m_bResult;
}

int Request::getInnerError() const
{
	return m_eInnerError;
}

void Request::onResult()
{
	if (isTimeout())
	{
		onTimeout();
		return;
	}
	
	if (isTcpError())
	{
		onTcpError();
		return;
	}

	onResponse(getResponse());
}

void Request::onTimeout()
{
	m_eInnerError = TimeoutError;
	getProtocolCallback()->onResponse(this, NULL);
}

void Request::onTcpError()
{
	m_eInnerError = TcpError;
	getProtocolCallback()->onResponse(this, NULL);
}

RemoteResponse* Request::getResponse()
{
	return m_pResponse;
}

bool Request::isTimeout() const
{
	return m_bTimeout;
}

bool Request::isTcpError() const
{
	return m_bTcpError;
}
}
