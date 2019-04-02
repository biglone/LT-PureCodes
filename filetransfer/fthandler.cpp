#include "fthandler.h"
#include <QDebug>

FTHandler::FTHandler(FTHandlerDelegate &delegate, QObject *parent)
	: QThread(parent), m_delegate(delegate), m_exit(false)
{

}

FTHandler::~FTHandler()
{
}

bool FTHandler::init(const QString &address)
{
	m_address = address;
	m_exit = false;
	doInit();
	start();
	return true;
}

void FTHandler::release()
{
	doRelease();

	m_exit = true;
	if (!m_curJobId.isEmpty())
	{
		cancelJob(m_curJobId);
	}
	m_semaphone.release();
	wait();

	foreach (FTJob *job, m_mapJob.values())
	{
		job->transferOver(FTJob::OC_CANCEL); 
		delete job;
		job = 0;
	}
	m_listJob.clear();
	m_mapJob.clear();
}

FTJob *FTHandler::addJob(FTJob *job)
{
	Q_ASSERT(job);

	// this thread is not running
	if (isExitSet())
	{
		job->transferOver(FTJob::OC_FAIL);
		delete job;
		return 0;
	}

	m_mutex.lock();
	QString uuid = job->uuid();
	if (m_mapJob.contains(uuid))
	{
		// return the original job
		delete job;
		job = m_mapJob[uuid];
	}
	else
	{
		m_delegate.preAddJob(this, job);

		// add to job list
		m_listJob.append(job);
		m_mapJob[uuid] = job;

		m_delegate.postAddJob(this, job);
	}
	m_mutex.unlock();

	m_semaphone.release();

	return job;
}

bool FTHandler::cancelJob(const QString &jobId)
{
	bool canceled = false;
	bool delJob = false;
	FTJob *job = 0;

	m_mutex.lock();
	do 
	{
		if (!m_mapJob.contains(jobId))
			break;
		
		job = m_mapJob[jobId];
		job->cancel();
		
		if (jobId != m_curJobId)
		{
			int i = 0;
			for (i = 0; i < m_listJob.count(); ++i)
			{
				FTJob *j = m_listJob[i];
				if (j->uuid() == jobId)
					break;
			}
			if (i < m_listJob.count())
			{
				m_listJob.removeAt(i);
				m_mapJob.remove(jobId);
				delJob = true;
			}
		}
		canceled = true;
	} while(0);
	m_mutex.unlock();

	if (delJob)
	{
		job->transferOver(FTJob::OC_CANCEL); 
		delete job;
		job = 0;
	}

	return canceled;
}

int FTHandler::jobCount()
{
	int count = 0;
	m_mutex.lock();
	count = m_mapJob.count();
	m_mutex.unlock();
	return count;
}

QStringList FTHandler::jobIds()
{
	QStringList ids;
	m_mutex.lock();
	ids = m_mapJob.keys();
	m_mutex.unlock();
	return ids;
}

void FTHandler::run()
{
	FTJob *job = 0;
	while (!isExitSet())
	{
		// 等待任务
		m_semaphone.acquire();

		// 把队列中所有任务都执行完
		while (!isExitSet())
		{
			job = fetchJob();
			if (!job)
			{
				// 已经没有job了,这一次处理结束,继续等待,等待的时候结束连接
				closeConnection();
				break;
			}

			do 
			{
				if (job->isCancel())
				{
					// 在我处理之前用户已经取消了,则直接下一个job
					job->transferOver(FTJob::OC_CANCEL);
					break;
				}

				m_curJobId = job->uuid();

				switch (job->type())
				{
				case FTJob::UPLOAD:
					{
						upload(job);
					}
					break;
				case FTJob::DOWNLOAD:
					{
						download(job);
					}
					break;
				default:
					{
						Q_ASSERT(0);
					}
					break;
				}
			} while(0);

			deleteJob(job);
			m_curJobId = "";
		}
	}

	// 结束连接
	closeConnection();
}

FTJob *FTHandler::fetchJob()
{
	FTJob *job = 0;

	m_mutex.lock();
	if (!m_listJob.isEmpty())
	{
		job = m_listJob.takeFirst();
	}
	m_mutex.unlock();
	
	return job;
}

void FTHandler::deleteJob(FTJob *job)
{
	Q_ASSERT(job);

	m_mutex.lock();
	m_mapJob.remove(job->uuid());
	delete job;
	m_mutex.unlock();
}
