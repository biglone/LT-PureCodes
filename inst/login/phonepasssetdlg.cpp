#include "phonepasssetdlg.h"
#include "ui_phonepasssetdlg.h"
#include "companyloginmanager.h"
#include "PmApp.h"

const int kMaxCodeLeftTime = 120; // 120s

PhonePassSetDlg::PhonePassSetDlg(const QString &loginPhone, const QString &showPhone, QWidget *parent)
	: FramelessDialog(parent), m_loginPhone(loginPhone)
{
	ui = new Ui::PhonePassSetDlg();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(285, 316);
	setResizeable(false);

	initUI(showPhone);

	setSkin();
}

PhonePassSetDlg::~PhonePassSetDlg()
{
	delete ui;
}

void PhonePassSetDlg::setSkin()
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

	ui->labelCodeLeftTime->setStyleSheet("color: red;");

	ui->labelStatus->setStyleSheet("color: #470010;");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->btnOK->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void PhonePassSetDlg::getCode()
{
	qPmApp->getCompanyLoginManager()->phonePassCode(m_loginPhone);

	stopCodeLeftTimer();
	ui->btnCode->setEnabled(false);
}

void PhonePassSetDlg::onOKClicked()
{
	QString code = ui->lineEditCode->text();
	QString pass1 = ui->lineEditPass1->text();
	QString pass2 = ui->lineEditPass2->text();

	if (code.isEmpty())
	{
		ui->labelStatus->setText(tr("Code is empty"));
		ui->lineEditCode->setFocus();
		return;
	}

	if (pass1.isEmpty() || pass2.isEmpty())
	{
		ui->labelStatus->setText(tr("Password is empty"));
		ui->lineEditPass1->setFocus();
		return;
	}

	if (pass1 != pass2)
	{
		ui->labelStatus->setText(tr("Two time password is not same"));
		ui->lineEditPass1->setFocus();
		return;
	}

	if (pass1.length() < 6)
	{
		ui->labelStatus->setText(tr("Password can't be less than 6 characters"));
		ui->lineEditPass1->setFocus();
		return;
	}

	stopCodeLeftTimer();
	ui->labelStatus->clear();

	qPmApp->getCompanyLoginManager()->setPhonePass(m_loginPhone, code, pass1);
	startSetPhonePass();
}

void PhonePassSetDlg::onTextEdited()
{
	ui->labelStatus->clear();
}

void PhonePassSetDlg::onPhonePassCodeOK()
{
	startCodeLeftTimer();

	ui->labelStatus->setText(tr("Code have been sent"));
}

void PhonePassSetDlg::onPhonePassCodeFailed(int retCode, const QString &desc)
{
	ui->btnCode->setEnabled(true);
	
	ui->labelStatus->setText(tr("Get code failed: %1(%2)").arg(desc).arg(retCode));
}

void PhonePassSetDlg::onSetPhonePassOK()
{
	endSetPhonePass();

	ui->labelStatus->setText(tr("Set password successfully, use new password login"));

	ui->btnCode->setEnabled(false);
	ui->btnOK->setEnabled(false);
}

void PhonePassSetDlg::onSetPhonePassFailed(int retCode, const QString &desc)
{
	endSetPhonePass();

	ui->labelStatus->setText(tr("Set password failed: %1(%2)").arg(desc).arg(retCode));
}

void PhonePassSetDlg::onCodeLeftTimer()
{
	m_codeLeftTime--;
	ui->labelCodeLeftTime->setText(QString("%1s").arg(m_codeLeftTime));
	if (m_codeLeftTime <= 0)
	{
		stopCodeLeftTimer();

		ui->btnCode->setEnabled(true);
	}
}

void PhonePassSetDlg::initUI(const QString &showPhone)
{
	ui->labelPhone->setText(showPhone);
	
	QRegExp reg("[0-9a-zA-Z]{6,16}");
	QRegExpValidator *pValidator = new QRegExpValidator(reg, this);
	ui->lineEditPass1->setPlaceholderText(tr("Phone password, 6-16 characters"));
	ui->lineEditPass1->setMaxLength(16);
	ui->lineEditPass1->setValidator(pValidator);
	ui->lineEditPass2->setPlaceholderText(tr("Repeat password"));
	ui->lineEditPass2->setMaxLength(16);
	ui->lineEditPass2->setValidator(pValidator);
	
	ui->labelCodeLeftTime->clear();
	ui->labelStatus->clear();

	m_codeLeftTimer.setInterval(1000);
	m_codeLeftTimer.setSingleShot(false);
	connect(&m_codeLeftTimer, SIGNAL(timeout()), this, SLOT(onCodeLeftTimer()));

	connect(ui->lineEditCode, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->lineEditPass1, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->lineEditPass2, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));

	connect(ui->btnCode, SIGNAL(clicked()), this, SLOT(getCode()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

	connect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassCodeOK()), this, SLOT(onPhonePassCodeOK()));
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassCodeFailed(int, QString)), this, SLOT(onPhonePassCodeFailed(int, QString)));
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(setPhonePassOK()), this, SLOT(onSetPhonePassOK()));
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(setPhonePassFailed(int, QString)), this, SLOT(onSetPhonePassFailed(int, QString)));
}

void PhonePassSetDlg::startCodeLeftTimer()
{
	m_codeLeftTime = kMaxCodeLeftTime;
	ui->labelCodeLeftTime->setText(QString("%1s").arg(m_codeLeftTime));
	m_codeLeftTimer.start();
}

void PhonePassSetDlg::stopCodeLeftTimer()
{
	m_codeLeftTimer.stop();
	ui->labelCodeLeftTime->clear();
	m_codeLeftTime = kMaxCodeLeftTime;
}

void PhonePassSetDlg::startSetPhonePass()
{
	ui->btnCode->setEnabled(false);
	ui->lineEditCode->setEnabled(false);
	ui->lineEditPass1->setEnabled(false);
	ui->lineEditPass2->setEnabled(false);
	ui->btnOK->setEnabled(false);
	ui->btnCancel->setEnabled(false);
}

void PhonePassSetDlg::endSetPhonePass()
{
	ui->btnCode->setEnabled(true);
	ui->lineEditCode->setEnabled(true);
	ui->lineEditPass1->setEnabled(true);
	ui->lineEditPass2->setEnabled(true);
	ui->btnOK->setEnabled(true);
	ui->btnCancel->setEnabled(true);
}
