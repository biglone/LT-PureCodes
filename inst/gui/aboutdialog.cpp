#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "Constants.h"
#include "settings/GlobalSettings.h"
#include <QUrl>
#include <QDesktopServices>

CAboutDialog::CAboutDialog(QWidget *parent)
	: FramelessDialog(parent)
	, m_nCount(10)
{
	ui = new Ui::CAboutDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("About ") + GlobalSettings::title());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(314, 179);
	setResizeable(false);

	ui->labVersion->setText(APP_VERSION);
	ui->labelCompany->setText(GlobalSettings::company());
	ui->labelCompanyUrl->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelCompanyUrl->setText(GlobalSettings::companyUrl());
	ui->labelCompanyTel->setText(GlobalSettings::companyTel());
	if (GlobalSettings::hideCompanyTel())
	{
		ui->labelCompanyTelTag->setVisible(false);
		ui->labelCompanyTel->setVisible(false);
	}

	m_qsBtnText = ui->pBtnEnter->text();
	ui->pBtnEnter->setText(QString("%1(%2)").arg(m_qsBtnText).arg(m_nCount));

	m_Timer.setInterval(1000);
	connect(&m_Timer, SIGNAL(timeout()), SLOT(on_timer_timeout()));
	m_Timer.start();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	setSkin();
}

CAboutDialog::~CAboutDialog()
{
	delete ui;
}

void CAboutDialog::setSkin()
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

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pBtnEnter->setStyleSheet(qss);
		qssFile.close();
	}
}

void CAboutDialog::on_pBtnEnter_clicked()
{
	close();
}

void CAboutDialog::on_timer_timeout()
{
	--m_nCount;
	
	if (m_nCount < 0)
	{
		close();
		return;
	}

	ui->pBtnEnter->setText(QString("%1(%2)").arg(m_qsBtnText).arg(m_nCount));
}

void CAboutDialog::on_labelCompanyUrl_clicked()
{
	QString urlStr = ui->labelCompanyUrl->text();
	QDesktopServices::openUrl(QUrl::fromUserInput(urlStr));
}

