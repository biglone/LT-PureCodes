#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QTimer>
#include "framelessdialog.h"

namespace Ui {class CAboutDialog;};

class CAboutDialog : public FramelessDialog
{
	Q_OBJECT

public:
	explicit CAboutDialog(QWidget *parent = 0);
	~CAboutDialog();

public slots:
	virtual void setSkin();

private slots:
	void on_pBtnEnter_clicked();
	void on_timer_timeout();
	void on_labelCompanyUrl_clicked();

private:
	Ui::CAboutDialog *ui;
	QTimer  m_Timer;

	QString m_qsBtnText;
	int     m_nCount;
};


#endif // ABOUTDIALOG_H
