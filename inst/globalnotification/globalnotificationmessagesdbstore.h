#ifndef GLOBALNOTIFICATIONMESSAGESDBSTORE_H
#define GLOBALNOTIFICATIONMESSAGESDBSTORE_H

#include <QObject>
#include <QThread>
#include <QSemaphore>

class GlobalNotificationMessagesWriter;
class GlobalNotificationMessagesDeleter;
class GlobalNotificationMessagesReader;
class GlobalNotificationMsg;
namespace DB 
{
	class GlobalNotificationMessagesDB;
};

class GlobalNotificationMessagesDBStore : public QObject
{
	Q_OBJECT

public:
	static const qint64 ERROR_SEQUENCE = -1; // return this if want to do anything before calling init()

	GlobalNotificationMessagesDBStore(QObject *parent = 0);
	~GlobalNotificationMessagesDBStore();

	bool isInited() const;
	void init();
	void release();

	qint64 appendMessage(const GlobalNotificationMsg &msg);
	qint64 removeMessages(const QString &globalNotificationId);
	qint64 getMessages(const QString &globalNotificationId, quint64 lastInnerId = 0, int count = 10);
	qint64 appendUnreadMsgCount(const QString &globalNotificationId, int count);

Q_SIGNALS:
	void messageWrited(qint64 seq, const QString &msgId, quint64 innerId);
	void messagesRemoved(qint64 seq, const QString &globalNotificationId);
	void messagesGot(qint64 seq, const QList<GlobalNotificationMsg> &msgs, const QString &globalNotificationId, quint64 lastInnerId, int count);
	void unreadMsgCountWrited(qint64 seq, const QString &globalNotificationId, int count);
	void finish();

private:
	QThread                      m_thread;
	QSemaphore                   m_sem;
	GlobalNotificationMessagesWriter  *m_writer;
	GlobalNotificationMessagesDeleter *m_deleter;
	GlobalNotificationMessagesReader  *m_reader;
	DB::GlobalNotificationMessagesDB  *m_msgDB;
};


#endif // GLOBALNOTIFICATIONMESSAGESDBSTORE_H
