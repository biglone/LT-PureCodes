///@file Thread.h 线程类头文件.

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
* @brief 线程操作类.
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
		WAIT_PEEK = 0,                  ///< 立即返回
		WAIT_INFINITE = 0xFFFFFFFF,     ///< 等待
	};

	typedef int FT_ROUTINE(void*);
	typedef void* Handle;

public:
	CThread();
	virtual ~CThread();

    /// 创建线程
	virtual bool Create(FT_ROUTINE* fpRoutine, void* pArg = 0);
	/// 等待线程退出
    virtual bool Wait(unsigned int nTimeout = WAIT_INFINITE);
	/// 终止线程
    virtual void Terminate();

    /// 设置优先级
	virtual bool SetPriority(CThread::Priority ePrior);
    /// 获取优先级
	virtual CThread::Priority GetPriority() const;
	
#if defined(_WIN32)
    /// 获取句柄
	virtual HANDLE GetHandle() const;
#else //_WIN32
	virtual pthread_t GetHandle() const;
#endif //_WIN32

    /// 获取线程号
	virtual unsigned int GetId() const;

    /// 挂起(秒)
    static void sleep(unsigned long s);
    /// 挂起(毫秒)
    static void usleep(unsigned long ms);

    /// 获取当前线程号
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
* @brief 线程体基类.
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
    /// 开始
	bool Start();
    /// 停止
	virtual void Stop(unsigned int uTimeout = CThread::WAIT_INFINITE, bool bViolent = true);

    /// 设置退出标记
	void SetExit();
	/// 退出标记是否已设置
    bool IsExitSet();

    /// 设置优先级
	bool SetPriority(CThread::Priority ePrior);
	/// 获取优先级
    CThread::Priority GetPriority() const;

    /// 获取线程号
	unsigned int GetThreadId();

protected:
    /// 线程执行体,由派生类实现
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

