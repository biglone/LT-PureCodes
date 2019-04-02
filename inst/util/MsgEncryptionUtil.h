#ifndef _MSG_ENCRYPTION_UTIL_H_
#define _MSG_ENCRYPTION_UTIL_H_

#include <QString>
#include <QByteArray>

class MsgEncryptionUtil
{
public:
	static QByteArray generatePassword(const QString &seed);

	static QByteArray encrypt(const QByteArray &text, const QByteArray &password);

	static QByteArray decrypt(const QByteArray &secret, const QByteArray &password);
};

#endif // _MSG_ENCRYPTION_UTIL_H_
