#include "globalnotificationwebview.h"
#include "ui_globalnotificationwebview.h"
#include <QWebFrame>
#include <QWebPage>
#include <QWebElement>
#include <QUrl>
#include "Account.h"
#include <QDesktopServices>
#include "globalnotificationmanager.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "pmessagebox.h"
#include <QMenu>
#include <QAction>
#include <QNetworkRequest>
#include "http/HttpPool.h"

QPointer<GlobalNotificationWebView> GlobalNotificationWebView::s_webView;

GlobalNotificationWebView::GlobalNotificationWebView(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::GlobalNotificationWebView();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setMainLayout(ui->verticalLayoutMain);
	resize(652, 632);
	setResizeable(false);
	setMaximizeable(false);

	initUI();

	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
}

GlobalNotificationWebView::~GlobalNotificationWebView()
{
	delete ui;
}

void GlobalNotificationWebView::setGlobalNotificationId(const QString &subId)
{
	m_globalNotificationId = subId;
}

GlobalNotificationWebView *GlobalNotificationWebView::getWebView()
{
	if (s_webView.isNull())
	{
		s_webView = new GlobalNotificationWebView();
	}
	return s_webView.data();
}

void GlobalNotificationWebView::toSubDetail(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	if (!qPmApp->getModelManager()->hasGlobalNotificationItem(globalNotificationId))
	{
		PMessageBox::warning(this, tr("Tip"), tr("You need to follow the globalnotification first"));
		return;
	}

	emit openGlobalNotificationDetail(globalNotificationId);
}

void GlobalNotificationWebView::openUrl(const QString &url)
{
	if (url.isEmpty())
		return;

	QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void GlobalNotificationWebView::addMenuAction(QMenu *menu, const QWebElement &webElement)
{
	if (menu)
	{
		QString tagName = webElement.tagName();
		if (0 == tagName.compare("IMG", Qt::CaseInsensitive))
		{
			menu->clear();

			QStringList attNames = webElement.attributeNames();
			QString srcName;
			foreach (QString attName, attNames)
			{
				if (0 == attName.compare("SRC", Qt::CaseInsensitive))
				{
					srcName = attName;
					break;
				}
			}

			if (!srcName.isEmpty())
			{
				QString imagePath = webElement.attribute(srcName, "");
				if (!imagePath.isEmpty())
				{	
					QUrl url = QUrl::fromPercentEncoding(imagePath.toUtf8());
					if (url.isValid())
					{
						m_downloadImageAction->setData(url.toString());
						menu->addAction(m_downloadImageAction);
					}
				}
			}
		}
	}
}

void GlobalNotificationWebView::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	setBG(bgPixmap, bgSizes);	

	ui->title->setStyleSheet("font-size: 12pt; color: white;");

	ui->webView->setStyleSheet("QWidget#webView {background: white;}");
}

void GlobalNotificationWebView::load(const QString &url)
{
	if (url.isEmpty())
		return;
	
	QNetworkRequest request;
	request.setUrl(QUrl::fromUserInput(url));
	if (HttpPool::needApiCheck())
	{
		// add api check header
		HttpPool::addRequestApiCheck(request);
	}
	ui->webView->load(request);
}

void GlobalNotificationWebView::populateJavaScriptWindowObject()
{
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("clientDelegate", this);
}

void GlobalNotificationWebView::on_toolButtonRefresh_clicked()
{
	ui->webView->reload();
}

void GlobalNotificationWebView::downloadImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	if (action != m_downloadImageAction)
		return;

	QString urlString = action->data().toString();
	if (!urlString.isEmpty())
	{
		emit openAttach(m_globalNotificationId, urlString, QString());
	}
}

void GlobalNotificationWebView::initUI()
{
	setWindowTitle(ui->title->text());

	ui->webView->setMenuDelegate(this);
	ui->webView->page()->setNetworkAccessManager(GlobalNotificationManager::getGlobalNotificationWebViewHttpManager());
	connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));

	// download image action
	m_downloadImageAction = new QAction(this);
	m_downloadImageAction->setText(tr("Download Image"));
	connect(m_downloadImageAction, SIGNAL(triggered()), this, SLOT(downloadImage()));
}
