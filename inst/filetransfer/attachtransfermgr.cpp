#include <QDebug>
#include <QDir>
#include <QTimer>
#include "attachtransfermgr.h"
#include "attachreply.h"
#include "fileservfthandler.h"
#include "fastdfsfthandler.h"

CAttachTransferMgr::CAttachTransferMgr(QObject *parent)
	: QObject(parent), m_imminentUpload(0), m_imminentDownload(0), m_useFastdfs(false)
{
	for (int i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		m_upload[i] = 0;
		m_download[i] = 0;
	}

	m_downloadDir = ".";
}

CAttachTransferMgr::~CAttachTransferMgr()
{
	release();
}

bool CAttachTransferMgr::init(bool useFastdfs, const QString &addressStr)
{
	release();

	m_useFastdfs = useFastdfs;
	int i = 0;
	for (i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		m_upload[i] = createFTHandler(useFastdfs);
		m_download[i] = createFTHandler(useFastdfs);
	}
	m_imminentUpload = createFTHandler(useFastdfs);
	m_imminentDownload = createFTHandler(useFastdfs);

	bool bRet = false;
	do 
	{
		qDebug() << __FUNCTION__ << useFastdfs << addressStr;

		bool uploadDownloadOK = true;
		for (i = 0; i < kMaxFileTransferHandlerCount; ++i)
		{
			if(!m_upload[i]->init(addressStr))
			{
				qWarning() << Q_FUNC_INFO << "create m_upload failed.";
				uploadDownloadOK = false;
				break;
			}

			if(!m_download[i]->init(addressStr))
			{
				qWarning() << Q_FUNC_INFO << "create m_download failed.";
				uploadDownloadOK = false;
				break;
			}
		}

		if (!uploadDownloadOK)
			break;
		
		if(!m_imminentUpload->init(addressStr))
		{
			qWarning() << Q_FUNC_INFO << "create m_imminentUpload failed.";
			break;
		}

		if(!m_imminentDownload->init(addressStr))
		{
			qWarning() << Q_FUNC_INFO << "create m_imminentDownload failed.";
			break;
		}

		bRet = true;
	} while (0);

	if (!bRet)
	{
		release();
	}

	return bRet;
}

void CAttachTransferMgr::release()
{
	for (int i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		if (m_upload[i])
		{
			m_upload[i]->release();
			delete m_upload[i];
			m_upload[i] = 0;
		}

		if (m_download[i])
		{
			m_download[i]->release();
			delete m_download[i];
			m_download[i] = 0;
		}
	}

	if (m_imminentUpload)
	{
		m_imminentUpload->release();
		delete m_imminentUpload;
		m_imminentUpload = 0;
	}

	if (m_imminentDownload)
	{
		m_imminentDownload->release();
		delete m_imminentDownload;
		m_imminentDownload = 0;
	}

	qDebug() << __FUNCTION__;
}

void CAttachTransferMgr::setDownloadDir(const QString &rsDownloadDir)
{
	m_downloadDir = rsDownloadDir;
}

CAttachReply *CAttachTransferMgr::addUpload(const bean::AttachItem &attachItem)
{
	CAttachReply *reply = 0;
	do 
	{
		// 优先选择包含该id的上传，如果没有包含该id，则优先选择当前个数少的上传
		int jobCount[kMaxFileTransferHandlerCount];
		FTHandler *uploadHandler = 0;
		int i = 0;
		for (i = 0; i < kMaxFileTransferHandlerCount; ++i)
		{
			jobCount[i] = -1;
			if (m_upload[i])
			{
				QStringList jobIds = m_upload[i]->jobIds();
				jobCount[i] = jobIds.count();
				if (jobIds.contains(attachItem.uuid()))
				{
					uploadHandler = m_upload[i];
					break;
				}
			}
		}

		if (!uploadHandler)
		{
			int minCount = 1000000;
			int minIndex = -1;
			for (i = 0; i < kMaxFileTransferHandlerCount; ++i)
			{
				if (jobCount[i] < minCount)
				{
					minCount = jobCount[i];
					minIndex = i;
				}
			}
			if (minCount >= 0 && minIndex >= 0)
				uploadHandler = m_upload[minIndex];
		}

		if (!uploadHandler)
		{
			break;
		}

		reply = new CAttachReply(FTJob::UPLOAD, attachItem);
		if (!reply)
		{
			break;
		}

		reply = static_cast<CAttachReply *>(uploadHandler->addJob(reply));
	} while (0);

	return reply;
}

CAttachReply *CAttachTransferMgr::addDownload(const bean::AttachItem &attachItem, bool continuous /*= false*/)
{
	CAttachReply *reply = 0;
	do 
	{
		// 优先选择包含该id的下载，如果没有包含该id，则优先选择当前个数少的下载
		int jobCount[kMaxFileTransferHandlerCount];
		FTHandler *downloadHandler = 0;
		int i = 0;
		for (i = 0; i < kMaxFileTransferHandlerCount; ++i)
		{
			jobCount[i] = -1;
			if (m_download[i])
			{
				QStringList jobIds = m_download[i]->jobIds();
				jobCount[i] = jobIds.count();
				if (jobIds.contains(attachItem.uuid()))
				{
					downloadHandler = m_download[i];
					break;
				}
			}
		}

		if (!downloadHandler)
		{
			int minCount = 1000000;
			int minIndex = -1;
			for (i = 0; i < kMaxFileTransferHandlerCount; ++i)
			{
				if (jobCount[i] < minCount)
				{
					minCount = jobCount[i];
					minIndex = i;
				}
			}
			if (minCount >= 0 && minIndex >= 0)
				downloadHandler = m_download[minIndex];
		}

		if (!downloadHandler)
		{
			break;
		}

		bean::AttachItem item = attachItem;
		if (item.filepath().isEmpty())
		{
			item.setFilePath(QDir(m_downloadDir).absoluteFilePath(item.filename()));
		}

		reply = new CAttachReply(FTJob::DOWNLOAD, item, continuous);
		if (!reply)
		{
			break;
		}

		reply = static_cast<CAttachReply *>(downloadHandler->addJob(reply));
	} while (0);

	return reply;
}

CAttachReply *CAttachTransferMgr::addImminentUpload(const bean::AttachItem &attachItem)
{
	CAttachReply *reply = 0;
	do 
	{
		if (!m_imminentUpload)
		{
			break;
		}

		reply = new CAttachReply(FTJob::UPLOAD, attachItem);
		if (!reply)
		{
			break;
		}

		reply = static_cast<CAttachReply *>(m_imminentUpload->addJob(reply));
	} while (0);

	return reply;
}

CAttachReply *CAttachTransferMgr::addImminentDownload(const bean::AttachItem& attachItem, bool continuous /*= false*/)
{
	CAttachReply *reply = 0;
	do 
	{
		if (!m_imminentDownload)
		{
			break;
		}

		bean::AttachItem item = attachItem;
		if (item.filepath().isEmpty())
		{
			item.setFilePath(QDir(m_downloadDir).absoluteFilePath(item.filename()));
		}

		reply = new CAttachReply(FTJob::DOWNLOAD, item, continuous);
		if (!reply)
		{
			break;
		}

		reply = static_cast<CAttachReply *>(m_imminentDownload->addJob(reply));
	} while (0);

	return reply;
}

bool CAttachTransferMgr::cancelUpload(const bean::AttachItem &attachItem)
{
	for (int i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		if (m_upload[i])
		{
			QStringList jobIds = m_upload[i]->jobIds();
			if (jobIds.contains(attachItem.uuid()))
			{
				return m_upload[i]->cancelJob(attachItem.uuid());
			}
		}
	}
	return false;
}

bool CAttachTransferMgr::cancelDownload(const bean::AttachItem &attachItem)
{
	for (int i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		if (m_download[i])
		{
			QStringList jobIds = m_download[i]->jobIds();
			if (jobIds.contains(attachItem.uuid()))
			{
				return m_download[i]->cancelJob(attachItem.uuid());
			}
		}
	}
	return false;
}

bool CAttachTransferMgr::cancelImminentUpload(const bean::AttachItem& attachItem)
{
	if (m_imminentUpload)
		return false;

	return m_imminentUpload->cancelJob(attachItem.uuid());
}

bool CAttachTransferMgr::cancelImminentDownload(const bean::AttachItem &attachItem)
{
	if (!m_imminentDownload)
		return false;

	return m_imminentDownload->cancelJob(attachItem.uuid());
}

void CAttachTransferMgr::preAddJob(FTHandler *ftHandler, FTJob *job)
{
	if (!job)
		return;

	CAttachReply *reply = static_cast<CAttachReply *>(job);
	if (!reply)
		return;

	reply->setUseFastdfs(m_useFastdfs);

	for (int i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		if (ftHandler == m_upload[i])
		{
			emit preUpload(reply, job->uuid());
			return;
		}

		if (ftHandler == m_download[i])
		{
			emit preDownload(reply, job->uuid());
			return;
		}
	}

	if (ftHandler == m_imminentUpload)
	{
		emit preImminentUpload(reply, job->uuid());
		return;
	}

	if (ftHandler == m_imminentDownload)
	{
		emit preImminentDownload(reply, job->uuid());
		return;
	}
	
	qWarning() << Q_FUNC_INFO << " no ftHandler";
}

void CAttachTransferMgr::postAddJob(FTHandler *ftHandler, FTJob *job)
{
	if (!job)
		return;

	CAttachReply *reply = static_cast<CAttachReply *>(job);
	if (!reply)
		return;

	bean::AttachItem &attachItem = reply->attachItem();

	for (int i = 0; i < kMaxFileTransferHandlerCount; ++i)
	{
		if (ftHandler == m_upload[i])
		{
			qDebug() << Q_FUNC_INFO << "add to upload hander: "
				<< i
				<< attachItem.uuid() 
				<< attachItem.messageType() << attachItem.from()
				<< attachItem.transferType() << attachItem.filepath();
			return;
		}

		if (ftHandler == m_download[i])
		{
			qDebug() << Q_FUNC_INFO << "add to download hander: "
				<< i
				<< attachItem.uuid() 
				<< attachItem.messageType() << attachItem.from()
				<< attachItem.transferType() << attachItem.filepath();
			return;
		}
	}

	if (ftHandler == m_imminentUpload)
	{
		qDebug() << Q_FUNC_INFO << "add to imminent upload hander: "
			<< attachItem.uuid() 
			<< attachItem.messageType() << attachItem.from()
			<< attachItem.transferType() << attachItem.filepath();
		return;
	}

	if (ftHandler == m_imminentDownload)
	{
		qDebug() << Q_FUNC_INFO << "add to imminent download hander: "
			<< attachItem.uuid() 
			<< attachItem.messageType() << attachItem.from()
			<< attachItem.transferType() << attachItem.filepath();
		return;
	}

	qWarning() << Q_FUNC_INFO << " no ftHandler";
}

FTHandler *CAttachTransferMgr::createFTHandler(bool useFastdfs)
{
	FTHandler *handler = 0;
	if (!useFastdfs)
	{
		handler = new FileServFTHandler(*this, this);
	}
	else
	{
		handler = new FastdfsFTHandler(*this, this);
	}
	return handler;
}