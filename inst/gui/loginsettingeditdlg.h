#ifndef LOGINSETTINGEDITDLG_H
#define LOGINSETTINGEDITDLG_H

#include "framelessdialog.h"
namespace Ui {class LoginSettingEditDlg;};

class LoginSettingEditDlg : public FramelessDialog
{
	Q_OBJECT

public:
	LoginSettingEditDlg(const QString &settingName, QWidget *parent = 0);
	~LoginSettingEditDlg();

	void setManagerAddress(const QString &managerAddress);
	QString managerAddress() const;
	

public slots:
	void setSkin();

private:
	Ui::LoginSettingEditDlg *ui;
};

#endif // LOGINSETTINGEDITDLG_H
