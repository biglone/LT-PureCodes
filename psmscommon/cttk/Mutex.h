///@file Mutex.h 互斥体类头文件.

#ifndef _CTTK_MUTEX_H_INCLUDED
#define _CTTK_MUTEX_H_INCLUDED

#if defined(_WIN32)
#  include <winsock2.h>
#  include <windows.h>
#  define MUTEX_T CRITICAL_SECTION
#else //_WIN32
#  include <pthread.h>
#  define MUTEX_T pthread_mutex_t
#endif //_WIN32

namespace cttk
{

/**
* @brief 互斥体类.
* @version 1.0
* @date 2009-03-18
*/
class CMutex
{
public:
	CMutex();
	~CMutex();

public:
    /// 锁定
	void Lock() const;
    /// 解锁
	void Unlock() const;

private:
	mutable MUTEX_T m_mutex;
};

/**
* @brief 智能互斥体类.
* @version 1.0
* @date 2009-03-18
*/
class CAutoMutex
{
public:
	CAutoMutex(const CMutex* pMutex);
	~CAutoMutex();

private:
	const CMutex* m_pMutex;
};

}

#endif //_CTTK_MUTEX_H_INCLUDED


