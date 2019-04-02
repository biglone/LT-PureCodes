#ifndef GLOBALNOTIFICATIONDETAILDIALOG_H
#define GLOBALNOTIFICATIONDETAILDIALOG_H

#include "framelessdialog.h"

namespace Ui {class GlobalNotificationDetailDialog;};

class GlobalNotificationDetailDialog : public FramelessDialog
{
	Q_OBJECT

public:
	GlobalNotificationDetailDialog(const QString &globalNotificationId, QWidget *parent = 0);
	~GlobalNotificationDetailDialog();

public slots:
	void setSkin();
	void onUnsubscribeFailed();

Q_SIGNALS:
	void openGlobalNotificationHistory(const QString &globalNotificationId);
	void openGlobalNotificationMsg(const QString &globalNotificationId);

protected:
	void paintEvent(QPaintEvent *ev);

private slots:
	void on_pushButtonUnfollow_clicked();
	void on_pushButtonHistory_clicked();
	void on_pushButtonMsg_clicked();
	void onLogoChanged(const QString &globalNotificationId);

private:
	void initUI();
	void setLogoLabel();

private:
	Ui::GlobalNotificationDetailDialog *ui;

	QString m_globalNotificationId;
};

#endif // GLOBALNOTIFICATIONDETAILDIALOG_H
