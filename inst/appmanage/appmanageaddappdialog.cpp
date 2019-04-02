#include "appmanageaddappdialog.h"
#include "ui_appmanageaddappdialog.h"
#include "util/FileDialog.h"
#include "pmessagebox.h"
#include <QStandardPaths>
#include <QFileInfo>

AppManageAddAppDialog::AppManageAddAppDialog(Mode mode /*= AddMode*/,QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_mode(mode)
{
	ui = new Ui::AppManageAddAppDialog();
	ui->setupUi(this);

	setMainLayout(ui->verticalLayoutMain);

	initUI();

	setFixedSize(QSize(470, 228));
	setResizeable(false);
	setMaximizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

AppManageAddAppDialog::~AppManageAddAppDialog()
{
	delete ui;
}

QString AppManageAddAppDialog::appName() const
{
	return ui->lineEditName->text().trimmed();
}

void AppManageAddAppDialog::setAppName(const QString &name)
{
	ui->lineEditName->setText(name);
}

QString AppManageAddAppDialog::appPath() const
{
	return ui->lineEditPath->text().trimmed();
}

void AppManageAddAppDialog::setAppPath(const QString &path)
{
	ui->lineEditPath->setText(path);
}

void AppManageAddAppDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 0;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {color: white; font-size: 12pt;}");
}

void AppManageAddAppDialog::accept()
{
	QString name = appName();
	QString path = appPath();
	if (name.isEmpty())
	{
		PMessageBox::warning(this, tr("Tip"), tr("App name can't be empty"));
		return;
	}

	if (path.isEmpty())
	{
		PMessageBox::warning(this, tr("Tip"), tr("App path can't be empty"));
		return;
	}

	QFileInfo fi(path);
	if (!fi.exists())
	{
		PMessageBox::warning(this, tr("Tip"), tr("App path does not exist"));
		return;
	}

	QDialog::accept();
}

void AppManageAddAppDialog::on_pushButtonBrowse_clicked()
{
	QString dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString filePath = FileDialog::getOpenFileName(this, tr("Choose File"), dir);
	if (!filePath.isEmpty())
	{
		ui->lineEditPath->setText(filePath);

		QFileInfo fi(filePath);
		ui->lineEditName->setText(fi.baseName());
	}
}

void AppManageAddAppDialog::initUI()
{
	ui->lineEditName->setMaxLength(10);

	if (m_mode == AddMode)
	{
		ui->title->setText(tr("Add App"));
		ui->lineEditName->setEnabled(true);
		ui->lineEditPath->setEnabled(true);
		ui->pushButtonBrowse->setEnabled(true);
	}
	else
	{
		ui->title->setText(tr("Edit App"));
		ui->lineEditName->setEnabled(true);
		ui->lineEditPath->setEnabled(false);
		ui->pushButtonBrowse->setEnabled(false);
	}
	setWindowTitle(ui->title->text());
}
