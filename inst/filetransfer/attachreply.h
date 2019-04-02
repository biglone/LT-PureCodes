#ifndef ATTACHREPLY_H
#define ATTACHREPLY_H

#include <QObject>
#include "bean/attachitem.h"
#include "ftjob.h"
#include "util/ZipUtil.h"

class CAttachReply : public QObject, public FTJob, public ZipDelegate
{
	Q_OBJECT

public:
	CAttachReply(Type eType, const bean::AttachItem &item, bool continuous = false, QObject *parent = 0);
	virtual ~CAttachReply();

public:
	bean::AttachItem &attachItem() { return m_attachItem; }
	QString typeName() const;
	static QString typeName(FTJob::Type typeVal);
	void setTransferPerKb(int nPerKb = 1) { m_nTransferPreKb = nPerKb; }
	void setError(bean::AttachItem::TransferResult eResult, const QString &rsError);
	void setUseFastdfs(bool use);
	bool useFastdfs() const;

Q_SIGNALS:
	void error(const QString &rsUuid, const QString &rsType, int nResult, const QString &rsError, int msgType);
	void progress(const QString &rsUuid, const QString &rsType, int nKb, int nKbTotal);
	void finish(const QString &rsUuid, const QString &rsType, int nResult, int msgType);
	void downloadChanged(const QString &rsUuid, const QString &filePath);

protected: // from FTJob
	virtual FTJob::Type type();
	virtual bool isContinuous();
	virtual QFile* file();
	virtual QString transferName();
	virtual QString uuid();
	virtual QString userId();
	virtual int transferPerKb();
	virtual bool transferKb(int nKb, int nKbTotal);
	virtual void transferOver(OverCode oc, const QString &param = QString());
	virtual FTJob::OverCode preTransfer();
	virtual FTJob::OverCode postTransfer();

protected: // from ZipDelegate
	virtual bool shouldZipContinue();

private:
	QString dirZipFilePath();

private:
	FTJob::Type        m_eType;
	bean::AttachItem   m_attachItem;
	int                m_nTransferPreKb;
	QFile             *m_pFile;
	bool               m_bContinuous;
	bool               m_useFastdfs;
};

#endif // ATTACHREPLY_H
