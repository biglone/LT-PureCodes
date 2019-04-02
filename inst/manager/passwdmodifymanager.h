#ifndef PASSWDMODIFYMANAGER_H
#define PASSWDMODIFYMANAGER_H

#include <QObject>
#include <QString>
#include "Constants.h"

class HttpPool;

class PasswdModifyManager : public QObject
{
	Q_OBJECT

public:
	PasswdModifyManager(QObject *parent = 0);
	~PasswdModifyManager();

	void modify(const QString &phone, const QString &oldPasswd, const QString &newPasswd);

Q_SIGNALS:
	void passwdModifyOK();
	void passwdModifyFailed(const QString &desc);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	HttpPool *m_httpPool;
	int       m_requestId;
};

#endif // PASSWDMODIFYMANAGER_H
