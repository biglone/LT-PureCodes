#include "FileUtil.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN

bool FileUtil::removePath(const QString &path)
{
	bool result = true;
	QFileInfo info(path);
	if (info.isDir()) 
	{
		QDir dir(path);
		foreach (const QString &entry, dir.entryList(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot)) 
		{
			result &= removePath(dir.absoluteFilePath(entry));
		}
		QString dirName = info.fileName();
		if (!dirName.isEmpty())
		{
			if (!info.dir().rmdir(dirName))
			{
				qDebug() << Q_FUNC_INFO << "remove dir failed: " << dirName;
				return false;
			}
		}
	} 
	else 
	{
		result = QFile::remove(path);
		if (!result)
		{
			qDebug() << Q_FUNC_INFO << "remove file failed: " << path;
		}
	}
	return result;
}

qint64 FileUtil::fileSize(const QString &path)
{
	qint64 size = 0;

	QFileInfo fi(path);
	if (!fi.isSymLink())
	{
		return fi.size();
	}

	// deal with symbol link size
	wchar_t filePath[256];
	int len = path.toWCharArray(filePath);
	filePath[len] = (wchar_t)('\0');

	HANDLE h = CreateFileW(filePath,               // file to open
		GENERIC_READ,          // open for reading
		FILE_SHARE_READ,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL);                 // no attr. template

	if (h == INVALID_HANDLE_VALUE) 
	{ 
		qDebug() << Q_FUNC_INFO << "Could not open file:" << GetLastError();
		return size; 
	}


	LARGE_INTEGER liSize;
	if (!GetFileSizeEx(h, &liSize))
	{
		qDebug() << Q_FUNC_INFO << "GetFileSizeEx failed.";
		CloseHandle(h);
		return size;
	}

	size = liSize.QuadPart;

	CloseHandle(h);

	return size;
}

bool FileUtil::fileExists(const QString &path)
{
	QFileInfo fi(path);
	if (fi.exists())
	{
		return true;
	}

	if (fi.isSymLink()) // symbol link returns false if the symbol target does not exist
	{
		wchar_t filePath[256];
		int len = path.toWCharArray(filePath);
		filePath[len] = (wchar_t)('\0');

		HANDLE h = CreateFileW(filePath,               // file to open
			GENERIC_READ,          // open for reading
			FILE_SHARE_READ,       // share for reading
			NULL,                  // default security
			OPEN_EXISTING,         // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL);                 // no attr. template

		if (h == INVALID_HANDLE_VALUE) 
		{ 
			return false;
		}
		
		CloseHandle(h);

		return true;
	}
	else
	{
		return false;
	}
}

QStringList FileUtil::allDirFiles(const QDir &d)
{
	QStringList list;
	QStringList entries = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::NoSymLinks);

	foreach (QString file, entries) 
	{
		QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));
		if (finfo.isDir()) 
		{
			QDir subDir(finfo.filePath());
			list << allDirFiles(subDir);
		} 
		else
		{
			list << QDir::toNativeSeparators(finfo.filePath());
		}
	}

	return list;
}

qint64 FileUtil::dirSize(const QDir &d)
{
	QStringList files = allDirFiles(d);
	if (files.isEmpty())
		return true;

	qint64 fileSize = 0;
	foreach (QString file, files)
	{
		QFileInfo finfo(file);
		fileSize += finfo.size();
	}

	return fileSize;
}