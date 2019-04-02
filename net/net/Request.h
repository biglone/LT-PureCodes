#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>
#include "XmlMsg.h"

#include "SeqGenerator.h"

#include "net_global.h"

namespace net
{


class RemoteResponse;

class NET_EXPORT Request : public XmlMsg
{
public:
	static void setDefaultTimeout(int timeout);

	enum InnerError
	{
		NoError,
		TimeoutError,
		TcpError
	};

public:
	Request();
	virtual ~Request();

	void setTcpError();

	void setIsTimeout();

	void setTimeout(int timeout);

	void setBeginTime();

	int diffFromBeginTimeInSeconds() const;

	bool checkTimeout();

	void setResponse(net::RemoteResponse* response);

	std::string getSeq();

	void setMessage(const std::string& message);
	std::string getMessage() const;

	void setResult(bool result);

	bool getResult() const;

	int getInnerError() const;

	// ICallback.onRequestResult()中调用该函数
	void onResult();

	virtual int getType() = 0;

	virtual bool isRequest() { return true; }

protected:
	// 超时
	void onTimeout();

	void onTcpError();

	// 当被回调时,判断是否有应答,如果没有应答,应该是超时,返回null为没有应答
	RemoteResponse* getResponse();

	// 有结果
	virtual void onResponse(net::RemoteResponse* response) = 0;

private:
	// 当被回调时,判断是否超时,如果超时,应该没有应答,返回true为超时
	bool isTimeout() const;

	// 当被回调时,判断是否Tcp断掉,如果断掉,应该没有应答,返回true为断掉
	bool isTcpError() const;

private:
	int                      m_nTimeout;    // ms
	int                      m_nBeginMs;    // 初始时间
	std::string              m_sSeq;        // seq,每个request需要有唯一seq,相当于id

	bool                     m_bTimeout;   // 是否超时
	bool                     m_bTcpError;
	bool                     m_bResult;

	net::RemoteResponse*     m_pResponse;   // 请求的应答


	static int               s_default_timeout; // 默认超时时间
	static SeqGenerator      seqGenerator;

	InnerError               m_eInnerError;

	std::string              m_sMessage;
};

}
#endif