#ifndef MSGMULTISENDMEMBERDLG_H
#define MSGMULTISENDMEMBERDLG_H

#include "framelessdialog.h"

extern const int KMaxSendCount;

namespace Ui {class MsgMultiSendMemberDlg;};

class MsgMultiSendMemberDlg : public FramelessDialog
{
	Q_OBJECT

public:
	MsgMultiSendMemberDlg(const QStringList &members, QWidget *parent = 0);
	~MsgMultiSendMemberDlg();

	QStringList memberIds() const;

public Q_SLOTS:
	void setSkin();

	void accept();

private Q_SLOTS:
	void onSelectionChanged();
	void onMaximizeStateChanged(bool isMaximized);

private:
	Ui::MsgMultiSendMemberDlg *ui;
};

#endif // MSGMULTISENDMEMBERDLG_H
