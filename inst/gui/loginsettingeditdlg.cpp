#include "loginsettingeditdlg.h"
#include "ui_loginsettingeditdlg.h"

LoginSettingEditDlg::LoginSettingEditDlg(const QString &settingName, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::LoginSettingEditDlg();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(settingName);
	setWindowTitle(settingName);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(298, 202);
	setResizeable(false);

	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

LoginSettingEditDlg::~LoginSettingEditDlg()
{
	delete ui;
}

void LoginSettingEditDlg::setManagerAddress(const QString &managerAddress)
{
	ui->leManagerAddress->setText(managerAddress);
}

QString LoginSettingEditDlg::managerAddress() const
{
	return ui->leManagerAddress->text();
}

void LoginSettingEditDlg::setSkin()
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
		ui->btnOK->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

