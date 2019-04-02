#include "logcleaner.h"
#include <QDir>
#include <QDebug>
#include <QDateTime>

LogCleaner::LogCleaner(const QString &logDirPath, QObject *parent)
	: QThread(parent), m_logDirPath(logDirPath)
{

}

LogCleaner::~LogCleaner()
{

}

void LogCleaner::run()
{
	QDir logDir(m_logDirPath);

	// clear unused log files
	QFileInfoList fileInfoList = logDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks);
	int removedFiles = 0;
	foreach (QFileInfo fi, fileInfoList)
	{
		QDateTime dt = QDateTime::currentDateTime();
		QDateTime dtFile = fi.lastModified();
		if (dtFile.daysTo(dt) > 3) // 3 days ago
		{
			QFile::remove(fi.absoluteFilePath());
			if (++removedFiles >= 10)
			{
				// one time remove 10 files maximum
				break;
			}
		}
	}
	qDebug() << "log cleaner removed: " << removedFiles << " log files";
}
