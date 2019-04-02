#include <QDebug>
#include "login/Account.h"
#include "model/ModelManager.h"
#include "pmclient/PmClient.h"
#include "PsgManager.h"
#include "PmApp.h"
#include "manager/presencemanager.h"
#include "StatusChanger.h"

StatusChanger::StatusChanger(QObject *parent)
	: QObject(parent)
	, m_bStartChanged(false)
	, m_eCurStatus(Status_Offline)
	, m_ePreStatus(Status_Offline)
{
}

StatusChanger::~StatusChanger()
{
}

void StatusChanger::reset()
{
	m_bStartChanged  = false;
	m_eCurStatus     = Status_Offline;
	m_ePreStatus     = Status_Offline;
}

QString StatusChanger::status2Str(StatusChanger::Status status)
{
	QString str = "offline";
	switch (status)
	{
	case Status_Online:
		str = "online";
		break;
	case Status_Away:
		str = "away";
		break;
	case Status_Chat:
		str = "chat";
		break;
	case Status_Dnd:
		str = "dnd";
		break;
	case Status_Xa:
		str = "xa";
		break;
	case Status_Offline:
		str = "offline";
		break;
	default:
		break;
	}
	return str;
}

StatusChanger::Status StatusChanger::str2Status(const QString &str)
{
	StatusChanger::Status status = Status_Offline;
	if (str == "online")
	{
		status = Status_Online;
	}
	else if (str == "away")
	{
		status = Status_Away;
	}
	else if (str == "chat")
	{
		status = Status_Chat;
	}
	else if (str == "dnd")
	{
		status = Status_Dnd;
	}
	else if (str == "xa")
	{
		status = Status_Xa;
	}
	else if (str == "offline")
	{
		status = Status_Offline;
	}
	return status;
}

StatusChanger::Status StatusChanger::curStatus() const
{
	return m_eCurStatus;
}

void StatusChanger::setStatus(int status)
{
	Status s = (Status)status;
	setStatus(s);
}

void StatusChanger::setStatus(Status eStatus)
{
	if (eStatus == m_eCurStatus)
		return;

	if (m_bStartChanged)
	{
		qDebug() << Q_FUNC_INFO << "status is changing..., current is: " << status2Str(m_eCurStatus);
		return;
	}

	qDebug() << Q_FUNC_INFO << "change status from: " << status2Str(m_eCurStatus) << " to: " << status2Str(eStatus);

	m_bStartChanged = true;

	if (m_eCurStatus == Status_Offline)
	{
		if (qPmApp->GetLoginMgr()->isLogined())
		{
			sendPresence(eStatus);
		}
		else
		{	
			// login
			startLogin();

			m_ePreStatus = eStatus;
		}
	}
	else
	{
		if (eStatus == Status_Offline)
		{
			if (qPmApp->GetLoginMgr()->isLogined())
			{
				sendPresence(eStatus);

				// logout
				qPmApp->GetLoginMgr()->logout();
			}
		}
		else
		{
			// change to other presence status
			sendPresence(eStatus);
		}
	}
}

void StatusChanger::onPresenceReceived(const QString &id)
{
	Account *account = qPmApp->getAccount();
	QString accountId = account->id();
	if (id == accountId) // from self
	{
		emit statusChanged(m_eCurStatus);
	}
}

void StatusChanger::onLoginMgrLogined()
{
	sendPresence(m_ePreStatus);
}

void StatusChanger::onLoginMgrLogouted()
{
	m_eCurStatus = Status_Offline;
	m_bStartChanged = false;
	emit statusChanged(m_eCurStatus);
}

void StatusChanger::onLoginMgrLoginError(const QString& err)
{
	Q_UNUSED(err);

	m_eCurStatus = Status_Offline;
	m_bStartChanged = false;
	emit statusChanged(m_eCurStatus);
}

void StatusChanger::startLogin()
{
	CLoginMgr *loginMgr = qPmApp->GetLoginMgr();

	disconnect(loginMgr, SIGNAL(logouted()), this, SLOT(onLoginMgrLogouted()));
	disconnect(loginMgr, SIGNAL(loginError(QString)), this, SLOT(onLoginMgrLoginError(QString)));

	connect(loginMgr, SIGNAL(logouted()), this, SLOT(onLoginMgrLogouted()));
	connect(loginMgr, SIGNAL(loginError(QString)), this, SLOT(onLoginMgrLoginError(QString)));

	loginMgr->login();
}

void StatusChanger::sendPresence(Status eStatus)
{
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	presenceManager->sendPresence(eStatus);

	m_eCurStatus = eStatus;
	qDebug() << Q_FUNC_INFO << "current status: " << status2Str(m_eCurStatus) << "original changed: " << m_bStartChanged;
	m_bStartChanged = false;
}

