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

	// job由用户new，线程delete，如果加入的job的uuid已经存在，则会返回原来的job
	FTJob *addJob(FTJob *job);

	// jobId即为job->uuid()
	bool cancelJob(const QString &jobId);

	// 所有job的个数，包括当前还在进行的任务
	int jobCount();

	// 所有job的id，包括当前还在进行的任务
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
	QList<FTJob *>                  m_listJob;  // 用户加,线程取走
	QMap<QString, FTJob *>          m_mapJob;   // 用户加,用户改,线程erase
	QMutex                          m_mutex;    // 锁list,map
	QSemaphore                      m_semaphone;// 线程等待,用户激活
	QString                         m_curJobId;
	QString                         m_address;
	volatile bool                   m_exit; 
	FTHandlerDelegate              &m_delegate;
};

#endif // FTHANDLER_H
