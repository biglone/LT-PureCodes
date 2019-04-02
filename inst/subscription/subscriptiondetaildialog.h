#ifndef SUBSCRIPTIONDETAILDIALOG_H
#define SUBSCRIPTIONDETAILDIALOG_H

#include "framelessdialog.h"

namespace Ui {class SubscriptionDetailDialog;};

class SubscriptionDetailDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SubscriptionDetailDialog(const QString &subscriptionId, QWidget *parent = 0);
	~SubscriptionDetailDialog();

public slots:
	void setSkin();
	void onUnsubscribeFailed();

Q_SIGNALS:
	void openSubscriptionHistory(const QString &subscriptionId);
	void openSubscriptionMsg(const QString &subscriptionId);

protected:
	void paintEvent(QPaintEvent *ev);

private slots:
	void on_pushButtonUnfollow_clicked();
	void on_pushButtonHistory_clicked();
	void on_pushButtonMsg_clicked();
	void onLogoChanged(const QString &subscriptionId);

private:
	void initUI();
	void setLogoLabel();

private:
	Ui::SubscriptionDetailDialog *ui;

	QString m_subscriptionId;
};

#endif // SUBSCRIPTIONDETAILDIALOG_H
