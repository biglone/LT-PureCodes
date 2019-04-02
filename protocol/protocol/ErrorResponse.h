#ifndef _ERRORRESPONSE_H_
#define _ERRORRESPONSE_H_

#include <string>
#include "iks/iksemel.h"

#include "protocol_global.h"

namespace protocol
{

class PROTOCOL_EXPORT ErrorResponse
{
private:
	ErrorResponse();

public:
	~ErrorResponse() {}

	static ErrorResponse* Parse(iks* pnIks);

	static void Log(const std::string& rsWhere, ErrorResponse* pEr);

	std::string getErrcode()
	{
		return m_sErrcode;
	}

	std::string getErrmsg()
	{
		return m_sErrmsg;
	}



private:
	std::string m_sErrcode;
	std::string m_sErrmsg;

};

}
#endif