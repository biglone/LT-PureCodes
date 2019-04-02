#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "framelessdialog.h"
namespace Ui {class RegisterDialog;};

class RegisterDialog : public FramelessDialog
{
	Q_OBJECT

public:
	RegisterDialog(QWidget *parent = 0);
	~RegisterDialog();

public Q_SLOTS:
	virtual void setSkin();

private Q_SLOTS:
	void on_tBtnMoreJoin_clicked();
	void on_tBtnMoreFrom_clicked();
	void on_tBtnMoreCountry_clicked();
	
	void onOKClicked();

	void onRegisterOK();
	void onRegisterFailed(int retCode, const QString &desc);

	void onTextEdited();

private:
	void initUI();
	void beginRegister();
	void endRegister();

private:
	Ui::RegisterDialog *ui;

	QString m_countryCode;
	QString m_toJoinCompanyId;
	QString m_toJoinCompanyName;
};

#endif // REGISTERDIALOG_H
