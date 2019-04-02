#include "PmApp.h"
#include "statuschanger/StatusChanger.h"
#include "BaseProcessor.h"
#include "loginmgr.h"
#include "autostatus.h"

const int s_nDelay = 10 * 1000; // ms, 10s

CAutoStatus::CAutoStatus(QObject *parent)
	: QObject(parent)
	, m_bIsStart(false)
{
	m_statusTimer.setSingleShot(true);
	m_statusTimer.setInterval(s_nDelay);
	connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

CAutoStatus::~CAutoStatus()
{
}

void CAutoStatus::start()
{
	if (m_bIsStart)
		return;

	m_bIsStart = true;

	StatusChanger* pStatusChanger = qPmApp->getStatusChanger();
	m_nCurStatus = pStatusChanger->curStatus();

	disconnect(pStatusChanger, SIGNAL(statusChanged(int)), this, SLOT(onStatusChanged(int)));
	connect(pStatusChanger, SIGNAL(statusChanged(int)), this, SLOT(onStatusChanged(int)));

	disconnect(qPmApp->GetLoginMgr(), SIGNAL(loginError(QString)), this, SLOT(slot_loginError(QString)));
	connect(qPmApp->GetLoginMgr(), SIGNAL(loginError(QString)), this, SLOT(slot_loginError(QString)));
}

void CAutoStatus::stop()
{
	if (!m_bIsStart)
		return;

	m_statusTimer.stop();
	
	m_bIsStart = false;

	StatusChanger* pStatusChanger = qPmApp->getStatusChanger();
	disconnect(pStatusChanger, SIGNAL(statusChanged(int)), this, SLOT(onStatusChanged(int)));

	disconnect(qPmApp->GetLoginMgr(), SIGNAL(loginError(QString)), this, SLOT(slot_loginError(QString)));
}

void CAutoStatus::slot_loginError(const QString& /*error*/)
{
	m_statusTimer.start();
}

void CAutoStatus::onTimeout()
{
	if (qPmApp->getBaseProcessor()->isKicked())
		return;

	qPmApp->getStatusChanger()->setStatus(m_nCurStatus);
}

void CAutoStatus::onStatusChanged(int nStatus)
{
	if (qPmApp->GetLoginMgr()->isLogined())
		m_nCurStatus = nStatus;
}
