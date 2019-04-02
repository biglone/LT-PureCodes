#include "subscriptionmessagesdbstore.h"
#include "SubscriptionMessagesDB.h"
#include "common/datetime.h"
#include <QDebug>
#include <QBasicTimer>
#include <QMutex>
#include <QTimerEvent>
#include <QTimer>

static qint64 subscriptionMessagesStoreSequence()
{
	static qint64 seed = 1;
	qint64 seq = CDateTime::currentMSecsSinceEpoch() + seed;
	++seed;
	return seq;
}

//////////////////////////////////////////////////////////////////////////
// class WriteWrapper
class WriteWrapper
{
public:
	enum StoreType
	{
		StoreMessage,
		StoreUnread
	};

	WriteWrapper() : m_unread(0), m_seq(0)
	{
		m_storeType = StoreMessage;
	}

	WriteWrapper(const SubscriptionMsg &msg) : m_unread(0), m_seq(0)
	{
		m_storeType = StoreMessage;
		m_msg = msg;
	}

	WriteWrapper(const QString &subscriptionId, int count) : m_seq(0)
	{
		m_storeType = StoreUnread;
		m_subscriptionId = subscriptionId;
		m_unread = count;
	}

public:
	qint64    m_seq;
	StoreType m_storeType;

	// for StoreMessage
	SubscriptionMsg m_msg;

	// for StoreUnread
	QString m_subscriptionId;
	int     m_unread;
};

//////////////////////////////////////////////////////////////////////////
// class SubscriptionMessagesWriter
class SubscriptionMessagesWriter : public QObject
{
	Q_OBJECT

public:
	explicit SubscriptionMessagesWriter(QSemaphore &sem, DB::SubscriptionMessagesDB *msgDB, QObject *parent = 0);
	virtual ~SubscriptionMessagesWriter();

public:
	bool hasLeftToWrite() const;

Q_SIGNALS:
	void storeFinished();
	void messageWrited(qint64 seq, const QString &msgId, quint64 innerId);
	void unreadMsgCountWrited(qint64 seq, const QString &subscriptionId, int count);

public slots:
	void append(qint64 seq, const SubscriptionMsg &msg);
	void append(qint64 seq, const QString &subscriptionId, int count);
	void finish();

protected:
	void timerEvent(QTimerEvent *e);

private slots:
	void next();

private:
	void save();
	void store(const WriteWrapper &writeWrapper);

private:
	QList<WriteWrapper>         m_cache;
	QSemaphore                 &m_finishSem;
	DB::SubscriptionMessagesDB *m_msgDB;
	QBasicTimer                 m_timer;
};

SubscriptionMessagesWriter::SubscriptionMessagesWriter(QSemaphore &sem, 
	DB::SubscriptionMessagesDB *msgDB, QObject *parent /*= 0*/) 
	: QObject(parent), m_finishSem(sem), m_msgDB(msgDB)
{
	Q_ASSERT(m_msgDB != 0);
}

SubscriptionMessagesWriter::~SubscriptionMessagesWriter()
{
}

bool SubscriptionMessagesWriter::hasLeftToWrite() const
{
	return (m_cache.count() > 0);
}

void SubscriptionMessagesWriter::append(qint64 seq, const SubscriptionMsg &msg)
{
	WriteWrapper writeWrapper(msg);
	writeWrapper.m_seq = seq;
	m_cache.append(writeWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void SubscriptionMessagesWriter::append(qint64 seq, const QString &subscriptionId, int count)
{
	WriteWrapper writeWrapper(subscriptionId, count);
	writeWrapper.m_seq = seq;
	m_cache.append(writeWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void SubscriptionMessagesWriter::finish()
{
	save();
	m_finishSem.release(1);
}

void SubscriptionMessagesWriter::timerEvent(QTimerEvent *e)
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

void SubscriptionMessagesWriter::next()
{	
	if (m_cache.isEmpty() && m_timer.isActive())
	{
		emit storeFinished();
		m_timer.stop();
		return;
	}
	
	WriteWrapper writeWrapper = m_cache.takeFirst();
	store(writeWrapper);

	int leftCount = m_cache.count();
	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO << " take 1 subscription msg write, left: " << leftCount;
}

void SubscriptionMessagesWriter::save()
{
	foreach (WriteWrapper writeWrapper, m_cache)
	{
		store(writeWrapper);
	}
	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO  << "all subscription messages were written: " << m_cache.count();
	m_cache.clear();
}

void SubscriptionMessagesWriter::store(const WriteWrapper &writeWrapper)
{
	if (writeWrapper.m_storeType == WriteWrapper::StoreMessage)
	{
		quint64 innerId = m_msgDB->storeMessage(writeWrapper.m_msg);
		emit messageWrited(writeWrapper.m_seq, writeWrapper.m_msg.msgId(), innerId);
	}
	else if (writeWrapper.m_storeType == WriteWrapper::StoreUnread)
	{
		m_msgDB->setUnreadMsgCount(writeWrapper.m_subscriptionId, writeWrapper.m_unread);
		emit unreadMsgCountWrited(writeWrapper.m_seq, writeWrapper.m_subscriptionId, writeWrapper.m_unread);
	}
}

//////////////////////////////////////////////////////////////////////////
// class SubscriptionMessagesDeleter
class SubscriptionMessagesDeleter : public QObject
{
	Q_OBJECT

public:
	class DelParam
	{
	public:
		qint64  m_seq;
		QString m_subscriptionId;
	};

public:
	SubscriptionMessagesDeleter(SubscriptionMessagesWriter *writer, DB::SubscriptionMessagesDB *msgDB, QObject *parent = 0);
	virtual ~SubscriptionMessagesDeleter();

	qint64 removeMessages(const QString &subscriptionId);

Q_SIGNALS:
	void messagesRemoved(qint64 seq, const QString &subscriptionId);

private slots:
	void next();
	void onWriterFinish();

private:
	SubscriptionMessagesWriter          *m_writer;
	DB::SubscriptionMessagesDB          *m_msgDB;
	QList<DelParam>                      m_delCache;
	QMutex                               m_mutex;
};

SubscriptionMessagesDeleter::SubscriptionMessagesDeleter(SubscriptionMessagesWriter *writer, 
	DB::SubscriptionMessagesDB *msgDB, QObject *parent /*= 0*/)
	: QObject(parent)
	, m_writer(writer)
	, m_msgDB(msgDB)
{
	Q_ASSERT(m_msgDB != 0);
	Q_ASSERT(m_writer != 0);

	connect(m_writer, SIGNAL(storeFinished()), this, SLOT(onWriterFinish()));
}

SubscriptionMessagesDeleter::~SubscriptionMessagesDeleter()
{
}

qint64 SubscriptionMessagesDeleter::removeMessages(const QString &subscriptionId)
{
	DelParam param;
	param.m_seq = subscriptionMessagesStoreSequence();
	param.m_subscriptionId = subscriptionId;

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

	return param.m_seq;
}

void SubscriptionMessagesDeleter::next()
{
	if (!m_writer->hasLeftToWrite())
	{
		QTimer::singleShot(0, this, SLOT(onWriterFinish()));
	}
}

void SubscriptionMessagesDeleter::onWriterFinish()
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
		} while (0);
	}

	m_msgDB->removeMessages(param.m_subscriptionId);
	emit messagesRemoved(param.m_seq, param.m_subscriptionId);

	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO << " remove messages: " << param.m_subscriptionId;

	QTimer::singleShot(0, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// class SubscriptionMessagesReader
class SubscriptionMessagesReader : public QObject
{
	Q_OBJECT

public:
	SubscriptionMessagesReader(SubscriptionMessagesWriter *writer, DB::SubscriptionMessagesDB *msgDB, QObject *parent = 0);
	virtual ~SubscriptionMessagesReader();

	qint64 appendSearch(const QString &subscriptionId, quint64 lastInnerId = 0, int count = 10);

Q_SIGNALS:
	void messagesGot(qint64 seq, const QList<SubscriptionMsg> &msgs, const QString &subscriptionId, quint64 lastInnerId, int count);

private slots:
	void next();
	void onWriterFinish();

private:
	struct ReaderParam
	{
		ReaderParam() : lastInnerId(-1), count(0) {}

		qint64            seq;
		QString           subscriptionId;
		quint64           lastInnerId;
		int               count;
	};

private:
	SubscriptionMessagesWriter          *m_writer;
	DB::SubscriptionMessagesDB          *m_msgDB;
	QList<ReaderParam>                   m_searchCache;
	QMutex                               m_mutex;
};

SubscriptionMessagesReader::SubscriptionMessagesReader(SubscriptionMessagesWriter *writer, 
	DB::SubscriptionMessagesDB *msgDB, QObject *parent /*= 0*/)
	: QObject(parent)
	, m_writer(writer)
	, m_msgDB(msgDB)
{
	Q_ASSERT(m_writer != 0);
	Q_ASSERT(m_msgDB != 0);

	connect(m_writer, SIGNAL(storeFinished()), this, SLOT(onWriterFinish()));
}

SubscriptionMessagesReader::~SubscriptionMessagesReader()
{
}

qint64 SubscriptionMessagesReader::appendSearch(const QString &subscriptionId, 
	quint64 lastInnerId /*= 0*/, int count /*= 10*/)
{
	ReaderParam param;
	param.seq = subscriptionMessagesStoreSequence();
	param.subscriptionId = subscriptionId;
	param.lastInnerId = lastInnerId;
	param.count = count;

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

void SubscriptionMessagesReader::next()
{
	if (!m_writer->hasLeftToWrite())
	{
		QTimer::singleShot(0, this, SLOT(onWriterFinish()));
	}
}

void SubscriptionMessagesReader::onWriterFinish()
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

	QList<SubscriptionMsg> msgs = m_msgDB->getMessages(param.subscriptionId, param.lastInnerId, param.count);
	emit messagesGot(param.seq, msgs, param.subscriptionId, param.lastInnerId, param.count);

	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO << " got subscription messages: "
		<< param.seq << param.subscriptionId << msgs.count() << param.lastInnerId << param.count;

	QTimer::singleShot(0, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SubscriptionMessagesDBStore
static void registerMetaType()
{
	static bool isInit = false;
	if (!isInit)
	{
		qRegisterMetaType<SubscriptionMsg>("SubscriptionMsg");
		qRegisterMetaType<QList<SubscriptionMsg>>("QList<SubscriptionMsg>");
		isInit = true;
	}
}

SubscriptionMessagesDBStore::SubscriptionMessagesDBStore(QObject *parent /*= 0*/)
	: QObject(parent), m_sem(0)
{
	registerMetaType();

	m_msgDB = new DB::SubscriptionMessagesDB("SubscriptionMessagesDBStore");
	m_writer = new SubscriptionMessagesWriter(m_sem, m_msgDB);
	m_reader = new SubscriptionMessagesReader(m_writer, m_msgDB);
	m_deleter = new SubscriptionMessagesDeleter(m_writer, m_msgDB);

	bool connectOK = false;
	connectOK = connect(m_writer, SIGNAL(messageWrited(qint64, QString, quint64)), 
		this, SIGNAL(messageWrited(qint64, QString, quint64)));
	Q_ASSERT(connectOK);
	connectOK = connect(m_deleter, SIGNAL(messagesRemoved(qint64, QString)), 
		this, SIGNAL(messagesRemoved(qint64, QString)));
	Q_ASSERT(connectOK);
	connectOK = connect(m_reader, SIGNAL(messagesGot(qint64, QList<SubscriptionMsg>, QString, quint64, int)), 
		this, SIGNAL(messagesGot(qint64, QList<SubscriptionMsg>, QString, quint64, int)));
	Q_ASSERT(connectOK);
	connectOK = connect(m_writer, SIGNAL(unreadMsgCountWrited(qint64, QString, int)), 
		this, SIGNAL(unreadMsgCountWrited(qint64, QString, int)));
	Q_ASSERT(connectOK);
	connectOK = connect(this, SIGNAL(finish()), m_writer, SLOT(finish()), Qt::QueuedConnection);
	Q_ASSERT(connectOK);

	m_writer->moveToThread(&m_thread);
	m_reader->moveToThread(&m_thread);
	m_deleter->moveToThread(&m_thread);
}

SubscriptionMessagesDBStore::~SubscriptionMessagesDBStore()
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

bool SubscriptionMessagesDBStore::isInited() const
{
	return m_thread.isRunning();
}

void SubscriptionMessagesDBStore::init()
{
	if (!isInited())
	{
		m_thread.start();
		qDebug() << "|||" << Q_FUNC_INFO << "main thread: " << QThread::currentThread() << "message thread: " << &m_thread;
	}
}

void SubscriptionMessagesDBStore::release()
{
	if (isInited())
	{
		emit finish();
		m_sem.acquire(1);
		m_thread.quit();
		m_thread.wait();
	}
}

qint64 SubscriptionMessagesDBStore::appendMessage(const SubscriptionMsg &msg)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	qint64 seq = subscriptionMessagesStoreSequence();
	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(qint64, seq), Q_ARG(SubscriptionMsg, msg)); 
	return seq;
}

qint64 SubscriptionMessagesDBStore::removeMessages(const QString &subscriptionId)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_deleter->removeMessages(subscriptionId);
}

qint64 SubscriptionMessagesDBStore::getMessages(const QString &id, quint64 lastInnerId /*= 0*/, int count /*= 10*/)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_reader->appendSearch(id, lastInnerId, count);
}

qint64 SubscriptionMessagesDBStore::appendUnreadMsgCount(const QString &subscriptionId, int count)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	qint64 seq = subscriptionMessagesStoreSequence();
	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, 
		Q_ARG(qint64, seq), Q_ARG(QString, subscriptionId), Q_ARG(int, count));
	return seq;
}

#include "subscriptionmessagesdbstore.moc"