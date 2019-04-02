#ifndef GLOBALNOTIFICATIONDOWNLOADDIALOG_H
#define GLOBALNOTIFICATIONDOWNLOADDIALOG_H

#include "FramelessDialog.h"
#include <QPointer>

namespace Ui {class GlobalNotificationDownloadDialog;};

class GlobalNotificationDownloadDialog : public FramelessDialog
{
	Q_OBJECT

public:
	GlobalNotificationDownloadDialog(QWidget *parent = 0);
	~GlobalNotificationDownloadDialog();

	static GlobalNotificationDownloadDialog *getDialog();

	void addDownload(const QString &urlStr, const QString &name);

public slots:
	void setSkin();

protected:
	void closeEvent(QCloseEvent *e);

private:
	void initUI();

private:
	Ui::GlobalNotificationDownloadDialog *ui;

	static QPointer<GlobalNotificationDownloadDialog> s_dialog;
};

#endif // GLOBALNOTIFICATIONDOWNLOADDIALOG_H
