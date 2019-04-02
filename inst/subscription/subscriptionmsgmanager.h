#ifndef SUBSCRIPTIONMSGMANAGER_H
#define SUBSCRIPTIONMSGMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QPointer>
#include <QMap>
#include <QList>
#include "subscriptionmsg.h"

class SubscriptionMsgDialog;
class SubscriptionManager;
class SubscriptionMessagesDBStore;

class SubscriptionMsgManager : public QObject
{
	Q_OBJECT

public:
	SubscriptionMsgManager(SubscriptionManager *subscriptionManager, QObject *parent = 0);
	~SubscriptionMsgManager();

	void start();
	void stop();
	void release();

	int unreadMsgCount(const QString &subscriptionId);
	void setUnreadMsgCount(const QString &subscriptionId, int count);
	QMap<QString, int> allUnreadMsgCount() const;

	qint64 getMessagesFromDB(const QString &subscriptionId, quint64 lastInnerId = 0, int count = 10);
	bool removeMessageOfSubscription(const QString &subscriptionId);

public slots:
	void getMessages();
	void openSubscriptionMsgDialog(const QString &subscriptionId);
	void closeSubscriptionMsgDialog(const QString &subscriptionId);
	SubscriptionMsg sendMsg(const QString &subscriptionId, const QString &content);
	void recvMsg(const SubscriptionMsg &msg);
	void openTitle(const QString &subscriptionId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name);

Q_SIGNALS:
	void unreadMsgChanged(const QString &subscriptionId, int count);
	void sendMsgFailed(const QString &subscriptionId);
	void openSubscriptionDetail(const QString &subscriptionId);
	void clickMenuFailed(const QString &subscriptionId);
	void viewMaterial(const QString &uid);
	void messagesGot(qint64 seq, const QList<SubscriptionMsg> &msgs);

private slots:
	void onGetMsgNumberFinished(bool ok, const QMap<QString, int> &msgNumbers);
	void onGetMessagesFinished(bool ok, const QString &subscriptionId, const QList<SubscriptionMsg> &messages);
	void onSendMsgFinished(bool ok, const QString &subscriptionId, const SubscriptionMsg &msg);
	void onClickMenuFinished(bool ok, const QString &subscriptionId, const QString &key, const SubscriptionMsg &msg);
	void openUrl(const QString &subscriptionId, const QString &url);
	void clickMenu(const QString &subscriptionId, const QString &key);

private:
	bool hasSubscriptionDialog(const QString &subscriptionId) const;
	SubscriptionMsgDialog *createDialog(const QString &subscriptionId);
	void reportLastSequence();
	void flashTaskBar(QWidget *widget);
	void addLastContact(bool send, const SubscriptionMsg &msg);
	void addMessage(const SubscriptionMsg &msg);
	QString makeMessageText(const SubscriptionMsg &msg, const QString &name) const;
	QString makeSubscriptionStamp(quint64 innerId) const;

private:
	QScopedPointer<SubscriptionMessagesDBStore>         m_subscriptionMessageDB;
	quint64                                             m_lastSequence;
	QMap<QString, int>                                  m_unreadMsgCount;
	QMap<QString, QPointer<SubscriptionMsgDialog>>      m_dialogs;
	SubscriptionManager                                *m_subscriptionManager;
	quint64                                             m_currentMsgInnerId;
	bool                                                m_running;
};

#endif // SUBSCRIPTIONMSGMANAGER_H
