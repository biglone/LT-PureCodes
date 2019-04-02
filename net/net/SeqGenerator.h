#ifndef _SEQGENERATOR_H_
#define _SEQGENERATOR_H_

#include <string>
#include "net_global.h"

namespace net
{

class NET_EXPORT SeqGenerator
{
public:
	SeqGenerator();
	~SeqGenerator();

	std::string getSeq();

private:
	int IncreaseSeqId();

private:
	int                 seqId;
	std::string              sb;
};
}
#endif