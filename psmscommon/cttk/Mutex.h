///@file Mutex.h ��������ͷ�ļ�.

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
* @brief ��������.
* @version 1.0
* @date 2009-03-18
*/
class CMutex
{
public:
	CMutex();
	~CMutex();

public:
    /// ����
	void Lock() const;
    /// ����
	void Unlock() const;

private:
	mutable MUTEX_T m_mutex;
};

/**
* @brief ���ܻ�������.
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


