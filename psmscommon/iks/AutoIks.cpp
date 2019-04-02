#include "AutoIks.h"

CAutoIks::CAutoIks(iks*& pIks)
: m_pIks(pIks)
{

}

CAutoIks::~CAutoIks()
{
	if (m_pIks)
	{
		iks_delete(m_pIks);
		m_pIks = 0;
	}
}

iks* CAutoIks::GetIks() const
{
	return m_pIks;
}
