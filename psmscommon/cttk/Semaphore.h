///@file Semaphore.h 信号量类头文件.

#ifndef _CTTK_SEMAPHORE_H_INCLUDED
#define _CTTK_SEMAPHORE_H_INCLUDED

#if defined(_WIN32)
#  include <winsock2.h>
#  include <windows.h>
#  define SEMAPHORE_T HANDLE
#else //_WIN32
#  include <semaphore.h>
#  define SEMAPHORE_T sem_t
#endif //_WIN32

# define SEM_VALUE_MAX ((int) ((~0u) >> 1))

#include <errno.h>

namespace cttk
{

/**
* @brief 信号量类.
* @version 1.0
* @date 2009-03-18
*/
class CSemaphore
{
public:
	CSemaphore(int nInit = 0, int nMax = SEM_VALUE_MAX);
	~CSemaphore();

public:
    /// 等待信号
	void Wait()     const;
    /// 等待信号(非阻塞)
    int  TryWait()  const;
    /// 增加信号量
    int  Post()     const;
    /// 信号量
    int  Value()    const;

private:
    SEMAPHORE_T m_sem;
};

}

#endif //_CTTK_SEMAPHORE_H_INCLUDED


