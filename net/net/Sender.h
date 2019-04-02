#ifndef _SENDER_H_
#define _SENDER_H_

#include <QObject>
#include <deque>
#include "cttk/Mutex.h"

namespace net 
{
class TcpClient;
class XmlMsg;
class Request;
class Sender : public QObject
{
	Q_OBJECT

public:
	Sender(TcpClient* pTcpClient);
	~Sender();

	bool post(XmlMsg* xmlMsg, bool bEmergency);

	std::string postHeartbeat();

	bool cancel(Request* request);

public Q_SLOTS:
	void onSend();
	
private:
    /**
     * 发送队列中的n个msg
     * @param listMsg,要发送的队列
     * @param n,要发送的个数,<=0则表示全部发完
     * @return
     */
	bool Send(std::deque<XmlMsg*>& listMsg, cttk::CMutex& rMutex, int n);

private:
	TcpClient*              m_pTcpClient;

	std::deque<XmlMsg*>     m_listEmergency;
	std::deque<XmlMsg*>     m_listNormal;

	cttk::CMutex            m_mtxEmergency;
	cttk::CMutex            m_mtxNormal;
};
}
#endif