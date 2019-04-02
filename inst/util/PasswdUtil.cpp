#include "PasswdUtil.h"
#include <QCryptographicHash>

QByteArray PasswdUtil::toCryptogramPasswd(const QString &id, const QString &passwd)
{
	QString passwdText = id + passwd + "*!!";
	QByteArray raw = QCryptographicHash::hash(passwdText.toLatin1(), QCryptographicHash::Md5);
	QByteArray secret = raw.toHex().toLower();
	return secret;
}