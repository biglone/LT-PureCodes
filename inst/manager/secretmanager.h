#ifndef SECRETMANAGER_H
#define SECRETMANAGER_H

#include <QObject>
#include <QMap>

class HttpPool;

class SecretManager : public QObject
{
	Q_OBJECT

public:
	SecretManager(QObject *parent = 0);
	~SecretManager();

	void setSecretAck(const QString &fromUid, const QString &toUid, const QString &stamp);
	void requestSecretAck(const QString &stamp, const QString &fromUid, const QString &toId);
	void clearSecretRead();
	void setSecretRead(const QString &stamp, int state = 1);

Q_SIGNALS:
	void setSecretAckOK(const QString &stamp);
	void setSecretAckFailed(const QString &stamp, const QString &errMsg);
	void requestSecretAckFinished(const QString &fromUid, const QString &toUid, const QString &stamp, int readState);
	void requestSecretAckFailed(const QString &stamp, const QString &errMsg);

private slots:
	void onHttpRequestFinished(int id, bool error, int httpCode, const QByteArray &recvData);

private:
	HttpPool          *m_httpPool;
	QMap<QString, int> m_readStates;
	QMap<int, QString> m_requestStamps;
	QMap<int, QString> m_setStamps;
};

#endif // SECRETMANAGER_H
