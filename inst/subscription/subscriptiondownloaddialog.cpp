#include "subscriptiondownloaddialog.h"
#include "ui_subscriptiondownloaddialog.h"
#include <QUrl>
#include <QNetworkRequest>
#include "PmApp.h"

QPointer<SubscriptionDownloadDialog> SubscriptionDownloadDialog::s_dialog;

SubscriptionDownloadDialog::SubscriptionDownloadDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::SubscriptionDownloadDialog();
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

SubscriptionDownloadDialog::~SubscriptionDownloadDialog()
{
	delete ui;
}

SubscriptionDownloadDialog *SubscriptionDownloadDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new SubscriptionDownloadDialog();
	}
	return s_dialog.data();
}

void SubscriptionDownloadDialog::addDownload(const QString &urlStr, const QString &name)
{
	if (urlStr.isEmpty())
		return;

	ui->downloadWidget->addItem(urlStr, name);
}

void SubscriptionDownloadDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	setBG(bgPixmap, bgSizes);	

	ui->title->setStyleSheet("font-size: 12pt; color: white;");
}

void SubscriptionDownloadDialog::closeEvent(QCloseEvent *e)
{
	if (!(qPmApp->isShutingDown() || qPmApp->isLogout())) // if not logout or exit, need query to close session
	{
		hide();
		e->ignore();
		return;
	}

	e->accept();
}

void SubscriptionDownloadDialog::initUI()
{
}

