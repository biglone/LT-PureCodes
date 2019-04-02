#include "unionchatclosetipdialog.h"
#include "ui_unionchatclosetipdialog.h"

UnionChatCloseTipDialog::UnionChatCloseTipDialog(QWidget *parent)
	: FramelessDialog(parent), m_closeResult(CloseAll)
{
	ui = new Ui::UnionChatCloseTipDialog();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->labelTitle->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(357, 181);
	setResizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), SLOT(reject()));
	connect(ui->pbtnCloseAll, SIGNAL(clicked()), SLOT(accept()));
	connect(ui->pbtnCloseCurrent, SIGNAL(clicked()), SLOT(accept()));

	setSkin();
}

UnionChatCloseTipDialog::~UnionChatCloseTipDialog()
{
	delete ui;
}

UnionChatCloseTipDialog::CloseResult UnionChatCloseTipDialog::closeResult() const
{
	return m_closeResult;
}

bool UnionChatCloseTipDialog::isAlwaysCloseAllChecked() const
{
	return ui->checkBoxCloseAll->isChecked();
}

void UnionChatCloseTipDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->labelTitle->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// bottom bar button style
	QFile qssFile;
	QString btnQss;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		btnQss = qssFile.readAll();
		qssFile.close();
	}
	ui->pbtnCloseAll->setStyleSheet(btnQss);
	ui->pbtnCloseCurrent->setStyleSheet(btnQss);
}

void UnionChatCloseTipDialog::accept()
{
	QPushButton *pushButton = qobject_cast<QPushButton *>(sender());
	if (pushButton == ui->pbtnCloseAll)
	{
		m_closeResult = CloseAll;
	}
	else if (pushButton == ui->pbtnCloseCurrent)
	{
		m_closeResult = CloseCurrent;
	}

	QDialog::accept();
}

void UnionChatCloseTipDialog::on_checkBoxCloseAll_toggled(bool checked)
{
	if (checked)
		ui->pbtnCloseCurrent->setEnabled(false);
	else
		ui->pbtnCloseCurrent->setEnabled(true);
}

