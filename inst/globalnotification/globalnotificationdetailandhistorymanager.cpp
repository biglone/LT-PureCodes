#include "globalnotificationdetailandhistorymanager.h"
#include "widgetmanager.h"
#include "globalnotificationdetaildialog.h"
#include "globalnotificationhistorydialog.h"
#include "pmessagebox.h"

GlobalNotificationDetailAndHistoryManager::GlobalNotificationDetailAndHistoryManager(QObject *parent /*= 0*/)
	: QObject(parent)
{

}

GlobalNotificationDetailAndHistoryManager::~GlobalNotificationDetailAndHistoryManager()
{

}

bool GlobalNotificationDetailAndHistoryManager::hasGlobalNotificationDetailDialog(const QString &globalNotificationId)
{
	bool hasDetail = false;
	if (m_detailDialogs.contains(globalNotificationId))
	{
		if (!m_detailDialogs[globalNotificationId].isNull())
		{
			hasDetail = true;
		}
	}
	return hasDetail;
}

GlobalNotificationDetailDialog *GlobalNotificationDetailAndHistoryManager::globalNotificationDetailDialog(const QString &globalNotificationId)
{
	GlobalNotificationDetailDialog *dlg = 0;
	if (hasGlobalNotificationDetailDialog(globalNotificationId))
	{
		dlg = m_detailDialogs[globalNotificationId].data();
	}
	return dlg;
}

bool GlobalNotificationDetailAndHistoryManager::hasGlobalNotificationHistoryDialog(const QString &globalNotificationId)
{
	bool hasHistory = false;
	if (m_historyDialogs.contains(globalNotificationId))
	{
		if (!m_historyDialogs[globalNotificationId].isNull())
		{
			hasHistory = true;
		}
	}
	return hasHistory;
}

GlobalNotificationHistoryDialog *GlobalNotificationDetailAndHistoryManager::globalNotificationHistoryDialog(const QString &globalNotificationId)
{
	GlobalNotificationHistoryDialog *dlg = 0;
	if (hasGlobalNotificationHistoryDialog(globalNotificationId))
	{
		dlg = m_historyDialogs[globalNotificationId].data();
	}
	return dlg;
}

void GlobalNotificationDetailAndHistoryManager::openGlobalNotificationDetail(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	GlobalNotificationDetailDialog *dlg = globalNotificationDetailDialog(globalNotificationId);
	if (!dlg)
	{
		dlg = createDetailDialog(globalNotificationId);
	}
	WidgetManager::showActivateRaiseWindow(dlg);
}

void GlobalNotificationDetailAndHistoryManager::openGlobalNotificationHistory(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	GlobalNotificationHistoryDialog *dlg = globalNotificationHistoryDialog(globalNotificationId);
	if (!dlg)
	{
		dlg = createHistoryDialog(globalNotificationId);
	}
	WidgetManager::showActivateRaiseWindow(dlg);
}

void GlobalNotificationDetailAndHistoryManager::onGlobalNotificationUnsubscribed(bool ok, const QString &globalNotificationId)
{
	if (ok)
	{
		GlobalNotificationDetailDialog *detailDlg = globalNotificationDetailDialog(globalNotificationId);
		if (detailDlg)
		{
			detailDlg->close();
			m_detailDialogs.remove(globalNotificationId);
		}

		GlobalNotificationHistoryDialog *historyDlg = globalNotificationHistoryDialog(globalNotificationId);
		if (historyDlg)
		{
			historyDlg->close();
			m_historyDialogs.remove(globalNotificationId);
		}
	}
}

GlobalNotificationDetailDialog *GlobalNotificationDetailAndHistoryManager::createDetailDialog(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return 0;

	QPointer<GlobalNotificationDetailDialog> dlg = new GlobalNotificationDetailDialog(globalNotificationId);
	m_detailDialogs.insert(globalNotificationId, dlg);
	connect(dlg.data(), SIGNAL(openGlobalNotificationMsg(QString)), this, SIGNAL(openGlobalNotificationMsg(QString)));
	connect(dlg.data(), SIGNAL(openGlobalNotificationHistory(QString)), this, SLOT(openGlobalNotificationHistory(QString)));
	return dlg.data();
}

GlobalNotificationHistoryDialog *GlobalNotificationDetailAndHistoryManager::createHistoryDialog(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return 0;

	QPointer<GlobalNotificationHistoryDialog> dlg = new GlobalNotificationHistoryDialog(globalNotificationId);
	connect(dlg.data(), SIGNAL(openTitle(QString, QString, QString)), this, SIGNAL(openTitle(QString, QString, QString)));
	connect(dlg.data(), SIGNAL(openAttach(QString, QString, QString)), this, SIGNAL(openAttach(QString, QString, QString)));
	connect(dlg.data(), SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(dlg.data(), SIGNAL(openGlobalNotificationDetail(QString)), this, SLOT(openGlobalNotificationDetail(QString)));
	m_historyDialogs.insert(globalNotificationId, dlg);
	return dlg.data();
}