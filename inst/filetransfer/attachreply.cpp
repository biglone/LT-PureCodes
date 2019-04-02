#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QFile>
#include <QStandardPaths>
#include "attachreply.h"
#include <QImage>

CAttachReply::CAttachReply( Type eType, const bean::AttachItem &item, bool continuous /*= false*/, QObject *parent /*= 0*/ )
: QObject(parent)
, m_eType(eType)
, m_nTransferPreKb(1)
, m_pFile(0)
, m_bContinuous(continuous)
, m_useFastdfs(false)
{
	m_attachItem = item;

	qint64 nSize = item.size()/1024;
	if (nSize > 1000)
	{
		m_nTransferPreKb = nSize/100;
	}
}

CAttachReply::~CAttachReply()
{
}

QString CAttachReply::typeName() const
{
	return typeName(this->m_eType);
}

QString CAttachReply::typeName(FTJob::Type typeVal)
{
	QString sRet = "";
	switch (typeVal)
	{
	case FTJob::UPLOAD:
		sRet = "upload";
		break;
	case FTJob::DOWNLOAD:
		sRet = "download";
		break;
	}

	return sRet;
}

void CAttachReply::setError(bean::AttachItem::TransferResult eResult, const QString &rsError)
{
	emit error(m_attachItem.uuid(), typeName(), int(eResult), rsError, m_attachItem.messageType());
}

void CAttachReply::setUseFastdfs(bool use)
{
	m_useFastdfs = use;
}

bool CAttachReply::useFastdfs() const
{
	return m_useFastdfs;
}

FTJob::Type CAttachReply::type()
{
	return m_eType;
}

bool CAttachReply::isContinuous()
{
	return m_bContinuous;
}

QFile *CAttachReply::file()
{
	do 
	{
		if (m_pFile)
			break;

		QIODevice::OpenMode op = QIODevice::ReadOnly;
		QString attachFileName = m_attachItem.filename();
		QString attachFilePath = m_attachItem.filepath();
		if (m_eType == FTJob::DOWNLOAD)
		{
			op = m_bContinuous ? QIODevice::Append : QIODevice::WriteOnly;

			// default attach do not allow to overwrite the old file
			if (m_attachItem.transferType() == bean::AttachItem::Type_Default && !isContinuous())
			{
				QFileInfo attFileInfo(attachFileName);
				QString suffix = attFileInfo.suffix();
				QString attFileFilter = suffix.isEmpty() ? QString("%1*").arg(attFileInfo.completeBaseName()) :
					QString("%1*.%2").arg(attFileInfo.completeBaseName()).arg(suffix);

				QFileInfo attFilePathInfo(attachFilePath);
				QDir attDir = attFilePathInfo.absoluteDir();

				QFileInfoList fileInfoList = attDir.entryInfoList(QStringList() << attFileFilter, QDir::Files|QDir::NoDotAndDotDot|QDir::System);
				QStringList existFileNames;
				foreach (QFileInfo fi, fileInfoList)
				{
					existFileNames.append(fi.fileName());
				}
				if (!existFileNames.isEmpty())
				{
					int i = 1;
					bool renamed = false;
					while (existFileNames.contains(attachFileName))
					{
						if (!renamed)
							renamed = true;

						attachFileName = suffix.isEmpty() ? QString("%1(%2)").arg(attFileInfo.completeBaseName()).arg(i) : 
							QString("%1(%2).%3").arg(attFileInfo.completeBaseName()).arg(i).arg(suffix);
						++i;
					}
					if (renamed)
					{
						attachFilePath = attDir.absoluteFilePath(attachFileName);
						m_attachItem.setFilePath(attachFilePath);

						// file name is renamed
						emit downloadChanged(m_attachItem.uuid(), attachFilePath);
					}
				}
			}
			else if (m_attachItem.transferType() == bean::AttachItem::Type_Dir)
			{
				QFileInfo attFileInfo(attachFileName);
				QString attFileFilter = QString("%1*").arg(attachFileName);
				QFileInfo attFilePathInfo(attachFilePath);
				QDir attDir = attFilePathInfo.absoluteDir();

				QFileInfoList fileInfoList = attDir.entryInfoList(QStringList() << attFileFilter, QDir::Dirs|QDir::NoDotAndDotDot);
				QStringList existFileNames;
				foreach (QFileInfo fi, fileInfoList)
				{
					existFileNames.append(fi.fileName());
				}
				if (!existFileNames.isEmpty())
				{
					int i = 1;
					bool renamed = false;
					while (existFileNames.contains(attachFileName))
					{
						if (!renamed)
							renamed = true;

						attachFileName = QString("%1(%2)").arg(attFileInfo.fileName()).arg(i);
						++i;
					}
					if (renamed)
					{
						attachFilePath = attDir.absoluteFilePath(attachFileName);
						m_attachItem.setFilePath(attachFilePath);

						// file name is renamed
						emit downloadChanged(m_attachItem.uuid(), attachFilePath);
					}
				}

				attachFilePath = dirZipFilePath();
			}
		}
		else // UPLOAD
		{
			// upload dir use dir zip file
			if (m_attachItem.transferType() == bean::AttachItem::Type_Dir)
			{
				attachFilePath = dirZipFilePath();
			}
		}

		m_pFile = new QFile(attachFilePath);
		if (!m_pFile->open(op))
		{
			qWarning() << Q_FUNC_INFO << attachFilePath + " open fail.";
			break;
		}
	} while (0);

	return m_pFile;
}

QString CAttachReply::transferName()
{
	QString tname;
	if (!m_useFastdfs)
		tname = m_attachItem.transfername();
	else
		tname = m_attachItem.source();
	return tname;
}

QString CAttachReply::uuid()
{
	return m_attachItem.uuid();
}

QString CAttachReply::userId()
{
	return m_attachItem.from();
}

int CAttachReply::transferPerKb()
{
	return m_nTransferPreKb;
}

bool CAttachReply::transferKb(int nKb, int nKbTotal)
{
	emit progress(m_attachItem.uuid(), typeName(), nKb, nKbTotal);
	return true;
}

void CAttachReply::transferOver(OverCode oc, const QString &param /*= QString()*/)
{
	if (m_pFile)
	{
		m_pFile->close();
		delete m_pFile;
		m_pFile = 0;
	}

	if (m_eType == FTJob::UPLOAD &&
		m_attachItem.transferType() == bean::AttachItem::Type_Dir)
	{
		// remove zip file
		QFile::remove(dirZipFilePath());
	}

	if (oc != OC_OK &&
		m_eType == FTJob::DOWNLOAD &&
		m_attachItem.transferType() == bean::AttachItem::Type_Dir)
	{
		// remove zip file
		QFile::remove(dirZipFilePath());
	}

	if (oc != OC_OK && 
		m_eType == FTJob::DOWNLOAD && 
		m_attachItem.transferType() == bean::AttachItem::Type_Default && 
		!isContinuous())
	{
		QFile::remove(m_attachItem.filepath());
	}

	if (oc == OC_OK)
	{
		oc = postTransfer();

		if (m_eType == FTJob::DOWNLOAD &&
			m_attachItem.transferType() == bean::AttachItem::Type_Dir)
		{
			qWarning() << Q_FUNC_INFO << "post transfer: " << m_attachItem.uuid() << (int)oc;

			// remove zip file
			QFile::remove(dirZipFilePath());
		}

		if (m_eType == FTJob::DOWNLOAD &&
			m_attachItem.transferType() == bean::AttachItem::Type_AutoDisplay)
		{
			// auto display generate thumbnail
			bool generate = bean::AttachItem::needGenerateThumbnail(m_attachItem.filepath(),
				QSize(m_attachItem.picWidth(), m_attachItem.picHeight()));
			if (generate)
			{
				if (!bean::AttachItem::generateThumbnail(m_attachItem.filepath()))
				{
					qWarning() << Q_FUNC_INFO << "generate thumbnail failed: " << m_attachItem.uuid() << m_attachItem.filepath();
				}
			}
		}
	}

	switch (oc)
	{
	case OC_OK:
		{
			qWarning() << Q_FUNC_INFO << "transfer ok: " 
				<< m_attachItem.uuid() 
				<< m_attachItem.messageType() << m_attachItem.from()
				<< m_attachItem.transferType() << m_attachItem.filepath()
				<< param;

			m_attachItem.setSource(param);
			emit finish(m_attachItem.uuid(), typeName(), int(bean::AttachItem::Transfer_Successful), m_attachItem.messageType());
		}
		break;
	case OC_FAIL:
		{
			qWarning() << Q_FUNC_INFO << "transfer failed: " 
				<< m_attachItem.messageType() << m_attachItem.from()
				<< m_attachItem.transferType() << m_attachItem.filepath();

			setError(bean::AttachItem::Transfer_Error, tr("Transfer error"));
		}
		break;
	case OC_CANCEL:
		{
			qWarning() << Q_FUNC_INFO << "transfer canceled: " 
				<< m_attachItem.messageType() << m_attachItem.from()
				<< m_attachItem.transferType() << m_attachItem.filepath();

			setError(bean::AttachItem::Transfer_Cancel, tr("Transfer cancel"));
		}
		break;
	}
}

FTJob::OverCode CAttachReply::preTransfer()
{
	if (m_eType == FTJob::UPLOAD &&
		m_attachItem.transferType() == bean::AttachItem::Type_Dir)
	{
		// zip the dir
		QDir dir(m_attachItem.filepath());
		QString zipFilePath = dirZipFilePath();
		QFile::remove(zipFilePath);
		ZipUtil::ZipResult ret = ZipUtil::archiveDir(zipFilePath, dir, QString(), this);
		if (ret == ZipUtil::ZipError)
			return FTJob::OC_FAIL;
		else if (ret == ZipUtil::ZipCancel)
			return FTJob::OC_CANCEL;
		else
			return FTJob::OC_OK;
	}
	return FTJob::OC_OK;
}

FTJob::OverCode CAttachReply::postTransfer()
{
	if (m_eType == FTJob::DOWNLOAD &&
		m_attachItem.transferType() == bean::AttachItem::Type_Dir)
	{
		// unzip the dir
		QString zipFilePath = dirZipFilePath();
		QString unZipDirPath = m_attachItem.filepath();
		ZipUtil::ZipResult ret = ZipUtil::extractDir(zipFilePath, unZipDirPath, QString(), this);
		if (ret == ZipUtil::ZipError)
			return FTJob::OC_FAIL;
		else if (ret == ZipUtil::ZipCancel)
			return FTJob::OC_CANCEL;
		else
			return FTJob::OC_OK;
	}
	return FTJob::OC_OK;
}

bool CAttachReply::shouldZipContinue()
{
	return !isCancel();
}

QString CAttachReply::dirZipFilePath()
{
	QString zipFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	zipFilePath += QString("/%1.%2").arg(m_attachItem.uuid()).arg(m_attachItem.format());
	zipFilePath = QDir::toNativeSeparators(zipFilePath);
	return zipFilePath;
}