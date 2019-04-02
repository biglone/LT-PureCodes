#ifndef REMOVEFILEOPTIONDIALOG_H
#define REMOVEFILEOPTIONDIALOG_H

#include "framelessdialog.h"

namespace Ui {class RemoveFileOptionDialog;};

class RemoveFileOptionDialog : public FramelessDialog
{
	Q_OBJECT

public:
	RemoveFileOptionDialog(QWidget *parent = 0);
	~RemoveFileOptionDialog();

	bool isRemoveFile() const;

public slots:
	virtual void setSkin();

private:
	Ui::RemoveFileOptionDialog *ui;
};

#endif // REMOVEFILEOPTIONDIALOG_H
