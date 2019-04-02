#include <limits>
#include "cttk/Base.h"

#include "SeqGenerator.h"
namespace net
{

SeqGenerator::SeqGenerator()
	: seqId(0)
	, sb("")
{

}

SeqGenerator::~SeqGenerator()
{
}

std::string SeqGenerator::getSeq()
{
	sb = cttk::datetime::currentdatetime2()
		+ "_" 
		+ cttk::str::tostr(IncreaseSeqId());

	return sb;
}

int SeqGenerator::IncreaseSeqId()
{
	int tmp = seqId;
	if (seqId == INT_MAX)
	{
		seqId = 0;
	}
	else
	{
		seqId++;
	}
	return tmp;
}

}