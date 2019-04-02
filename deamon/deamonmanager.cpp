#include "deamonmanager.h"
#include "localserver.h"
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QStringList>
#include <QProcess>
#include <QByteArray>
#include <QDomElement>
#include <QSettings>
#include <QDataStream>
#include "Constants.h"

static QList<QProcess *> s_instanceProcesses;

static QString applicationMsgCode2String(ns_local_comm::ApplicationMsgCode code)
{
	QString str;
	switch (code)
	{
	case ns_local_comm::AppClose:
		str = QString::fromLatin1("AppClose");
		break;
	case ns_local_comm::AppVersion:
		str = QString::fromLatin1("AppVersion");
		break;
	case ns_local_comm::AppLoadGlobalSetting:
		str = QString::fromLatin1("AppLoadGlobalSetting");
		break;
	case ns_local_comm::AppSaveGlobalSetting:
		str = QString::fromLatin1("AppSaveGlobalSetting");
		break;
	case ns_local_comm::AppLoadAccounts:
		str = QString::fromLatin1("AppLoadAccounts");
		break;
	case ns_local_comm::AppSaveAccounts:
		str = QString::fromLatin1("AppSaveAccounts");
		break;
	case ns_local_comm::AppSetLoginAccount:
		str = QString::fromLatin1("AppSetLoginAccount");
		break;
	case ns_local_comm::AppQueryAccountLogined:
		str = QString::fromLatin1("AppQueryAccountLogined");
		break;
	case ns_local_comm::AppSetLogoutAccount:
		str = QString::fromLatin1("AppSetLogoutAccount");
		break;
	case ns_local_comm::AppSetUpdate:
		str = QString::fromLatin1("AppSetUpdate");
		break;
	case ns_local_comm::AppQueryUpdate:
		str = QString::fromLatin1("AppQueryUpdate");
		break;
	default:
		str = QString::fromLatin1("UnknownCode");
		break;
	}
	return str;
}

PMDeamonManager * PMDeamonManager::s_instance = 0;

PMDeamonManager* PMDeamonManager::instance()
{
	if (!s_instance)
		s_instance = new PMDeamonManager();
	return s_instance;
}

void PMDeamonManager::destroyInstance()
{
	if (s_instance)
		delete s_instance;
	s_instance = 0;
}

PMDeamonManager::PMDeamonManager(QObject *parent) : QObject(parent), m_isUpdating(false)
{
	m_localServer = new LocalServer(LOCAL_COMM_SERVER_NAME, this);

	bool connected = false;
	connected = connect(m_localServer, SIGNAL(newSessionConnected(QString)), this, SLOT(onNewSessionConnected(QString)));
	Q_ASSERT(connected);

	connected = connect(m_localServer, SIGNAL(sessionDisconnected(QString)), this, SLOT(onSessionDisconnected(QString)));
	Q_ASSERT(connected);

	connected = connect(m_localServer, SIGNAL(messageReceived(QString, LocalCommMessage)), this, SLOT(onMessageReceived(QString, LocalCommMessage)));
	Q_ASSERT(connected);
}

PMDeamonManager::~PMDeamonManager()
{
}

bool PMDeamonManager::initialize()
{
	// start local server
	if (!m_localServer->start())
	{
		qDebug() << "Error: start local server failed.";
		return false;
	}

	m_sessionCheckTimer.setInterval(60*1000); // 60s
	m_sessionCheckTimer.setSingleShot(false);
	connect(&m_sessionCheckTimer, SIGNAL(timeout()), this, SLOT(checkSession()));
	m_sessionCheckTimer.start();
	qDebug() << "Start to check session count, every " << m_sessionCheckTimer.interval()/1000 << "s";

	return true;
}

bool PMDeamonManager::isAccountLogined(const QString &accountUid)
{
	return m_loginedAccounts.values().contains(accountUid);
}

bool PMDeamonManager::checkToStartInstance(const QString &params)
{
	QStringList args = params.split(" ");
	return startInstance(args);
}

bool PMDeamonManager::startInstance(const QStringList &args /*= QStringList()*/)
{
	// start a new instance
	QProcess *process = new QProcess();
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	if (env.contains("PATH") || env.contains("Path"))
	{
		// 重新设置PATH环境变量，因为在xp机器上与wps软件冲突，原因是wps在环境变量PATH中放置了自己的路径，路径中有Qt4的DLL，
		// 导致QtNetwork模块崩溃，至于为什么QtNetwork会使用PATH环境变量，还不清楚
		env.insert("Path", QApplication::applicationDirPath());
		env.insert("PATH", QApplication::applicationDirPath());
		process->setProcessEnvironment(env);
	}
	QString instPath = QApplication::applicationDirPath() + QString("/%1").arg(APP_EXE_NAME);
	process->start(instPath, args);
	bool bOk = process->waitForStarted();
	qDebug("%s %s %s", Q_FUNC_INFO, qPrintable(instPath), (bOk ? "true" : "false"));

	if (bOk)
	{
		s_instanceProcesses << process;
	}
	else
	{
		delete process;
		process = 0;
	}

	return bOk;
}

void PMDeamonManager::onMessageReceived(const QString &sessionId, const LocalCommMessage &msg)
{
	if (msg.messageType() == ns_local_comm::MsgApplication)
	{
		handleApplicationMsg(sessionId, msg);
		return;
	}
}

void PMDeamonManager::onNewSessionConnected(const QString &sessionId)
{
	qDebug() << "Deamon: PmApp connected: " << sessionId;
}

void PMDeamonManager::onSessionDisconnected(const QString &sessionId)
{
	qDebug() << "Deamon: PmApp disconnected: " << sessionId << "left session count: " << m_localServer->sessionCount();

	if (m_loginedAccounts.contains(sessionId))
	{
		qDebug() << "Deamon: session id is for: " << m_loginedAccounts[sessionId] << sessionId;
		m_loginedAccounts.remove(sessionId);
	}

	if (m_localServer->sessionCount() <= 0)
	{
		// exit application
		exitManager();
	}
}

void PMDeamonManager::checkSession()
{
	if (m_localServer->sessionCount() <= 0)
	{
		qDebug() << Q_FUNC_INFO << "session is empty, to exit manager";

		// exit application
		exitManager();
	}
}

void PMDeamonManager::exitManager()
{
	qDebug() << Q_FUNC_INFO;

	// exit from this application
	QApplication::exit();
}

void PMDeamonManager::onDeamonMessageReceived(const QString &message)
{
	QString fromPid;
	QString params;
	int index = message.indexOf(":");
	if (index != -1)
	{
		fromPid = message.left(index);
		params = message.mid(index+1);
	}
	else
	{
		params = message;
	}

	qDebug("%s: Deamon message from [%s]: %s", qPrintable(Q_FUNC_INFO), qPrintable(fromPid), qPrintable(params));

	if (!checkToStartInstance(params))
	{
		qDebug() << Q_FUNC_INFO << "start instance failed, exit manager.";
		exitManager();
	}
}

void PMDeamonManager::handleApplicationMsg(const QString &sessionId, const LocalCommMessage &msg)
{
	if (msg.messageType() != ns_local_comm::MsgApplication)
		return;

	qDebug() << "Deamon: received application message, request code: " 
		<< applicationMsgCode2String((ns_local_comm::ApplicationMsgCode)msg.requestCode())
		<< "from: " << sessionId;

	if (msg.requestCode() == ns_local_comm::AppSetLoginAccount)
	{
		QString accountUid = QString::fromUtf8(msg.data());
		m_loginedAccounts[sessionId] = accountUid;

		qDebug() << "Deamon: PmApp account logined: " << accountUid << sessionId << "already logined: " << m_loginedAccounts.values();

		return;
	}

	if (msg.requestCode() == ns_local_comm::AppSetLogoutAccount)
	{
		if (m_loginedAccounts.contains(sessionId))
		{
			QString uid = m_loginedAccounts[sessionId];
			m_loginedAccounts.remove(sessionId);

			qDebug() << "Deamon: PmApp account logout: " << uid << sessionId << "left logined: " << m_loginedAccounts.values();
		}
		return;
	}

	if (msg.requestCode() == ns_local_comm::AppQueryAccountLogined)
	{
		LocalCommMessage responseMsg = msg;
		QByteArray responseData;
		QString accountUid = QString::fromUtf8(msg.data());
		if (isAccountLogined(accountUid))
			responseData = QByteArray("true");
		else
			responseData = QByteArray("false");
		responseMsg.setResponseCode(ns_local_comm::AppQueryAccountLogined);
		responseMsg.setData(responseData);
		m_localServer->sendMessage(sessionId, responseMsg);
		return;
	}

	if (msg.requestCode() == ns_local_comm::AppSetUpdate)
	{
		QString update = QString::fromUtf8(msg.data());
		if (update == "true")
			m_isUpdating = true;
		else
			m_isUpdating = false;
		return;
	}

	if (msg.requestCode() == ns_local_comm::AppQueryUpdate)
	{
		LocalCommMessage responseMsg = msg;
		QByteArray responseData;
		if (m_isUpdating)
			responseData = QByteArray("true");
		else
			responseData = QByteArray("false");
		responseMsg.setResponseCode(ns_local_comm::AppQueryUpdate);
		responseMsg.setData(responseData);
		m_localServer->sendMessage(sessionId, responseMsg);
		return;
	}
}
