///@file Thread.h �߳���ͷ�ļ�.

#ifndef _CTTK_THREAD_H_INCLUDED
#define _CTTK_THREAD_H_INCLUDED

#if defined(__call_prefix)
#  undef __call_prefix
#endif //__call_prefix

#if defined(_WIN32)
#  include <winsock2.h>
#  include <windows.h>
#  define __call_prefix __stdcall
#else //_WIN32
#  include <pthread.h>
#  define __call_prefix 
#endif //_WIN32

namespace cttk
{

/**
* @brief �̲߳�����.
* @version 1.0
* @date 2009-03-18
*/
class CThread
{
public:
	enum Priority 
	{		
		PRIOR_IDLE = 0,
		PRIOR_LOWEST,
		PRIOR_BELOW_NORMAL,
		PRIOR_NORMAL,
		PRIOR_ABOVE_NORMAL,
		PRIOR_HIGHEST,
		PRIOR_TIME_CRITICAL,
		PRIOR_ERROR
	};

	enum WaitMode
	{ 
		WAIT_PEEK = 0,                  ///< ��������
		WAIT_INFINITE = 0xFFFFFFFF,     ///< �ȴ�
	};

	typedef int FT_ROUTINE(void*);
	typedef void* Handle;

public:
	CThread();
	virtual ~CThread();

    /// �����߳�
	virtual bool Create(FT_ROUTINE* fpRoutine, void* pArg = 0);
	/// �ȴ��߳��˳�
    virtual bool Wait(unsigned int nTimeout = WAIT_INFINITE);
	/// ��ֹ�߳�
    virtual void Terminate();

    /// �������ȼ�
	virtual bool SetPriority(CThread::Priority ePrior);
    /// ��ȡ���ȼ�
	virtual CThread::Priority GetPriority() const;
	
#if defined(_WIN32)
    /// ��ȡ���
	virtual HANDLE GetHandle() const;
#else //_WIN32
	virtual pthread_t GetHandle() const;
#endif //_WIN32

    /// ��ȡ�̺߳�
	virtual unsigned int GetId() const;

    /// ����(��)
    static void sleep(unsigned long s);
    /// ����(����)
    static void usleep(unsigned long ms);

    /// ��ȡ��ǰ�̺߳�
    static unsigned int GetCurrentThreadId();

private:
	static unsigned int __call_prefix _ThreadRoutine(void* pThis);
	int ThreadRoutine();

private:
	FT_ROUTINE* m_fpRoutine;
	void* m_pArg;

#if defined(_WIN32)
	HANDLE m_hThread;
	unsigned int m_nId;	
#else //_WIN32
	pthread_t m_hThread;
#endif //_WIN32
};

/**
* @brief �߳������.
* @version 1.0
* @date 2009-03-18
*/
class CThreadAdaptor
{
public:
    enum ReturnType
    {
        error = -1,
        success = 0
    };

protected:
	CThreadAdaptor();
	virtual ~CThreadAdaptor();

public:
    /// ��ʼ
	bool Start();
    /// ֹͣ
	virtual void Stop(unsigned int uTimeout = CThread::WAIT_INFINITE, bool bViolent = true);

    /// �����˳����
	void SetExit();
	/// �˳�����Ƿ�������
    bool IsExitSet();

    /// �������ȼ�
	bool SetPriority(CThread::Priority ePrior);
	/// ��ȡ���ȼ�
    CThread::Priority GetPriority() const;

    /// ��ȡ�̺߳�
	unsigned int GetThreadId();

protected:
    /// �߳�ִ����,��������ʵ��
	virtual ReturnType OnRun() = 0;

protected:
	static int _Run(void* pArg);
	int Run();

protected:
	CThread* m_pThread;
	bool m_bExit;
};

}

#endif //_CTTK_THREAD_H_INCLUDED

