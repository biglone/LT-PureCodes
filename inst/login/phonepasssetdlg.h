#ifndef PHONEPASSSETDLG_H
#define PHONEPASSSETDLG_H

#include "framelessdialog.h"
#include <QTimer>
namespace Ui {class PhonePassSetDlg;};

class PhonePassSetDlg : public FramelessDialog
{
	Q_OBJECT

public:
	PhonePassSetDlg(const QString &loginPhone, const QString &showPhone, QWidget *parent = 0);
	~PhonePassSetDlg();

public Q_SLOTS:
	virtual void setSkin();

private Q_SLOTS:
	void getCode();
	void onOKClicked();
	void onTextEdited();

	void onPhonePassCodeOK();
	void onPhonePassCodeFailed(int retCode, const QString &desc);
	void onSetPhonePassOK();
	void onSetPhonePassFailed(int retCode, const QString &desc);

	void onCodeLeftTimer();

private:
	void initUI(const QString &showPhone);
	void startCodeLeftTimer();
	void stopCodeLeftTimer();
	void startSetPhonePass();
	void endSetPhonePass();

private:
	Ui::PhonePassSetDlg *ui;
	
	QTimer  m_codeLeftTimer;
	int     m_codeLeftTime;
	QString m_loginPhone;
};

#endif // PHONEPASSSETDLG_H
