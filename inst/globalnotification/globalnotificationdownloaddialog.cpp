#include "globalnotificationdownloaddialog.h"
#include "ui_globalnotificationdownloaddialog.h"
#include <QUrl>
#include <QNetworkRequest>
#include "PmApp.h"

QPointer<GlobalNotificationDownloadDialog> GlobalNotificationDownloadDialog::s_dialog;

GlobalNotificationDownloadDialog::GlobalNotificationDownloadDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::GlobalNotificationDownloadDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(484, 532);
	setResizeable(false);
	setMaximizeable(false);

	initUI();

	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
}

GlobalNotificationDownloadDialog::~GlobalNotificationDownloadDialog()
{
	delete ui;
}

GlobalNotificationDownloadDialog *GlobalNotificationDownloadDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new GlobalNotificationDownloadDialog();
	}
	return s_dialog.data();
}

void GlobalNotificationDownloadDialog::addDownload(const QString &urlStr, const QString &name)
{
	if (urlStr.isEmpty())
		return;

	ui->downloadWidget->addItem(urlStr, name);
}

void GlobalNotificationDownloadDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	setBG(bgPixmap, bgSizes);	

	ui->title->setStyleSheet("font-size: 12pt; color: white;");
}

void GlobalNotificationDownloadDialog::closeEvent(QCloseEvent *e)
{
	if (!(qPmApp->isShutingDown() || qPmApp->isLogout())) // if not logout or exit, need query to close session
	{
		hide();
		e->ignore();
		return;
	}

	e->accept();
}

void GlobalNotificationDownloadDialog::initUI()
{
}

