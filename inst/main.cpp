#include <QObject>
#include <QIcon>
#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QDir>
#include <QString>
#include <QByteArray>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QDebug>
#include <QSysInfo>
#include <QFontDatabase>
#include <QDateTime>
#include <QDesktopServices>

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std;

#include "application/Logger.h"
using namespace application;

#include "cttk/Base.h"

#include "PmApp.h"

#include "Constants.h"
#include "minidump.h"

#include "focuslessstyle.h"
#include "log/logcleaner.h"

QString sDumpFile = QString("%1\\%2_%3.dmp")
	.arg(QDir::toNativeSeparators(QDir::tempPath()))
	.arg(APP_NAME)
	.arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz"));


void InitLog()
{
	QDir logDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	logDir.mkpath("log");
	logDir.cd("log");

	QString sPath = logDir.absolutePath();
	int nPid = QApplication::applicationPid();

	LOGGER_INIT(qPrintable(sPath), APP_NAME, nPid);
	LOGGER_SETLEVEL(LOGLEVEL_ALL);

	qDebug() << __FUNCTION__ << " app path:  " << sPath << " sysinfo: " << QSysInfo::windowsVersion();

	// clear unused log files
	LogCleaner *logCleaner = new LogCleaner(sPath);
	QObject::connect(logCleaner, SIGNAL(finished()), logCleaner, SLOT(deleteLater()));
	logCleaner->start(QThread::LowPriority);
}

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	CreateMiniDump(pExceptionInfo, qPrintable(sDumpFile));
	return EXCEPTION_EXECUTE_HANDLER;
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

int main(int argc, char* argv[])
{
#if defined(NDEBUG)
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
#endif

	int nRet = -2;
	HANDLE hMutex = NULL;
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	// QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
	// QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

	do 
	{
#if defined(_WIN32) // windows socket
		WSADATA wsaData;
		int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
		if (err != 0)
			break;
#endif //_WIN32

		// create a mutex for installation test
		QString sAppMutex = QString("%1_%2").arg(APP_NAME).arg(APP_ID);
		hMutex = CreateMutex(NULL, FALSE, qPrintable(sAppMutex));
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			qWarning() << "Install mutex: " << sAppMutex << " already exists";
			hMutex = NULL;
		}

		nRet = -1;
		QApplication app(argc, argv);

		// setup app style
		if (QSysInfo::windowsVersion() == QSysInfo::WV_XP)
			app.setStyle(new FocuslessStyle());

		PmApp pmApp(&app);
		if (!pmApp.connectToManager()) // connect manager failed
			break;

		// initialize log
		InitLog();
#if defined(NDEBUG)
		qInstallMessageHandler(myMessageOutput);
#endif

		// finish construct & start
		pmApp.construct();
		pmApp.restart();

		app.exec();

		nRet = 0;

	} while (0);

	if (hMutex != NULL)
		CloseHandle(hMutex);

	if (nRet > -2)
	{
#if defined(_WIN32) // windows socket
		WSACleanup();
#endif// _WIN32
	}

	return nRet;
}