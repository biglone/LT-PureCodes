#include "MsgEncryptionUtil.h"
#include <QCryptographicHash>
#include "openssl/rc4.h"

QByteArray MsgEncryptionUtil::generatePassword(const QString &seed)
{
	if (seed.isEmpty())
		return QByteArray();

	QByteArray raw = QCryptographicHash::hash(seed.toLatin1(), QCryptographicHash::Md5);
	QByteArray secret = raw.toHex().toLower();
	return secret;
}

QByteArray MsgEncryptionUtil::encrypt(const QByteArray &text, const QByteArray &password)
{
	if (text.isEmpty() || password.isEmpty())
		return text;

	// encrypt data
	int len = text.length();
	RC4_KEY rc4Key;
	RC4_set_key(&rc4Key, password.length(), (unsigned char*)password.constData());
	char *rc4Data = new char[len+1];
	memset(rc4Data, 0, len+1);
	RC4(&rc4Key, len, (unsigned char*)(text.constData()), (unsigned char*)rc4Data);
	rc4Data[len] = 0;

	QByteArray rc4Buffer(rc4Data, len);
	QByteArray secret = rc4Buffer.toHex();

	delete rc4Data;
	rc4Data = 0;

	return secret;
}

QByteArray MsgEncryptionUtil::decrypt(const QByteArray &secret, const QByteArray &password)
{
	if (secret.isEmpty() || password.isEmpty())
		return secret;

	QByteArray rawText = QByteArray::fromHex(secret);

	// decrypt content
	int len = rawText.length();
	RC4_KEY rc4key;
	char *rawData = new char[len+1];
	RC4_set_key(&rc4key, password.length(), (unsigned char*)password.constData());
	RC4(&rc4key, len, (unsigned char*)rawText.constData(), (unsigned char*)rawData);
	rawData[len] = 0;

	QByteArray text(rawData, len);

	delete[] rawData;
	rawData = 0;

	return text;
}