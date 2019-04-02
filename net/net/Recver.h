#ifndef _RECVER_H_
#define _RECVER_H_

#include <QObject>
#include "psmscommon/MessageParser.h"

namespace net
{
class TcpClient;

class Recver : public QObject, public psmscommon::CMessageParser
{
	Q_OBJECT

public:
	Recver(TcpClient* pTcpClient);
	~Recver();

public Q_SLOTS:
	void onReadyRead();

private:
	// from CMessageParser
	void OnNode(iks* pnNode);

private:
	TcpClient*   m_pTcpClient;
};

}
#endif