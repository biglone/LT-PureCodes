#ifndef PHONEACTIVEDLG_H
#define PHONEACTIVEDLG_H

#include "framelessdialog.h"
#include <QTimer>
namespace Ui {class PhoneActiveDlg;};

class PhoneActiveDlg : public FramelessDialog
{
	Q_OBJECT

public:
	PhoneActiveDlg(const QString &loginPhone, const QString &showPhone, const QString &uid, QWidget *parent = 0);
	~PhoneActiveDlg();

	bool isActived() const;

public Q_SLOTS:
	virtual void setSkin();

private Q_SLOTS:
	void getCode();
	void onOKClicked();
	void onTextEdited();
	void onCodeLeftTimer();

	void onJoinCompanyCodeOK();
	void onJoinCompanyCodeFailed(int retCode, const QString &desc);
	void onJoinCompanyOK();
	void onJoinCompanyFailed(int retCode, const QString &desc);

private:
	void initUI(const QString &showPhone);
	void startCodeLeftTimer();
	void stopCodeLeftTimer();
	void startActive();
	void endActive();

private:
	Ui::PhoneActiveDlg *ui;

	QTimer  m_codeLeftTimer;
	int     m_codeLeftTime;
	QString m_loginPhone;
	QString m_uid;
	bool    m_actived;
};

#endif // PHONEACTIVEDLG_H
