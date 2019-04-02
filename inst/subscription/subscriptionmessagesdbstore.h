#ifndef SUBSCRIPTIONMESSAGESDBSTORE_H
#define SUBSCRIPTIONMESSAGESDBSTORE_H

#include <QObject>
#include <QThread>
#include <QSemaphore>

class SubscriptionMessagesWriter;
class SubscriptionMessagesDeleter;
class SubscriptionMessagesReader;
class SubscriptionMsg;
namespace DB 
{
	class SubscriptionMessagesDB;
};

class SubscriptionMessagesDBStore : public QObject
{
	Q_OBJECT

public:
	static const qint64 ERROR_SEQUENCE = -1; // return this if want to do anything before calling init()

	SubscriptionMessagesDBStore(QObject *parent = 0);
	~SubscriptionMessagesDBStore();

	bool isInited() const;
	void init();
	void release();

	qint64 appendMessage(const SubscriptionMsg &msg);
	qint64 removeMessages(const QString &subscriptionId);
	qint64 getMessages(const QString &subscriptionId, quint64 lastInnerId = 0, int count = 10);
	qint64 appendUnreadMsgCount(const QString &subscriptionId, int count);

Q_SIGNALS:
	void messageWrited(qint64 seq, const QString &msgId, quint64 innerId);
	void messagesRemoved(qint64 seq, const QString &subscriptionId);
	void messagesGot(qint64 seq, const QList<SubscriptionMsg> &msgs, const QString &subscriptionId, quint64 lastInnerId, int count);
	void unreadMsgCountWrited(qint64 seq, const QString &subscriptionId, int count);
	void finish();

private:
	QThread                      m_thread;
	QSemaphore                   m_sem;
	SubscriptionMessagesWriter  *m_writer;
	SubscriptionMessagesDeleter *m_deleter;
	SubscriptionMessagesReader  *m_reader;
	DB::SubscriptionMessagesDB  *m_msgDB;
};


#endif // SUBSCRIPTIONMESSAGESDBSTORE_H
