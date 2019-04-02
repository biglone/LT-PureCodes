#include "subscriptionhistorydialog.h"
#include "ui_subscriptionhistorydialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "subscriptionmodel.h"
#include <QWebPage>
#include <QWebFrame>
#include "subscriptionmanager.h"
#include "subscriptionmsg4js.h"
#include "Account.h"
#include "util/AvatarUtil.h"
#include <QMovie>
#include <QNetworkDiskCache>
#include "subscriptionmsgwebpage.h"

static const int kDefaultMsgCount = 10;

SubscriptionHistoryDialog::SubscriptionHistoryDialog(const QString &subscriptionId, QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_subscriptionId(subscriptionId), m_webPage(0)
{
	ui = new Ui::SubscriptionHistoryDialog();
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

	SubscriptionManager *subscriptionManager = qPmApp->getSubscriptionManager();
	connect(subscriptionManager, SIGNAL(getHistoryMessagesFinished(bool, QString, QList<SubscriptionMsg>)),
		this, SLOT(onGetHistoryMessagesFinished(bool, QString, QList<SubscriptionMsg>)));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
}

SubscriptionHistoryDialog::~SubscriptionHistoryDialog()
{
	delete ui;
}

void SubscriptionHistoryDialog::setSkin()
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

void SubscriptionHistoryDialog::populateJavaScriptWindowObject()
{
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("Message4Js", m_message4Js);
}

void SubscriptionHistoryDialog::onMessage4JsLoadSucceeded()
{
	emit initUIComplete();
}

void SubscriptionHistoryDialog::fetchHistoryMsg()
{
	SubscriptionManager *subscriptionManager = qPmApp->getSubscriptionManager();
	QString selfId = Account::instance()->id();
	subscriptionManager->getHistoryMessages(selfId, m_subscriptionId, m_oldestSequence, kDefaultMsgCount);
}

void SubscriptionHistoryDialog::onGetHistoryMessagesFinished(bool ok, 
															 const QString &subscriptionId, 
															 const QList<SubscriptionMsg> &msgs)
{
	if (subscriptionId != m_subscriptionId)
		return;

	bool finishMoreMsg = (!m_oldestSequence.isEmpty());

	if (ok && !msgs.isEmpty())
	{
		if (finishMoreMsg)
			m_message4Js->insertMessagesAtTop(msgs);
		else
			m_message4Js->setMessages(msgs);

		SubscriptionMsg msg = msgs[0];
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

void SubscriptionHistoryDialog::showTip(const QString &tip)
{
	ui->tipWidget->setTipText(tip);
	ui->tipWidget->autoShow();
}

void SubscriptionHistoryDialog::on_pushButtonRetry_clicked()
{
	ui->labelLoading->setVisible(true);
	ui->labelTip->setText(tr("Load chat history..."));
	ui->pushButtonRetry->setVisible(false);
	ui->stackedWidget->setCurrentIndex(2);
	fetchHistoryMsg();
}

void SubscriptionHistoryDialog::onUserChanged(const QString &uid)
{
	if (uid == Account::instance()->id())
	{
		if (m_webPage)
		{
			m_webPage->onAvatarChanged(uid);
		}
	}
}

void SubscriptionHistoryDialog::onLogoChanged(const QString &subscriptionId)
{
	if (subscriptionId == m_subscriptionId)
	{
		// icon
		ModelManager *modelManager = qPmApp->getModelManager();
		QPixmap logo = modelManager->subscriptionLogo(m_subscriptionId);
		setWindowIcon(QIcon(logo));

		// chat icon
		if (m_webPage)
		{
			m_webPage->onSubscriptionAvatarChanged(subscriptionId);
		}
	}
}

void SubscriptionHistoryDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
	SubscriptionDetail subscription = subscriptionModel->subscription(m_subscriptionId);

	QPixmap logo = modelManager->subscriptionLogo(m_subscriptionId);
	setWindowIcon(QIcon(logo));

	ui->title->setText(subscription.name() + tr("'s Chat History"));
	setWindowTitle(ui->title->text());

	// tip widget
	ui->tipWidget->setTipPixmap(QPixmap(":/images/Icon_76.png"));
	ui->tipWidget->stopShow();

	// web view related
	m_webPage = new SubscriptionMsgWebPage(ui->webView);
	connect(m_webPage, SIGNAL(avatarClicked(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(m_webPage, SIGNAL(subscriptionAvatarClicked(QString)), this, SIGNAL(openSubscriptionDetail(QString)));
	ui->webView->setPage(m_webPage);

	QString selfId = Account::instance()->id();
	QString selfName = modelManager->userName(selfId);
	m_message4Js = new SubscriptionMsg4Js(this);
	m_message4Js->setUid(selfId);
	m_message4Js->setUName(selfName);
	QDir avatarDir(Account::instance()->avatarPath());
	QString avatarName = AvatarUtil::avatarName(selfId);
	QString avatarPath = avatarDir.absoluteFilePath(avatarName);
	m_message4Js->setUAvatar(avatarPath);

	m_message4Js->setSubscriptionId(subscription.id());
	m_message4Js->setSubscriptionName(subscription.name());
	QDir logoDir(Account::instance()->subscriptionDir());
	QString logoName = SubscriptionModel::logoFileName(subscription.logo());
	QString logoPath = logoDir.absoluteFilePath(logoName);
	m_message4Js->setSubscriptionLogo(logoPath);
	m_message4Js->moreMsgTipClose();
	connect(m_message4Js, SIGNAL(fetchHistoryMessage()), this, SLOT(fetchHistoryMsg()));
	connect(m_message4Js, SIGNAL(loadSucceeded()), this, SLOT(onMessage4JsLoadSucceeded()));
	connect(m_message4Js,SIGNAL(openTitle(QString, QString, QString)), this, SIGNAL(openTitle(QString, QString, QString)));
	connect(m_message4Js, SIGNAL(openAttach(QString, QString, QString)), this, SIGNAL(openAttach(QString, QString, QString)));
	connect(this, SIGNAL(initUIComplete()), m_message4Js, SIGNAL(initUIComplete()));

	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onUserChanged(QString)));
	connect(subscriptionModel, SIGNAL(subscriptionLogoChanged(QString)), this, SLOT(onLogoChanged(QString)));

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
