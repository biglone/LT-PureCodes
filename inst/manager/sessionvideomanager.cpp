#include "sessionvideomanager.h"
#include "widgetmanager.h"
#include "sessionvideodialog.h"

SessionVideoManager::SessionVideoManager(QObject *parent /*= 0*/)
	: QObject(parent)
{
}

SessionVideoManager::~SessionVideoManager()
{
	foreach (QPointer<SessionVideoDialog> videoDialog, m_videoDialogs.values())
	{
		if (!videoDialog.isNull())
		{
			videoDialog.data()->deleteLater();
		}
	}
	m_videoDialogs.clear();
}

void SessionVideoManager::startVideo(const QString &id, rtcsession::Session *pSession)
{
	QPointer<SessionVideoDialog> videoDialog = fetchVideoDialog(id, pSession);
	if (!videoDialog.isNull())
	{
		WidgetManager::showActivateRaiseWindow(videoDialog.data());
	}
}

bool SessionVideoManager::hasVideoSession(const QString &id)
{
	if (m_videoDialogs.contains(id) && !m_videoDialogs.value(id).isNull())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SessionVideoManager::closeAllVideoSessions()
{
	foreach (QPointer<SessionVideoDialog> videoDialog, m_videoDialogs.values())
	{
		if (!videoDialog.isNull())
		{
			videoDialog.data()->close();
		}
	}
	m_videoDialogs.clear();
}

SessionVideoDialog *SessionVideoManager::videoDialog(const QString &id) const
{
	if (m_videoDialogs.contains(id))
	{
		return m_videoDialogs[id];
	}
	return 0;
}

QPointer<SessionVideoDialog> SessionVideoManager::fetchVideoDialog(const QString &id, rtcsession::Session *pSession)
{
	if (m_videoDialogs.contains(id) && !m_videoDialogs.value(id).isNull())
	{
		return m_videoDialogs.value(id);
	}

	SessionVideoDialog *videoDialog = new SessionVideoDialog(id, pSession);
	QPointer<SessionVideoDialog> pVideoDialog(videoDialog);
	m_videoDialogs[id] = pVideoDialog;

	return pVideoDialog;
}
