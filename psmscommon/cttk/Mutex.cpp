#include "Mutex.h"

namespace cttk
{

CMutex::CMutex()
{
#if defined(_WIN32)
	InitializeCriticalSection(&m_mutex);
#else //_WIN32
	pthread_mutex_init(&m_mutex, NULL);
#endif //_WIN32
}

CMutex::~CMutex()
{
#if defined(_WIN32)
	DeleteCriticalSection(&m_mutex);
#else //_WIN32
	pthread_mutex_destroy(&m_mutex);
#endif //_WIN32
}

void CMutex::Lock() const
{
#if defined(_WIN32)
	EnterCriticalSection(&m_mutex);
#else //_WIN32
	pthread_mutex_lock(&m_mutex);
#endif //_WIN32
}

void CMutex::Unlock() const
{
#if defined(_WIN32)
	LeaveCriticalSection(&m_mutex);
#else //_WIN32
	pthread_mutex_unlock(&m_mutex);
#endif //_WIN32
}

CAutoMutex::CAutoMutex(const CMutex* pMutex)
: m_pMutex(pMutex)
{
	m_pMutex->Lock();
}

CAutoMutex::~CAutoMutex()
{
	m_pMutex->Unlock();
}

}
