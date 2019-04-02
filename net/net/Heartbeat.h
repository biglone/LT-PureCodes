#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include "XmlMsg.h"
#include "SeqGenerator.h"

namespace net
{

class Heartbeat : public XmlMsg
{
public:
	Heartbeat();
	std::string getSeq();

	// XmlMsg -------------
	std::string getBuffer();
private:
	static SeqGenerator  seqGenerator;

	std::string          m_sSeq;
};

}
#endif