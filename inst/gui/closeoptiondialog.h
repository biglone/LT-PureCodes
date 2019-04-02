#ifndef CLOSEOPTIONDIALOG_H
#define CLOSEOPTIONDIALOG_H

#include "framelessdialog.h"
namespace Ui {class CloseOptionDialog;};

class CloseOptionDialog : public FramelessDialog
{
	Q_OBJECT

public:
	CloseOptionDialog(QWidget *parent = 0);
	~CloseOptionDialog();

	void setCloseHide(bool closeHide);

	bool isCloseHide() const;
	bool isRemind() const;

public slots:
	virtual void setSkin();

private:
	Ui::CloseOptionDialog *ui;
};

#endif // CLOSEOPTIONDIALOG_H
