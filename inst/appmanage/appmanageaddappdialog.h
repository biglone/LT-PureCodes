#ifndef APPMANAGEADDAPPDIALOG_H
#define APPMANAGEADDAPPDIALOG_H

#include "FramelessDialog.h"
namespace Ui {class AppManageAddAppDialog;};

class AppManageAddAppDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum Mode
	{
		AddMode,
		EditMode
	};

public:
	AppManageAddAppDialog(Mode mode = AddMode, QWidget *parent = 0);
	~AppManageAddAppDialog();

	QString appName() const;
	void setAppName(const QString &name);
	QString appPath() const;
	void setAppPath(const QString &path);

public slots:
	virtual void setSkin();
	virtual void accept();

private slots:
	void on_pushButtonBrowse_clicked();

private:
	void initUI();

private:
	Ui::AppManageAddAppDialog *ui;
	Mode m_mode;
};

#endif // APPMANAGEADDAPPDIALOG_H
