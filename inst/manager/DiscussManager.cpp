#include <QDebug>
#include <QMutex>

#include "pmclient/PmClient.h"
#include "protocol/ProtocolConst.h"
#include "protocol/ProtocolType.h"
#include "protocol/DiscussRequest.h"
#include "protocol/DiscussResponse.h"
#include "protocol/DiscussNotification.h"
#include "pmclient/PmClientInterface.h"

#include "PmApp.h"
#include "ModelManager.h"
#include "Account.h"
#include "changenoticemgr.h"
#include "http/HttpPool.h"
#include "util/JsonParser.h"
#include "settings/GlobalSettings.h"
#include "groupsmembermanager.h"

#include "DiscussManager.h"

static const int kDiscussRequestTimeout = 60; // 60s 

//////////////////////////////////////////////////////////////////////////
// class DiscussWapper
class DiscussWapper
{
public:
	DiscussWapper(DiscussManager::Type type = DiscussManager::Invalid)
		: m_type(type)
	{
		m_handleId = createHandleId();
	}

	DiscussWapper& operator=(const DiscussWapper &other)
	{
		m_type        = other.m_type;
		m_handleId    = other.m_handleId;
		m_seq         = other.m_seq;
		m_disucssId   = other.m_disucssId;
		m_discussName = other.m_discussName;
		m_memberId     = other.m_memberId;
		m_memberCardName = other.m_memberCardName;
		m_members     = other.m_members;

		return (*this);
	}

	void setId(const QString &id)
	{
		m_disucssId = id;
	}

	void setMemberId(const QString &usrId)
	{
		m_memberId = usrId;
	}

	void setName(const QString &name)
	{
		m_discussName = name;
	}

	void setMemberCardName(const QString &cardName)
	{
		m_memberCardName = cardName;
	}

	void setSeq(const QString &seq)
	{
		m_seq = seq;
	}

	void setMembers(const QStringList &ids)
	{
		m_members = ids;
	}

	DiscussManager::Type type() const
	{
		return m_type;
	}

	int handleId() const
	{
		return m_handleId;
	}

	QString id() const
	{
		return m_disucssId;
	}

	QString memberId() const
	{
		return m_memberId;
	}

	QString name() const 
	{
		return m_discussName;
	}

	QString memberCardName() const
	{
		return m_memberCardName;
	}

	QString seq() const
	{
		return m_seq;
	}

	QStringList members() const
	{
		return m_members;
	}

private:
	static int createHandleId()
	{
		static QMutex s_mutex;
		static int i = 0;

		s_mutex.lock();
		++i;
		s_mutex.unlock();
		return i;
	}

	int                  m_handleId;
	DiscussManager::Type m_type;
	QString              m_seq;
	QString              m_disucssId;
	QString              m_discussName;
	QString              m_memberId;
	QString              m_memberCardName;
	QStringList          m_members;
};

//////////////////////////////////////////////////////////////////////////
// class DiscussResHandle
class DiscussResHandle : public QObject, public IPmClientResponseHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientResponseHandler);

public:
	DiscussResHandle(DiscussManager *pMgr);

public slots:
	void append(const DiscussWapper &wapper);

public:
	void syncDiscuss(const QString &id = "");

public:
	// IPmClientResponseHandler ---------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onRequestResult(int handleId, net::Request* req, protocol::Response* res);

private slots:
	void next();
	void setError(int type, const QString &idorName, const QString &seq, const QString &err);
	void processOtherOperator(int type, const QString &seq, const QString &id);

private:
	void processDiscuss(net::Request* req, protocol::Response* res);
	bool processResponseError(net::Request* req);

private:
	int m_handleId;
	DiscussManager *m_pMgr;

	QMap<int, DiscussWapper> m_wappers;
	QList<int>               m_queue;
	QMap<QString, int>       m_results;
};

DiscussResHandle::DiscussResHandle( DiscussManager *pMgr )
: m_handleId(-1)
, m_pMgr(pMgr)
{
	Q_ASSERT(m_pMgr != NULL);
}


void DiscussResHandle::append( const DiscussWapper &wapper )
{
	m_wappers.insert(wapper.handleId(), wapper);
	m_queue.append(wapper.handleId());
	QTimer::singleShot(10, this, SLOT(next()));
}

void DiscussResHandle::syncDiscuss( const QString &id /*= ""*/ )
{
	protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_Sync, id.toUtf8().constData());
	pReq->setTimeout(kDiscussRequestTimeout);
	PmClient::instance()->send(pReq);
}

bool DiscussResHandle::initObject()
{
	m_handleId = PmClient::instance()->insertResponseHandler(this);
	if (m_handleId < 0)
	{
		qDebug() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void DiscussResHandle::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_handleId);
	m_handleId = -1;
}

QObject* DiscussResHandle::instance()
{
	return this;
}

int DiscussResHandle::handledId() const
{
	return m_handleId;
}

QList<int> DiscussResHandle::types() const
{
	QList<int> ret;
	ret << protocol::Request_Discuss_Discuss;
	return ret;
}

bool DiscussResHandle::onRequestResult( int handleId, net::Request* req, protocol::Response* res )
{
	if (m_handleId != handleId)
	{
		return false;
	}

	int type = req->getType();

	// process
	switch (type)
	{
	case protocol::Request_Discuss_Discuss:
		processDiscuss(req, res);
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error";
	}

	return true;
}

void DiscussResHandle::processDiscuss( net::Request* req, protocol::Response* res )
{
	if (processResponseError(req)) // error
	{
		return;
	}

	// process new os
	protocol::DiscussResponse *pRes = static_cast<protocol::DiscussResponse *>(res);
	Q_ASSERT(pRes != NULL);

	int type = pRes->getActionType();
	QString discussId = QString::fromUtf8(pRes->getDiscussId().c_str());
	if (type == protocol::DiscussRequest::Action_Sync)
	{
		std::vector<protocol::DiscussRequest::DiscussItem> discussItem = pRes->getDiscussItems();
		if (discussId.isEmpty())
		{
			QStringList ids;
			QStringList names;
			QStringList creators;
			QStringList times;
			QStringList versions;
			int len = discussItem.size();
			for (int i = 0; i < len; ++i)
			{
				ids << QString::fromUtf8(discussItem[i].id.c_str());
				names << QString::fromUtf8(discussItem[i].name.c_str());
				creators << QString::fromUtf8(discussItem[i].creator.c_str());
				times << QString::fromUtf8(discussItem[i].time.c_str());
				versions << QString::fromUtf8(discussItem[i].version.c_str());
			}
			QMetaObject::invokeMethod(m_pMgr, "processDiscussList", Qt::QueuedConnection, 
				Q_ARG(QStringList, ids), Q_ARG(QStringList, names), Q_ARG(QStringList, creators), 
				Q_ARG(QStringList, times), Q_ARG(QStringList, versions));
		}
		else
		{
			QString discussName = QString::fromUtf8(pRes->getDiscussName().c_str());
			QString discussTime = QString::fromUtf8(pRes->getDiscussTime().c_str());
			QString discussCreator = QString::fromUtf8(pRes->getDiscussCreator().c_str());
			QString discussVersion = QString::fromUtf8(pRes->getDiscussVersion().c_str());
			QStringList ids;
			QStringList names;
			QStringList addedIds;
			QStringList cardNames;
			int len = discussItem.size();
			for (int i = 0; i < len; ++i)
			{
				ids << QString::fromUtf8(discussItem[i].id.c_str());
				names << QString::fromUtf8(discussItem[i].name.c_str());
				addedIds << QString::fromUtf8(discussItem[i].creator.c_str());
				cardNames << QString::fromUtf8(discussItem[i].cardName.c_str());
			}

			QMetaObject::invokeMethod(m_pMgr, "processDiscussMembers", Qt::QueuedConnection, 
				Q_ARG(QString, discussId), Q_ARG(QString, discussName), Q_ARG(QString, discussCreator), 
				Q_ARG(QString, discussTime), Q_ARG(QString, discussVersion),
				Q_ARG(QStringList, ids), Q_ARG(QStringList, names), 
				Q_ARG(QStringList, addedIds), Q_ARG(QStringList, cardNames));
		}
	}
	else
	{
		QMetaObject::invokeMethod(this, "processOtherOperator", Qt::QueuedConnection, Q_ARG(int, type), 
			Q_ARG(QString, QString::fromUtf8(req->getSeq().c_str())), Q_ARG(QString, discussId));
	}
}

bool DiscussResHandle::processResponseError( net::Request* req )
{
	bool bError = !req->getResult();

	if (bError)
	{
		int type = protocol::DiscussRequest::Action_None;
		QString idorName;
		protocol::DiscussRequest* pDisReq = static_cast<protocol::DiscussRequest*>(req);
		if (pDisReq)
		{
			type = pDisReq->actionType();
			idorName = QString::fromUtf8(pDisReq->getIdorName().c_str());
		}

		QMetaObject::invokeMethod(this, "setError", Qt::QueuedConnection, Q_ARG(int, type), 
			Q_ARG(QString, idorName),
			Q_ARG(QString, QString::fromUtf8(req->getSeq().c_str())), 
			Q_ARG(QString, QString::fromUtf8(req->getMessage().c_str())));
	}

	return bError;
}

void DiscussResHandle::next()
{
	if (m_queue.isEmpty())
	{
		return;
	}

	int handleId = 0;
	do 
	{
		handleId = m_queue.takeFirst();
		if (m_wappers.contains(handleId))
		{
			if (m_wappers[handleId].type() != DiscussManager::Invalid)
			{
				break;
			}
		}
	} while (!m_queue.isEmpty());

	DiscussWapper &wapper = m_wappers[handleId];
	
	switch (wapper.type())
	{
	case DiscussManager::CreateAndAdd:
		if (wapper.id().isEmpty())
		{
			if (wapper.seq().isEmpty()) 
			{
				protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_Create, wapper.name().toUtf8().constData());
				pReq->setTimeout(kDiscussRequestTimeout);
				wapper.setSeq(QString::fromUtf8(pReq->getSeq().c_str()));
				PmClient::instance()->send(pReq);
			}
			m_results.insert(wapper.seq(), wapper.handleId());
		}
		else
		{
			protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_Add, wapper.id().toUtf8().constData());
			pReq->setTimeout(kDiscussRequestTimeout);
			foreach (QString uid, wapper.members())
			{
				pReq->addDiscussItem(uid.toUtf8().constData());
			}
			wapper.setSeq(QString::fromUtf8(pReq->getSeq().c_str()));
			PmClient::instance()->send(pReq);

			m_results.insert(wapper.seq(), wapper.handleId());
		}
		break;
	case DiscussManager::Add:
		{
			if (wapper.seq().isEmpty())
			{
				protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_Add, wapper.id().toUtf8().constData());
				pReq->setTimeout(kDiscussRequestTimeout);
				foreach (QString uid, wapper.members())
				{
					pReq->addDiscussItem(uid.toUtf8().constData());
				}
				wapper.setSeq(QString::fromUtf8(pReq->getSeq().c_str()));
				PmClient::instance()->send(pReq);

				m_results.insert(wapper.seq(), wapper.handleId());
			}
		}
		break;
	case DiscussManager::Quit:
		{
			protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_Quit, wapper.id().toUtf8().constData());
			pReq->setTimeout(kDiscussRequestTimeout);
			foreach (QString uid, wapper.members())
			{
				pReq->addDiscussItem(uid.toUtf8().constData());
			}
			wapper.setSeq(QString::fromUtf8(pReq->getSeq().c_str()));
			PmClient::instance()->send(pReq);

			m_results.insert(wapper.seq(), wapper.handleId());
		}
		break;
	case DiscussManager::ChangeName:
		{
			protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_ChangeName, wapper.id().toUtf8().constData());
			pReq->setTimeout(kDiscussRequestTimeout);
			pReq->addDiscussItem(wapper.id().toUtf8().constData(), wapper.name().toUtf8().constData());
			wapper.setSeq(QString::fromUtf8(pReq->getSeq().c_str()));
			PmClient::instance()->send(pReq);

			m_results.insert(wapper.seq(), wapper.handleId());
		}
		break;
	case DiscussManager::ChangeCardName:
		{
			protocol::DiscussRequest *pReq = new protocol::DiscussRequest(protocol::DiscussRequest::Action_ChangeCardName, wapper.id().toUtf8().constData());
			pReq->setTimeout(kDiscussRequestTimeout);
			pReq->addDiscussItem(wapper.memberId().toUtf8().constData(), wapper.memberCardName().toUtf8().constData());
			wapper.setSeq(QString::fromUtf8(pReq->getSeq().c_str()));
			PmClient::instance()->send(pReq);

			m_results.insert(wapper.seq(), wapper.handleId());
		}
	default:
		break;
	}
}

void DiscussResHandle::setError( int type, const QString &idorName, const QString &seq, const QString &err )
{
	int handledId = -1;
	QString discussId;
	QString discussName;
	QStringList members;
	QString op;
	if (type == protocol::DiscussRequest::Action_Sync)
	{
		op = tr("Sync discuss");
		QString errmsg = QString("%1%2(%3)").arg(op).arg(tr(" failed")).arg(err);
		qWarning() << Q_FUNC_INFO << errmsg << idorName;

		if (idorName.isEmpty())
		{
			QMetaObject::invokeMethod(m_pMgr, "error", Q_ARG(QString, errmsg));
		}
		else
		{
			QMetaObject::invokeMethod(m_pMgr, "onDiscussMembersFailed", Q_ARG(QString, idorName), Q_ARG(QString, errmsg));
		}
	}
	else
	{
		if (m_results.contains(seq))
		{
			handledId = m_results.take(seq);
			if (m_wappers.contains(handledId))
			{
				DiscussWapper wapper = m_wappers.take(handledId);
				discussId = wapper.id();
				discussName = wapper.name();
				members = wapper.members();
				switch (wapper.type())
				{
				case DiscussManager::CreateAndAdd:
					op = tr("Create discuss");
					break;
				case DiscussManager::Add:
					op = tr("Add member");
					break;
				case DiscussManager::Quit:
					op = tr("Quit discuss");
					break;
				case DiscussManager::ChangeName:
					op = tr("Change discuss name");
					break;
				case DiscussManager::ChangeCardName:
					op = tr("Change card name in discuss \"%1\"").arg(discussName);
				}
			}
		}
		QString errmsg = QString("%1%2(%3)").arg(op).arg(tr(" failed")).arg(err);
		qWarning() << Q_FUNC_INFO << errmsg << discussId << discussName << members;

		QMetaObject::invokeMethod(m_pMgr, "discussError", Q_ARG(int, handledId), Q_ARG(int, (int)type), Q_ARG(QString, errmsg),
			Q_ARG(QString, discussId), Q_ARG(QString, discussName), Q_ARG(QStringList, members));
	}
}

void DiscussResHandle::processOtherOperator( int type, const QString &seq, const QString &id )
{
	if (!m_results.contains(seq))
	{
		qDebug() << Q_FUNC_INFO << seq << " m_results does not find.";
		return;
	}

	int handleId = m_results.take(seq);
	if (!m_wappers.contains(handleId))
	{
		qDebug() << Q_FUNC_INFO << handleId << " m_wappers does not find.";
		return;
	}

	DiscussWapper wapper = m_wappers.take(handleId);

	switch (wapper.type())
	{
	case DiscussManager::CreateAndAdd:
		{
			if (type == protocol::DiscussRequest::Action_Create)
			{
				wapper.setId(id);
				m_wappers.insert(wapper.handleId(), wapper);
				m_queue.prepend(wapper.handleId());
			}
			else if (type == protocol::DiscussRequest::Action_Add)
			{
				QMetaObject::invokeMethod(m_pMgr, "createdDiscuss", Q_ARG(int, handleId), Q_ARG(QString, wapper.id()));
			}
			else
			{
				qDebug() << Q_FUNC_INFO << "type is failed.";
			}
		}
		break;
	case DiscussManager::Add:
		{
			QMetaObject::invokeMethod(m_pMgr, "addedMembers", Q_ARG(int, handleId), Q_ARG(QString, wapper.id()));
		}
		break;
	case DiscussManager::Quit:
		{
			QMetaObject::invokeMethod(m_pMgr, "quitedDiscuss", Q_ARG(int, handleId), Q_ARG(QString, wapper.id()));
		}
		break;
	case DiscussManager::ChangeName:
		{
			QMetaObject::invokeMethod(m_pMgr, "nameChanged", Q_ARG(int, handleId), Q_ARG(QString, wapper.id()), Q_ARG(QString, wapper.name()));
		}
		break;
	case DiscussManager::ChangeCardName:
		{
			QMetaObject::invokeMethod(m_pMgr, "cardNameChanged", Q_ARG(int, handleId), Q_ARG(QString, wapper.id()), Q_ARG(QString, wapper.memberCardName()));
		}
	}

	QTimer::singleShot(10, this, SLOT(next()));
}

//////////////////////////////////////////////////////////////////////////
// class DiscussNtfHandle
class DiscussNtfHandle : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler);

public:
	DiscussNtfHandle(DiscussManager *pMgr);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);
	
private:
	int m_handleId;
	DiscussManager *m_pMgr;
};

DiscussNtfHandle::DiscussNtfHandle( DiscussManager *pMgr )
: m_pMgr(pMgr)
, m_handleId(-1)
{
	Q_ASSERT(m_pMgr != NULL);
}

bool DiscussNtfHandle::initObject()
{
	m_handleId = PmClient::instance()->insertNotificationHandler(this);
	if (m_handleId < 0)
	{
		qDebug() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void DiscussNtfHandle::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_handleId);
	m_handleId = -1;
}

QObject* DiscussNtfHandle::instance()
{
	return this;
}

QList<int> DiscussNtfHandle::types() const
{
	return QList<int>() << protocol::DISCUSS;
}

int DiscussNtfHandle::handledId() const
{
	return m_handleId;
}

bool DiscussNtfHandle::onNotication( int handleId, protocol::SpecificNotification* sn )
{
	if (m_handleId != handleId)
		return false;

	protocol::DiscussNotification *pIn = static_cast<protocol::DiscussNotification *>(sn);

	if (pIn)
	{
		bool isQuitNotify = (QString::compare(QString::fromUtf8(pIn->type.c_str()),
			                                  QString::fromUtf8("quit"),
											  Qt::CaseSensitive) == 0);
		if (isQuitNotify)
		{
			QString id = QString::fromUtf8(pIn->id.c_str());
			QMetaObject::invokeMethod(m_pMgr, "quitedDiscuss", Q_ARG(QString, id));
		}
		else
		{
			// sync members
			QStringList members;
			QStringList memberNames;
			QStringList addedIds;
			QStringList cardNames;
			int len = pIn->members.size();
			for (int i = 0; i < len; ++i)
			{
				members << QString::fromUtf8(pIn->members[i].c_str());
				memberNames << QString::fromUtf8(pIn->memberNames[i].c_str());
				addedIds << QString::fromUtf8(pIn->addedIds[i].c_str());
				cardNames << QString::fromUtf8(pIn->cardNames[i].c_str());
			}

			QString id = QString::fromUtf8(pIn->id.c_str());
			QString name = QString::fromUtf8(pIn->name.c_str());
			QString creator = QString::fromUtf8(pIn->creator.c_str());
			QString time = QString::fromUtf8(pIn->time.c_str());
			QString version = QString::fromUtf8(pIn->version.c_str());

			QMetaObject::invokeMethod(m_pMgr, "processDiscussMembers", Qt::QueuedConnection, Q_ARG(QString, id), 
				Q_ARG(QString, name), Q_ARG(QString, creator), 
				Q_ARG(QString, time), Q_ARG(QString, version),
				Q_ARG(QStringList, members), Q_ARG(QStringList, memberNames), 
				Q_ARG(QStringList, addedIds), Q_ARG(QStringList, cardNames));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
DiscussManager::DiscussManager( QObject *parent /*= 0*/ )
: QObject(parent)
{
	m_pResHandle.reset(new DiscussResHandle(this));
	m_pNtfHandle.reset(new DiscussNtfHandle(this));

	m_pHttpPool = qPmApp->getHttpPool();
	connect(m_pHttpPool, SIGNAL(requestFinished(int, bool, int, QByteArray)), 
		this, SLOT(onHttpRequestFinished(int, bool, int, QByteArray)));
}

DiscussManager::~DiscussManager()
{

}

QObject* DiscussManager::instance()
{
	return this;
}

QString DiscussManager::name() const
{
	return "DiscussManager";
}

bool DiscussManager::start()
{
	syncDiscuss();

	return true;
}

void DiscussManager::processDiscussList(const QStringList &ids, const QStringList &names, const QStringList &creators, 
										const QStringList &times, const QStringList &versions)
{
	QMap<QString, QString> discussVersions;
	for (int k = 0; k < ids.count(); ++k)
	{
		discussVersions.insert(ids[k], versions[k]);
	}

	qPmApp->getGroupsMemberManager()->setDiscussNewVersions(discussVersions);
	
	qPmApp->getModelManager()->setDiscusses(ids, names, creators, times);

	emit discussOK();
	emit finished();

	// sync next discuss members
	m_syncDiscussIds = ids;
	syncNextDiscussMembers();
}

void DiscussManager::processDiscussMembers(const QString &id, 
										   const QString &name, 
										   const QString &creator, 
										   const QString &time, 
										   const QString &version,
										   const QStringList &members,
										   const QStringList &memberNames,
										   const QStringList &addedIds /*= QStringList()*/,
										   const QStringList &cardNames /*= QStringList()*/)
{
	QString selfId = Account::instance()->id();
	if (!members.contains(selfId)) // you're deleted from this discuss
	{
		ChangeNoticeMgr *changeNoticeMgr = qPmApp->getChangeNoticeMgr();
		changeNoticeMgr->postDiscussChangeNotice(id, "kick");
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();

	bool added = false;
	if (!modelManager->hasDiscussItem(id))
	{
		modelManager->addDiscuss(id, name, creator, time);
		added = true;
	}
	else
	{
		modelManager->modifyDiscuss(id, name, creator, time);
		added = false;
	}

	modelManager->setDiscussMembers(id, version, members, memberNames, addedIds, added, cardNames);

	emit notifyDiscussChanged(id);

	// sync next discuss members
	m_syncDiscussIds.removeAll(id);
	syncNextDiscussMembers();
}

void DiscussManager::processDiscussMembers(const QString &id, 
	                                       const QString &version,
										   const QStringList &members, 
										   const QStringList &memberNames, 
										   const QStringList &addedIds,
										   const QStringList &cardNames)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasDiscussItem(id))
		return;

	modelManager->setDiscussMembers(id, version, members, memberNames, addedIds, false, cardNames);

	// sync next discuss members
	m_syncDiscussIds.removeAll(id);
	syncNextDiscussMembers();
}

void DiscussManager::onDiscussMembersFailed(const QString &id, const QString &desc)
{
	Q_UNUSED(desc);

	// sync next discuss members
	m_syncDiscussIds.removeAll(id);
	syncNextDiscussMembers();
}

void DiscussManager::onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData)
{
	if (m_kickIds.contains(requestId))
	{
		QPair<QString, QString> pair = m_kickIds.value(requestId);
		QString discussId = pair.first;
		QString uid = pair.second;

		m_kickIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "kick failed: " << httpCode;
			emit discussKickFailed(discussId, uid, tr("Network error, code:%1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "kick failed: " << errMsg;
			emit discussKickFailed(discussId, uid, errMsg);
			return;
		}

		emit discussKickOK(discussId, uid);
		return;
	}

	if (m_disbandIds.contains(requestId))
	{
		QString discussId = m_disbandIds.value(requestId);
		m_disbandIds.remove(requestId);

		if (error)
		{
			qWarning() << Q_FUNC_INFO << "disband failed: " << httpCode;
			emit discussDisbandFailed(discussId, tr("Network error, code:%1").arg(httpCode));
			return;
		}

		bool err = true;
		QString errMsg;
		JsonParser::parse(recvData, err, errMsg);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << "disband failed: " << errMsg;
			emit discussDisbandFailed(discussId, errMsg);
			return;
		}

		emit discussDisbandOK(discussId);
		return;
	}
}

bool DiscussManager::initObject()
{
	return m_pResHandle->initObject() && m_pNtfHandle->initObject();
}

void DiscussManager::removeObject()
{
	m_pResHandle->removeObject();
	m_pNtfHandle->removeObject();
}

void DiscussManager::syncDiscuss(const QString &discussId /*= QString()*/)
{
	m_pResHandle->syncDiscuss(discussId);
}

int DiscussManager::createDiscuss( const QString &name, const QStringList &uids )
{
	DiscussWapper wapper(DiscussManager::CreateAndAdd);
	wapper.setName(name);
	wapper.setMembers(uids);
	m_pResHandle->append(wapper);
	return wapper.handleId();
}

int DiscussManager::addMembers( const QString &id, const QString &name, const QStringList &uids )
{
	DiscussWapper wapper(DiscussManager::Add);
	wapper.setId(id);
	wapper.setName(name);
	wapper.setMembers(uids);
	m_pResHandle->append(wapper);
	return wapper.handleId();
}

int DiscussManager::quitDiscuss( const QString &id, const QString &name, const QString &uid )
{
	DiscussWapper wapper(DiscussManager::Quit);
	wapper.setId(id);
	wapper.setName(name);
	wapper.setMembers(QStringList() << uid);
	m_pResHandle->append(wapper);
	return wapper.handleId();
}

int DiscussManager::changeName(const QString &id, const QString &name)
{
	DiscussWapper wapper(DiscussManager::ChangeName);
	wapper.setId(id);
	wapper.setName(name);
	m_pResHandle->append(wapper);
	return wapper.handleId();
}

bool DiscussManager::kick(const QString &discussId, const QString &uid, const QString &by)
{
	if (discussId.isEmpty() || uid.isEmpty() || by.isEmpty())
		return false;

	QPair<QString, QString> pair(discussId, uid);
	if (m_kickIds.values().contains(pair))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("discussid", discussId);
	params.insert("userid", uid);
	params.insert("by", by);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/discuss/member/remove").arg(loginConfig.managerUrl);
	int requestId = m_pHttpPool->addRequest(HttpRequest::PostRequet, QUrl::fromUserInput(urlString), params);
	m_kickIds.insert(requestId, pair);
	return true;
}

bool DiscussManager::disband(const QString &discussId, const QString &by)
{
	if (discussId.isEmpty() || by.isEmpty())
		return false;

	if (m_disbandIds.values().contains(discussId))
		return false;

	QMultiMap<QString, QString> params;
	params.insert("discussid", discussId);
	params.insert("by", by);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/discuss/disband").arg(loginConfig.managerUrl);
	int requestId = m_pHttpPool->addRequest(HttpRequest::GetRequest, QUrl::fromUserInput(urlString), params);
	m_disbandIds.insert(requestId, discussId);
	return true;
}

int DiscussManager::changeCardName(const QString &discussId, const QString &discussName, 
	                               const QString &uid, const QString &cardName)
{
	DiscussWapper wapper(DiscussManager::ChangeCardName);
	wapper.setId(discussId);
	wapper.setName(discussName);
	wapper.setMemberId(uid);
	wapper.setMemberCardName(cardName);
	m_pResHandle->append(wapper);
	return wapper.handleId();
}

void DiscussManager::syncNextDiscussMembers()
{
	// next discuss member
	if (!m_syncDiscussIds.isEmpty())
	{
		QString fetchId = m_syncDiscussIds.takeFirst();
		qPmApp->getGroupsMemberManager()->fetchDiscussMembers(fetchId);
	}
}

#include "DiscussManager.moc"
