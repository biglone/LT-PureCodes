#include "application/Logger.h"
using namespace application;

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTime>
#include <Windows.h>
#include "pmessagebox.h"

#include "Constants.h"

#include "pmclient/PmClient.h"
#include "PmApp.h"
#include "buddymgr.h"
#include "updatemanager.h"
#include "widgetmanager.h"

#include "updatedialog.h"
#include "ui_updatedialog.h"

#ifdef Q_OS_WIN
#include <ShellAPI.h>
#endif // Q_OS_WIN

CUpdateDialog *CUpdateDialog::s_pIns = 0;

CUpdateDialog::CUpdateDialog(QWidget *parent)
	: FramelessDialog(parent)
	, ui(new Ui::CUpdateDialog)
	, m_bSettedLastVersion(false)
	, m_downloading(false)
{
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint); // show top most

	ui->centerStackedWidget->setCurrentIndex(0);
	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(472, 320);
	setResizeable(false);

	ui->title->setVisible(false);
	ui->labelUpdateTip->setText(tr("Downloading..."));
	ui->labelProgress->setText(QString("0%"));

	ui->progressBar->setMinimum(0);
	ui->progressBar->setMaximum(100);
	ui->progressBar->setValue(0);
	ui->progressBar->setTextVisible(false);

	UpdateManager *updateManager = qPmApp->getUpdateManager();
	connect(updateManager, SIGNAL(downloadError(QString)), this, SLOT(onError(QString)));
	connect(updateManager, SIGNAL(downloadProgress(qint64, qint64)), SLOT(slot_downloadProgress(qint64, qint64)));
	connect(updateManager, SIGNAL(downloadFinished(QString)), SLOT(slot_downloadFinish(QString)));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(closeUpdate()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(qPmApp, SIGNAL(updateCheckFinished(bool)), this, SLOT(onUpdatingChecked(bool)));
	connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(onProgressValueChanged(int)));
	connect(ui->btnDownload, SIGNAL(clicked()), this, SLOT(checkIfUpdating()));

	setSkin();
}

CUpdateDialog::~CUpdateDialog()
{
	qDebug() << __FUNCTION__;

	delete ui;
	s_pIns = 0;
}

void CUpdateDialog::setLastestVersion(const QString &toVer, const QString &toMsg, const QString &downloadUrl)
{
	if (m_bSettedLastVersion)
		return;

	m_bSettedLastVersion = true;
	m_sFilename = "";
	m_toVersion = toVer;
	m_downloadUrl = downloadUrl;

	// ask update page
	ui->labelAskUpdate->setText(
		tr("Find new version(%1), update now?\nThe program can't login if don't update").arg(m_toVersion));

	// download page
	QString sText = QString(tr("Update from v%1 to v%2:")).arg(APP_VERSION).arg(toVer);
	ui->labVersion->setText(sText);

	ui->textBrowser->append(toMsg);
	ui->textBrowser->moveCursor(QTextCursor::Start);

	// show dialog
	ui->centerStackedWidget->setCurrentIndex(0);
	WidgetManager::showActivateRaiseWindow(this);
}

void CUpdateDialog::checkIfUpdating()
{
	qPmApp->checkIfUpdating();

	qDebug() << Q_FUNC_INFO << "start check other updating";
}

void CUpdateDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_1.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 1;
	bgSizes.topBarHeight = 121;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// set text browser style
	ui->textBrowser->setStyleSheet("border-image: none; border: none; background-color: white;");

	// version style
	ui->labVersion->setStyleSheet("font-size: 11pt; color: black;");

	// ask update style
	ui->labelAskUpdate->setStyleSheet("font-size: 11pt;");

	// progress bar style
	ui->progressBar->setStyleSheet(
		"QProgressBar {"
		"border-image: none;"
		"border: none;"
		"border-radius: 5px;"
		"background-color: rgb(225, 225, 225);"
		"}"

		"QProgressBar::chunk {"
		"border-image: none;"
		"background-color: rgb(18, 112, 184);"
		"width: 20px;"
		"}");
}

void CUpdateDialog::startDownload()
{
	UpdateManager *updateManager = qPmApp->getUpdateManager();
	updateManager->startUpdate(m_toVersion, m_downloadUrl);
	ui->progressBar->setValue(0);

	qPmApp->notifyUpdating(true);
	m_downloading = true;
}

void CUpdateDialog::slot_downloadProgress(qint64 nBytesReceived, qint64 nBytesTotal)
{
	ui->progressBar->setValue(int(100.0 * nBytesReceived / nBytesTotal));
}

void CUpdateDialog::slot_downloadFinish(const QString& rsFilename)
{
	m_downloading = false;
	qPmApp->notifyUpdating(false);

	qDebug() << Q_FUNC_INFO << rsFilename;

	m_sFilename = rsFilename;
	ui->labelUpdateTip->setText("");

	// update directly
	installUpdate();
}

void CUpdateDialog::closeEvent(QCloseEvent *e)
{
	Q_UNUSED(e);
	
	if (m_downloading)
	{
		qPmApp->notifyUpdating(false);
	}

	qPmApp->quit();
}

void CUpdateDialog::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		return;

	FramelessDialog::keyPressEvent(e);
}

void CUpdateDialog::installUpdate()
{
	if (!QFile::exists(m_sFilename))
		return;

	// start launcher
	const QString launcherName = "updatelauncher.exe";
	QString launcherPath = QCoreApplication::applicationDirPath() + "/" + launcherName;

	QString sAbsoluteFile = QDir::toNativeSeparators(m_sFilename);
	QString fileNameValue = QString::fromLatin1(sAbsoluteFile.toUtf8().toBase64());
	QString fileNameArg = QString("path=%1").arg(fileNameValue);
	HINSTANCE nRet = ShellExecute(NULL, "open", qPrintable(launcherPath), qPrintable(fileNameArg), NULL, SW_SHOWNORMAL);
	if ((int)nRet < 32)
	{
		qWarning() << Q_FUNC_INFO << " ShellExecute ret: " << (int)nRet;
	}

	close();
}

void CUpdateDialog::onError(const QString& rsError)
{
	m_downloading = false;
	qPmApp->notifyUpdating(false);

	qDebug() << Q_FUNC_INFO << rsError;

	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Downloading Update Failed"), 
		rsError+tr("\n\nDo you want to retry"), QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret == QDialogButtonBox::Yes)
	{
		checkIfUpdating();
	}
	else
	{
		close();
	}
}

void CUpdateDialog::onUpdatingChecked(bool update)
{
	qDebug() << Q_FUNC_INFO << update;

	if (!update)
	{
		setFixedSize(472, 356);
		ui->centerStackedWidget->setCurrentIndex(1);

		// do the update stuff
		startDownload();
	}
	else
	{
		// someone is updating
		PMessageBox::information(this, 
			tr("Update"), 
			tr("There is updating on this machine.\nPress OK to exit"),
			QDialogButtonBox::Ok);

		close();
	}
}

void CUpdateDialog::onProgressValueChanged(int v)
{
	int progress = (int)(((float)v)/((float)ui->progressBar->maximum())*100);
	ui->labelProgress->setText(QString("%1%").arg(progress));
}

void CUpdateDialog::closeUpdate()
{
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Exit Update"), 
		tr("Do you want to exit update"), QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret == QDialogButtonBox::Yes)
		close();
}

CUpdateDialog * CUpdateDialog::instance()
{
	if (!s_pIns)
	{
		s_pIns = new CUpdateDialog();
	}
	return s_pIns;
}
