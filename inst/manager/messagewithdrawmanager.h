#ifndef MESSAGEWITHDRAWMANAGER_H
#define MESSAGEWITHDRAWMANAGER_H

#include <QObject>
#include "bean/bean.h"
#include <QScopedPointer>

class WithdrawResHandler;
class WithdrawNtfHandler;

class MessageWithdrawManager : public QObject
{
	Q_OBJECT

public:
	MessageWithdrawManager(QObject *parent = 0);
	~MessageWithdrawManager();

	void initObject();
	void removeObject();

	void syncWithdraws(const QString &uid, const QString &withdrawId);
	void withdraw(bean::MessageType chatType, const QString &toId, const QString &fromId, const QString &timeStamp);
	void setLastWithdrawId(const QString &withdrawId, bool force = false);
	QString lastWithdrawId() const;

Q_SIGNALS:
	void withdrawOK(bean::MessageType chatType, const QString &toId, const QString &fromId, 
		            const QString &timeStamp, const QString &withdrawId);
	void withdrawFailed(bean::MessageType chatType, const QString &toId, const QString &fromId, const QString &timeStamp);
	void messageWithdrawed(bean::MessageType chatType, const QString &toId, const QString &fromId, 
		                   const QString &timeStamp, const QString &withdrawId);

private:
	QScopedPointer<WithdrawResHandler> m_resHandler;
	QScopedPointer<WithdrawNtfHandler> m_ntfHandler;
	QString                            m_withdrawId;
};

#endif // MESSAGEWITHDRAWMANAGER_H
