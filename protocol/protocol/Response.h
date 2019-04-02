#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <string>

#include "protocol_global.h"

namespace net
{
	class RemoteResponse;
}

namespace protocol
{
class ErrorResponse;
class PROTOCOL_EXPORT Response
{
public:
	Response(net::RemoteResponse* pRr);
	virtual ~Response();

	bool isError();
	std::string getErrcode();
	std::string getErrmsg();
	bool getPError();
	std::string getModule();
	std::string getFrom();

	/**
     * 解析应答
     * true     正常应答,得到正常应答结果
     * false    错误应答,a.协议信令本身错误,b.errcoder+errmsg类型应答
     *          请进行类似操作 er = ErrorResponse.Parse(login);
     *          如果是协议信令本身错误,请调用setPError(true)
     */
    virtual bool Parse() = 0;

protected:
	void setPError(bool bPError);
	void setModule(const std::string& rsModel);
	void setFrom(const std::string& rsFrom);

protected:
	net::RemoteResponse*   m_pRR;        // 应答消息本身
    ErrorResponse*         m_pER;        // 正常错误应答
    bool                   m_bPError;    // 是否协议错误
	std::string            m_sModule;    // 模块,如果子类需要,则在parse中解析出来
	std::string            m_sFrom;      // from,如果子类需要,则在parse中解析出来
};
}
#endif