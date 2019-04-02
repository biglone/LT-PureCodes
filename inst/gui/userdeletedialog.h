#ifndef USERDELETEDIALOG_H
#define USERDELETEDIALOG_H

#include "framelessdialog.h"
namespace Ui {class UserDeleteDialog;};

class UserDeleteDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum DeleteOption
	{
		DeleteUser,
		DeleteAll
	};

public:
	UserDeleteDialog(const QString &user, QWidget *parent = 0);
	~UserDeleteDialog();

	DeleteOption deleteOption() const;

public slots:
	void setSkin();

private slots:
	void on_pushButtonOK_clicked();
	void on_pushButtonCancel_clicked();
	void on_btnClose_clicked();

private:
	Ui::UserDeleteDialog *ui;
};

#endif // USERDELETEDIALOG_H
