#include "maxtsmanager.h"
#include "Account.h"
#include "offlinemsgmanager.h"

MaxTsManager::MaxTsManager(OfflineMsgManager &offlineMsgManager, QObject *parent /*= 0*/)
	: QObject(parent), m_offlineMsgManager(offlineMsgManager), m_setCount(0)
{
	m_setTimer.setInterval(180*1000); // 3 minutes
	m_setTimer.setSingleShot(false);
	connect(&m_setTimer, SIGNAL(timeout()), this, SLOT(onSetTimeout()));
}

MaxTsManager::~MaxTsManager()
{

}

void MaxTsManager::init(const QString &maxTs)
{
	m_maxTs = maxTs;

	m_setTimer.stop();
	m_setTimer.start();
}

void MaxTsManager::stop()
{
	m_maxTs.clear();

	m_setTimer.stop();
}

void MaxTsManager::setMaxTs(const QString &ts)
{
	if (ts.isEmpty())
	{
		return;
	}

	bool newValid = isTsValid(ts);
	if (!newValid)
	{
		return;
	}

	bool orignalValid = isTsValid(m_maxTs);
	if ((!orignalValid) || (ts > m_maxTs))
	{
		m_maxTs = ts;
		Account::settings()->setMaxMsgTs(m_maxTs);

		m_setCount++;

		/* // do not compute count to report ts
		if (m_setCount >= 20)
		{
			reportMaxTs();
		}
		*/
	}
}

QString MaxTsManager::maxTs() const
{
	return m_maxTs;
}

void MaxTsManager::reportMaxTs()
{
	if (m_setCount > 0)
	{
		// report max ts
		m_offlineMsgManager.reportMaxTs(m_maxTs);

		// reset state
		m_setCount = 0;
		m_setTimer.stop();
		m_setTimer.start();
	}
}

void MaxTsManager::onSetTimeout()
{
	reportMaxTs();
}

bool MaxTsManager::isTsValid(const QString &ts) const
{
	bool valid = false;

	if (ts.isEmpty())
	{
		return valid;
	}

	for (int i = 0; i < ts.length(); i++)
	{
		QChar c = ts[i];
		if (!c.isDigit())
		{
			return valid;
		}
	}

	valid = true;
	return valid;
}