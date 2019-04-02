#ifndef __PASSWD_UTIL_H__
#define __PASSWD_UTIL_H__

#include <QString>
#include <QByteArray>

class PasswdUtil
{
public:
	static QByteArray toCryptogramPasswd(const QString &id, const QString &passwd);
};

#endif // __PASSWD_UTIL_H__