#ifndef SUBSCRIPTIONDETAILANDHISTORYMANAGER_H
#define SUBSCRIPTIONDETAILANDHISTORYMANAGER_H

#include <QObject>
#include <QPointer>
#include <QMap>

class SubscriptionDetailDialog;
class SubscriptionHistoryDialog;

class SubscriptionDetailAndHistoryManager : public QObject
{
	Q_OBJECT

public:
	SubscriptionDetailAndHistoryManager(QObject *parent = 0);
	~SubscriptionDetailAndHistoryManager();

	bool hasSubscriptionDetailDialog(const QString &subscriptionId);
	SubscriptionDetailDialog *subscriptionDetailDialog(const QString &subscriptionId);

	bool hasSubscriptionHistoryDialog(const QString &subscriptionId);
	SubscriptionHistoryDialog *subscriptionHistoryDialog(const QString &subscriptionId);

Q_SIGNALS:
	void openSubscriptionMsg(const QString &subscriptionId);
	void openTitle(const QString &subscriptionId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name);
	void viewMaterial(const QString &uid);

public slots:
	void openSubscriptionDetail(const QString &subscriptionId);
	void openSubscriptionHistory(const QString &subscriptionId);
	void onSubscriptionUnsubscribed(bool ok, const QString &subscriptionId);

private:
	SubscriptionDetailDialog *createDetailDialog(const QString &subscriptionId);
	SubscriptionHistoryDialog *createHistoryDialog(const QString &subscriptionId);

private:
	QMap<QString, QPointer<SubscriptionDetailDialog>>  m_detailDialogs;
	QMap<QString, QPointer<SubscriptionHistoryDialog>> m_historyDialogs;
};

#endif // SUBSCRIPTIONDETAILANDHISTORYMANAGER_H
