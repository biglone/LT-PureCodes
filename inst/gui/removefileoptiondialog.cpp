#include "removefileoptiondialog.h"
#include "ui_removefileoptiondialog.h"

RemoveFileOptionDialog::RemoveFileOptionDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::RemoveFileOptionDialog();
	ui->setupUi(this);

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("Delete File Records"));
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(286, 162);
	setResizeable(false);

	ui->checkBoxRemoveFile->setChecked(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->pushButtonOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

RemoveFileOptionDialog::~RemoveFileOptionDialog()
{
	delete ui;
}

bool RemoveFileOptionDialog::isRemoveFile() const
{
	return ui->checkBoxRemoveFile->isChecked();
}

void RemoveFileOptionDialog::setSkin()
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
	QString btnQss;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		btnQss = qssFile.readAll();
		qssFile.close();
	}
	ui->pushButtonOk->setStyleSheet(btnQss);
	ui->pushButtonClose->setStyleSheet(btnQss);
}


