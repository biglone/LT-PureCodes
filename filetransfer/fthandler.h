#ifndef FTHANDLER_H
#define FTHANDLER_H

#include "filetransfer_global.h"
#include <QThread>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QSemaphore>
#include "ftjob.h"

class FTHandler;

class FILETRANSFER_EXPORT FTHandlerDelegate
{
public:
	virtual void preAddJob(FTHandler *ftHandler, FTJob *job) = 0;
	virtual void postAddJob(FTHandler *ftHandler, FTJob *job) = 0;
};

class FILETRANSFER_EXPORT FTHandler : public QThread
{
	Q_OBJECT

public:
	FTHandler(FTHandlerDelegate &delegate, QObject *parent = 0);
	virtual ~FTHandler();

public:
	bool init(const QString &address);
	void release();

	// job���û�new���߳�delete����������job��uuid�Ѿ����ڣ���᷵��ԭ����job
	FTJob *addJob(FTJob *job);

	// jobId��Ϊjob->uuid()
	bool cancelJob(const QString &jobId);

	// ����job�ĸ�����������ǰ���ڽ��е�����
	int jobCount();

	// ����job��id��������ǰ���ڽ��е�����
	QStringList jobIds();

protected:
	void run();

protected:
	virtual void upload(FTJob *job) = 0;
	virtual void download(FTJob *job) = 0;
	virtual void closeConnection() = 0;
	virtual void doInit() {}
	virtual void doRelease() {}

private:
	FTJob *fetchJob();
	void deleteJob(FTJob *job);
	volatile bool isExitSet() const {return m_exit;}

protected:
	QList<FTJob *>                  m_listJob;  // �û���,�߳�ȡ��
	QMap<QString, FTJob *>          m_mapJob;   // �û���,�û���,�߳�erase
	QMutex                          m_mutex;    // ��list,map
	QSemaphore                      m_semaphone;// �̵߳ȴ�,�û�����
	QString                         m_curJobId;
	QString                         m_address;
	volatile bool                   m_exit; 
	FTHandlerDelegate              &m_delegate;
};

#endif // FTHANDLER_H
