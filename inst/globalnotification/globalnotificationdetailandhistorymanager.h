#ifndef GLOBALNOTIFICATIONDETAILANDHISTORYMANAGER_H
#define GLOBALNOTIFICATIONDETAILANDHISTORYMANAGER_H

#include <QObject>
#include <QPointer>
#include <QMap>

class GlobalNotificationDetailDialog;
class GlobalNotificationHistoryDialog;

class GlobalNotificationDetailAndHistoryManager : public QObject
{
	Q_OBJECT

public:
	GlobalNotificationDetailAndHistoryManager(QObject *parent = 0);
	~GlobalNotificationDetailAndHistoryManager();

	bool hasGlobalNotificationDetailDialog(const QString &globalNotificationId);
	GlobalNotificationDetailDialog *globalNotificationDetailDialog(const QString &globalNotificationId);

	bool hasGlobalNotificationHistoryDialog(const QString &globalNotificationId);
	GlobalNotificationHistoryDialog *globalNotificationHistoryDialog(const QString &globalNotificationId);

Q_SIGNALS:
	void openGlobalNotificationMsg(const QString &globalNotificationId);
	void openTitle(const QString &globalNotificationId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name);
	void viewMaterial(const QString &uid);

public slots:
	void openGlobalNotificationDetail(const QString &globalNotificationId);
	void openGlobalNotificationHistory(const QString &globalNotificationId);
	void onGlobalNotificationUnsubscribed(bool ok, const QString &globalNotificationId);

private:
	GlobalNotificationDetailDialog *createDetailDialog(const QString &globalNotificationId);
	GlobalNotificationHistoryDialog *createHistoryDialog(const QString &globalNotificationId);

private:
	QMap<QString, QPointer<GlobalNotificationDetailDialog>>  m_detailDialogs;
	QMap<QString, QPointer<GlobalNotificationHistoryDialog>> m_historyDialogs;
};

#endif // GLOBALNOTIFICATIONDETAILANDHISTORYMANAGER_H
