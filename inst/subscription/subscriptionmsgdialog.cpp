#include "subscriptionmsgdialog.h"
#include "ui_subscriptionmsgdialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "subscriptionmodel.h"
#include "minisplitter.h"
#include "subscriptionmsgmanager.h"
#include "Account.h"
#include "settings/AccountSettings.h"
#include "subscriptionmsg4js.h"
#include <QWebPage>
#include <QWebFrame>
#include "util/AvatarUtil.h"
#include "clickablelabel.h"
#include "subscriptionmenuitem.h"
#include <QDebug>
#include "subscriptionmanager.h"
#include <QMovie>
#include <QWebElement>
#include "gui/guiconstants.h"
#include "subscriptionmsgwebpage.h"

static const int kDefaultMsgCount = 10;
static const int kBorderWidth = 5;

SubscriptionMsgDialog::SubscriptionMsgDialog(const QString &id, QWidget *parent)
	: FramelessDialog(parent), m_subscriptionId(id), 
	m_oldestInnerId(-1), m_message4Js(0), m_subscriptionMenu(this), m_webPage(0),
	m_messagesSequence(-1)
{
	ui = new Ui::SubscriptionMsgDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setMainLayout(ui->verticalLayoutMain);
	resize(588, GuiConstants::WidgetSize::ChatHeight+2*kBorderWidth);
	setMinimumSize(588, GuiConstants::WidgetSize::ChatHeight+2*kBorderWidth);
	setResizeable(true);
	setMaximizeable(true);

	initUI();

	setSkin();

	connect(ui->btnClose2, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize2, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->btnMaximize2, SIGNAL(clicked()), this, SLOT(triggerMaximize()));
	connect(&m_subscriptionMenu, SIGNAL(aboutToHide()), this, SLOT(onSubscriptionMenuHide()));
	connect(qPmApp->getSubscriptionMsgManager(), SIGNAL(messagesGot(qint64, QList<SubscriptionMsg>)),
		this, SLOT(onGotHistoryMsg(qint64, QList<SubscriptionMsg>)));

	// refresh menu list
	qPmApp->getSubscriptionManager()->getMenu(m_subscriptionId);
}

SubscriptionMsgDialog::~SubscriptionMsgDialog()
{
	delete ui;
}

QString SubscriptionMsgDialog::subscriptionId() const
{
	return m_subscriptionId;
}

void SubscriptionMsgDialog::getMessages()
{
	m_messagesSequence = qPmApp->getSubscriptionMsgManager()->getMessagesFromDB(m_subscriptionId);
}

void SubscriptionMsgDialog::setMessages(const QList<SubscriptionMsg> &msgs)
{
	if (msgs.isEmpty())
		return;

	m_message4Js->setMessages(msgs);

	if (msgs.count() < kDefaultMsgCount)
		m_message4Js->moreMsgTipClose();
	else
		m_message4Js->moreMsgTipShow();

	// update oldest inner id
	SubscriptionMsg msg = msgs[0];
	m_oldestInnerId = msg.innerId();
}

void SubscriptionMsgDialog::appendMessage(const SubscriptionMsg &msg)
{
	if (msg.innerId() != 0)
	{
		if (msg.innerId() < m_oldestInnerId)
			m_oldestInnerId = msg.innerId();
	}

	m_message4Js->appendMessage(msg);
}

void SubscriptionMsgDialog::addMenuAction(QMenu *menu, const QWebElement &webElement)
{
	if (menu)
	{
		if (webElement.hasClass("news_media_thumb") || 
			webElement.hasClass("news_item_thumb") ||
			webElement.hasClass("image_media_thumb"))
		{
			menu->clear();

			QString urlString = webElement.attribute("src");
			QUrl url = QUrl::fromPercentEncoding(urlString.toUtf8());
			if (url.isValid())
			{
				m_downloadImageAction->setData(url.toString());
				menu->addAction(m_downloadImageAction);
			}
		}
	}
}

void SubscriptionMsgDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_10.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 95;
	setBG(bgPixmap, bgSizes);

	ui->titlebar->setStyleSheet(
		"QWidget#titlebar {"
		"	background: rgb(242, 242, 242);"
		"	border-bottom: 1px solid rgb(217, 216, 221);"
		"}");
	ui->labelName->setStyleSheet("font-size: 13pt; color: #333333;");
	ui->labelId->setStyleSheet("font-size: 9pt; color: #666666;");
	ui->textEdit->setStyleSheet(
		"QTextEdit#textEdit {"
			"border: none;"
			"border-image: none;"
			"background: rgb(255, 255, 255);"
		"}");
	ui->msgTopWidget->setStyleSheet("QWidget#msgTopWidget {background: transparent; border: none; border-bottom: 1px solid rgb(219, 219, 219);}");
	ui->sendBar->setStyleSheet("QWidget#sendBar {background: rgb(255, 255, 255); border: none;}");

	ui->loadPage->setStyleSheet("QWidget#loadPage {background: rgb(255, 255, 255);}");
	ui->labelLoadingText->setStyleSheet("color: rgb(128, 128, 128);");

	ui->menuBar->setStyleSheet("QWidget#menuBar {background-color: white;}");

	QString switchMenuButtonStyle = QString(
		"QPushButton {"
		"padding-left: 0px;"
		"padding-right: 0px;"
		"border: none;"
		"border-image: none;"
		"background-color: rgb(255, 255, 255);"
		"color: rgb(128, 128, 128);"
		"image: url(:/subscription/menuswitch.png);"
		"}"
		"QPushButton:!checked:hover:!pressed {"
		"border-image: none;"
		"image: url(:/subscription/menuswitch_down.png);"
		"}"
		"QPushButton:!checked:hover:pressed {"
		"border-image: none;"
		"image: url(:/subscription/menuswitch_down.png);"
		"}");
	ui->btnShowMenu->setStyleSheet(switchMenuButtonStyle);
	ui->btnHideMenu->setStyleSheet(switchMenuButtonStyle);

	m_subscriptionMenu.setStyleSheet(
		"QMenu {"
		"	border-image: none;"
		"	background-image: none;"
		"	margin: 0px;"
		"	border-radius: 2px;"
		"	border: 1px solid rgb(194, 194, 194);"
		"	background: white;"
		"	color: #666666;"
		"}"
		"QMenu::item {"
		"	border-image: none;"
		"	background-image: none;"
		"	padding-top: 7px;"
		"	padding-left: 20px;"
		"	padding-right: 20px;"
		"	padding-bottom: 7px;"
		"	color: #666666;"
		"}"
		"QMenu::item:selected:!disabled {"
		"	border-image: none;"
		"	background-image: none;"
		"	border: 0px;"
		"	background: rgb(226, 226, 226);"
		"	color: #666666;"
		"}");
}

void SubscriptionMsgDialog::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		close();
		return;
	}

	FramelessDialog::keyPressEvent(event);
}

void SubscriptionMsgDialog::on_btnSend_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		showTip(tr("You are offline, can't send message, please try when online"));
		return;
	}

	QString content = ui->textEdit->toPlainText();
	if (content.trimmed().isEmpty())
	{
		showTip(tr("Please input message content"));
		return;
	}

	const int kMaxLength = 600;
	if (content.length() > kMaxLength)
	{
		showTip(tr("Message content no more than %1 characters").arg(kMaxLength));
		return;
	}

	ui->textEdit->setPlainText("");
	ui->textEdit->clear();

	SubscriptionMsg msg = qPmApp->getSubscriptionMsgManager()->sendMsg(m_subscriptionId, content);
	if (!msg.id().isEmpty())
	{
		appendMessage(msg);
	}
}

void SubscriptionMsgDialog::showTip(const QString &tip)
{
	ui->tipWidget->setTipText(tip);
	ui->tipWidget->autoShow();
}

void SubscriptionMsgDialog::on_btnSendSetting_clicked()
{
	int nSendType = 0;
	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		nSendType = accountSettings->getSendType();

	m_sendShortcutMenu.actions().value(nSendType)->setChecked(true);

	QPoint pos;
	pos.setX(6);
	pos.setY(-m_sendShortcutMenu.sizeHint().height()-1);

	m_sendShortcutMenu.setGeometry(QRect(ui->btnSendSetting->mapToGlobal(pos), m_sendShortcutMenu.size()));
	m_sendShortcutMenu.exec(ui->btnSendSetting->mapToGlobal(pos));
}

void SubscriptionMsgDialog::slot_sendshortkey_changed(QAction* action)
{
	int nSendType = m_sendShortcutMenu.actions().indexOf(action);
	ui->btnSend->setToolTip(action->data().toString());

	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		accountSettings->setSendType(nSendType);
}

void SubscriptionMsgDialog::on_icon_clicked()
{
	emit openSubscriptionDetail(m_subscriptionId);
}

void SubscriptionMsgDialog::on_btnExit_clicked()
{
	close();
}

void SubscriptionMsgDialog::fetchHistoryMsg()
{
	qint64 seq = qPmApp->getSubscriptionMsgManager()->getMessagesFromDB(m_subscriptionId, m_oldestInnerId, kDefaultMsgCount);
	m_historySequences.append(seq);
}

void SubscriptionMsgDialog::populateJavaScriptWindowObject()
{
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("Message4Js", m_message4Js);
}

void SubscriptionMsgDialog::onMessage4JsLoadSucceeded()
{
	QMovie *movie = ui->labelLoadingIcon->movie();
	if (movie)
		movie->stop();
	ui->stackedWidget->setCurrentIndex(1);
	
	QTimer::singleShot(0, this, SIGNAL(initUIComplete()));
}

void SubscriptionMsgDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
	SubscriptionDetail subscription = subscriptionModel->subscription(m_subscriptionId);

	// icon
	ui->icon->setClickable(true);
	QPixmap origLogo = qPmApp->getModelManager()->subscriptionLogo(m_subscriptionId);
	QPixmap logo = origLogo.scaled(ui->icon->sizeHint(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->icon->setPixmap(logo);
	setWindowIcon(QIcon(origLogo));

	ui->labelName->setText(subscription.name());
	ui->labelId->setText(tr("ID: %1").arg(subscription.num()));
	setWindowTitle(ui->labelName->text());

	// splitter
	MiniSplitter *splitter = new MiniSplitter(this);
	splitter->setOrientation(Qt::Vertical);
	splitter->addWidget(ui->msgTopWidget);
	splitter->addWidget(ui->msgBottomWidget);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 0);
	splitter->setSizes(QList<int>() << 263 << 140);
	splitter->setChildrenCollapsible(false);

	QVBoxLayout *splitterLayout = new QVBoxLayout();
	splitterLayout->addWidget(splitter);
	splitterLayout->setSpacing(0);
	splitterLayout->setContentsMargins(0, 0, 0, 0);
	ui->msgArea->setLayout(splitterLayout);
	m_chatSplitter = splitter;

	connect(ui->btnShowMenu, SIGNAL(clicked()), this, SLOT(showMenuBar()));
	connect(ui->btnHideMenu, SIGNAL(clicked()), this, SLOT(hideMenuBar()));
	
	// web view related
	m_webPage = new SubscriptionMsgWebPage(ui->webView);
	connect(m_webPage, SIGNAL(avatarClicked(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(m_webPage, SIGNAL(subscriptionAvatarClicked(QString)), this, SIGNAL(openSubscriptionDetail(QString)));
	ui->webView->setPage(m_webPage);
	ui->webView->setMenuDelegate(this);
	ui->webView->page()->setNetworkAccessManager(SubscriptionManager::getSubscriptionWebViewHttpManager());

	QString selfId = Account::instance()->id();
	QString selfName = modelManager->userName(selfId);
	m_message4Js = new SubscriptionMsg4Js(this);
	m_message4Js->setUid(selfId);
	m_message4Js->setUName(selfName);
	QDir avatarDir(Account::instance()->avatarPath());
	QString avatarName = AvatarUtil::avatarName(selfId);
	QString avatarPath = avatarDir.absoluteFilePath(avatarName);
	m_message4Js->setUAvatar(avatarPath);

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoadingIcon->setMovie(movie);
	movie->start();
	ui->stackedWidget->setCurrentIndex(0);

	m_message4Js->setSubscriptionId(subscription.id());
	m_message4Js->setSubscriptionName(subscription.name());
	QDir logoDir(Account::instance()->subscriptionDir());
	QString logoName = SubscriptionModel::logoFileName(subscription.logo());
	QString logoPath = logoDir.absoluteFilePath(logoName);
	m_message4Js->setSubscriptionLogo(logoPath);

	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onUserChanged(QString)));
	connect(subscriptionModel, SIGNAL(subscriptionLogoChanged(QString)), this, SLOT(onLogoChanged(QString)));

	connect(m_message4Js, SIGNAL(fetchHistoryMessage()), this, SLOT(fetchHistoryMsg()));
	connect(m_message4Js, SIGNAL(loadSucceeded()), this, SLOT(onMessage4JsLoadSucceeded()));
	connect(m_message4Js,SIGNAL(openTitle(QString, QString, QString)), this, SIGNAL(openTitle(QString, QString, QString)));
	connect(m_message4Js, SIGNAL(openAttach(QString, QString, QString)), this, SIGNAL(openAttach(QString, QString, QString)));
	connect(this, SIGNAL(cleanup()), m_message4Js, SIGNAL(cleanup()));
	connect(this, SIGNAL(initUIComplete()), m_message4Js, SIGNAL(initUIComplete()));

	ui->webView->setUrl(QUrl("qrc:/html/subscriptionmsglist.html"));
	connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));
	
	// send signal
	connect(ui->textEdit, SIGNAL(sendMessage()), SLOT(on_btnSend_clicked()));
	connect(ui->textEdit, SIGNAL(showInfoTip(QString)), SLOT(showTip(QString)));

	// tip widget
	ui->tipWidget->setTipPixmap(QPixmap(":/images/Icon_76.png"));
	ui->tipWidget->stopShow();

	// send settings
	QAction* pAction = 0;
	QActionGroup* pActionGroup = new QActionGroup(this);
	m_sendShortcutMenu.setObjectName(QString::fromLatin1("SendShortcutMenu"));

	pAction = m_sendShortcutMenu.addAction(tr("Press Enter to send message"));
	pAction->setCheckable(true);
	pAction->setData(tr("Press Enter to send message, press Ctrl+Enter to change line"));
	pActionGroup->addAction(pAction);

	pAction = m_sendShortcutMenu.addAction(tr("Press Ctrl+Enter to send message"));
	pAction->setCheckable(true);
	pAction->setData(tr("Press Ctrl+Enter to send message, press Enter to change line"));
	pActionGroup->addAction(pAction);

	connect(pActionGroup, SIGNAL(triggered(QAction*)), SLOT(slot_sendshortkey_changed(QAction*)));

	// self settings
	int nSendType = 0;
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
		nSendType = accountSettings->getSendType();

	if(nSendType == 0)
	{
		pAction = m_sendShortcutMenu.actions().value(0);
		pAction->setChecked(true);
		ui->btnSend->setToolTip(pAction->data().toString());
	}
	else
	{
		pAction = m_sendShortcutMenu.actions().value(1);
		pAction->setChecked(true);
		ui->btnSend->setToolTip(pAction->data().toString());
	}

	// menu
	setSubscriptionMenu();
	connect(subscriptionModel, SIGNAL(subscriptionMenuChanged(QString)), this, SLOT(onSubscriptionMenuChanged(QString)));

	SubscriptionMenu *subscriptionMenu = subscriptionModel->subscriptionMenu(m_subscriptionId);
	if (!subscriptionMenu)
	{
		ui->menuBar->setVisible(false);
		ui->msgBottomWidget->setVisible(true);
	}
	else
	{
		ui->menuBar->setVisible(true);
		ui->msgBottomWidget->setVisible(false);
	}

	// download image action
	m_downloadImageAction = new QAction(this);
	m_downloadImageAction->setText(tr("Download Image"));
	connect(m_downloadImageAction, SIGNAL(triggered()), this, SLOT(downloadImage()));

	// only support plain text
	ui->textEdit->setAcceptRichText(false);
}

void SubscriptionMsgDialog::onMaximizeStateChanged(bool isMaximized)
{
	if (isMaximized)
	{
		ui->btnMaximize2->setChecked(true);
		ui->btnMaximize2->setToolTip(tr("Restore"));
	}
	else
	{
		ui->btnMaximize2->setChecked(false);
		ui->btnMaximize2->setToolTip(tr("Maximize"));
	}
}

void SubscriptionMsgDialog::onSubscriptionMenuChanged(const QString &subscriptionId)
{
	if (m_subscriptionId == subscriptionId)
	{
		setSubscriptionMenu();

		ModelManager *modelManager = qPmApp->getModelManager();
		SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
		SubscriptionMenu *subscriptionMenu = subscriptionModel->subscriptionMenu(m_subscriptionId);
		if (!subscriptionMenu)
			hideMenuBar();
		else
			showMenuBar();
	}
}

void SubscriptionMsgDialog::onSubscriptionMenuButtonToggled(bool checked)
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	if (!button)
		return;

	QAction *menuAction = m_menuActions.value(button, 0);
	if (!menuAction)
		return;

	if (checked)
	{
		bool hasChecked = false;
		foreach (QPushButton *menuButton, m_menuButtons)
		{
			if (menuButton != button && menuButton->isChecked())
			{
				hasChecked = true;
			}
		}
		if (hasChecked)
		{
			foreach (QPushButton *menuButton, m_menuButtons)
			{
				menuButton->setChecked(false);
			}
			return;
		}

		QList<QAction *> subMenuActions = m_subMenuActions.value(button, QList<QAction *>());
		if (subMenuActions.count() > 0)
		{
			// show sub-menu
			m_subscriptionMenu.clear();
			m_subscriptionMenu.addActions(subMenuActions);
			QPoint pt = button->mapToGlobal(QPoint(0, -2));
			QPoint menuPt;
			menuPt.setX(pt.x()+(button->width()-m_subscriptionMenu.sizeHint().width())/2);
			menuPt.setY(pt.y()-m_subscriptionMenu.sizeHint().height());
			m_subscriptionMenu.exec(menuPt);
		}
		else
		{
			// trigger action
			triggerMenuAction(menuAction);
			button->setChecked(false);
		}
	}
}

void SubscriptionMsgDialog::onSubscriptionSubMenuClicked()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	triggerMenuAction(action);
}

void SubscriptionMsgDialog::downloadImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	if (action != m_downloadImageAction)
		return;

	QString urlString = action->data().toString();
	QUrl url = QUrl::fromUserInput(urlString);
	if (url.isValid())
	{
		emit openAttach(m_subscriptionId, url.toString(), QString());
	}
}

void SubscriptionMsgDialog::onUserChanged(const QString &uid)
{
	if (uid == Account::instance()->id())
	{
		if (m_webPage)
		{
			m_webPage->onAvatarChanged(uid);
		}
	}
}

void SubscriptionMsgDialog::onLogoChanged(const QString &subscriptionId)
{
	if (subscriptionId == m_subscriptionId)
	{
		// icon
		ModelManager *modelManager = qPmApp->getModelManager();
		QPixmap origLogo = modelManager->subscriptionLogo(m_subscriptionId);
		QPixmap logo = origLogo.scaled(ui->icon->sizeHint(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui->icon->setPixmap(logo);
		setWindowIcon(QIcon(origLogo));

		// chat icon
		if (m_webPage)
		{
			m_webPage->onSubscriptionAvatarChanged(subscriptionId);
		}
	}
}

void SubscriptionMsgDialog::showMenuBar()
{
	if (!ui->menuBar->isVisible())
	{
		ui->menuBar->setVisible(true);
		ui->msgBottomWidget->setVisible(false);
	}
}

void SubscriptionMsgDialog::hideMenuBar()
{
	if (ui->menuBar->isVisible())
	{
		ui->menuBar->setVisible(false);
		ui->msgBottomWidget->setVisible(true);
		m_chatSplitter->setSizes(QList<int>() << 263 << 140);
	}
}

void SubscriptionMsgDialog::onSubscriptionMenuHide()
{
	QPoint cursorPos = QCursor::pos();
	QPushButton *menuButton = 0;
	bool clickInButton = false;
	foreach (menuButton, m_menuButtons)
	{
		QPoint pt = menuButton->mapToGlobal(QPoint(0, 0));
		QRect rt(pt, menuButton->size());

		if (rt.contains(cursorPos))
		{
			clickInButton = true;
			break;
		}
	}

	if (!clickInButton)
	{
		foreach (menuButton, m_menuButtons)
		{
			menuButton->setChecked(false);
		}
	}
}

void SubscriptionMsgDialog::setSubscriptionMenu()
{
	// remove original
	m_subscriptionMenu.close();
	QHBoxLayout *menuLayout = static_cast<QHBoxLayout *>(ui->menuContainer->layout());
	if (menuLayout)
	{
		foreach (QWidget *w, m_menuWidgets)
		{
			menuLayout->removeWidget(w);
			w->deleteLater();
		}
		delete menuLayout;
		menuLayout = 0;
	}
	m_menuWidgets.clear();
	m_menuButtons.clear();
	qDeleteAll(m_menuActions.values());
	m_menuActions.clear();
	foreach (QList<QAction *> subMenuActions, m_subMenuActions.values())
	{
		qDeleteAll(subMenuActions);
	}
	m_subMenuActions.clear();

	// set menu
	ModelManager *modelManager = qPmApp->getModelManager();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
	SubscriptionMenu *subscriptionMenu = subscriptionModel->subscriptionMenu(m_subscriptionId);
	if (!subscriptionMenu)
		return;

	menuLayout = new QHBoxLayout(ui->menuContainer);
	menuLayout->setContentsMargins(0, 0, 0, 0);
	menuLayout->setSpacing(0);

	QString menuButtonStyle = QString(
		"QPushButton {"
		"border: none;"
		"border-image: none;"
		"background-color: rgb(255, 255, 255);"
		"color: rgb(128, 128, 128);"
		"border-right: 1px solid rgb(229, 229, 229);"
		"}"
		"QPushButton:hover:!pressed {"
		"border-image: none;"
		"background-color: rgb(241, 245, 243);"
		"}"
		"QPushButton:hover:pressed {"
		"border-image: none;"
		"background-color: rgb(241, 245, 243);"
		"}");
	
	QList<SubscriptionMenuItem *> menuItems = subscriptionMenu->menuItems();
	for (int i = 0; i < menuItems.count(); ++i)
	{
		SubscriptionMenuItem *menuItem = menuItems[i];
		if (!menuItem)
			continue;

		// add button
		QPushButton *button = new QPushButton(this);
		button->setCheckable(true);
		button->setChecked(false);
		button->setMinimumHeight(ui->menuContainer->height()-1);
		button->setStyleSheet(menuButtonStyle);
		if (menuItem->subMenus().count() > 0)
		{
			button->setIcon(QIcon(":/subscription/menu.png"));
			button->setText(menuItem->name());
		}
		else
		{
			button->setText(menuItem->name());
		}
		connect(button, SIGNAL(toggled(bool)), this, SLOT(onSubscriptionMenuButtonToggled(bool)));
		menuLayout->addWidget(button);
		m_menuButtons.append(button);
		m_menuWidgets.append(button);

		// create menu actions
		QAction *action = new QAction(this);
		action->setText(menuItem->name());
		m_menuActions.insert(button, action);

		SubscriptionMenuItem::MenuItemType menuType = menuItem->type();
		if (menuItem->subMenus().count() > 0)
		{
			QList<QAction *> subActions;
			QList<SubscriptionMenuItem *> subMenus = menuItem->subMenus();
			for (int i = 0; i < subMenus.count(); ++i)
			{
				SubscriptionMenuItem *subMenu = subMenus[i];
				SubscriptionMenuItem::MenuItemType subMenuType = subMenu->type();
				QAction *subAction = new QAction(this);
				subAction->setText(subMenu->name());
				if (subMenuType == SubscriptionMenuItem::ClickItem || subMenuType == SubscriptionMenuItem::MediaItem)
				{
					QString actionData = QString("%1 %2").arg((int)subMenuType).arg(subMenu->key());
					subAction->setData(actionData);
				}
				else // url
				{
					QString actionData = QString("%1 %2").arg((int)subMenuType).arg(subMenu->url());
					subAction->setData(actionData);
				}
				connect(subAction, SIGNAL(triggered()), this, SLOT(onSubscriptionSubMenuClicked()));
				subActions.append(subAction);
			}

			m_subMenuActions.insert(button, subActions);
		}
		else if (menuType == SubscriptionMenuItem::ClickItem || menuType == SubscriptionMenuItem::MediaItem)
		{
			QString actionData = QString("%1 %2").arg((int)menuType).arg(menuItem->key());
			action->setData(actionData);
		}
		else // if (menuType == SubscriptionMenuItem::ViewItem)
		{
			QString actionData = QString("%1 %2").arg((int)menuType).arg(menuItem->url());
			action->setData(actionData);
		}
	}
}

void SubscriptionMsgDialog::triggerMenuAction(const QAction *action)
{
	if (!action)
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		showTip(tr("You are offline, can't use menu, please try when online"));
		return;
	}

	QString actionData = action->data().toString();
	QStringList params = actionData.split(" ");
	if (params.count() != 2)
		return;

	int menuType = params[0].toInt();
	if (menuType == SubscriptionMenuItem::ClickItem || menuType == SubscriptionMenuItem::MediaItem)
	{
		// request click
		emit clickMenu(m_subscriptionId, params[1]);
	}
	else if (menuType == SubscriptionMenuItem::ViewItem)
	{
		// open url
		emit openUrl(m_subscriptionId, params[1]);
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "invalid menu type: " << menuType;
	}
}

void SubscriptionMsgDialog::onGotHistoryMsg(qint64 seq, const QList<SubscriptionMsg> &msgs)
{
	if (m_messagesSequence == seq)
	{
		m_messagesSequence = -1;
		setMessages(msgs);
	}
	else
	{
		int index = m_historySequences.indexOf(seq);
		if (index < 0)
			return;

		m_historySequences.removeAt(index);

		if (!msgs.isEmpty())
		{
			m_message4Js->insertMessagesAtTop(msgs);

			// update oldest inner id
			SubscriptionMsg msg = msgs[0];
			m_oldestInnerId = msg.innerId();
		}
		m_message4Js->moreMsgFinished();

		if (msgs.count() < kDefaultMsgCount)
			m_message4Js->moreMsgTipClose();
		else
			m_message4Js->moreMsgTipShow();
	}
}

