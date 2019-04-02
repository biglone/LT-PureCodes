#include "ProtocolConst.h"
#include "ErrorResponse.h"

namespace protocol
{
	ErrorResponse::ErrorResponse()
		: m_sErrcode("")
		, m_sErrmsg("")
	{}

	ErrorResponse* ErrorResponse::Parse(iks* pnIks)
	{
		ErrorResponse* pEe = 0;

		const char* pszErrcode = iks_find_attrib(pnIks, protocol::ATTRIBUTE_ERRCODE);
		const char* pszErrmsg = iks_find_attrib(pnIks, protocol::ATTRIBUTE_ERRMSG);

		if (pszErrcode && pszErrmsg)
		{
			pEe             = new ErrorResponse();
			pEe->m_sErrcode = pszErrcode;
			pEe->m_sErrmsg  = pszErrmsg;
		}

		return pEe;
	}

	void ErrorResponse::Log(const std::string& /*rsWhere*/, ErrorResponse* /*pEr*/)
	{
		//
	}
}