#include "closeoptiondialog.h"
#include "ui_closeoptiondialog.h"

CloseOptionDialog::CloseOptionDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::CloseOptionDialog();
	ui->setupUi(this);

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("Close Option"));
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(302, 202);
	setResizeable(false);

	ui->radioButtonPanelHide->setChecked(true);
	ui->radioButtonPanelExit->setChecked(false);

	ui->checkBoxRemind->setChecked(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(accept()));

	setSkin();
}

CloseOptionDialog::~CloseOptionDialog()
{
	delete ui;
}

void CloseOptionDialog::setCloseHide(bool closeHide)
{
	ui->radioButtonPanelHide->setChecked(closeHide);
	ui->radioButtonPanelExit->setChecked(!closeHide);
}

bool CloseOptionDialog::isCloseHide() const
{
	return ui->radioButtonPanelHide->isChecked();
}

bool CloseOptionDialog::isRemind() const
{
	return !ui->checkBoxRemind->isChecked();
}

void CloseOptionDialog::setSkin()
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
		ui->pushButtonClose->setStyleSheet(qss);
		qssFile.close();
	}
}

