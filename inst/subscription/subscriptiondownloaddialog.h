#ifndef SUBSCRIPTIONDOWNLOADDIALOG_H
#define SUBSCRIPTIONDOWNLOADDIALOG_H

#include "FramelessDialog.h"
#include <QPointer>

namespace Ui {class SubscriptionDownloadDialog;};

class SubscriptionDownloadDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SubscriptionDownloadDialog(QWidget *parent = 0);
	~SubscriptionDownloadDialog();

	static SubscriptionDownloadDialog *getDialog();

	void addDownload(const QString &urlStr, const QString &name);

public slots:
	void setSkin();

protected:
	void closeEvent(QCloseEvent *e);

private:
	void initUI();

private:
	Ui::SubscriptionDownloadDialog *ui;

	static QPointer<SubscriptionDownloadDialog> s_dialog;
};

#endif // SUBSCRIPTIONDOWNLOADDIALOG_H
