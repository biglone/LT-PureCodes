#ifndef _CONNECTER_H_
#define _CONNECTER_H_

#include <QObject>
#include "cttk/Mutex.h"

namespace net
{

class TcpClient;

class Connecter: public QObject
{
	Q_OBJECT

public:
	Connecter(TcpClient* pTcpClient);
	~Connecter();

public Q_SLOTS:
	void connect();

private:
	TcpClient*      m_pTcpClient;
};

}
#endif