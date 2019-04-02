#ifndef PASSWDMODIFYDLG_H
#define PASSWDMODIFYDLG_H

#include "framelessdialog.h"
#include <QPointer>

class PasswdModifyManager;

namespace Ui 
{
	class PasswdModifyDlg;
};

class PasswdModifyDlg : public FramelessDialog
{
	Q_OBJECT

public:
	explicit PasswdModifyDlg(QWidget *parent = 0);
	~PasswdModifyDlg();

public slots:
	virtual void setSkin();

private slots:
	void on_btnOk_clicked();

	void on_btnCancel_clicked();

	void on_leditOld_textChanged(const QString &text);

	void on_leditNew_textChanged(const QString &text);

	void on_leditConfirm_textChanged(const QString &text);

	void onPasswdModifyFailed(const QString &rsError);

	void onPasswdModifyOK();

private:
	bool checkPasswd(const QString &rsPasswd);
	void beginModifyTransaction();
	void endModifyTransaction();

private:
	Ui::PasswdModifyDlg          *ui;
	QPointer<PasswdModifyManager> m_passwdModifyManager;
};

#endif // PASSWDMODIFYDLG_H
