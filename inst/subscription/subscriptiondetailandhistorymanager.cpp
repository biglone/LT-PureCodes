#include "subscriptiondetailandhistorymanager.h"
#include "widgetmanager.h"
#include "subscriptiondetaildialog.h"
#include "subscriptionhistorydialog.h"
#include "pmessagebox.h"

SubscriptionDetailAndHistoryManager::SubscriptionDetailAndHistoryManager(QObject *parent /*= 0*/)
	: QObject(parent)
{

}

SubscriptionDetailAndHistoryManager::~SubscriptionDetailAndHistoryManager()
{

}

bool SubscriptionDetailAndHistoryManager::hasSubscriptionDetailDialog(const QString &subscriptionId)
{
	bool hasDetail = false;
	if (m_detailDialogs.contains(subscriptionId))
	{
		if (!m_detailDialogs[subscriptionId].isNull())
		{
			hasDetail = true;
		}
	}
	return hasDetail;
}

SubscriptionDetailDialog *SubscriptionDetailAndHistoryManager::subscriptionDetailDialog(const QString &subscriptionId)
{
	SubscriptionDetailDialog *dlg = 0;
	if (hasSubscriptionDetailDialog(subscriptionId))
	{
		dlg = m_detailDialogs[subscriptionId].data();
	}
	return dlg;
}

bool SubscriptionDetailAndHistoryManager::hasSubscriptionHistoryDialog(const QString &subscriptionId)
{
	bool hasHistory = false;
	if (m_historyDialogs.contains(subscriptionId))
	{
		if (!m_historyDialogs[subscriptionId].isNull())
		{
			hasHistory = true;
		}
	}
	return hasHistory;
}

SubscriptionHistoryDialog *SubscriptionDetailAndHistoryManager::subscriptionHistoryDialog(const QString &subscriptionId)
{
	SubscriptionHistoryDialog *dlg = 0;
	if (hasSubscriptionHistoryDialog(subscriptionId))
	{
		dlg = m_historyDialogs[subscriptionId].data();
	}
	return dlg;
}

void SubscriptionDetailAndHistoryManager::openSubscriptionDetail(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	SubscriptionDetailDialog *dlg = subscriptionDetailDialog(subscriptionId);
	if (!dlg)
	{
		dlg = createDetailDialog(subscriptionId);
	}
	WidgetManager::showActivateRaiseWindow(dlg);
}

void SubscriptionDetailAndHistoryManager::openSubscriptionHistory(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	SubscriptionHistoryDialog *dlg = subscriptionHistoryDialog(subscriptionId);
	if (!dlg)
	{
		dlg = createHistoryDialog(subscriptionId);
	}
	WidgetManager::showActivateRaiseWindow(dlg);
}

void SubscriptionDetailAndHistoryManager::onSubscriptionUnsubscribed(bool ok, const QString &subscriptionId)
{
	if (ok)
	{
		SubscriptionDetailDialog *detailDlg = subscriptionDetailDialog(subscriptionId);
		if (detailDlg)
		{
			detailDlg->close();
			m_detailDialogs.remove(subscriptionId);
		}

		SubscriptionHistoryDialog *historyDlg = subscriptionHistoryDialog(subscriptionId);
		if (historyDlg)
		{
			historyDlg->close();
			m_historyDialogs.remove(subscriptionId);
		}
	}
}

SubscriptionDetailDialog *SubscriptionDetailAndHistoryManager::createDetailDialog(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return 0;

	QPointer<SubscriptionDetailDialog> dlg = new SubscriptionDetailDialog(subscriptionId);
	m_detailDialogs.insert(subscriptionId, dlg);
	connect(dlg.data(), SIGNAL(openSubscriptionMsg(QString)), this, SIGNAL(openSubscriptionMsg(QString)));
	connect(dlg.data(), SIGNAL(openSubscriptionHistory(QString)), this, SLOT(openSubscriptionHistory(QString)));
	return dlg.data();
}

SubscriptionHistoryDialog *SubscriptionDetailAndHistoryManager::createHistoryDialog(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return 0;

	QPointer<SubscriptionHistoryDialog> dlg = new SubscriptionHistoryDialog(subscriptionId);
	connect(dlg.data(), SIGNAL(openTitle(QString, QString, QString)), this, SIGNAL(openTitle(QString, QString, QString)));
	connect(dlg.data(), SIGNAL(openAttach(QString, QString, QString)), this, SIGNAL(openAttach(QString, QString, QString)));
	connect(dlg.data(), SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(dlg.data(), SIGNAL(openSubscriptionDetail(QString)), this, SLOT(openSubscriptionDetail(QString)));
	m_historyDialogs.insert(subscriptionId, dlg);
	return dlg.data();
}