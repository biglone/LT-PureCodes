#include "ZipUtil.h"
#include "FileUtil.h"
#include "quazip.h"
#include "quazipfile.h"
#include "quazipnewinfo.h"
#include <QDebug>

ZipUtil::ZipResult ZipUtil::extractDir(const QString &filePath, 
	                                   const QString &extDirPath, 
	                                   const QString &singleFileName /*= QString()*/,
						               ZipDelegate *zipDelegate /*= 0*/) 
{
	ZipUtil::ZipResult ret = ZipUtil::ZipOK;

	QuaZip zip(filePath);
	zip.setFileNameCodec("UTF-8");
	if (!zip.open(QuaZip::mdUnzip)) 
	{
		qWarning("ZipUtil::extractDir testRead(): zip.open(): %d", zip.getZipError());
		ret = ZipUtil::ZipError;
		return ret;
	}

	/*
	qWarning("ZipUtil::extractDir %d entries\n", zip.getEntriesCount());
	qWarning("ZipUtil::extractDir Global comment: %s\n", zip.getComment().toLocal8Bit().constData());
	*/

	QuaZipFileInfo info;
	QuaZipFile file(&zip);
	QFile out;
	QString name;
	for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) 
	{
		if (zipDelegate && !zipDelegate->shouldZipContinue())
		{
			qWarning("ZipUtil::extractDir zip canceled");
			ret = ZipUtil::ZipCancel;
			break;
		}

		if (!zip.getCurrentFileInfo(&info)) 
		{
			qWarning("ZipUtil::extractDir testRead(): getCurrentFileInfo(): %d", zip.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}

		if (!singleFileName.isEmpty())
			if (!info.name.contains(singleFileName))
				continue;

		if (!file.open(QIODevice::ReadOnly)) 
		{
			qWarning("ZipUtil::extractDir testRead(): file.open(): %d", file.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}

		if (file.getZipError() != UNZ_OK) 
		{
			qWarning("ZipUtil::extractDir testRead(): file.getFileName(): %d", file.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}

		name = QString("%1/%2").arg(extDirPath).arg(file.getActualFileName());
		name = QDir::toNativeSeparators(name);
		QFileInfo outFileInfo(name);
		QDir subDir = outFileInfo.dir();
		if (!subDir.exists())
			subDir.mkpath(subDir.absolutePath());
		out.setFileName(name);
		if (!out.open(QIODevice::WriteOnly))
		{
			file.close();
			qWarning("ZipUtil::extractDir testRead(): out.open(): %s", out.errorString().toLocal8Bit().constData());
			ret = ZipUtil::ZipError;
			break;
		}

		// write to out
		const qint64 kMaxReadSize = 64*1024; // 64kb
		QByteArray content = file.read(kMaxReadSize);
		while (!content.isEmpty())
		{
			out.write(content);
			content = file.read(kMaxReadSize);
		}
		out.close();

		if (file.getZipError() != UNZ_OK) 
		{
			file.close();
			qWarning("ZipUtil::extractDir testRead(): file.getFileName(): %d", file.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}

		if (!file.atEnd()) 
		{
			file.close();
			qWarning("ZipUtil::extractDir testRead(): read all but not EOF");
			ret = ZipUtil::ZipError;
			break;
		}

		file.close();

		if (file.getZipError() != UNZ_OK) 
		{
			qWarning("ZipUtil::extractDir testRead(): file.close(): %d", file.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}
	}

	zip.close();

	if (ret == ZipUtil::ZipOK)
	{
		if (zip.getZipError() != UNZ_OK) 
		{
			qWarning("testRead(): zip.close(): %d", zip.getZipError());
			ret = ZipUtil::ZipError;
		}
	}

	return ret;
}

ZipUtil::ZipResult ZipUtil::archiveDir(const QString &filePath, 
	                                   const QDir &dir, 
									   const QString &comment /*= QString()*/,
									   ZipDelegate *zipDelegate /*= 0*/) 
{
	ZipUtil::ZipResult ret = ZipUtil::ZipOK;

	if (!dir.exists()) 
	{
		qWarning("ZipUtil::archiveDir dir.exists(%s)=FALSE", dir.absolutePath().toLocal8Bit().constData());
		ret = ZipUtil::ZipError;
		return ret;
	}

	QuaZip zip(filePath);
	zip.setFileNameCodec("UTF-8");
	if (!zip.open(QuaZip::mdCreate)) 
	{
		qWarning("ZipUtil::archiveDir testCreate(): zip.open(): %d", zip.getZipError());
		ret = ZipUtil::ZipError;
		return ret;
	}

	// get all dir file info
	QStringList sl = FileUtil::allDirFiles(dir);
	QFileInfoList files;
	foreach (QString fn, sl)
	{
		files << QFileInfo(fn);
	}

	QuaZipFile outFile(&zip);
	QFile inFile;
	foreach (QFileInfo fileInfo, files) 
	{
		if (zipDelegate && !zipDelegate->shouldZipContinue())
		{
			qWarning("ZipUtil::extractDir zip canceled");
			ret = ZipUtil::ZipCancel;
			break;
		}

		if (!fileInfo.isFile())
			continue;

		QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
		if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath()))) 
		{
			qWarning("ZipUtil::archiveDir testCreate(): outFile.open(): %d", outFile.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}

		inFile.setFileName(fileInfo.filePath());
		if (!inFile.open(QIODevice::ReadOnly)) 
		{
			outFile.close();
			qWarning("ZipUtil::archiveDir testCreate(): inFile.open(): %s", inFile.errorString().toLocal8Bit().constData());
			ret = ZipUtil::ZipError;
			break;
		}

		// write to out file
		const qint64 kMaxReadSize = 64*1024; // 64kb
		QByteArray content = inFile.read(kMaxReadSize);
		while (!content.isEmpty())
		{
			outFile.write(content);
			content = inFile.read(kMaxReadSize);
		}
		inFile.close();

		if (outFile.getZipError() != UNZ_OK) 
		{
			outFile.close();
			qWarning("ZipUtil::archiveDir testCreate(): outFile.putChar(): %d", outFile.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}

		outFile.close();

		if (outFile.getZipError() != UNZ_OK) 
		{
			qWarning("ZipUtil::archiveDir testCreate(): outFile.close(): %d", outFile.getZipError());
			ret = ZipUtil::ZipError;
			break;
		}
	}

	if (ret == ZipUtil::ZipOK)
	{
		// add comment
		if (!comment.isEmpty())
			zip.setComment(comment);
	}

	zip.close();

	if (ret == ZipUtil::ZipOK)
	{
		if (zip.getZipError() != UNZ_OK) 
		{
			qWarning("ZipUtil::archiveDir testCreate(): zip.close(): %d", zip.getZipError());
			ret = ZipUtil::ZipError;
		}
	}

	return ret;
}