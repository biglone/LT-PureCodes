#include "blacklistdialog.h"
#include "ui_blacklistdialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "loginmgr.h"
#include "pmessagebox.h"

QPointer<BlackListDialog> BlackListDialog::s_dlg;

BlackListDialog::BlackListDialog(QWidget *parent /*= 0*/)
	: FramelessDialog(parent)
{
	ui = new Ui::BlackListDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("Blocked List"));
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(299, 433);
	setResizeable(false);

	BlackListModel *blackListModel = qPmApp->getModelManager()->blackListModel();
	ui->listView->setBlackListModel(blackListModel);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->listView, SIGNAL(removeBlack(QString)), this, SLOT(onRemoveBlack(QString)));
	connect(ui->listView, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));

	setSkin();
}

BlackListDialog::~BlackListDialog()
{
	delete ui;
}

BlackListDialog *BlackListDialog::getBlackListDialog()
{
	if (s_dlg.isNull())
	{
		s_dlg = new BlackListDialog();
	}
	return s_dlg.data();
}

void BlackListDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	ui->listView->setStyleSheet("background: transparent; border: none;");
}

void BlackListDialog::onRemoveBlack(const QString &id)
{
	if (id.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't remove him out of blocked list"));
		return;
	}

	QString name = qPmApp->getModelManager()->userName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Remove out of blocked list"), 
		tr("Are you sure to remove %1 out of blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit removeBlack(id);
}

