#include "ExplorerUtil.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QProcess>

bool ExplorerUtil::selectFile(const QFileInfo &fi)
{
	QString cmd;
	if (fi.exists())
	{
		QString filePath = fi.absoluteFilePath();
		QString fileName = fi.fileName();
		filePath.replace(fileName, QString("\"") + fileName + QString("\""));
		cmd = QString("explorer.exe /select, %1").arg(QDir::toNativeSeparators(filePath));
	}
	else
	{
		cmd = QString("explorer.exe /e, %1").arg(QDir::toNativeSeparators(fi.dir().absolutePath()));
	}

	bool ok = QProcess::startDetached(cmd);
	qDebug() << cmd << " : " << ok;
	return ok;
}

bool ExplorerUtil::openDir(const QString &dirPath)
{
	QDir dir(dirPath);
	QString cmd = QString("explorer.exe /e, %1").arg(QDir::toNativeSeparators(dir.absolutePath()));
	
	bool ok = QProcess::startDetached(cmd);
	qDebug() << cmd << " : " << ok;
	return ok;
}

