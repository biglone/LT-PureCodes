#include <QApplication>
#include "qtsingleapplication.h"
#include <QTextCodec>
#include "Constants.h"
#include <QIcon>
#include <QDir>
#include <QDebug>
#include <QDesktopServices>
#include "deamonmanager.h"
#include <QStandardPaths>
#include <QThread>
#include <QProcess>
#include "log/logcleaner.h"

#include "application/Logger.h"
using namespace application;

static const wchar_t *kExitEventName = L"RTC_MODULE_EXIT_LINGTALK";
static HANDLE rtcExitEvent = NULL;

bool createRtcExitEvent()
{
	rtcExitEvent = CreateMutex(NULL, TRUE, kExitEventName);
	if (!rtcExitEvent)
	{
		qWarning() << "create rtc exit event failed: " << GetLastError();
		return false;
	}
	return true;
}

void closeRtcExitEvent()
{
	if (rtcExitEvent)
	{
		CloseHandle(rtcExitEvent);
		rtcExitEvent = NULL;
	}
}

void setRtcExit()
{
	if (rtcExitEvent)
	{
		ReleaseMutex(rtcExitEvent);
	}
}

QString InitLog()
{
	QDir logDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	logDir.mkpath("log");
	logDir.cd("log");

	QString sPath = logDir.absolutePath();
	int nPid = QApplication::applicationPid();

	LOGGER_INIT(qPrintable(sPath), APP_DEAMON_NAME, nPid);
	LOGGER_SETLEVEL(LOGLEVEL_ALL);

	qDebug() << __FUNCTION__ << " app path:  " << sPath << " sysinfo: " << QSysInfo::windowsVersion();
	return sPath;
}

void myMessageOutput(QtMsgType type, const QMessageLogContext & /*context*/, const QString &msg)
{
	switch (type) {
	 case QtDebugMsg:
		 LOG_DEBUG("%s", msg.toLocal8Bit().constData());
		 break;
	 case QtWarningMsg:
		 LOG_WARN("%s", msg.toLocal8Bit().constData());
		 break;
	 case QtCriticalMsg:
		 LOG_ERROR("%s", msg.toLocal8Bit().constData());
		 break;
	 case QtFatalMsg:
		 LOG_FATAl("%s", msg.toLocal8Bit().constData());
		 abort();
		 break;
	 default:
		 break;
	}
}

int main(int argc, char *argv[])
{
	// set text codec
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	// QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
	// QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

	QtSingleApplication a(APP_DEAMON_NAME, argc, argv);
	a.setOrganizationName(QObject::tr(ORG_NAME));
	a.setOrganizationDomain(QObject::tr(ORG_DOMAIN));
	a.setApplicationName(QObject::tr(APP_DEAMON_NAME));
	a.setApplicationVersion(QObject::tr(APP_VERSION));
	a.setWindowIcon(QIcon(":/app/app.png"));
	a.setQuitOnLastWindowClosed(false);

	// init log
	QString logPath = InitLog();

#if defined(NDEBUG)
	qInstallMessageHandler(myMessageOutput);
#endif

	// start param
	QString pid = QString("%1").arg(QApplication::applicationPid());
	QString params = "";
	QStringList args = a.arguments();
	for (int i = 1; i < args.length(); ++i)
	{
		qDebug(" %s %s %d - %s", qPrintable(pid), Q_FUNC_INFO, i, qPrintable(args[i]));
		params += QString::fromLatin1(args[i].toUtf8().toBase64()) + " ";
	}
	params = params.trimmed();

	// only run one instance of this application
	if (a.isRunning())
	{
		// Application has already run, send message to start a instance
		QString message = QString("%1:%2").arg(pid).arg(params);
		const int kMaxTryCount = 3;
		int tryCount = kMaxTryCount;
		bool sendOK = false;
		while (tryCount > 0)
		{
			sendOK = a.sendMessage(message);
			if (sendOK)
			{
				qDebug("%s %s send message: ok", qPrintable(pid), Q_FUNC_INFO);
				return 0;
			}
			else
			{
				qWarning("%s %s send message: failed, continue", qPrintable(pid), Q_FUNC_INFO);
			}

			QThread::msleep(100);
			--tryCount;
		}

		qWarning("%s %s send message: failed %d times", qPrintable(pid), Q_FUNC_INFO, kMaxTryCount);
		return -1;
	}

	if (!createRtcExitEvent())
	{
		qWarning() << Q_FUNC_INFO << "rtc module can't start";
	}
	else
	{
		QString rtcServerPath = QCoreApplication::applicationDirPath() + "/lingtalkrtc.exe";
		if (!QProcess::startDetached(rtcServerPath, QStringList()))
		{
			qWarning() << Q_FUNC_INFO << "start rtc module failed";
		}
	}

	// create manager and initialize it
	if (!PMDeamonManager::instance()->initialize())
	{
		// initialize failed, just return
		return -1;
	}

	QObject::connect(&a, SIGNAL(messageReceived(QString)), PMDeamonManager::instance(), SLOT(onDeamonMessageReceived(QString)));

	// start a pm instance
	if (!PMDeamonManager::instance()->checkToStartInstance(params))
	{
		// start pm instance failed
		qWarning("%s %s start instance failed", qPrintable(pid), Q_FUNC_INFO);
		return -1;
	}

	// start log cleaner
	LogCleaner *logCleaner = new LogCleaner(logPath);
	QObject::connect(logCleaner, SIGNAL(finished()), logCleaner, SLOT(deleteLater()));
	logCleaner->start(QThread::LowPriority);

	int ret = a.exec();

	setRtcExit();
	closeRtcExitEvent();

	PMDeamonManager::destroyInstance();

	return ret;
}
