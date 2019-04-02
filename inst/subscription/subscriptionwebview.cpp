#include "subscriptionwebview.h"
#include "ui_subscriptionwebview.h"
#include <QWebFrame>
#include <QWebPage>
#include <QWebElement>
#include <QUrl>
#include "Account.h"
#include <QDesktopServices>
#include "subscriptionmanager.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "pmessagebox.h"
#include <QMenu>
#include <QAction>
#include <QNetworkRequest>
#include "http/HttpPool.h"

QPointer<SubscriptionWebView> SubscriptionWebView::s_webView;

SubscriptionWebView::SubscriptionWebView(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::SubscriptionWebView();
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

SubscriptionWebView::~SubscriptionWebView()
{
	delete ui;
}

void SubscriptionWebView::setSubscriptionId(const QString &subId)
{
	m_subscriptionId = subId;
}

SubscriptionWebView *SubscriptionWebView::getWebView()
{
	if (s_webView.isNull())
	{
		s_webView = new SubscriptionWebView();
	}
	return s_webView.data();
}

void SubscriptionWebView::toSubDetail(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	if (!qPmApp->getModelManager()->hasSubscriptionItem(subscriptionId))
	{
		PMessageBox::warning(this, tr("Tip"), tr("You need to follow the subscription first"));
		return;
	}

	emit openSubscriptionDetail(subscriptionId);
}

void SubscriptionWebView::openUrl(const QString &url)
{
	if (url.isEmpty())
		return;

	QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void SubscriptionWebView::addMenuAction(QMenu *menu, const QWebElement &webElement)
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

void SubscriptionWebView::setSkin()
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

void SubscriptionWebView::load(const QString &url)
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

void SubscriptionWebView::populateJavaScriptWindowObject()
{
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("clientDelegate", this);
}

void SubscriptionWebView::on_toolButtonRefresh_clicked()
{
	ui->webView->reload();
}

void SubscriptionWebView::downloadImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	if (action != m_downloadImageAction)
		return;

	QString urlString = action->data().toString();
	if (!urlString.isEmpty())
	{
		emit openAttach(m_subscriptionId, urlString, QString());
	}
}

void SubscriptionWebView::initUI()
{
	setWindowTitle(ui->title->text());

	ui->webView->setMenuDelegate(this);
	ui->webView->page()->setNetworkAccessManager(SubscriptionManager::getSubscriptionWebViewHttpManager());
	connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));

	// download image action
	m_downloadImageAction = new QAction(this);
	m_downloadImageAction->setText(tr("Download Image"));
	connect(m_downloadImageAction, SIGNAL(triggered()), this, SLOT(downloadImage()));
}
