#ifndef _FILEUTIL_H_
#define _FILEUTIL_H_

#include <QString>
#include <QStringList>
#include <QDir>

class FileUtil
{
public:
	static bool removePath(const QString &path);

	static qint64 fileSize(const QString &path);

	static bool fileExists(const QString &path);

	static QStringList allDirFiles(const QDir &d);

	static qint64 dirSize(const QDir &d);
};

#endif // _FILEUTIL_H_
