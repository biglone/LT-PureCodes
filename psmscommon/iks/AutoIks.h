#ifndef _AUTOIKS_H_INCLUDED_20090620
#define _AUTOIKS_H_INCLUDED_20090620

#include "iksemel.h"

class CAutoIks
{
public:
	CAutoIks(iks*& pIks);
	~CAutoIks();

public:
	iks* GetIks() const;

private:
	iks*& m_pIks;

private:
	CAutoIks();
};


#endif //_AUTOIKS_H_INCLUDED_20090620
