#ifndef UNIONCHATCLOSETIPDIALOG_H
#define UNIONCHATCLOSETIPDIALOG_H

#include "framelessdialog.h"
namespace Ui {class UnionChatCloseTipDialog;};

class UnionChatCloseTipDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum CloseResult
	{
		CloseAll,
		CloseCurrent
	};

public:
	UnionChatCloseTipDialog(QWidget *parent = 0);
	~UnionChatCloseTipDialog();

	CloseResult closeResult() const;
	bool isAlwaysCloseAllChecked() const;

public slots:
	void setSkin();
	void accept();

private slots:
	void on_checkBoxCloseAll_toggled(bool checked);

private:
	Ui::UnionChatCloseTipDialog *ui;

	CloseResult m_closeResult;
};

#endif // UNIONCHATCLOSETIPDIALOG_H
