#ifndef GLOBALNOTIFICATIONHISTORYDIALOG_H
#define GLOBALNOTIFICATIONHISTORYDIALOG_H

#include "framelessdialog.h"
#include <QList>
#include "globalnotificationmsg.h"
namespace Ui {class GlobalNotificationHistoryDialog;};
class GlobalNotificationMsg4Js;
class GlobalNotificationMsgWebPage;

class GlobalNotificationHistoryDialog : public FramelessDialog
{
	Q_OBJECT

public:
	GlobalNotificationHistoryDialog(const QString &globalNotificationId, QWidget *parent = 0);
	~GlobalNotificationHistoryDialog();

Q_SIGNALS:
	void initUIComplete();
	void openTitle(const QString &globalNotificationId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name);
	void viewMaterial(const QString &uid);
	void openGlobalNotificationDetail(const QString &globalNotificationId);

public slots:
	void setSkin();

private slots:
	void populateJavaScriptWindowObject();
	void onMessage4JsLoadSucceeded();
	void fetchHistoryMsg();
	void onGetHistoryMessagesFinished(bool ok, const QString &globalNotificationId, const QList<GlobalNotificationMsg> &msgs);
	void showTip(const QString &tip);
	void on_pushButtonRetry_clicked();
	void onUserChanged(const QString &uid);
	void onLogoChanged(const QString &globalNotificationId);

private:
	void initUI();

private:
	Ui::GlobalNotificationHistoryDialog *ui;

	QString m_globalNotificationId;
	QString m_oldestSequence;

	GlobalNotificationMsg4Js *m_message4Js;

	GlobalNotificationMsgWebPage *m_webPage;
};

#endif // GLOBALNOTIFICATIONHISTORYDIALOG_H
