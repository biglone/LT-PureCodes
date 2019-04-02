#ifndef __LOGGABLE_H__
#define __LOGGABLE_H__

class QString;

class ILoggable
{
public:
	virtual void debug(const char *message) = 0;
	virtual void info(const char *message) = 0;
	virtual void warning(const char *message) = 0;
	virtual void logReceived(const char *message) = 0;
	virtual void logSent(const char *message) = 0;
};

#endif // __LOGGABLE_H__