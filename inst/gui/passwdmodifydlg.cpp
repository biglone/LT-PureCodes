#include <QtGui>
#include "passwdmodifydlg.h"
#include "ui_passwdmodifydlg.h"
#include "pmessagebox.h"
#include "util/PasswdUtil.h"
#include "login/Account.h"
#include "PmApp.h"
#include "loginmgr.h"
#include "passwdmodifymanager.h"

PasswdModifyDlg::PasswdModifyDlg(QWidget *parent /*= 0*/)
	: FramelessDialog(parent)
	, ui(new Ui::PasswdModifyDlg)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());

	QString sTitle = tr("Modify Password");
	ui->title->setText(sTitle);
	setWindowTitle(sTitle);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(364, 282);
	setResizeable(false);

	/*
	QRegExp reg("[0-9a-zA-Z]{6,16}");
	QRegExpValidator* pValidator = new QRegExpValidator(reg, this);

	ui->leditConfirm->setValidator(pValidator);
	ui->leditNew->setValidator(pValidator);
	*/
	ui->leditConfirm->setMaxLength(16);
	ui->leditNew->setMaxLength(16);
	
	m_passwdModifyManager = qPmApp->getPasswdModifyManager();
	connect(m_passwdModifyManager.data(), SIGNAL(passwdModifyOK()), this, SLOT(onPasswdModifyOK()));
	connect(m_passwdModifyManager.data(), SIGNAL(passwdModifyFailed(QString)), this, SLOT(onPasswdModifyFailed(QString)));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));

	setSkin();
}

PasswdModifyDlg::~PasswdModifyDlg()
{
	delete ui;
}

void PasswdModifyDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// set label color
	ui->labTip->setStyleSheet("color: rgb(136, 136, 136);");
	ui->labError->setStyleSheet("color: rgb(245, 112, 82);");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->btnOk->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void PasswdModifyDlg::on_btnOk_clicked()
{
	do 
	{
		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::information(this, tr("Tip"), tr("You are offline, can't modify password. Please try when online"));
			return;
		}

		QString sOld = ui->leditOld->text();
		QString sNew = ui->leditNew->text();
		QString sConfirm = ui->leditConfirm->text();

		if (sOld.isEmpty())
		{
			ui->labError->setText(tr("Current password can't be empty"));
			ui->leditOld->setFocus();
			break;
		}

		if (!checkPasswd(sNew))
		{
			ui->labError->setText(tr("Password should be between 6-16 characters"));
			ui->leditNew->setFocus();
			break;
		}
		
		if(!checkPasswd(sConfirm))
		{
			ui->labError->setText(tr("Password should be between 6-16 characters"));
			ui->leditConfirm->setFocus();
			break;
		}

		if (sNew.compare(sConfirm))
		{
			ui->labError->setText(tr("The two time inputs are not same"));
			ui->leditConfirm->setFocus();
			break;
		}

		beginModifyTransaction();

		QString phone = Account::instance()->loginPhone();
		QString oldPasswd = QString::fromLatin1(PasswdUtil::toCryptogramPasswd(phone, sOld));
		QString newPasswd = QString::fromLatin1(PasswdUtil::toCryptogramPasswd(phone, sNew));
		m_passwdModifyManager.data()->modify(phone, oldPasswd, newPasswd);

	} while (0);
}

void PasswdModifyDlg::on_btnCancel_clicked()
{
	close();
}

void PasswdModifyDlg::on_leditOld_textChanged(const QString &text)
{
	Q_UNUSED(text);
	ui->labError->clear();
}

void PasswdModifyDlg::on_leditNew_textChanged(const QString &text)
{
	Q_UNUSED(text);
	ui->labError->clear();
}

void PasswdModifyDlg::on_leditConfirm_textChanged(const QString &text)
{
	Q_UNUSED(text);
	ui->labError->clear();
}

bool PasswdModifyDlg::checkPasswd(const QString &rsPasswd)
{
	bool bRet = false;
	do
	{
		int nLen = rsPasswd.length();
		if (nLen < 6 || nLen > 16)
			break;

		bRet = true;
	} while(0);
	return bRet;
}

void PasswdModifyDlg::beginModifyTransaction()
{
	ui->leditOld->setEnabled(false);
	ui->leditNew->setEnabled(false);
	ui->leditConfirm->setEnabled(false);
	ui->btnOk->setEnabled(false);
	ui->btnCancel->setEnabled(false);
	ui->btnClose->setEnabled(false);
}

void PasswdModifyDlg::endModifyTransaction()
{
	ui->leditOld->setEnabled(true);
	ui->leditNew->setEnabled(true);
	ui->leditConfirm->setEnabled(true);
	ui->btnOk->setEnabled(true);
	ui->btnCancel->setEnabled(true);
	ui->btnClose->setEnabled(true);
}

void PasswdModifyDlg::onPasswdModifyFailed(const QString &rsError)
{
	endModifyTransaction();

	ui->labError->setText(rsError);
	ui->leditNew->setFocus();
}

void PasswdModifyDlg::onPasswdModifyOK()
{
	endModifyTransaction();

	close();
}