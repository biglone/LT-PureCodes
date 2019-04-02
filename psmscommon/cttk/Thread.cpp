#include "Thread.h"
#include "Base.h"

#if defined(_WIN32)
#  include <process.h>
#else //defined(_WIN32)
#  include <unistd.h>
#  include <sys/select.h>
#  include <sys/types.h>
#  include <sys/syscall.h>  
#  define gettid() syscall(__NR_gettid)
#endif //defined(_WIN32)

namespace cttk
{

CThread::CThread()
: m_fpRoutine(NULL)
, m_pArg(NULL)
, m_hThread(0)
#if defined(_WIN32)
, m_nId(-1)
#endif //_WIN32
{

}

CThread::~CThread()
{
	if (m_hThread)
	{
#if defined(_WIN32)
		::CloseHandle(m_hThread);
#endif //_WIN32
		m_hThread = 0;
	}
}

bool CThread::Create(FT_ROUTINE* fpRoutine, void* pArg/* = 0*/)
{
	if (!fpRoutine) return false;

	if (m_hThread) return false;

	m_fpRoutine = fpRoutine;	
	m_pArg = pArg;

#if defined(_WIN32)
	if (m_hThread) ::CloseHandle(m_hThread);

	m_hThread = (HANDLE) ::_beginthreadex(
		0, 
		0, 
		_ThreadRoutine,
		this, 
		0, 
		&m_nId);

	if (!m_hThread)
		return false;

#else //_WIN32
	int nRet = pthread_create(
		&m_hThread, 
		NULL, 
		(void* (*)(void*)) _ThreadRoutine, 
		(void*)this);

	if (nRet == -1)
	{
		return false;
	}
#endif //_WIN32

	return true;		
}

bool CThread::Wait(unsigned int nTimeout)
{
	if (!m_hThread) return false;

#if defined(_WIN32)
	DWORD dwRet = ::WaitForSingleObject(m_hThread, (DWORD)nTimeout);
	if (dwRet == WAIT_OBJECT_0)
	{
		::CloseHandle(m_hThread);
		m_hThread = 0;
		return true;
	}

#else //_WIN32
	if (nTimeout != WAIT_INFINITE)
		return false;

	if (pthread_join( m_hThread, 0 ) == 0)
	{
		m_hThread = 0;
		return true;
	}
#endif //_WIN32
	return false;
}

void CThread::Terminate()
{
	if (!m_hThread) return;

#if defined(_WIN32)
	::TerminateThread(m_hThread, -1);	
	::CloseHandle(m_hThread);
	m_hThread = 0;
#else //_WIN32
	pthread_cancel(m_hThread);
	m_hThread = 0;
#endif //_WIN32
	return;
}

bool CThread::SetPriority(CThread::Priority ePrior)
{
#if defined(_WIN32)
	static const int arPriors[] = 
	{
		THREAD_PRIORITY_IDLE,
		THREAD_PRIORITY_LOWEST,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_TIME_CRITICAL
	};
	return ::SetThreadPriority( m_hThread, arPriors[ePrior] ) != FALSE;
#else //_WIN32
	return true;
#endif //_WIN32
}

CThread::Priority CThread::GetPriority() const
{
#if defined(_WIN32)
	switch (::GetThreadPriority(m_hThread))
	{
	case THREAD_PRIORITY_TIME_CRITICAL	: return PRIOR_TIME_CRITICAL;
	case THREAD_PRIORITY_HIGHEST		: return PRIOR_HIGHEST;
	case THREAD_PRIORITY_ABOVE_NORMAL	: return PRIOR_ABOVE_NORMAL;
	case THREAD_PRIORITY_NORMAL			: return PRIOR_NORMAL;
	case THREAD_PRIORITY_BELOW_NORMAL	: return PRIOR_BELOW_NORMAL;
	case THREAD_PRIORITY_IDLE			: return PRIOR_IDLE;
	case THREAD_PRIORITY_LOWEST			: return PRIOR_LOWEST;
	default								: return PRIOR_NORMAL;
	}
#else //_WIN32
	return PRIOR_NORMAL;
#endif //_WIN32
}

#if defined(_WIN32)
HANDLE CThread::GetHandle() const
{
	return m_hThread;
}
#else //_WIN32
pthread_t CThread::GetHandle() const
{
	return m_hThread;
}
#endif //_WIN32

unsigned int CThread::GetId() const
{
#if defined(_WIN32)
	return m_nId;
#else //_WIN32
	return (unsigned int)pthread_self();
#endif //_WIN32
}

void CThread::sleep(unsigned long s)
{
#if defined(_WIN32)
    Sleep(s*1000);
#else //_WIN32
    ::sleep(s);
#endif //_WIN32
}

void CThread::usleep(unsigned long ms)
{
#if defined(_WIN32)
    Sleep(ms);
#else //_WIN32
    ::usleep(ms*1000);
#endif //_WIN32
}

unsigned int CThread::GetCurrentThreadId()
{
#if defined(_WIN32)
    return (unsigned int)::GetCurrentThreadId();
#else //_WIN32
    return (unsigned int)gettid();
#endif //_WIN32
}

unsigned int __call_prefix CThread::_ThreadRoutine(void* pThis)
{
	return reinterpret_cast<CThread*>(pThis)->ThreadRoutine(); 
}

int CThread::ThreadRoutine()
{
	int nRet = (*m_fpRoutine)(m_pArg);

	return nRet;
}

CThreadAdaptor::CThreadAdaptor()
: m_pThread(0)
, m_bExit(false)
{
	
}

CThreadAdaptor::~CThreadAdaptor()
{
	if (m_pThread)
	{
		Stop();
		delete m_pThread;
		m_pThread = NULL;
	}
}

bool CThreadAdaptor::Start()
{
	if (m_pThread) return false;

	m_pThread = new(std::nothrow) CThread();
	
	if (!m_pThread) return false;
	
	m_bExit = false;

	return m_pThread->Create(_Run, this);
}

void CThreadAdaptor::Stop(unsigned int uTimeout/* = CThread::WAIT_INFINITE*/, bool bViolent/* = true*/)
{
	if (!m_pThread || !m_pThread->GetHandle()) return;

	SetExit();

	do 
	{
		if (m_pThread->Wait(uTimeout))
			break;
		
		if (bViolent)
		{
			m_pThread->Terminate();
			break;
		}
	} while(0);

    SAFE_DELETE(m_pThread);

	return;
}

void CThreadAdaptor::SetExit()
{
	m_bExit = true;
}

bool CThreadAdaptor::IsExitSet()
{
	return m_bExit;
}

bool CThreadAdaptor::SetPriority(CThread::Priority ePrior)
{
	return m_pThread ? m_pThread->SetPriority(ePrior) : false;
}

CThread::Priority CThreadAdaptor::GetPriority() const
{
	return m_pThread ? m_pThread->GetPriority() : CThread::PRIOR_ERROR;
}

unsigned int CThreadAdaptor::GetThreadId()
{
	return m_pThread ? m_pThread->GetId() : -1;
}

int CThreadAdaptor::_Run(void* pArg)
{
	return pArg ? static_cast<CThreadAdaptor*>(pArg)->Run() : -1;
}

int CThreadAdaptor::Run()
{
	int nRet = OnRun();
	// 
	return nRet;
}

}
