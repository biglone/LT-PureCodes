#ifndef __THREAD_UTIL_H__
#define __THREAD_UTIL_H__

#include <QThread>

//////////////////////////////////////////////////////////////////////////
// class Thread: change protect static functions to public
class Thread : public QThread
{
public:
	static void msleep(unsigned long msecs)
	{
		QThread::msleep(msecs);
	}

	static void usleep(unsigned long usecs)
	{
		QThread::usleep(usecs);
	}

	static void sleep(unsigned long secs)
	{
		QThread::sleep(secs);
	}

private:
	Thread();
	~Thread();
};

#endif // __THREAD_UTIL_H__