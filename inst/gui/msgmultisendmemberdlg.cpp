#include "msgmultisendmemberdlg.h"
#include "ui_msgmultisendmemberdlg.h"
#include "Account.h"
#include "pmessagebox.h"

const int KMaxSendCount = 1200;

MsgMultiSendMemberDlg::MsgMultiSendMemberDlg(const QStringList &members, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::MsgMultiSendMemberDlg();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());
	setWindowModality(Qt::WindowModal);

	setMainLayout(ui->verticalLayoutMain);
	resize(658, 568);
	setMinimumSize(658, 568);
	setResizeable(true);
	setMaximizeable(true);

	QString selfId = Account::instance()->id();
	ui->centralPanel->init(SelectContactWidget::SelectAll, QStringList() << selfId, QStringList() << selfId, members, QStringList(), KMaxSendCount);
	if (members.count() > 0)
		ui->pushButtonOK->setEnabled(true);
	else
		ui->pushButtonOK->setEnabled(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->btnMaximize, SIGNAL(clicked()), this, SLOT(triggerMaximize()));
	connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->centralPanel, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

	setSkin();
}

MsgMultiSendMemberDlg::~MsgMultiSendMemberDlg()
{
	delete ui;
}

QStringList MsgMultiSendMemberDlg::memberIds() const
{
	return ui->centralPanel->selectionIds();
}

void MsgMultiSendMemberDlg::setSkin()
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
		ui->pushButtonOK->setStyleSheet(qss);
		ui->pushButtonCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void MsgMultiSendMemberDlg::accept()
{
	int selectCount = ui->centralPanel->selectionIds().count();
	if (selectCount > KMaxSendCount)
	{
		PMessageBox::information(this, tr("Tip"), tr("Multi-send message supports at most %1 members").arg(KMaxSendCount));
		return;
	}

	FramelessDialog::accept();
}

void MsgMultiSendMemberDlg::onSelectionChanged()
{
	QStringList selIds = ui->centralPanel->selectionIds();
	if (selIds.isEmpty())
		ui->pushButtonOK->setEnabled(false);
	else
		ui->pushButtonOK->setEnabled(true);
}

void MsgMultiSendMemberDlg::onMaximizeStateChanged(bool isMaximized)
{
	if (isMaximized)
	{
		ui->btnMaximize->setChecked(true);
		ui->btnMaximize->setToolTip(tr("Restore"));
	}
	else
	{
		ui->btnMaximize->setChecked(false);
		ui->btnMaximize->setToolTip(tr("Maximize"));
	}
}
