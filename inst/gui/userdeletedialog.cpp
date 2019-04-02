#include "userdeletedialog.h"
#include "ui_userdeletedialog.h"
#include <QFile>

UserDeleteDialog::UserDeleteDialog(const QString &user, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::UserDeleteDialog();
	ui->setupUi(this);

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);

	ui->labelDelete->setText(tr("Delete %1?").arg(user));
	ui->radioDeleteUser->setChecked(true);

	resize(398, 276);
	setResizeable(false);

	setSkin();
}

UserDeleteDialog::~UserDeleteDialog()
{
	delete ui;
}

UserDeleteDialog::DeleteOption UserDeleteDialog::deleteOption() const
{
	if (ui->radioDeleteUser->isChecked())
	{
		return DeleteUser;
	}
	else
	{
		return DeleteAll;
	}
}

void UserDeleteDialog::setSkin()
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

	ui->labelDelete->setStyleSheet("color: black; font-weight: bold;");
	ui->radioDeleteUser->setStyleSheet("color: black;");
	ui->radioDeleteAll->setStyleSheet("color: black;");
	ui->labelDeleteAll->setStyleSheet("color: rgb(128, 128, 128);");

	// bottom bar button style
	QFile qssFile;
	QString btnQss;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		btnQss = qssFile.readAll();
		ui->pushButtonOK->setStyleSheet(btnQss);
		ui->pushButtonCancel->setStyleSheet(btnQss);
		qssFile.close();
	}
}

void UserDeleteDialog::on_pushButtonOK_clicked()
{
	accept();
}

void UserDeleteDialog::on_pushButtonCancel_clicked()
{
	reject();
}

void UserDeleteDialog::on_btnClose_clicked()
{
	reject();
}

