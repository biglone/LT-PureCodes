#include "Semaphore.h"

namespace cttk
{

CSemaphore::CSemaphore(int nInit/* = 0*/, int nMax/* = SEM_VALUE_MAX*/)
{
#if defined(_WIN32)
    m_sem = CreateSemaphore(0,nInit,nMax,0);
#else //_WIN32
    sem_init(&m_sem,0,nInit);
#endif //_WIN32
}

CSemaphore::~CSemaphore()
{
#if defined(_WIN32)
	CloseHandle(m_sem);
#else //_WIN32
	sem_destroy(&m_sem);
#endif //_WIN32
}

void CSemaphore::Wait() const
{
#if defined(_WIN32)
    WaitForSingleObject((HANDLE)m_sem,INFINITE); 
#else //_WIN32
    sem_wait((sem_t*)&m_sem);
#endif //_WIN32
}

int CSemaphore::TryWait() const
{
#if defined(_WIN32)
    return ((WaitForSingleObject((HANDLE)m_sem,INFINITE)==WAIT_OBJECT_0)?0:EAGAIN);
#else //_WIN32
    return (sem_trywait((sem_t*)&m_sem)?errno:0);
#endif //_WIN32
}
int CSemaphore::Post() const
{
#if defined(_WIN32)
    return (ReleaseSemaphore((HANDLE)m_sem,1,0)?0:ERANGE);
#else //_WIN32
    return (sem_post((sem_t*)&m_sem)?errno:0);
#endif //_WIN32
}

int CSemaphore::Value() const
{
#if defined(_WIN32)
    LONG v = -1;
    ReleaseSemaphore((HANDLE)m_sem,0,&v);
    return v;
#else //_WIN32
    int v = -1;
    sem_getvalue((sem_t*)&m_sem,&v);
    return v;
#endif //_WIN32
}

}
