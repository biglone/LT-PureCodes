#ifndef ATTACHTRANSFERMGR_H
#define ATTACHTRANSFERMGR_H

#include <QObject>
#include "fthandler.h"
#include "base/Base.h"
#include "bean/attachitem.h"

class CAttachReply;

class CAttachTransferMgr : public QObject, public FTHandlerDelegate
{
	Q_OBJECT

public:
	explicit CAttachTransferMgr(QObject *parent = 0);
	~CAttachTransferMgr();

	QString downloadDirPath() const { return m_downloadDir; }
	bool useFastdfs() const { return m_useFastdfs; }

Q_SIGNALS:
	void preUpload(CAttachReply *reply, const QString &jobId);
	void preDownload(CAttachReply *reply, const QString &jobId);
	void preImminentUpload(CAttachReply *reply, const QString &jobId);
	void preImminentDownload(CAttachReply *reply, const QString &jobId);

public:
	bool init(bool useFastdfs, const QString &addressStr);
	void release();

	void setDownloadDir(const QString &rsDownloadDir);

public:
	CAttachReply *addUpload(const bean::AttachItem &attachItem);
	CAttachReply *addDownload(const bean::AttachItem &attachItem, bool continuous = false);

	CAttachReply *addImminentUpload(const bean::AttachItem &attachItem);
	CAttachReply *addImminentDownload(const bean::AttachItem &attachItem, bool continuous = false);

	bool cancelUpload(const bean::AttachItem &attachItem);
	bool cancelDownload(const bean::AttachItem &attachItem);

	bool cancelImminentUpload(const bean::AttachItem &attachItem);
	bool cancelImminentDownload(const bean::AttachItem &attachItem);

public: // FUNCTIONS FROM FTHandlerDelegate
	virtual void preAddJob(FTHandler *ftHandler, FTJob *job);
	virtual void postAddJob(FTHandler *ftHandler, FTJob *job);

private:
	FTHandler *createFTHandler(bool useFastdfs);

private:
	static const int kMaxFileTransferHandlerCount = 2;

	FTHandler   *m_upload[kMaxFileTransferHandlerCount];
	FTHandler   *m_download[kMaxFileTransferHandlerCount];
	FTHandler   *m_imminentUpload;
	FTHandler   *m_imminentDownload;

	QString      m_downloadDir;

	bool         m_useFastdfs;
};

#endif // ATTACHTRANSFERMGR_H
