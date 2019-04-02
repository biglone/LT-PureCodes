#ifndef __RECV_BEGIN_GENERATOR_H__
#define __RECV_BEGIN_GENERATOR_H__

#include <QByteArray>

class RecvBeginGenerator
{
public:
	virtual QByteArray generateRecvBegin() = 0;
};

#endif // __RECV_BEGIN_GENERATOR_H__