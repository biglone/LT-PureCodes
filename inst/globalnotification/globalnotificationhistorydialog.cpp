#include "globalnotificationhistorydialog.h"
#include "ui_globalnotificationhistorydialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "globalnotificationmodel.h"
#include <QWebPage>
#include <QWebFrame>
#include "globalnotificationmanager.h"
#include "globalnotificationmsg4js.h"
#include "Account.h"
#include "util/AvatarUtil.h"
#include <QMovie>
#include <QNetworkDiskCache>
#include "globalnotificationmsgwebpage.h"

static const int kDefaultMsgCount = 10;

GlobalNotificationHistoryDialog::GlobalNotificationHistoryDialog(const QString &globalNotificationId, QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_globalNotificationId(globalNotificationId), m_webPage(0)
{
	ui = new Ui::GlobalNotificationHistoryDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setMainLayout(ui->verticalLayoutMain);
	resize(588, 612);
	setResizeable(false);
	setMaximizeable(false);

	initUI();

	setSkin();

	GlobalNotificationManager *globalNotificationManager = qPmApp->getGlobalNotificationManager();
	connect(globalNotificationManager, SIGNAL(getHistoryMessagesFinished(bool, QString, QList<GlobalNotificationMsg>)),
		this, SLOT(onGetHistoryMessagesFinished(bool, QString, QList<GlobalNotificationMsg>)));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
}

GlobalNotificationHistoryDialog::~GlobalNotificationHistoryDialog()
{
	delete ui;
}

void GlobalNotificationHistoryDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("font-size: 12pt; color: white;");
	ui->labelNoMsgTip->setStyleSheet("color: rgb(128, 128, 128);");
	ui->labelTip->setStyleSheet("color: rgb(128, 128, 128);");
}

void GlobalNotificationHistoryDialog::populateJavaScriptWindowObject()
{
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("Message4Js", m_message4Js);
}

void GlobalNotificationHistoryDialog::onMessage4JsLoadSucceeded()
{
	emit initUIComplete();
}

void GlobalNotificationHistoryDialog::fetchHistoryMsg()
{
	GlobalNotificationManager *globalNotificationManager = qPmApp->getGlobalNotificationManager();
	QString selfId = Account::instance()->id();
	globalNotificationManager->getHistoryMessages(selfId, m_globalNotificationId, m_oldestSequence, kDefaultMsgCount);
}

void GlobalNotificationHistoryDialog::onGetHistoryMessagesFinished(bool ok, 
															 const QString &globalNotificationId, 
															 const QList<GlobalNotificationMsg> &msgs)
{
	if (globalNotificationId != m_globalNotificationId)
		return;

	bool finishMoreMsg = (!m_oldestSequence.isEmpty());

	if (ok && !msgs.isEmpty())
	{
		if (finishMoreMsg)
			m_message4Js->insertMessagesAtTop(msgs);
		else
			m_message4Js->setMessages(msgs);

		GlobalNotificationMsg msg = msgs[0];
		m_oldestSequence = msg.id();
	}

	if (finishMoreMsg)
	{
		m_message4Js->moreMsgFinished();

		if (!ok)
		{
			showTip(tr("Load chat history failed, please try again"));
		}
	}
	else
	{
		if (!ok)
		{
			ui->stackedWidget->setCurrentIndex(2);
			ui->labelLoading->setVisible(false);
			ui->labelTip->setText(tr("Load chat history failed, please try again"));
			ui->pushButtonRetry->setVisible(true);
		}
	}
	
	if (ok)
	{
		if (msgs.count() < kDefaultMsgCount)
			m_message4Js->moreMsgTipClose();
		else
			m_message4Js->moreMsgTipShow();

		if (!finishMoreMsg)
		{
			QTimer::singleShot(0, this, SIGNAL(initUIComplete()));
		}

		if (m_oldestSequence.isEmpty() && msgs.isEmpty()) // no messages
		{
			ui->stackedWidget->setCurrentIndex(1);
		}
		else
		{
			ui->stackedWidget->setCurrentIndex(0);
		}
	}
}

void GlobalNotificationHistoryDialog::showTip(const QString &tip)
{
	ui->tipWidget->setTipText(tip);
	ui->tipWidget->autoShow();
}

void GlobalNotificationHistoryDialog::on_pushButtonRetry_clicked()
{
	ui->labelLoading->setVisible(true);
	ui->labelTip->setText(tr("Load chat history..."));
	ui->pushButtonRetry->setVisible(false);
	ui->stackedWidget->setCurrentIndex(2);
	fetchHistoryMsg();
}

void GlobalNotificationHistoryDialog::onUserChanged(const QString &uid)
{
	if (uid == Account::instance()->id())
	{
		if (m_webPage)
		{
			m_webPage->onAvatarChanged(uid);
		}
	}
}

void GlobalNotificationHistoryDialog::onLogoChanged(const QString &globalNotificationId)
{
	if (globalNotificationId == m_globalNotificationId)
	{
		// icon
		ModelManager *modelManager = qPmApp->getModelManager();
		QPixmap logo = modelManager->globalNotificationLogo(m_globalNotificationId);
		setWindowIcon(QIcon(logo));

		// chat icon
		if (m_webPage)
		{
			m_webPage->onGlobalNotificationAvatarChanged(globalNotificationId);
		}
	}
}

void GlobalNotificationHistoryDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	GlobalNotificationModel *globalNotificationModel = modelManager->globalNotificationModel();
	GlobalNotificationDetail globalNotification = globalNotificationModel->globalNotification(m_globalNotificationId);

	QPixmap logo = modelManager->globalNotificationLogo(m_globalNotificationId);
	setWindowIcon(QIcon(logo));

	ui->title->setText(globalNotification.name() + tr("'s Chat History"));
	setWindowTitle(ui->title->text());

	// tip widget
	ui->tipWidget->setTipPixmap(QPixmap(":/images/Icon_76.png"));
	ui->tipWidget->stopShow();

	// web view related
	m_webPage = new GlobalNotificationMsgWebPage(ui->webView);
	connect(m_webPage, SIGNAL(avatarClicked(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(m_webPage, SIGNAL(globalNotificationAvatarClicked(QString)), this, SIGNAL(openGlobalNotificationDetail(QString)));
	ui->webView->setPage(m_webPage);

	QString selfId = Account::instance()->id();
	QString selfName = modelManager->userName(selfId);
	m_message4Js = new GlobalNotificationMsg4Js(this);
	m_message4Js->setUid(selfId);
	m_message4Js->setUName(selfName);
	QDir avatarDir(Account::instance()->avatarPath());
	QString avatarName = AvatarUtil::avatarName(selfId);
	QString avatarPath = avatarDir.absoluteFilePath(avatarName);
	m_message4Js->setUAvatar(avatarPath);

	m_message4Js->setGlobalNotificationId(globalNotification.id());
	m_message4Js->setGlobalNotificationName(globalNotification.name());
	QDir logoDir(Account::instance()->globalNotificationDir());
	QString logoName = GlobalNotificationModel::logoFileName(globalNotification.logo());
	QString logoPath = logoDir.absoluteFilePath(logoName);
	m_message4Js->setGlobalNotificationLogo(logoPath);
	m_message4Js->moreMsgTipClose();
	connect(m_message4Js, SIGNAL(fetchHistoryMessage()), this, SLOT(fetchHistoryMsg()));
	connect(m_message4Js, SIGNAL(loadSucceeded()), this, SLOT(onMessage4JsLoadSucceeded()));
	connect(m_message4Js,SIGNAL(openTitle(QString, QString, QString)), this, SIGNAL(openTitle(QString, QString, QString)));
	connect(m_message4Js, SIGNAL(openAttach(QString, QString, QString)), this, SIGNAL(openAttach(QString, QString, QString)));
	connect(this, SIGNAL(initUIComplete()), m_message4Js, SIGNAL(initUIComplete()));

	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onUserChanged(QString)));
	connect(globalNotificationModel, SIGNAL(globalNotificationLogoChanged(QString)), this, SLOT(onLogoChanged(QString)));

	QNetworkAccessManager *webNetworkAccessManager = ui->webView->page()->networkAccessManager();
	QNetworkDiskCache *diskCache = new QNetworkDiskCache(webNetworkAccessManager);
	if (diskCache)
	{
		QString location = QString("%1\\netcache").arg(Account::instance()->subscriptionPath());
		diskCache->setCacheDirectory(location);
		webNetworkAccessManager->setCache(diskCache);
	}

	ui->webView->setUrl(QUrl("qrc:/html/subscriptionmsglist.html"));
	connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));

	ui->stackedWidget->setCurrentIndex(2);
	QMovie *movie = new QMovie(":/images/loading_image_small.gif");
	movie->setParent(this);
	ui->labelLoading->setMovie(movie);
	movie->start();
	ui->labelTip->setText(tr("Load Chat History..."));
	ui->pushButtonRetry->setVisible(false);

	// search to the last page
	QTimer::singleShot(100, this, SLOT(fetchHistoryMsg()));
}
