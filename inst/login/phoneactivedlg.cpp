#include "phoneactivedlg.h"
#include "ui_phoneactivedlg.h"
#include "companyloginmanager.h"
#include "PmApp.h"

static const int kMaxCodeLeftTime = 120; // 120s

PhoneActiveDlg::PhoneActiveDlg(const QString &loginPhone, const QString &showPhone, const QString &uid, QWidget *parent)
	: FramelessDialog(parent), m_loginPhone(loginPhone), m_uid(uid), m_actived(false)
{
	ui = new Ui::PhoneActiveDlg();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(285, 232);
	setResizeable(false);

	initUI(showPhone);

	setSkin();
}

PhoneActiveDlg::~PhoneActiveDlg()
{
	delete ui;
}

bool PhoneActiveDlg::isActived() const
{
	return m_actived;
}

void PhoneActiveDlg::setSkin()
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

void PhoneActiveDlg::getCode()
{
	qPmApp->getCompanyLoginManager()->joinCompanyCode(m_loginPhone);

	stopCodeLeftTimer();
	ui->btnCode->setEnabled(false);
}

void PhoneActiveDlg::onOKClicked()
{
	QString code = ui->lineEditCode->text();
	if (code.isEmpty())
	{
		ui->labelStatus->setText(tr("Code is empty"));
		ui->lineEditCode->setFocus();
		return;
	}

	stopCodeLeftTimer();
	ui->labelStatus->clear();

	qPmApp->getCompanyLoginManager()->joinCompany(m_loginPhone, code, m_uid);
	startActive();
}

void PhoneActiveDlg::onTextEdited()
{
	ui->labelStatus->clear();
}

void PhoneActiveDlg::onCodeLeftTimer()
{
	m_codeLeftTime--;
	ui->labelCodeLeftTime->setText(QString("%1s").arg(m_codeLeftTime));
	if (m_codeLeftTime <= 0)
	{
		stopCodeLeftTimer();

		ui->btnCode->setEnabled(true);
	}
}

void PhoneActiveDlg::onJoinCompanyCodeOK()
{
	startCodeLeftTimer();

	ui->labelStatus->setText(tr("Code have been sent"));
}

void PhoneActiveDlg::onJoinCompanyCodeFailed(int retCode, const QString &desc)
{
	ui->btnCode->setEnabled(true);

	ui->labelStatus->setText(tr("Get code failed: %1(%2)").arg(desc).arg(retCode));
}

void PhoneActiveDlg::onJoinCompanyOK()
{
	m_actived = true;

	endActive();

	ui->labelStatus->setText(tr("Active successfully, please login"));

	ui->btnCode->setEnabled(false);
	ui->btnOK->setEnabled(false);
}

void PhoneActiveDlg::onJoinCompanyFailed(int retCode, const QString &desc)
{
	endActive();

	ui->labelStatus->setText(tr("Active failed: %1(%2)").arg(desc).arg(retCode));
}

void PhoneActiveDlg::initUI(const QString &showPhone)
{
	ui->labelPhone->setText(showPhone);
	ui->labelCodeLeftTime->clear();
	ui->labelStatus->clear();

	m_codeLeftTimer.setInterval(1000);
	m_codeLeftTimer.setSingleShot(false);
	connect(&m_codeLeftTimer, SIGNAL(timeout()), this, SLOT(onCodeLeftTimer()));

	connect(ui->lineEditCode, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));

	connect(ui->btnCode, SIGNAL(clicked()), this, SLOT(getCode()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
	
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(joinCompanyCodeOK()), this, SLOT(onJoinCompanyCodeOK()));
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(joinCompanyCodeFailed(int, QString)), this, SLOT(onJoinCompanyCodeFailed(int, QString)));
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(joinCompanyOK()), this, SLOT(onJoinCompanyOK()));
	connect(qPmApp->getCompanyLoginManager(), SIGNAL(joinCompanyFailed(int, QString)), this, SLOT(onJoinCompanyFailed(int, QString)));
}

void PhoneActiveDlg::startCodeLeftTimer()
{
	m_codeLeftTime = kMaxCodeLeftTime;
	ui->labelCodeLeftTime->setText(QString("%1s").arg(m_codeLeftTime));
	m_codeLeftTimer.start();
}

void PhoneActiveDlg::stopCodeLeftTimer()
{
	m_codeLeftTimer.stop();
	ui->labelCodeLeftTime->clear();
	m_codeLeftTime = kMaxCodeLeftTime;
}

void PhoneActiveDlg::startActive()
{
	ui->btnCode->setEnabled(false);
	ui->btnOK->setEnabled(false);
	ui->btnCancel->setEnabled(false);
}

void PhoneActiveDlg::endActive()
{
	ui->btnCode->setEnabled(true);
	ui->btnOK->setEnabled(true);
	ui->btnCancel->setEnabled(true);
}
