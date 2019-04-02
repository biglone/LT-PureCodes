#ifndef _MESSAGEDBSTORE_H_
#define _MESSAGEDBSTORE_H_

#include "bean/MessageBody.h"
#include <QThread>
#include <QSemaphore>

class MessageWriter;
class MessageDeleter;
class MessageReader;
namespace DB{
	class ComponentMessageDB;
}

class MessageDBStore : public QObject
{
Q_OBJECT

public:
	explicit MessageDBStore(QObject *parent = 0);
	virtual ~MessageDBStore();

public:
	void append(const bean::MessageBody &body);
	void append(bean::MessageType msgType, const QString &rsUuid, int nResult);
	void append(bean::MessageType msgType, const QString &rsUuid, const QString &filePath);
	void append(const QString &stamp, int readState);
	void append(int msgId, const bean::MessageBody &body);

	qint64 getMessages(bean::MessageType type, const QString &id, int page = 0, int countPerPage = 60, const QString &beginDate = "", const QString &endDate = "", const QString &keyword = "");
	qint64 getMessagesBeforeTime(bean::MessageType type, const QString &id, int page, int countPerPage, const QString &endDate);
	qint64 getContextMessages(bean::MessageType type, const QString &id, int msgId, int countPerPage = 60, const QString &beginDate = "", const QString &endDate = "");
	qint64 getMessageByStamp(bean::MessageType type, const QString &id, const QString &stamp);
	qint64 getMessageByMsgId(bean::MessageType type, int msgId);
	qint64 getAttachments(int page = 1, int countPerPage = 60, const QString &beginDate = "", const QString &keyword = "");

	qint64 removeMsgByMsgId(bean::MessageType type, int msgid);
	qint64 removeMsgByDate(bean::MessageType type, const QString &id, const QString &beginDate, const QString &endDate);
	qint64 removeAttachById(bean::MessageType type, int attachId);

	bool isInited() const;
	void init();
	void release();

Q_SIGNALS:
	void gotMessages(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs, int sum=0);
	void gotContextMessages(qint64 seq, int page, int maxPage, const bean::MessageBodyList &msgs, int sum=0);
	void gotMessageOfStamp(qint64 seq, const QString &stamp, const bean::MessageBody &msg);
	void gotMessageOfMsgId(qint64 seq, int msgId, const bean::MessageBody &msg);
	void gotAttachments(qint64 seq, int curPage, int maxPage, const QVariantList &attachs);
	void removed(qint64 seq);
	void finish();

private:
	QThread                m_thread;
	QSemaphore             m_sem;
	MessageWriter          *m_writer;
	MessageReader          *m_reader;
	MessageDeleter         *m_deleter;
	DB::ComponentMessageDB *m_msgDB;
};

#endif //_MESSAGEDBSTORE_H_
