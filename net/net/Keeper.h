#ifndef _KEEPER_H_
#define _KEEPER_H_

#include "cttk/Thread.h"

namespace net
{
class TcpClient;

class Keeper : public cttk::CThreadAdaptor
{
public:
	Keeper(TcpClient* pTcpClient);
	~Keeper();

private:
	cttk::CThreadAdaptor::ReturnType OnRun();

private:
	TcpClient*   m_pTcpClient;
};
}

#endif