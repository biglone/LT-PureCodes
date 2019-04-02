#include "globalnotificationmessagesdbstore.h"
#include "globalnotificationMessagesDB.h"
#include "globalnotificationmsg.h"
#include "common/datetime.h"
#include <QDebug>
#include <QBasicTimer>
#include <QMutex>
#include <QTimerEvent>
#include <QTimer>

static qint64 globalNotificationMessagesStoreSequence()
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

	WriteWrapper(const GlobalNotificationMsg &msg) : m_unread(0), m_seq(0)
	{
		m_storeType = StoreMessage;
		m_msg = msg;
	}

	WriteWrapper(const QString &globalNotificationId, int count) : m_seq(0)
	{
		m_storeType = StoreUnread;
		m_globalNotificationId = globalNotificationId;
		m_unread = count;
	}

public:
	qint64    m_seq;
	StoreType m_storeType;

	// for StoreMessage
	GlobalNotificationMsg m_msg;

	// for StoreUnread
	QString m_globalNotificationId;
	int     m_unread;
};

//////////////////////////////////////////////////////////////////////////
// class SubscriptionMessagesWriter
class GlobalNotificationMessagesWriter : public QObject
{
	Q_OBJECT

public:
	explicit GlobalNotificationMessagesWriter(QSemaphore &sem, DB::GlobalNotificationMessagesDB *msgDB, QObject *parent = 0);
	virtual ~GlobalNotificationMessagesWriter();

public:
	bool hasLeftToWrite() const;

Q_SIGNALS:
	void storeFinished();
	void messageWrited(qint64 seq, const QString &msgId, quint64 innerId);
	void unreadMsgCountWrited(qint64 seq, const QString &globalNotificationId, int count);

public slots:
	void append(qint64 seq, const GlobalNotificationMsg &msg);
	void append(qint64 seq, const QString &globalNotificationId, int count);
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
	DB::GlobalNotificationMessagesDB *m_msgDB;
	QBasicTimer                 m_timer;
};

GlobalNotificationMessagesWriter::GlobalNotificationMessagesWriter(QSemaphore &sem, 
	DB::GlobalNotificationMessagesDB *msgDB, QObject *parent /*= 0*/) 
	: QObject(parent), m_finishSem(sem), m_msgDB(msgDB)
{
	Q_ASSERT(m_msgDB != 0);
}

GlobalNotificationMessagesWriter::~GlobalNotificationMessagesWriter()
{
}

bool GlobalNotificationMessagesWriter::hasLeftToWrite() const
{
	return (m_cache.count() > 0);
}

void GlobalNotificationMessagesWriter::append(qint64 seq, const GlobalNotificationMsg &msg)
{
	WriteWrapper writeWrapper(msg);
	writeWrapper.m_seq = seq;
	m_cache.append(writeWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void GlobalNotificationMessagesWriter::append(qint64 seq, const QString &globalNotificationId, int count)
{
	WriteWrapper writeWrapper(globalNotificationId, count);
	writeWrapper.m_seq = seq;
	m_cache.append(writeWrapper);

	if (!m_timer.isActive())
	{
		m_timer.start(50, this);
	}
}

void GlobalNotificationMessagesWriter::finish()
{
	save();
	m_finishSem.release(1);
}

void GlobalNotificationMessagesWriter::timerEvent(QTimerEvent *e)
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

void GlobalNotificationMessagesWriter::next()
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
	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO << " take 1 globalNotification msg write, left: " << leftCount;
}

void GlobalNotificationMessagesWriter::save()
{
	foreach (WriteWrapper writeWrapper, m_cache)
	{
		store(writeWrapper);
	}
	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO  << "all globalNotification messages were written: " << m_cache.count();
	m_cache.clear();
}

void GlobalNotificationMessagesWriter::store(const WriteWrapper &writeWrapper)
{
	if (writeWrapper.m_storeType == WriteWrapper::StoreMessage)
	{
		quint64 innerId = m_msgDB->storeMessage(writeWrapper.m_msg);
		emit messageWrited(writeWrapper.m_seq, writeWrapper.m_msg.msgId(), innerId);
	}
	else if (writeWrapper.m_storeType == WriteWrapper::StoreUnread)
	{
		m_msgDB->setUnreadMsgCount(writeWrapper.m_globalNotificationId, writeWrapper.m_unread);
		emit unreadMsgCountWrited(writeWrapper.m_seq, writeWrapper.m_globalNotificationId, writeWrapper.m_unread);
	}
}

//////////////////////////////////////////////////////////////////////////
// class GlobalNotificationMessagesDeleter
class GlobalNotificationMessagesDeleter : public QObject
{
	Q_OBJECT

public:
	class DelParam
	{
	public:
		qint64  m_seq;
		QString m_globalNotificationId;
	};

public:
	GlobalNotificationMessagesDeleter(GlobalNotificationMessagesWriter *writer, DB::GlobalNotificationMessagesDB *msgDB, QObject *parent = 0);
	virtual ~GlobalNotificationMessagesDeleter();

	qint64 removeMessages(const QString &globalNotificationId);

Q_SIGNALS:
	void messagesRemoved(qint64 seq, const QString &globalNotificationId);

private slots:
	void next();
	void onWriterFinish();

private:
	GlobalNotificationMessagesWriter          *m_writer;
	DB::GlobalNotificationMessagesDB          *m_msgDB;
	QList<DelParam>                      m_delCache;
	QMutex                               m_mutex;
};

GlobalNotificationMessagesDeleter::GlobalNotificationMessagesDeleter(GlobalNotificationMessagesWriter *writer, 
	DB::GlobalNotificationMessagesDB *msgDB, QObject *parent /*= 0*/)
	: QObject(parent)
	, m_writer(writer)
	, m_msgDB(msgDB)
{
	Q_ASSERT(m_msgDB != 0);
	Q_ASSERT(m_writer != 0);

	connect(m_writer, SIGNAL(storeFinished()), this, SLOT(onWriterFinish()));
}

GlobalNotificationMessagesDeleter::~GlobalNotificationMessagesDeleter()
{
}

qint64 GlobalNotificationMessagesDeleter::removeMessages(const QString &globalNotificationId)
{
	DelParam param;
	param.m_seq = globalNotificationMessagesStoreSequence();
	param.m_globalNotificationId = globalNotificationId;

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

void GlobalNotificationMessagesDeleter::next()
{
	if (!m_writer->hasLeftToWrite())
	{
		QTimer::singleShot(0, this, SLOT(onWriterFinish()));
	}
}

void GlobalNotificationMessagesDeleter::onWriterFinish()
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

	m_msgDB->removeMessages(param.m_globalNotificationId);
	emit messagesRemoved(param.m_seq, param.m_globalNotificationId);

	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO << " remove messages: " << param.m_globalNotificationId;

	QTimer::singleShot(0, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// class GlobalNotificationMessagesReader
class GlobalNotificationMessagesReader : public QObject
{
	Q_OBJECT

public:
	GlobalNotificationMessagesReader(GlobalNotificationMessagesWriter *writer, DB::GlobalNotificationMessagesDB *msgDB, QObject *parent = 0);
	virtual ~GlobalNotificationMessagesReader();

	qint64 appendSearch(const QString &globalNotificationId, quint64 lastInnerId = 0, int count = 10);

Q_SIGNALS:
	void messagesGot(qint64 seq, const QList<GlobalNotificationMsg> &msgs, const QString &globalNotificationId, quint64 lastInnerId, int count);

private slots:
	void next();
	void onWriterFinish();

private:
	struct ReaderParam
	{
		ReaderParam() : lastInnerId(-1), count(0) {}

		qint64            seq;
		QString           globalNotificationId;
		quint64           lastInnerId;
		int               count;
	};

private:
	GlobalNotificationMessagesWriter          *m_writer;
	DB::GlobalNotificationMessagesDB          *m_msgDB;
	QList<ReaderParam>                   m_searchCache;
	QMutex                               m_mutex;
};

GlobalNotificationMessagesReader::GlobalNotificationMessagesReader(GlobalNotificationMessagesWriter *writer, 
	DB::GlobalNotificationMessagesDB *msgDB, QObject *parent /*= 0*/)
	: QObject(parent)
	, m_writer(writer)
	, m_msgDB(msgDB)
{
	Q_ASSERT(m_writer != 0);
	Q_ASSERT(m_msgDB != 0);

	connect(m_writer, SIGNAL(storeFinished()), this, SLOT(onWriterFinish()));
}

GlobalNotificationMessagesReader::~GlobalNotificationMessagesReader()
{
}

qint64 GlobalNotificationMessagesReader::appendSearch(const QString &globalNotificationId, 
	quint64 lastInnerId /*= 0*/, int count /*= 10*/)
{
	ReaderParam param;
	param.seq = globalNotificationMessagesStoreSequence();
	param.globalNotificationId = globalNotificationId;
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

void GlobalNotificationMessagesReader::next()
{
	if (!m_writer->hasLeftToWrite())
	{
		QTimer::singleShot(0, this, SLOT(onWriterFinish()));
	}
}

void GlobalNotificationMessagesReader::onWriterFinish()
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

	QList<GlobalNotificationMsg> msgs = m_msgDB->getMessages(param.globalNotificationId, param.lastInnerId, param.count);
	emit messagesGot(param.seq, msgs, param.globalNotificationId, param.lastInnerId, param.count);

	qDebug() << "|||" << QThread::currentThread() << Q_FUNC_INFO << " got globalNotification messages: "
		<< param.seq << param.globalNotificationId << msgs.count() << param.lastInnerId << param.count;

	QTimer::singleShot(0, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationMessagesDBStore
static void registerMetaType()
{
	static bool isInit = false;
	if (!isInit)
	{
		qRegisterMetaType<GlobalNotificationMsg>("GlobalNotificationMsg");
		qRegisterMetaType<QList<GlobalNotificationMsg>>("QList<GlobalNotificationMsg>");
		isInit = true;
	}
}

GlobalNotificationMessagesDBStore::GlobalNotificationMessagesDBStore(QObject *parent /*= 0*/)
	: QObject(parent), m_sem(0)
{
	registerMetaType();

	m_msgDB = new DB::GlobalNotificationMessagesDB("GlobalNotificationMessagesDBStore");
	m_writer = new GlobalNotificationMessagesWriter(m_sem, m_msgDB);
	m_reader = new GlobalNotificationMessagesReader(m_writer, m_msgDB);
	m_deleter = new GlobalNotificationMessagesDeleter(m_writer, m_msgDB);

	bool connectOK = false;
	connectOK = connect(m_writer, SIGNAL(messageWrited(qint64, QString, quint64)), 
		this, SIGNAL(messageWrited(qint64, QString, quint64)));
	Q_ASSERT(connectOK);
	connectOK = connect(m_deleter, SIGNAL(messagesRemoved(qint64, QString)), 
		this, SIGNAL(messagesRemoved(qint64, QString)));
	Q_ASSERT(connectOK);
	connectOK = connect(m_reader, SIGNAL(messagesGot(qint64, QList<GlobalNotificationMsg>, QString, quint64, int)), 
		this, SIGNAL(messagesGot(qint64, QList<GlobalNotificationMsg>, QString, quint64, int)));
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

GlobalNotificationMessagesDBStore::~GlobalNotificationMessagesDBStore()
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

bool GlobalNotificationMessagesDBStore::isInited() const
{
	return m_thread.isRunning();
}

void GlobalNotificationMessagesDBStore::init()
{
	if (!isInited())
	{
		m_thread.start();
		qDebug() << "|||" << Q_FUNC_INFO << "main thread: " << QThread::currentThread() << "message thread: " << &m_thread;
	}
}

void GlobalNotificationMessagesDBStore::release()
{
	if (isInited())
	{
		emit finish();
		m_sem.acquire(1);
		m_thread.quit();
		m_thread.wait();
	}
}

qint64 GlobalNotificationMessagesDBStore::appendMessage(const GlobalNotificationMsg &msg)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	qint64 seq = globalNotificationMessagesStoreSequence();
	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, Q_ARG(qint64, seq), Q_ARG(GlobalNotificationMsg, msg)); 
	return seq;
}

qint64 GlobalNotificationMessagesDBStore::removeMessages(const QString &globalNotificationId)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_deleter->removeMessages(globalNotificationId);
}

qint64 GlobalNotificationMessagesDBStore::getMessages(const QString &id, quint64 lastInnerId /*= 0*/, int count /*= 10*/)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	return m_reader->appendSearch(id, lastInnerId, count);
}

qint64 GlobalNotificationMessagesDBStore::appendUnreadMsgCount(const QString &globalNotificationId, int count)
{
	if (!isInited())
	{
		qWarning() << Q_FUNC_INFO << "not init.";
		return -1;
	}

	qint64 seq = globalNotificationMessagesStoreSequence();
	QMetaObject::invokeMethod(m_writer, "append", Qt::QueuedConnection, 
		Q_ARG(qint64, seq), Q_ARG(QString, globalNotificationId), Q_ARG(int, count));
	return seq;
}

#include "globalnotificationmessagesdbstore.moc"