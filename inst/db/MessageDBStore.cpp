#include "MessageDBStore.h"
#include <QDebug>
#include <QMutex>
#include "common/datetime.h"
#include <QBasicTimer>
#include <QTimer>
#include <QTimerEvent>
#include "ComponentMessageDB.h"
#include "MessageView.h"
#include "AttachmentDB.h"

static qint64 getMessageStoreSequence()
{
	static qint64 seed = 1;
	qint64 seq = CDateTime::currentMSecsSinceEpoch() + seed;
	++seed;
	return seq;
}

//////////////////////////////////////////////////////////////////////////
// class MessageWrapper
class MessageWrapper
{
public:
	enum StoreType
	{
		StoreMessage,
		StoreResult,
		StoreAttName,
		StoreReadState,
		ReplaceMessage
	};

	MessageWrapper() : m_msgBody(), m_attItem()
	{
		m_storeType = StoreMessage;
	}

	MessageWrapper(const bean::MessageBody &messageBody) : m_attItem()
	{
		m_storeType = StoreMessage;
		m_msgBody = messageBody;
	}

	MessageWrapper(bean::MessageType msgType, const QString &rsUuid, int nResult) : m_msgBody()
	{
		m_storeType = StoreResult;
		m_attItem.setMessageType(msgType);
		m_attItem.setUuid(rsUuid);
		m_attItem.setTransferResult(nResult);
	}

	MessageWrapper(bean::MessageType msgType, const QString &rsUuid, const QString &filePath) : m_msgBody()
	{
		m_storeType = StoreAttName;
		m_attItem.setMessageType(msgType);
		m_attItem.setUuid(rsUuid);
		m_attItem.setFilePath(filePath);
	}

	MessageWrapper(const QString &stamp, int readState) : m_msgBody()
	{
		m_storeType = StoreReadState;
		m_msgBody.setMessageType(bean::Message_Chat);
		m_msgBody.setStamp(stamp);
		m_msgBody.setReadState(readState);
	}

	MessageWrapper(int msgId, const bean::MessageBody &messagebody)
	{
		m_storeType = ReplaceMessage;
		m_msgBody = messagebody;
		m_msgBody.setMessageid(msgId);
	}

	void storeMessage(DB::ComponentMessageDB *msgDB);

	MessageWrapper& operator=(const MessageWrapper &msgWrapper)
	{
		if (this != &msgWrapper)
		{
			this->m_storeType = msgWrapper.m_storeType;
			this->m_msgBody = msgWrapper.m_msgBody;
			this->m_attItem = msgWrapper.m_attItem;
		}
		return *this;
	}

private:
	StoreType   m_storeType;

	// for StoreMessage
	bean::MessageBody m_msgBody;

	// for StoreResult & StoreAttName
	bean::AttachItem  m_attItem;
};

void MessageWrapper::storeMessage(DB::ComponentMessageDB *msgDB)
{
	if (m_storeType == MessageWrapper::StoreMessage)
	{
		msgDB->storeMessage(m_msgBody);
	}
	else if (m_storeType == MessageWrapper::StoreResult)
	{
		msgDB->switchDB(m_attItem.messageType());
		msgDB->storeAttachResult(m_attItem.uuid(), m_attItem.transferResult());
	}
	else if (m_storeType == MessageWrapper::StoreAttName)
	{
		msgDB->switchDB(m_attItem.messageType());
		msgDB->updateAttachName(m_attItem.uuid(), m_attItem.filepath());
	}
	else if (m_storeType == MessageWrapper::StoreReadState)
	{
		msgDB->switchDB(m_msgBody.messageType());
		msgDB->storeReadState(m_msgBody.stamp(), m_msgBody.readState());
	}
	else if (m_storeType == MessageWrapper::ReplaceMessage)
	{
		msgDB->replaceMessage(m_msgBody.messageid(), m_msgBody);
	}
}

//////////////////////////////////////////////////////////////////////////
// class MessageWriter
class MessageWriter : public QObject
{
	Q_OBJECT
public:
	explicit MessageWriter(QSemaphore &sem, DB::ComponentMessageDB *msgDB, QObject *parent = 0);
	virtual ~MessageWriter();

public:
	bool hasLeftToWrite() const;

Q_SIGNALS:
	void storeFinished();

public slots:
	void append(const bean::MessageBody &body);
	void append(int msgType, const QString& rsUuid, int nResult);
	void append(int msgType, const QString& rsUuid, const QString& filePath);
	void append(const QString &stamp, int readState);
	void append(int msgType, const bean::MessageBody &body);
	void finish();

protected:
	void timerEvent(QTimerEvent *e);

private slots:
	void save();
	void next();

private:
	QList<MessageWrapper>   m_cache;
	QSemaphore             &m_finishSem;
	DB::ComponentMessageDB *m_msgDB;
	QBasicTimer             m_timer;
};

MessageWriter::MessageWriter(QSemaphore &sem, DB::ComponentMessageDB *msgDB, QObject *parent /*= 0*/ ) 
: QObject(parent), m_finishSem(sem), m_msgDB(msgDB)
{
}

MessageWriter::~MessageWriter()
{
}

bool MessageWriter::hasLeftToWrite() const
{
	return (m_cache.count() > 0);
}

void MessageWriter::append(const bean::MessageBody &body)
{
	MessageWrapper msgWrapper(body);
	m_cache.append(msgWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void MessageWriter::append(int msgType, const QString& rsUuid, int nResult)
{
	MessageWrapper msgWrapper((bean::MessageType)msgType, rsUuid, nResult);
	m_cache.append(msgWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void MessageWriter::append(int msgType, const QString& rsUuid, const QString& filePath)
{
	MessageWrapper msgWrapper((bean::MessageType)msgType, rsUuid, filePath);
	m_cache.append(msgWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void MessageWriter::append(const QString &stamp, int readState)
{
	MessageWrapper msgWrapper(stamp, readState);
	m_cache.append(msgWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void MessageWriter::append(int msgId, const bean::MessageBody &body)
{
	MessageWrapper msgWrapper(msgId, body);
	m_cache.append(msgWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void MessageWriter::finish()
{
	save();
	m_finishSem.release(1);
}

void MessageWriter::timerEvent( QTimerEvent *e )
{
	if (e->timerId() == m_timer.timerId())
	{
		next();
	}
	else
	{
		QObject::timerEvent(e);
	}
}

void MessageWriter::save()
{
	foreach (MessageWrapper msgWrapper, m_cache)
	{
		msgWrapper.storeMessage(m_msgDB);
	}

	qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO << "all messages were written: " << m_cache.count();
	m_cache.clear();
}

void MessageWriter::next()
{
	if (m_cache.isEmpty() && m_timer.isActive())
	{
		emit storeFinished();
		m_timer.stop();
		return;
	}

	MessageWrapper msgWrapper = m_cache.takeFirst();
	msgWrapper.storeMessage(m_msgDB);
	
	int leftCount = m_cache.count();
	qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO << " take 1 msg write, left: " << leftCount;
}

//////////////////////////////////////////////////////////////////////////
// class MessageDeleter
class MessageDeleter : public QObject
{
	Q_OBJECT
public:
	enum DelType
	{
		Type_ByMsgID,
		Type_ByDate,
		Type_Attach
	};

	struct DelParam
	{
		DelParam() : seq(-1), type(Type_ByMsgID), msgtype(bean::Message_Invalid), msgid(-1) {}

		qint64 seq;
		DelType type;
		bean::MessageType msgtype;
		int msgid;
		QString id;
		QString beginDate;
		QString endDate;
	};

public:
	MessageDeleter(DB::ComponentMessageDB *msgDB, MessageWriter *writer, QObject *parent = 0);
	virtual ~MessageDeleter();

	qint64 removeMsgById(bean::MessageType type, int msgid);
	qint64 removeMsgByDate(bean::MessageType type, const QString &id, const QString &beginDate, const QString &endDate);
	qint64 removeAttachById(bean::MessageType type, int attachId);

Q_SIGNALS:
	void removed(qint64 seq);

private slots:
	void next();

	void onWriterFinish();

private:
	MessageWriter          *m_writer;
	DB::ComponentMessageDB *m_msgDB;
	QList<DelParam>        m_delCache;
	QMutex                 m_mutex;
};

MessageDeleter::MessageDeleter(DB::ComponentMessageDB *msgDB, MessageWriter *writer, QObject *parent /*= 0*/)
: QObject(parent)
, m_msgDB(msgDB)
, m_writer(writer)
{
	Q_ASSERT(m_msgDB != 0);
	Q_ASSERT(m_writer != 0);

	connect(m_writer, SIGNAL(storeFinished()), this, SLOT(onWriterFinish()));
}

MessageDeleter::~MessageDeleter()
{
}

qint64 MessageDeleter::removeMsgById(bean::MessageType type, int msgid)
{
	DelParam param;
	param.seq = getMessageStoreSequence();
	param.msgtype = type;
	param.msgid = msgid;

	bool bStart = false;
	{
		QMutexLocker lock(&m_mutex);
		bStart = m_delCache.isEmpty();
		m_delCache.append(param);
	}

	if (bStart)
	{
		QTimer::singleShot(0, this, SLOT(next()));
	}

	return param.seq;
}

qint64 MessageDeleter::removeMsgByDate(bean::MessageType type, const QString &id, const QString &beginDate, const QString &endDate)
{
	DelParam param;
	param.seq = getMessageStoreSequence();
	param.type = Type_ByDate;
	param.msgtype = type;
	param.id = id;
	param.beginDate = beginDate;
	param.endDate = endDate;

	bool bStart = false;
	{
		QMutexLocker lock(&m_mutex);
		bStart = m_delCache.isEmpty();
		m_delCache.append(param);
	}

	if (bStart)
	{
		QTimer::singleShot(0, this, SLOT(next()));
	}

	return param.seq;
}

qint64 MessageDeleter::removeAttachById(bean::MessageType type, int attachId)
{
	DelParam param;
	param.seq = getMessageStoreSequence();
	param.type = Type_Attach;
	param.msgtype = type;
	param.msgid = attachId;

	bool bStart = false;
	{
		QMutexLocker lock(&m_mutex);
		bStart = m_delCache.isEmpty();
		m_delCache.append(param);
	}

	if (bStart)
	{
		QTimer::singleShot(0, this, SLOT(next()));
	}

	return param.seq;
}

void MessageDeleter::next()
{
	if (!m_writer->hasLeftToWrite())
	{
		QTimer::singleShot(0, this, SLOT(onWriterFinish()));
	}
}

void MessageDeleter::onWriterFinish()
{
	DelParam param;
	{
		QMutexLocker lock(&m_mutex);

		do 
		{
			if (m_delCache.isEmpty())
			{
				return;
			}

			param = m_delCache.takeFirst();
		} while (bean::Message_Invalid == param.msgtype);
	}

	switch (param.type)
	{
	case Type_ByMsgID:
		{
			m_msgDB->switchDB(param.msgtype);
			m_msgDB->removeMsgByMsgId(param.msgid);
		}
		break;
	case Type_ByDate:
		{
			m_msgDB->switchDB(param.msgtype);
			QList<int> lstRet = m_msgDB->getMessageIds(param.id, param.beginDate, param.endDate);
			foreach (int msgid, lstRet)
			{
				m_msgDB->removeMsgByMsgId(msgid);
			}
		}
		break;
	case Type_Attach:
		{
			DB::AttachmentDB *attachmentDB = m_msgDB->attachmentDB();
			attachmentDB->remove(param.msgid, param.msgtype);
		}
		break;
	default:
		break;
	}

	qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO << " removed 1: " << param.seq;

	emit removed(param.seq);

	QTimer::singleShot(0, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// class MessageReader
class MessageReader : public QObject
{
	Q_OBJECT

public:
	enum DateTimeType
	{
		BeginDateTime,
		EndDateTime,
		ContextDateTime,
		TimeStamp,
		AttachBegingDate,
		MsgId
	};

public:
	MessageReader(DB::ComponentMessageDB *msgDB, MessageWriter *writer, QObject *parent = 0);
	virtual ~MessageReader();

	qint64 appendSearch(bean::MessageType type, const QString &id, int page, int countPerPage, 
		const QString &beginDateTime, const QString &endDateTime, DateTimeType dateTimeType, const QString &keyword,
		int msgId = -1);

Q_SIGNALS:
	void gotMessages(qint64 seq, int page, int maxPage, const bean::MessageBodyList &msgs, int sum = 0);
	void gotContextMessages(qint64 seq, int page, int maxPage, const bean::MessageBodyList &msgs, int sum = 0);
	void gotMessageOfStamp(qint64 seq, const QString &stamp, const bean::MessageBody &msg);
	void gotMessageOfMsgId(qint64 seq, int msgId, const bean::MessageBody &msg);
	void gotAttachments(qint64 seq, int curPage, int maxPage, const QVariantList &attachs);

private slots:
	void next();

	void onWriterFinish();

private:
	struct ReaderParam
	{
		ReaderParam() : type(bean::Message_Invalid), page(0), countPerPage(60), msgId(-1) {}

		qint64            seq;
		bean::MessageType type;
		QString           id;
		int               page;
		int               countPerPage;
		QString           beginDateTime;
		QString           endDateTime;
		DateTimeType      dateTimeType;
		QString           keyword;
		int               msgId;
	};

private:
	MessageWriter          *m_writer;
	DB::ComponentMessageDB *m_msgDB;
	DB::MessageView        *m_msgView;
	QList<ReaderParam>      m_searchCache;
	QMutex                  m_mutex;
};

MessageReader::MessageReader(DB::ComponentMessageDB *msgDB, MessageWriter *writer, QObject *parent /*= 0*/) 
: QObject(parent)
, m_msgDB(msgDB)
, m_writer(writer)
{
	Q_ASSERT(m_writer != 0);
	Q_ASSERT(m_msgDB != 0);

	m_msgView = new DB::MessageView("MessageDBStore");

	connect(m_writer, SIGNAL(storeFinished()), this, SLOT(onWriterFinish()));
}

MessageReader::~MessageReader()
{
	delete m_msgView;
	m_msgView = 0;
}

qint64 MessageReader::appendSearch(bean::MessageType type, const QString &id, int page, int countPerPage, 
								   const QString &beginDateTime, const QString &endDateTime, DateTimeType dateTimeType, 
								   const QString &keyword, int msgId /*= -1*/)
{
	ReaderParam param;
	param.seq = getMessageStoreSequence();
	param.type = type;
	param.id = id;
	param.page = page;
	param.countPerPage = countPerPage;
	param.beginDateTime = beginDateTime;
	param.endDateTime = endDateTime;
	param.dateTimeType = dateTimeType;
	param.keyword = keyword;
	param.msgId = msgId;

	bool bStart = false;
	{
		QMutexLocker lock(&m_mutex);
		bStart = m_searchCache.isEmpty();
		m_searchCache.append(param);
	}

	if (bStart)
	{
		QTimer::singleShot(0, this, SLOT(next()));
	}

	return param.seq;
}

void MessageReader::next()
{
	if (!m_writer->hasLeftToWrite())
	{
		QTimer::singleShot(0, this, SLOT(onWriterFinish()));
	}
}

void MessageReader::onWriterFinish()
{
	ReaderParam param;
	{
		QMutexLocker lock(&m_mutex);

		do 
		{
			if (m_searchCache.isEmpty())
			{
				return;
			}

			param = m_searchCache.takeFirst();
		} while (0);
	}

	if (param.type == bean::Message_Invalid)
	{
		if (param.dateTimeType == BeginDateTime)
		{
			int count = m_msgView->getMessageCount(param.beginDateTime, param.endDateTime, param.keyword);

			int maxPage = count / param.countPerPage + (count % param.countPerPage ? 1 : 0);

			int page = param.page;
			if (page == 0 || page > maxPage)
			{
				page = maxPage;
			}

			qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
				<< "-------------- search begin time history: "
				<< " begin: " << param.beginDateTime
				<< " end: " << param.endDateTime
				<< " keyword: " << param.keyword
				<< " page: " << page
				<< " maxpage: " << maxPage
				<< " count: " << count;

			bean::MessageBodyList msgs;
			if (count > 0)
			{
				int pageIndex = page - 1;
				int offset = count - param.countPerPage*(maxPage-pageIndex);
				if (offset < 0)
					offset = 0;
				int limit = 0;
				if (pageIndex == 0)
					limit = count%param.countPerPage;
				if (limit == 0)
					limit = param.countPerPage;

				msgs = m_msgView->getMessages(offset, limit, param.beginDateTime, param.endDateTime, param.keyword);
			}

			emit gotMessages(param.seq, page, maxPage, msgs, count);
		}

		if (param.dateTimeType == AttachBegingDate)
		{
			DB::AttachmentDB *attachmentDB = m_msgDB->attachmentDB();
			int count = attachmentDB->getAttachmentCount(param.beginDateTime, param.keyword);

			int maxPage = count / param.countPerPage + (count % param.countPerPage ? 1 : 0);

			int page = param.page;
			if (page > maxPage)
			{
				page = maxPage;
			}

			qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
				<< "-------------- search attachments: "
				<< " begin: " << param.beginDateTime
				<< " keyword: " << param.keyword
				<< " page: " << page
				<< " maxpage: " << maxPage
				<< " count: " << count;

			QVariantList attachs;
			if (count > 0)
			{
				int pageIndex = page - 1;
				int offset = param.countPerPage*pageIndex;
				if (offset < 0)
					offset = 0;
				int limit = 0;
				if (page == maxPage)
					limit = count%param.countPerPage;
				if (limit == 0)
					limit = param.countPerPage;

				attachs = attachmentDB->getAttachments(offset, limit, param.beginDateTime, param.keyword);
			}

			emit gotAttachments(param.seq, page, maxPage, attachs);
		}
	}
	else
	{
		m_msgDB->switchDB((bean::MessageType)(param.type));

		if (param.dateTimeType == BeginDateTime)
		{
			int count = m_msgDB->getMessageCount(param.id, param.beginDateTime, param.endDateTime, param.keyword);

			int maxPage = count / param.countPerPage + (count % param.countPerPage ? 1 : 0);

			int page = param.page;
			if (page == 0 || page > maxPage)
			{
				page = maxPage;
			}

			qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
				<< "-------------- begin time history: "
				<< " id " << param.id
				<< " seq : " << param.seq 
				<< " begin: " << param.beginDateTime
				<< " end: " << param.endDateTime
				<< " keyword: " << param.keyword
				<< " page: " << page
				<< " maxpage: " << maxPage
				<< " count: " << count;

			bean::MessageBodyList msgs;
			if (count > 0)
			{
				int pageIndex = page - 1;
				int offset = count - param.countPerPage*(maxPage-pageIndex);
				if (offset < 0)
					offset = 0;
				int limit = 0;
				if (pageIndex == 0)
					limit = count%param.countPerPage;
				if (limit == 0)
					limit = param.countPerPage;

				msgs = m_msgDB->getMessages(param.id, offset, limit, param.beginDateTime, param.endDateTime, param.keyword);
			}

			emit gotMessages(param.seq, page, maxPage, msgs, count);
		}
		else if (param.dateTimeType == EndDateTime)
		{
			int count = m_msgDB->getMessageCountBeforeTime(param.id, param.endDateTime);

			int maxPage = count / param.countPerPage + (count % param.countPerPage ? 1 : 0);

			int page = param.page;
			if (page == 0 || page > maxPage)
			{
				page = maxPage;
			}

			qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
				<< "-------------- end time history: "
				<< " id " << param.id
				<< " seq : " << param.seq 
				<< " date: " << param.endDateTime
				<< " keyword: " << param.keyword
				<< " page: " << page
				<< " maxpage: " << maxPage
				<< " count: " << count;

			bean::MessageBodyList msgs;
			if (count > 0)
			{
				int pageIndex = page - 1;
				int offset = count - param.countPerPage*(maxPage-pageIndex);
				if (offset < 0)
					offset = 0;
				int limit = 0;
				if (pageIndex == 0)
					limit = count%param.countPerPage;
				if (limit == 0)
					limit = param.countPerPage;

				msgs = m_msgDB->getMessagesBeforeTime(param.id, offset, limit, param.endDateTime);
			}

			emit gotMessages(param.seq, page, maxPage, msgs, count);
		}
		else if (param.dateTimeType == ContextDateTime)
		{
			QList<int> msgIds = m_msgDB->getMessageIds(param.id, param.beginDateTime, param.endDateTime);
			int count = msgIds.count();
			int maxPage = count / param.countPerPage + (count % param.countPerPage ? 1 : 0);
			int msgIndex = msgIds.indexOf(param.msgId);
			if (msgIndex < 0)
			{
				qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO << "-------------- can't find context message: " << param.msgId << msgIds;
			}
			else
			{
				int page = maxPage - (count-msgIndex-1) / param.countPerPage;
				if (page == 0 || page > maxPage)
				{
					page = maxPage;
				}

				qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
					<< "-------------- context time history: "
					<< " id " << param.id
					<< " seq : " << param.seq 
					<< " date: " << param.endDateTime
					<< " keyword: " << param.keyword
					<< " page: " << page
					<< " maxpage: " << maxPage
					<< " count: " << count;

				bean::MessageBodyList msgs;
				if (count > 0)
				{
					int pageIndex = page - 1;
					int offset = count - param.countPerPage*(maxPage-pageIndex);
					if (offset < 0)
						offset = 0;
					int limit = 0;
					if (pageIndex == 0)
						limit = count%param.countPerPage;
					if (limit == 0)
						limit = param.countPerPage;

					msgs = m_msgDB->getMessages(param.id, offset, limit, param.beginDateTime, param.endDateTime);
				}

				emit gotContextMessages(param.seq, page, maxPage, msgs, count);
			}
		}
		else if (param.dateTimeType == TimeStamp)
		{
			// param.keyword stores the stamp
			bean::MessageBody msg = m_msgDB->getMessageByStamp(param.keyword);
			qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
				<< "-------------- message by stamp: " 
				<< param.type 
				<< param.id 
				<< param.keyword 
				<< "message ok: " << (msg.isValid());
			emit gotMessageOfStamp(param.seq, param.keyword, msg);
		}
		else if (param.dateTimeType == MsgId)
		{
			bean::MessageBody msg = m_msgDB->getMessageById(param.msgId);
			qDebug() << "---" << QThread::currentThread() << Q_FUNC_INFO 
				<< "-------------- message by msgId: " 
				<< param.type 
				<< param.msgId 
				<< "message ok: " << (msg.isValid());
			emit gotMessageOfMsgId(param.seq, param.msgId, msg);
		}
	}
	
	QTimer::singleShot(0, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS MessageDBStore
MessageDBStore::MessageDBStore(QObject *parent /*= 0*/)
: QObject(parent), m_sem(0)
{
	QList<bean::MessageType> msgTypes;
	msgTypes << bean::Message_Chat << bean::Message_GroupChat << bean::Message_DiscussChat;
	m_msgDB = new DB::ComponentMessageDB(msgTypes, "MessageDBStore");
	m_writer = new MessageWriter(m_sem, m_msgDB);
	m_reader = new MessageReader(m_msgDB, m_writer);
	m_deleter = new MessageDeleter(m_msgDB, m_writer);

	QObject::connect(m_reader, SIGNAL(gotMessages(qint64, int, int, bean::MessageBodyList, int)), this, SIGNAL(gotMessages(qint64, int, int, bean::MessageBodyList, int)));
	QObject::connect(m_reader, SIGNAL(gotContextMessages(qint64, int, int, bean::MessageBodyList, int)), this, SIGNAL(gotContextMessages(qint64, int, int, bean::MessageBodyList, int)));
	QObject::connect(m_reader, SIGNAL(gotMessageOfStamp(qint64, QString, bean::MessageBody)), this, SIGNAL(gotMessageOfStamp(qint64, QString, bean::MessageBody)));
	QObject::connect(m_reader, SIGNAL(gotMessageOfMsgId(qint64, int, bean::MessageBody)), this, SIGNAL(gotMessageOfMsgId(qint64, int, bean::MessageBody)));
	QObject::connect(m_reader, SIGNAL(gotAttachments(qint64, int, int, QVariantList)), this, SIGNAL(gotAttachments(qint64, int, int, QVariantList)));
	QObject::connect(m_deleter, SIGNAL(removed(qint64)), this, SIGNAL(removed(qint64)));
	QObject::connect(this, SIGNAL(finish()), m_writer, SLOT(finish()), Qt::QueuedConnection);

	m_writer->moveToThread(&m_thread);
	m_reader->moveToThread(&m_thread);
	m_deleter->moveToThread(&m_thread);
}

MessageDBStore::~MessageDBStore()
{
	release();
	delete m_writer;
	m_writer = 0;
	delete m_deleter;
	m_deleter = 0;
	delete m_reader;
	m_reader = 0;
	delete m_msgDB;
	m_msgDB = 0;
}

void MessageDBStore::append(const bean::MessageBody &body)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return;
	}

	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(bean::MessageBody, body));
}

void MessageDBStore::append(bean::MessageType msgType, const QString &rsUuid, int nResult)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return;
	}

	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(int, ((int)msgType)), Q_ARG(QString, rsUuid), Q_ARG(int, nResult));
}

void MessageDBStore::append(bean::MessageType msgType, const QString &rsUuid, const QString &filePath)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return;
	}

	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(int, ((int)msgType)), Q_ARG(QString, rsUuid), Q_ARG(QString, filePath));
}

void MessageDBStore::append(const QString &stamp, int readState)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return;
	}

	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(QString, stamp), Q_ARG(int, readState));
}

void MessageDBStore::append(int msgId, const bean::MessageBody &body)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return;
	}

	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(int, msgId), Q_ARG(bean::MessageBody, body));
}

qint64 MessageDBStore::getMessages(bean::MessageType type, const QString &id, int page /*= 0*/, int countPerPage /*= 60*/, 
								   const QString &beginDate /*= ""*/, const QString &endDate /*= ""*/, const QString &keyword /*= ""*/)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_reader->appendSearch(type, id, page, countPerPage, beginDate, endDate, MessageReader::BeginDateTime, keyword);
}

qint64 MessageDBStore::getMessagesBeforeTime(bean::MessageType type, const QString &id, int page, int countPerPage, const QString &endDate)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_reader->appendSearch(type, id, page, countPerPage, "", endDate, MessageReader::EndDateTime, "");
}

qint64  MessageDBStore::getContextMessages(bean::MessageType type, const QString &id, int msgId, int countPerPage /*= 60*/, 
										   const QString &beginDate /*= ""*/, const QString &endDate /*= ""*/)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_reader->appendSearch(type, id, 0, countPerPage, beginDate, endDate, MessageReader::ContextDateTime, "", msgId);
}

qint64 MessageDBStore::getMessageByStamp(bean::MessageType type, const QString &id, const QString &stamp)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	if (type == bean::Message_Invalid || id.isEmpty() || stamp.isEmpty())
		return -1;

	return m_reader->appendSearch(type, id, 0, 0, "", "", MessageReader::TimeStamp, stamp);
}

qint64 MessageDBStore::getMessageByMsgId(bean::MessageType type, int msgId)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	if (type == bean::Message_Invalid || msgId < 0)
		return -1;

	return m_reader->appendSearch(type, "", 0, 0, "", "", MessageReader::MsgId, "", msgId);
}

qint64 MessageDBStore::getAttachments(int page /* = 1 */, int countPerPage /* = 60 */, const QString &beginDate /* = "" */, const QString &keyword /* = "" */)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_reader->appendSearch(bean::Message_Invalid, "", page, countPerPage, beginDate, "", MessageReader::AttachBegingDate, keyword);
}

qint64 MessageDBStore::removeMsgByMsgId(bean::MessageType type, int msgid)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_deleter->removeMsgById(type, msgid);
}

qint64 MessageDBStore::removeMsgByDate(bean::MessageType type, const QString &id, const QString &beginDate, const QString &endDate)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_deleter->removeMsgByDate(type, id, beginDate, endDate);
}

qint64 MessageDBStore::removeAttachById(bean::MessageType type, int attachId)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_deleter->removeAttachById(type, attachId);
}

bool MessageDBStore::isInited() const
{
	return m_thread.isRunning();
}

void MessageDBStore::init()
{
	if (!isInited())
	{
		m_thread.start();
		qDebug() << "---" << Q_FUNC_INFO << "main thread: " << QThread::currentThread() << "message thread: " << &m_thread;
	}
}

void MessageDBStore::release()
{
	if (isInited())
	{
		emit finish();
		m_sem.acquire(1);
		m_thread.quit();
		m_thread.wait();
	}
}

#include "MessageDBStore.moc"
