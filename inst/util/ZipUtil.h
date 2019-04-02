#ifndef _ZIPUTIL_H_
#define _ZIPUTIL_H_

#include <QString>
#include <QDir>

class ZipDelegate
{
public:
	virtual bool shouldZipContinue() = 0;
};

class ZipUtil
{
public:
	enum ZipResult
	{
		ZipOK,
		ZipError,
		ZipCancel
	};

public:
	static ZipResult extractDir(const QString &filePath, const QString &extDirPath, 
		const QString &singleFileName = QString(), ZipDelegate *zipDelegate = 0);

	static ZipResult archiveDir(const QString &filePath, const QDir &dir, 
		const QString &comment = QString(), ZipDelegate *zipDelegate = 0);
};

#endif // _ZIPUTIL_H_
