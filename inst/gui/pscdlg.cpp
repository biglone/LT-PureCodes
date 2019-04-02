#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QDebug>

#include "cttk/Base.h"	

#include "passwdmodifydlg.h"
#include "aboutdialog.h"

#include "model/ModelManager.h"
#include "model/lastcontactmodeldef.h"
#include "model/rostermodeldef.h"
#include "model/groupmodeldef.h"
#include "model/orgstructitemdef.h"
#include "model/orgstructmodeldef.h"
#include "model/sortfilterproxymodel.h"
#include "model/DiscussModeldef.h"
#include "model/DiscussItemdef.h"

#include "manager/presencemanager.h"
#include "manager/DiscussManager.h"
#include "manager/groupmanager.h"

#include "BaseProcessor.h"

#include "Constants.h"

#include "settings/GlobalSettings.h"

#include "login/Account.h"
#include "buddymgr.h"
#include "PmApp.h"

#include "statuschanger/StatusChanger.h"

#include "bean/DetailItem.h"

#include "pscdlg.h"
#include "ui_pscdlg.h"

#include "flickerhelper.h"

#include "unreadmessagebox.h"

#include "ModifyProcess.h"

#include "filterlineedit.h"

#include "model/groupitemdef.h"

#include "editfilteritemdelegate.h"
#include "editfiltertreeview.h"

#include "rostertreeview.h"
#include "grouplistview.h"
#include "lastcontactview.h"
#include "OrganizationTreeView.h"

#include "FileManagerDlg.h"

#include "manager/rostermanager.h"

#include "contactcard.h"
#include "myinfodialog.h"
#include "widgetmanager.h"
#include "manager/contactinfomanager.h"
#include "chatdialog.h"
#include "groupdialog.h"
#include "DiscussDialog.h"
#include "systemsettingsdialog.h"
#include "messagemanagerdlg.h"
#include "CreateDiscussDialog.h"
#include "groupitemlistmodeldef.h"

#include "qxtglobalshortcut.h"
#include "SnapShot.h"
#include "util/PlayBeep.h"
#include "closeoptiondialog.h"
#include "logger/logger.h"
#include "unreadmsgmodel.h"
#include "pmessagebox.h"
#include "debugdlg.h"
#include "grouppanel.h"
#include "discusstreeview.h"
#include "msgmultisendmemberdlg.h"
#include "shortcutconflictdlg.h"
#include "util/PinYinConverter.h"
#include <QPropertyAnimation>
#include "ComponentMessageDB.h"
#include "LastContactDB.h"
#include "searchdialog.h"
#include "addfriendlistdlg.h"
#include "addfriendrequestdlg.h"
#include "changenoticemgr.h"
#include "unreadmsgitem.h"
#include "interphonedialog.h"
#include "interphonemanager.h"
#include "subscriptionmodel.h"
#include "subscriptionmanager.h"
#include "subscriptionmsgmanager.h"
#include "subscriptiondetailandhistorymanager.h"
#include "blacklistdialog.h"
#include "blacklistmodel.h"
#include "configmanager.h"
#include "plaintextlineinput.h"
#include "addfriendmanager.h"
#include "appmanagedialog.h"
#include "unionchatdialog.h"
#include "httpimagetoolbutton.h"
#include "subscriptiondialog.h"
#include "subscriptionlastmsgdialog.h"
#include "subscriptionlastmsgmodel.h"
#include "appmanagemodel.h"
#include "guiconstants.h"

static const int kPageIndexLastContact  = 0;
static const int kPageIndexRoster       = 1;
static const int kPageIndexOs           = 2;
static const int kPageIndexGroup        = 3;

CPscDlg *CPscDlg::s_pscDlg = 0;

CPscDlg::CPscDlg(QWidget *parent) 
	: FramelessDialog(parent)
	, ui(new Ui::CPscDlg)
	, m_pStatusMenu(0)
	, m_bClose(false)
	, m_bKick(false)
	, m_rosterView(0)
	, m_orgView(0)
	, m_groupView(0)
	, m_lastContactView(0)
	, m_dockReady(false)
	, m_dockState(false)
	, m_leftKeyPressed(false)
{
    ui->setupUi(this);

	setWindowTitle(GlobalSettings::title());

	setTopmost(Account::settings()->isPscTopmost());

	setAttribute(Qt::WA_DeleteOnClose, true);
	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	setMouseTracking(true);
	setMainLayout(ui->verticalLayoutMain);

	m_pFlickerHelper = new FlickerHelper(this);

	InitUI();

	setMinimumSize(QSize(AccountSettings::kDefaultPscWidth, AccountSettings::kDefaultPscHeight));
	setMaximumWidth(754);
	setResizeable(true);
	setMaximizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(slot_clickedClose()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(slot_clickedMinimized()));
	connect(qPmApp->getUnreadMsgModel(), SIGNAL(unreadItemCountChanged()), this, SLOT(onUnreadMsgCountChanged()));
	connect(qPmApp->getUnreadMsgModel(), SIGNAL(preIgnoreAll()), this, SLOT(onUnreadMsgPreIgnoreAll()));

	setSkin();

	s_pscDlg = this;
}

CPscDlg::~CPscDlg()
{
	s_pscDlg = 0;

	if (m_pPasswdModifyDlg)
	{
		m_pPasswdModifyDlg.data()->deleteLater();
	}

	if (m_pAboutDialog)
	{
		m_pAboutDialog.data()->deleteLater();
	}

	if (m_pUnreadMessageBox)
	{
		m_pUnreadMessageBox.data()->deleteLater();
	}

	if (m_pContactCard)
	{
		m_pContactCard.data()->deleteLater();
	}

	delete ui;
}

CPscDlg *CPscDlg::instance()
{
	return s_pscDlg;
}

void CPscDlg::openChat(const QString &id)
{
	qPmApp->getBuddyMgr()->openChat(id);
}

void CPscDlg::openChat(const QStringList &ids)
{
	foreach (QString id, ids)
	{
		openChat(id);
	}
}

void CPscDlg::openGroupChat(const QString &id)
{
	qPmApp->getBuddyMgr()->openGroupChat(id);
}

void CPscDlg::openGroupChat(const QStringList &ids)
{
	foreach (QString id, ids)
	{
		openGroupChat(id);
	}
}

void CPscDlg::openDiscussChat( const QString &id )
{
	qPmApp->getBuddyMgr()->openDiscussChat(id);
}

void CPscDlg::openDiscussChat( const QStringList &ids )
{
	foreach (QString id, ids)
	{
		openDiscussChat(id);
	}
}

void CPscDlg::openAllUnreadChats(const QList<int> &msgTypes, const QStringList &ids)
{
	qPmApp->getBuddyMgr()->openAllUnreadChats(msgTypes, ids);
}

void CPscDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 0;
	setBG(bgPixmap, bgSizes);

	// set qss file
	QFile qssFile(":/qss/pscdlg.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		setStyleSheet(qss);
		qssFile.close();
	}

	// set page widget tab style
	WidgetBorder pageWidgetBorder;
	pageWidgetBorder.left = 2;
	pageWidgetBorder.right = 2;
	StyleUrls pageWidgetUrls;
	pageWidgetUrls.normal = QString(":/theme/maintab/main_tab_normal.png");
	pageWidgetUrls.hover = QString(":/theme/maintab/main_tab_normal.png");
	pageWidgetUrls.selected = QString(":/theme/maintab/main_tab_selected.png");
	WidgetBorderStyle pageWidgetBorderStyle;
	pageWidgetBorderStyle.border = pageWidgetBorder;
	pageWidgetBorderStyle.urls = pageWidgetUrls;
	ui->pageWidget->setTabButtonBorderStyle(pageWidgetBorderStyle);
	ui->pageWidget->setTabButtonSeperator(QString());

	// set button menu style
	StylePushButton::Info menuBtnInfo;
	menuBtnInfo.urlNormal = QString(":/images/Icon_127.png");
	menuBtnInfo.urlHover = QString(":/images/Icon_127_hover.png");
	menuBtnInfo.urlPressed = QString(":/images/Icon_127_pressed.png");
	menuBtnInfo.tooltip = tr("Open Main Menu");
	ui->btnMenu->setInfo(menuBtnInfo);

	// set search style
	StyleToolButton::Info searchBtnStyle;
	searchBtnStyle.urlNormal = QString(":/images/Icon_119.png");
	searchBtnStyle.urlHover = QString(":/images/Icon_119_hover.png");
	searchBtnStyle.urlPressed = QString(":/images/Icon_119_pressed.png");
	searchBtnStyle.tooltip = tr("Search");
	ui->btnSearch->setInfo(searchBtnStyle);

	// set button system setting style
	StyleToolButton::Info sysSettingBtnInfo;
	sysSettingBtnInfo.urlNormal = QString(":/images/Icon_78.png");
	sysSettingBtnInfo.urlHover = QString(":/images/Icon_78_hover.png");
	sysSettingBtnInfo.urlPressed = QString(":/images/Icon_78_pressed.png");
	sysSettingBtnInfo.tooltip = tr("Open System Setting");
	ui->btnSystemSettings->setInfo(sysSettingBtnInfo);

	// set button message manager style
	StyleToolButton::Info msgManagerBtnInfo;
	msgManagerBtnInfo.urlNormal = QString(":/images/Icon_93.png");
	msgManagerBtnInfo.urlHover = QString(":/images/Icon_93_hover.png");
	msgManagerBtnInfo.urlPressed = QString(":/images/Icon_93_pressed.png");
	msgManagerBtnInfo.tooltip = tr("Open Message Manager");
	ui->btnMsgManager->setInfo(msgManagerBtnInfo);

	StyleToolButton::Info fileManagerBtnInfo;
	fileManagerBtnInfo.urlNormal = QString(":/images/Icon_117.png");
	fileManagerBtnInfo.urlHover = QString(":/images/Icon_117_hover.png");
	fileManagerBtnInfo.urlPressed = QString(":/images/Icon_117_pressed.png");
	fileManagerBtnInfo.tooltip = tr("Open File Manager");
	ui->btnFileManager->setInfo(fileManagerBtnInfo);

	/*
	// set button send mail style
	StyleToolButton::Info sendMailBtnInfo;
	sendMailBtnInfo.urlNormal = QString(":/images/Icon_104.png");
	sendMailBtnInfo.urlHover = QString(":/images/Icon_104_hover.png");
	sendMailBtnInfo.urlPressed = QString(":/images/Icon_104_hover.png");
	sendMailBtnInfo.tooltip = tr("Mail to Others");
	ui->btnSendMail->setInfo(sendMailBtnInfo);
	*/

	// set button add friend list style
	StyleToolButton::Info addFriendListBtnInfo;
	addFriendListBtnInfo.urlNormal = QString(":/images/Icon_111.png");
	addFriendListBtnInfo.urlHover = QString(":/images/Icon_111_hover.png");
	addFriendListBtnInfo.urlPressed = QString(":/images/Icon_111_pressed.png");
	addFriendListBtnInfo.tooltip = tr("New Friends");
	ui->btnAddFriendList->setInfo(addFriendListBtnInfo, StyleToolButton::State1st);

	addFriendListBtnInfo.urlNormal = QString(":/images/Icon_111_emphasis.png");
	addFriendListBtnInfo.urlHover = QString(":/images/Icon_111_emphasis_hover.png");
	addFriendListBtnInfo.urlPressed = QString(":/images/Icon_111_emphasis_pressed.png");
	addFriendListBtnInfo.tooltip = tr("New Friends");
	ui->btnAddFriendList->setInfo(addFriendListBtnInfo, StyleToolButton::State2nd);
	ui->btnAddFriendList->setState(StyleToolButton::State1st);

	// set button interphone style
	StyleToolButton::Info interphoneBtnInfo;
	interphoneBtnInfo.urlNormal = QString(":/images/Icon_123.png");
	interphoneBtnInfo.urlHover = QString(":/images/Icon_123_hover.png");
	interphoneBtnInfo.urlPressed = QString(":/images/Icon_123_pressed.png");
	interphoneBtnInfo.tooltip = tr("Create Interphone");
	ui->btnInterphone->setInfo(interphoneBtnInfo);

	/*
	// notice button
	StyleToolButton::Info noticeBtnInfo;
	noticeBtnInfo.urlNormal = QString(":/images/Icon_116.png");
	noticeBtnInfo.urlHover = QString(":/images/Icon_116_hover.png");
	noticeBtnInfo.urlPressed = QString(":/images/Icon_116_pressed.png");
	noticeBtnInfo.tooltip = tr("Open Notice Message");
	ui->tBtnNotice->setInfo(noticeBtnInfo);
	*/

	// set button menu style
	StylePushButton::Info menuBtnAppManage;
	menuBtnAppManage.urlNormal = QString(":/images/Icon_79.png");
	menuBtnAppManage.urlHover = QString(":/images/Icon_79_hover.png");
	menuBtnAppManage.urlPressed = QString(":/images/Icon_79_pressed.png");
	menuBtnAppManage.tooltip = tr("Open App Manager");
	ui->btnAppManage->setInfo(menuBtnAppManage);
}

void CPscDlg::init()
{
	setRosterModel(qPmApp->getModelManager()->rosterModel());
	setOrgModel(qPmApp->getModelManager()->orgStructModel());
	setGroupModel(qPmApp->getModelManager()->groupModel());
	setDiscussModel(qPmApp->getModelManager()->discussModel());
	setLastContactModel(qPmApp->getModelManager()->lastContactModel());

	bean::DetailItem* pItem = qPmApp->getModelManager()->detailItem(qPmApp->getAccount()->id());

	// user info 
	QString sName = qPmApp->getModelManager()->userName(qPmApp->getAccount()->id());
	ui->labName->setText(sName);
	ui->leditSignature->setText(qPmApp->getModelManager()->getSignature(pItem));

	// check to show unread subscription message
	SubscriptionMsgManager *subscriptionMsgManager = qPmApp->getSubscriptionMsgManager();
	QMap<QString, int> allSubscriptionUnreadMsgCount = subscriptionMsgManager->allUnreadMsgCount();
	foreach (QString subscriptionId, allSubscriptionUnreadMsgCount.keys())
	{
		int msgCount = allSubscriptionUnreadMsgCount[subscriptionId];
		onSubscriptionUnreadMsgChanged(subscriptionId, msgCount);
	}

	// link items
	QString linkItemsStr = GlobalSettings::linkItems();
	if (!linkItemsStr.isEmpty())
	{
		QList<GlobalSettings::LinkItem> linkItems = GlobalSettings::parseLinkItems(linkItemsStr);
		QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
		QString tBtnStyleSheet = QString(
			"QToolButton{"
			"	border: none;"
			"	padding: 2px;"
			"	background-color: transparent;"
			"	border-image: none;"
			"}"
			"QToolButton:pressed{"
			"	border: 1px solid rgba(255, 255, 255, 48);"
			"	background-color: transparent;"
			"	border-image: none;"
			"}"
			"QToolButton:hover:!pressed{"
			"	border: 1px solid rgba(255, 255, 255, 48);"
			"	background-color: transparent;"
			"	border-image: none;"
			"}"
			);
		for (int i = linkItems.count()-1; i >= 0; --i)
		{
			GlobalSettings::LinkItem linkItem = linkItems[i];
			HttpImageToolButton *linkButton = new HttpImageToolButton(this);
			linkButton->setStyleSheet(tBtnStyleSheet);
			linkButton->setNetworkAccessManager(naManager);
			linkButton->setCacheDir(Account::instance()->cachePath());
			linkButton->setIconSize(QSize(16, 16));
			linkButton->setToolTip(linkItem.name);
			linkButton->setData("url", linkItem.linkUrl);
			linkButton->setIcon(QIcon(":/images/Icon_131.png"));
			if (!linkItem.iconUrl.isEmpty())
			{
				linkButton->setHttpUrl(linkItem.iconUrl);
			}
			connect(linkButton, SIGNAL(clicked()), this, SLOT(onLinkItemClicked()));
			ui->pluginLayout->insertWidget(1, linkButton); // that is always a subscription button 
		}
	}

	// status
	StatusChanger* pStatusChanger = qPmApp->getStatusChanger();
	slot_statusChanged(pStatusChanger->curStatus());
	connect(pStatusChanger, SIGNAL(statusChanged(int)), this, SLOT(slot_statusChanged(int)));

	// avatar
	setAvatar(pStatusChanger->curStatus());

	// unread message box
	m_pUnreadMessageBox = new UnreadMessageBox(qPmApp->getUnreadMsgModel(), qPmApp->getSystemTray());

	// contact card
	m_pContactCard = new ContactCard();
	connect(m_pContactCard.data(), SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));
	connect(m_pContactCard.data(), SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)));
	
	// contact info manager
	m_pContactInfoManager.reset(new ContactInfoManager());

	// shortcut key
	m_pScreenshot = new SnapShot(this);
	connect(m_pScreenshot, SIGNAL(snapShotted(QString)), SLOT(onScreenshotOk(QString)));
	connect(m_pScreenshot, SIGNAL(snapShotCancelled()), SLOT(onScreenshotCancel()));

	m_pScreenshotShortcut = new QxtGlobalShortcut(this);
	connect(m_pScreenshotShortcut, SIGNAL(activated()), this, SLOT(screenshot()));

	m_pTakeMsgShortcut = new QxtGlobalShortcut(this);
	connect(m_pTakeMsgShortcut, SIGNAL(activated()), qPmApp, SLOT(slot_takeMsg()));

	setShortcutKey();

	// dock
	m_dockInAnimation = new QPropertyAnimation(this, "pos", this);
	m_dockInAnimation->setDuration(200);
	connect(m_dockInAnimation, SIGNAL(finished()), SLOT(onDockInFinished()));

	m_dockOutAnimation = new QPropertyAnimation(this, "pos", this);
	m_dockOutAnimation->setDuration(200);
	connect(m_dockOutAnimation, SIGNAL(finished()), SLOT(onDockOutFinished()));

	m_dockCheckTimer = new QTimer(this);
	m_dockCheckTimer->setInterval(500);
	m_dockCheckTimer->setSingleShot(false);
	connect(m_dockCheckTimer, SIGNAL(timeout()), this, SLOT(checkDock()));
	m_dockCheckTimer->start();

	// init signals & slots
	connect(qPmApp->getBaseProcessor(), SIGNAL(kicked()), this, SLOT(slot_receiveKick()));
	
	connect(qPmApp->getModelManager(), SIGNAL(detailChanged(QString)), SLOT(onMyInfoChanged(QString)));
	connect(qPmApp->getModelManager(), SIGNAL(detailChanged(QString)), SLOT(onDetailChanged(QString)));
	connect(qPmApp->getModelManager(), SIGNAL(removeSubscription(QString)), SLOT(removeSubscription(QString)));

	connect(qPmApp->getPresenceManager(), SIGNAL(presenceReceived(QString, int, int)), 
		m_rosterView, SLOT(onPresenceChanged(QString)));
	connect(qPmApp->getPresenceManager(), SIGNAL(presenceCleared()),
		m_rosterView, SLOT(onPresenceCleared()));

	connect(m_pContactInfoManager.data(), SIGNAL(chat(QString)), this, SLOT(openChat(QString)));
	connect(m_pContactInfoManager.data(), SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)));
	
	connect(m_rosterView, SIGNAL(chat(QString)), this, SLOT(openChat(QString)));
	connect(m_rosterView, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)));
	connect(m_rosterView, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));
	connect(m_rosterView, SIGNAL(viewPastChat(QString)), this, SLOT(viewContactHistory(QString)));
	connect(m_rosterView, SIGNAL(msgMultiSend(QStringList)), this, SLOT(msgMultiSend(QStringList)));
	connect(m_rosterView, SIGNAL(addBlack(QString)), this, SLOT(addBlack(QString)));
	connect(m_rosterView, SIGNAL(removeBlack(QString)), this, SLOT(removeBlack(QString)));
	connect(m_rosterView, SIGNAL(manageBlack()), this, SLOT(manageBlack()));
	connect(m_rosterView, SIGNAL(openSubscription()), this, SLOT(openSubscriptionDialog()));

	connect(m_orgView, SIGNAL(chat(QString)), this, SLOT(openChat(QString)));
	connect(m_orgView, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)));
	connect(m_orgView, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));
	connect(m_orgView, SIGNAL(viewPastChat(QString)), this, SLOT(viewContactHistory(QString)));
	connect(m_orgView, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)));
	connect(m_orgView, SIGNAL(msgMultiSend(QStringList)), this, SLOT(msgMultiSend(QStringList)));
	connect(m_orgView, SIGNAL(addBlack(QString)), this, SLOT(addBlack(QString)));
	connect(m_orgView, SIGNAL(removeBlack(QString)), this, SLOT(removeBlack(QString)));
	connect(m_orgView, SIGNAL(manageBlack()), this, SLOT(manageBlack()));

	connect(m_groupView, SIGNAL(chat(QString)), this, SLOT(openGroupChat(QString)));
	connect(m_groupView, SIGNAL(viewPastChat(QString)), this, SLOT(viewGroupHistory(QString)));
	connect(m_groupView, SIGNAL(removeGroupChat(QString)), this, SLOT(removeGroupChat(QString)));

	connect(m_discussView, SIGNAL(chat(QString)), this, SLOT(openDiscussChat(QString)));
	connect(m_discussView, SIGNAL(viewPastChat(QString)), this, SLOT(viewDiscussHistory(QString)));
	connect(m_discussView, SIGNAL(exitDiscuss(QString)), this, SLOT(exitDiscuss(QString)));
	connect(m_discussView, SIGNAL(removeDiscussChat(QString)), this, SLOT(removeDiscussChat(QString)));
	connect(m_discussView, SIGNAL(changeName(QString)), this, SLOT(changeDiscussName(QString)));

	connect(m_lastContactView, SIGNAL(chat(QString)), this, SLOT(openChat(QString)));
	connect(m_lastContactView, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)));
	connect(m_lastContactView, SIGNAL(groupChat(QString)), this, SLOT(openGroupChat(QString)));
	connect(m_lastContactView, SIGNAL(discussChat(QString)), this, SLOT(openDiscussChat(QString)));
	connect(m_lastContactView, SIGNAL(viewPastChat(QString)), this, SLOT(viewContactHistory(QString)));
	connect(m_lastContactView, SIGNAL(groupViewPastChat(QString)), this, SLOT(viewGroupHistory(QString)));
	connect(m_lastContactView, SIGNAL(discussViewPastChat(QString)), this, SLOT(viewDiscussHistory(QString)));
	connect(m_lastContactView, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));
	connect(m_lastContactView, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)));
	connect(m_lastContactView, SIGNAL(msgMultiSend(QStringList, QString)), this, SLOT(msgMultiSend(QStringList, QString)));
	connect(m_lastContactView, SIGNAL(openSubscriptionMsg(QString)), this, SLOT(openSubscriptionMsg(QString)));
	connect(m_lastContactView, SIGNAL(openSubscriptionDetail(QString)), this, SLOT(openSubscriptionDetail(QString)));
	connect(m_lastContactView, SIGNAL(openSubscriptionHistory(QString)), this, SLOT(openSubscriptionHistory(QString)));
	connect(m_lastContactView, SIGNAL(addBlack(QString)), this, SLOT(addBlack(QString)));
	connect(m_lastContactView, SIGNAL(removeBlack(QString)), this, SLOT(removeBlack(QString)));
	connect(m_lastContactView, SIGNAL(manageBlack()), this, SLOT(manageBlack()));
	connect(m_lastContactView, SIGNAL(removeGroupChat(QString)), this, SLOT(removeGroupChat(QString)));
	connect(m_lastContactView, SIGNAL(removeDiscussChat(QString)), this, SLOT(removeDiscussChat(QString)));

	connect(qPmApp, SIGNAL(addFlickering(QString, bean::MessageType)), this, SLOT(addFlickering(QString, bean::MessageType)));
	connect(qPmApp, SIGNAL(removeFlickering(QString, bean::MessageType)), this, SLOT(removeFlickering(QString, bean::MessageType)));
	connect(qPmApp, SIGNAL(clearFlickering()), this, SLOT(clearFlickering()));

	connect(m_pUnreadMessageBox.data(), SIGNAL(openChat(QString)), this, SLOT(openChat(QString)));
	connect(m_pUnreadMessageBox.data(), SIGNAL(openGroupChat(QString)), this, SLOT(openGroupChat(QString)));
	connect(m_pUnreadMessageBox.data(), SIGNAL(openDiscuss(QString)), this, SLOT(openDiscussChat(QString)));
	connect(m_pUnreadMessageBox.data(), SIGNAL(openAllUnreadChats(QList<int>, QStringList)), 
		this, SLOT(openAllUnreadChats(QList<int>, QStringList)));

	connect(ui->leditSignature, SIGNAL(editingFinished()), this, SLOT(setSignature()));

	// filter line edit signal & slot
	connect(ui->leditFilter, SIGNAL(filterChanged(QString)), this, SLOT(editFilterChanged(QString)));
	connect(ui->leditFilter, SIGNAL(gainFocus()), this, SLOT(editFilterGainFocus()));

	connect(m_rosterView, SIGNAL(showCard(QString, int)), this, SLOT(showContactCard(QString, int)));
	connect(m_rosterView, SIGNAL(hideCard()), this, SLOT(hideContactCard()));
	connect(m_orgView, SIGNAL(showCard(QString, int)), this, SLOT(showContactCard(QString, int)));
	connect(m_orgView, SIGNAL(hideCard()), this, SLOT(hideContactCard()));
	connect(m_lastContactView, SIGNAL(showCard(QString, int)), this, SLOT(showContactCard(QString, int)));
	connect(m_lastContactView, SIGNAL(hideCard()), this, SLOT(hideContactCard()));
	connect(m_discussView, SIGNAL(disbandDiscuss(QString)), this, SLOT(disbandDiscuss(QString)));

	connect(ui->pageSearch, SIGNAL(chat(QString)), this, SLOT(openChat(QString)));
	connect(ui->pageSearch, SIGNAL(groupChat(QString)), this, SLOT(openGroupChat(QString)));
	connect(ui->pageSearch, SIGNAL(discussChat(QString)), this, SLOT(openDiscussChat(QString)));
	connect(ui->pageSearch, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));
	connect(ui->pageSearch, SIGNAL(subscriptionChat(QString)), this, SLOT(openSubscriptionMsg(QString)));
	connect(ui->pageSearch, SIGNAL(selectSearchItem(QString, int, QString)), this, SLOT(editFilterSelectItem(QString, int, QString)));

	connect(ui->mainStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(mainPageChanged(int)));

	connect(qPmApp->getBuddyMgr(), SIGNAL(chatDialogCreated(CChatDialog *)), this, SLOT(onChatDialogCreated(CChatDialog *)));
	connect(qPmApp->getBuddyMgr(), SIGNAL(groupDialogCreated(CGroupDialog *)), this, SLOT(onGroupDialogCreated(CGroupDialog *)));
	connect(qPmApp->getBuddyMgr(), SIGNAL(discussDialogCreated(DiscussDialog *)), this, SLOT(onDiscussDialogCreated(DiscussDialog *)));
	connect(qPmApp->getBuddyMgr(), SIGNAL(rosterAddMsgActivated()), this, SLOT(on_btnAddFriendList_clicked()));
	connect(qPmApp->getBuddyMgr(), SIGNAL(openSubscriptionLastMsg()),this, SLOT(openSubscriptionLastMsgDialog()));

	connect(qPmApp->getModelManager()->lastContactModel(), SIGNAL(unreadMsgRecordRemoved()), this, SLOT(onUnreadMsgCountChanged()));
	connect(qPmApp->getOfflineMsgManager(), SIGNAL(offlineChanged()), qPmApp->getModelManager()->lastContactModel(), SLOT(onUnreadItemCountChanged()));

	connect(qPmApp->getRosterManager(), SIGNAL(modifyRosterError(QString, int, QStringList, QStringList, QStringList, QList<int>)),
		this, SLOT(modifyRosterFailed(QString, int, QStringList, QStringList, QStringList, QList<int>)));
	connect(qPmApp->getRosterManager(), SIGNAL(rosterModified(QStringList, QStringList, QStringList, QList<int>)),
		this, SLOT(rosterModified(QStringList, QStringList, QStringList, QList<int>)));

	connect(m_groupPanel, SIGNAL(createDiscuss()), this, SLOT(createDiscussDialog()));

	DiscussManager *dm = qPmApp->getDiscussManager();
	connect(dm, SIGNAL(createdDiscuss(int, QString)), this, SLOT(onCreatedDiscuss(int, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(addedMembers(int, QString)), this, SLOT(onAddedMembers(int, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(quitedDiscuss(int, QString)), this, SLOT(onQuitedDiscuss(int, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(quitedDiscuss(QString)), this, SLOT(onQuitedDiscuss(QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(nameChanged(int, QString, QString)), this, SLOT(onDiscussNameChanged(int, QString, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(discussError(int, int, QString, QString, QString, QStringList)), 
		this, SLOT(onDiscussError(int, int, QString, QString, QString, QStringList)), Qt::UniqueConnection);
	connect(dm, SIGNAL(notifyDiscussChanged(QString)), this, SLOT(onNotifyDiscussChanged(QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(discussKickFailed(QString, QString, QString)), this, SLOT(onDiscussKickFailed(QString, QString, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(discussKickOK(QString, QString)), this, SLOT(onDiscussKickOK(QString, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(discussDisbandFailed(QString, QString)), this, SLOT(onDiscussDisbandFailed(QString, QString)), Qt::UniqueConnection);
	connect(dm, SIGNAL(discussDisbandOK(QString)), this, SLOT(onDiscussDisbandOK(QString)), Qt::UniqueConnection);
	connect(qPmApp->getModelManager(), SIGNAL(discussNewAdded(QString)), this, SLOT(onDiscussNewAdded(QString)), Qt::UniqueConnection);

	connect(qPmApp, SIGNAL(prepareForQuit()), this, SLOT(prepareForQuit()));

	ChangeNoticeMgr *changeNoticeMgr = qPmApp->getChangeNoticeMgr();
	connect(changeNoticeMgr, SIGNAL(rosterAddNotice(int, QString)), this, SLOT(onRosterAddNotice(int, QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(rosterAddResponded(QString)), this, SLOT(onRosterAddResponded(QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(groupChangeNotice(QString)), this, SLOT(onGroupChangeNotice(QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(discussChangeNotice(QString)), this, SLOT(onDiscussChangeNotice(QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(deleteFriend(QString)), this, SLOT(onDeleteFriendOK(QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(configChanged(QString)), this, SLOT(onConfigChanged(QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(passwdModified()), this, SLOT(onPasswdModified()), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(userDeleted(QString)), this, SLOT(onUserDeleted(QString)), Qt::UniqueConnection);
	connect(changeNoticeMgr, SIGNAL(userFrozen(QString)), this, SLOT(onUserFrozen(QString)), Qt::UniqueConnection);

	AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
	connect(addFriendManager, SIGNAL(refreshOK()), this, SLOT(onRosterAddList()), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(deleteFriendOK(QString)), this, SLOT(onDeleteFriendOK(QString)), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(deleteFriendFailed(QString, QString)), this, SLOT(onDeleteFriendFailed(QString, QString)), Qt::UniqueConnection);

	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
	connect(interphoneManager, SIGNAL(addInterphoneFinished(bool, QString)), m_lastContactView, SLOT(onAddInterphone(bool, QString)));
	connect(interphoneManager, SIGNAL(quitInterphoneFinished(bool, QString)), m_lastContactView, SLOT(onQuitInterphone(bool, QString)));
	connect(interphoneManager, SIGNAL(interphonesCleared()), m_lastContactView, SLOT(onInterphoneCleared()));
	connect(qPmApp->getBuddyMgr(), SIGNAL(interphoneStarted(QString)), m_lastContactView, SLOT(onStartInterphone(QString)));
	connect(qPmApp->getBuddyMgr(), SIGNAL(interphoneFinished(QString)), m_lastContactView, SLOT(onFinishInterphone(QString)));

	connect(subscriptionMsgManager, SIGNAL(unreadMsgChanged(QString, int)), this, SLOT(onSubscriptionUnreadMsgChanged(QString, int)));
	connect(changeNoticeMgr, SIGNAL(hasSubscriptionMsg()), subscriptionMsgManager, SLOT(getMessages()), Qt::UniqueConnection);
	connect(subscriptionMsgManager, SIGNAL(openSubscriptionDetail(QString)), this, SLOT(openSubscriptionDetail(QString)));
	connect(subscriptionMsgManager, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));

	SubscriptionManager *subscriptionManager = qPmApp->getSubscriptionManager();
	connect(subscriptionManager, SIGNAL(unsubscribeFinished(bool, QString)), this, SLOT(onSubscriptionUnsubscribed(bool, QString)));
	connect(changeNoticeMgr, SIGNAL(subscriptionSubscribed(QString)), subscriptionManager, SLOT(onSubscriptionSubscribed(QString)));
	connect(changeNoticeMgr, SIGNAL(subscriptionUnsubscribed(QString)), subscriptionManager, SLOT(onSubscriptionUnsubscribed(QString)));
	connect(subscriptionManager, SIGNAL(subscriptionSubscribed(QString)), this, SLOT(onSubscriptionSubscribed(QString)));
	connect(subscriptionManager, SIGNAL(subscriptionUnsubscribed(QString)), this, SLOT(onSubscriptionUnsubscribed(QString)));

	SubscriptionDetailAndHistoryManager *subscriptionDetailAndHistoryManager = qPmApp->getSubscriptionDetailAndHistoryManager();
	connect(subscriptionDetailAndHistoryManager, SIGNAL(openSubscriptionMsg(QString)), this, SLOT(openSubscriptionMsg(QString)));
	connect(subscriptionDetailAndHistoryManager, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)));

	BlackListModel *blackListModel = qPmApp->getModelManager()->blackListModel();
	connect(blackListModel, SIGNAL(blackListChanged()), this, SIGNAL(blackListChanged()));

	GroupManager *groupManager = qPmApp->getGroupManager();
	connect(groupManager, SIGNAL(getGroupLogoFinished(QString, int, QImage)), this, SLOT(getGroupLogoFinished(QString, int, QImage)));
	connect(groupManager, SIGNAL(changeCardNameFailed(QString, QString)), this, SLOT(changeGroupCardNameFailed(QString, QString)));

	connect(qPmApp->GetLoginMgr(), SIGNAL(validateFailed()), this, SLOT(onValidateFailed()));

	connect(qPmApp->getConfigManager(), SIGNAL(config3GotOk(QStringList)), CPscDlg::instance(), SLOT(setSilence(QStringList)));
}

void CPscDlg::setRosterModel(RosterModel* model)
{
	if (!model)
		return;

	m_rosterView->setRosterModel(model);
	if (model->proxyModel())
	{
		model->proxyModel()->sort(0);
	}
}

void CPscDlg::setOrgModel( OrgStructModel* model )
{
	if (!model)
		return;

	m_orgView->setOrgStructModel(model);
}

void CPscDlg::setGroupModel(GroupModel* model)
{
	if (!model)
		return;
	
	m_groupView->setGroupModel(model);
	if (model->proxyModel())
	{
		model->proxyModel()->sort(0);
	}
}

void CPscDlg::setDiscussModel(DiscussModel* model)
{
	if (!model)
		return;

	m_discussView->setDiscussModel(model);
	if (model->proxyModel())
	{
		model->proxyModel()->sort(0);
	}
}

void CPscDlg::setLastContactModel(LastContactModel* model)
{
	if (!model)
		return;

	m_lastContactView->setLastContactModel(model);
}

void CPscDlg::dockShow()
{
	if (m_dockState)
	{
		// dock out without check the cursor position
		dockOut(false);
	}
}

void CPscDlg::InitUI()
{
	ui->title->setText(Account::instance()->companyName());

	// allow avatar widget to be clickable
	ui->avatarWidget->setClickable(true);
	ui->avatarWidget->setHoverPixmap(QPixmap(":/images/avatar_hover.png"));

	// signature edit style
	ui->leditSignature->setBorderColor(QColor("#dfeefa"));
	ui->leditSignature->setTextColor(QColor(64, 64, 64), QColor(190, 217, 255));
	ui->leditSignature->setPlaceholderText(tr("Edit What's Up"));

	// filter line edit
	QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
		QLatin1String("edit-clear-locationbar-rtl") :
	QLatin1String("edit-clear-locationbar-ltr"),
		QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/images/Icon_32.png"))));
	ui->leditFilter->setButtonPixmap(FilterLineEdit::Left, icon.pixmap(16));
	ui->leditFilter->setButtonVisible(FilterLineEdit::Left, true);
	ui->leditFilter->setButtonToolTip(FilterLineEdit::Left, QString());
	ui->leditFilter->setAutoHideButton(FilterLineEdit::Left, false);
	ui->leditFilter->setPlaceholderText(tr("Search"));
	ui->leditFilter->setKeyDelegate(ui->pageSearch->treeViewSearch());

	// views
	InitViews();

	ui->mainStackedWidget->setCurrentIndex(0);

	// init main menu
	m_mainMenu.setObjectName(QString::fromLatin1("mainMenu"));
	m_mainMenu.addAction(tr("Modify Password"), this, SLOT(slot_modify_passwd()));
	m_mainMenu.addAction(tr("About ") + GlobalSettings::title(), this, SLOT(slot_open_about()));
	Account *pAccount = Account::instance();
	QList<CompanyInfo> companys = pAccount->companyInfos();
	if (companys.count() > 1)
	{
		QMenu *companyMenu = m_mainMenu.addMenu(tr("Switch Corporation"));
		foreach (CompanyInfo company, companys)
		{
			if (pAccount->id() != company.uid)
			{
				QAction *companyAction = companyMenu->addAction(company.companyName, this, SLOT(slot_menu_switch_company()));
				companyAction->setData(company.uid);
			}
		}
	}
	m_mainMenu.addAction(tr("Log Out"), this, SLOT(slot_menu_logout()));
	m_mainMenu.addAction(tr("Exit"), this, SLOT(slot_menu_exit()));
	connect(&m_mainMenu, SIGNAL(aboutToHide()), this, SLOT(slotMainMenuHide()));

	// init status menu
	InitStatusMenu();

	// init position
	InitPosition();

	// set flicker helper
	m_rosterView->setFlickerHelper(m_pFlickerHelper);
	m_orgView->setFlickerHelper(m_pFlickerHelper);
	m_groupView->setFlickerHelper(m_pFlickerHelper);
	m_discussView->setFlickerHelper(m_pFlickerHelper);
	m_lastContactView->setFlickerHelper(m_pFlickerHelper);
	m_pFlickerHelper->addFlickerWidget(m_rosterView);
	m_pFlickerHelper->addFlickerWidget(m_orgView);
	m_pFlickerHelper->addFlickerWidget(m_groupView);
	m_pFlickerHelper->addFlickerWidget(m_discussView);
	m_pFlickerHelper->addFlickerWidget(m_lastContactView);

	// set app buttons
	setAppButtons();

	// config interphone function
	if (GlobalSettings::interphoneDisabled())
		ui->btnInterphone->setVisible(false);
	else
		ui->btnInterphone->setVisible(true);
}

void CPscDlg::InitViews()
{
	m_lastContactView = new LastContactView();
	ui->pageWidget->addTab(m_lastContactView, 10, tr("Messages"));

	m_rosterView = new RosterTreeView();
	QWidget *rosterWidget = new QWidget(this);
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, rosterWidget);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(m_rosterView, 1);
	ui->pageWidget->addTab(rosterWidget, 20, tr("Friends"));

	m_orgView = new OrganizationTreeView();
	ui->pageWidget->addTab(m_orgView, 30, tr("Corporation"));

	m_groupView = new GroupListView();
	m_discussView = new DiscussTreeView();
	m_groupPanel = new GroupPanel(m_groupView, m_discussView, this);
	ui->pageWidget->addTab(m_groupPanel, 40, tr("Groups/Discusses"));

	ui->pageWidget->setTabButtonIcon(m_lastContactView, QString(":/images/Icon_54_normal.png"), QString(":/images/Icon_54_on.png"), QString(":/images/Icon_54_hover.png"));
	ui->pageWidget->setTabButtonIcon(rosterWidget, QString(":/images/Icon_52_normal.png"), QString(":/images/Icon_52_on.png"), QString(":/images/Icon_52_hover.png"));
	ui->pageWidget->setTabButtonIcon(m_orgView, QString(":/images/Icon_51_normal.png"), QString(":/images/Icon_51_on.png"), QString(":/images/Icon_51_hover.png"));
	ui->pageWidget->setTabButtonIcon(m_groupPanel, QString(":/images/Icon_53_normal.png"), QString(":/images/Icon_53_on.png"), QString(":/images/Icon_53_hover.png"));
	
	ui->pageWidget->setCurrentIndex(kPageIndexLastContact);
}

void CPscDlg::InitStatusMenu()
{
	m_pStatusMenu = new QMenu(this);
	m_pStatusMenu->setObjectName(QString::fromLatin1("statusMenu"));

	QAction* pAction = 0;
	// online
	pAction = new QAction(this);
	pAction->setIcon(QIcon(":/images/Icon_17.png"));
	pAction->setText(tr("Online"));
	pAction->setData(StatusChanger::Status_Online);
	m_pStatusMenu->addAction(pAction);

	/*
	// dnd
	pAction = new QAction(this);
	pAction->setIcon(QIcon(":/images/Icon_19.png"));
	pAction->setText(tr("Dnd"));
	pAction->setData(StatusChanger::Status_Dnd);
	m_pStatusMenu->addAction(pAction);

	// xa
	pAction = new QAction(this);
	pAction->setIcon(QIcon(":/images/Icon_23.png"));
	pAction->setText(tr("Xa"));
	pAction->setData(StatusChanger::Status_Xa);
	m_pStatusMenu->addAction(pAction);
	*/

	// offline
	pAction = new QAction(this);
	pAction->setIcon(QIcon(":/images/Icon_25.png"));
	pAction->setText(tr("Offline"));
	pAction->setData(StatusChanger::Status_Offline);
	m_pStatusMenu->addAction(pAction);

	ui->btnStatus->setClickable(true);
	ui->btnStatus->setPixmap(m_pStatusMenu->actions().first()->icon().pixmap(16, 16));

	connect(m_pStatusMenu, SIGNAL(triggered(QAction*)), this, SLOT(slot_statusMenu_triggered(QAction*)));
}

void CPscDlg::InitPosition()
{
	// set psc dialog's position
	bool bOkGeometry = false;
	int x = 0;
	int y = 0;
	int w = AccountSettings::kDefaultPscWidth;
	int h = AccountSettings::kDefaultPscHeight;

	// last position settings
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
	{
		x = accountSettings->getPscX();
		y = accountSettings->getPscY();
		w = accountSettings->getPscWidth();
		h = accountSettings->getPscHeight();
		bOkGeometry = true;
	}

	QRect desktopRect = QApplication::desktop()->availableGeometry();
	QRect r = geometry();
	r.setSize(QSize(w, h));
	if (bOkGeometry)
	{
		r.moveTo(x, y);
		if (!desktopRect.contains(r.topLeft()) || !desktopRect.contains(r.bottomRight()))
		{
			x = desktopRect.width() - r.width() - 30;
			y = 20;	
		}
	}
	else
	{
		x = desktopRect.width() - r.width() - 30;
		y = 20;	
	}

	setGeometry(x, y, w, h);
}

void CPscDlg::SavePosition()
{
	QRect rect = this->geometry();
	// last position settings
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
	{
		accountSettings->setPscX(rect.topLeft().x());
		accountSettings->setPscY(rect.topLeft().y());
		accountSettings->setPscWidth(rect.width());
		accountSettings->setPscHeight(rect.height());
	}
}

void CPscDlg::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Escape:
		return;
	default:
		break;
	}

	return FramelessDialog::keyPressEvent(e);
}

void CPscDlg::closeEvent(QCloseEvent *e)
{
	if (!m_bClose && !m_bKick && !qPmApp->isLogout())
	{
		e->ignore();
	}
	else
	{
		hide();
	}

	// save position
	SavePosition();
}

void CPscDlg::moveEvent(QMoveEvent *e)
{
	FramelessDialog::moveEvent(e);
}

void CPscDlg::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		m_leftKeyPressed = true;
	}

	if (ui->leditSignature->hasFocus())
	{
		setSignature();
	}

	FramelessDialog::mousePressEvent(e);
}

void CPscDlg::mouseMoveEvent(QMouseEvent *e)
{
	if (!m_dockState && m_leftKeyPressed && Account::settings()->isPscEdgeHide())
	{
		QRect rect = frameGeometry();
		QRect availableRect = QApplication::desktop()->availableGeometry();
		if (((availableRect.right() - rect.right() <= 10) && (availableRect.right() - rect.right() >= 0)) ||
			((rect.right() - availableRect.right() > 0) && (rect.right() - availableRect.right() <= 10)))
		{
			// dock state
			m_dockReady = true;
			QRect dockRect = rect;
			dockRect.moveRight(availableRect.right());
			move(dockRect.topLeft());
		}
		else
		{
			if (m_dockReady)
			{
				setTopmost(Account::settings()->isPscTopmost());
			}
			m_dockReady = false;
		}
	}

	FramelessDialog::mouseMoveEvent(e);
}

void CPscDlg::mouseReleaseEvent(QMouseEvent *e)
{
	m_leftKeyPressed = false;

	FramelessDialog::mouseReleaseEvent(e);
}

void CPscDlg::hideEvent(QHideEvent *e)
{
	FramelessDialog::hideEvent(e);
}

void CPscDlg::enterEvent(QEvent *e)
{
	FramelessDialog::enterEvent(e);
}

void CPscDlg::leaveEvent(QEvent *e)
{
	FramelessDialog::leaveEvent(e);
}

void CPscDlg::slot_clickedMinimized()
{
	hide();
}

void CPscDlg::slot_clickedClose()
{
	AccountSettings *accountSettings = Account::settings();
	if (GlobalSettings::isCloseOptionOn())
	{
		CloseOptionDialog dlg;
		dlg.setCloseHide(accountSettings->isPscCloseHide());
		QPoint topLeft = this->geometry().topLeft();
		topLeft.setX(topLeft.x() - 20);
		topLeft.setY(topLeft.y() + 160);
		QRect rcAvailable = QApplication::desktop()->availableGeometry();
		if (topLeft.x() < rcAvailable.left())
			topLeft.setX(rcAvailable.left());
		if (topLeft.x() + dlg.width() > rcAvailable.right())
			topLeft.setX(rcAvailable.right() - dlg.width());
		dlg.move(topLeft);
		if (dlg.exec() == QDialog::Rejected)
			return;

		bool closeHide = dlg.isCloseHide();
		bool isRemind = dlg.isRemind();
		accountSettings->setPscCloseHide(closeHide);
		GlobalSettings::setCloseOptionOn(isRemind);
	}

	if (Account::settings()->isPscCloseHide())
	{
		slot_clickedMinimized();
	}
	else
	{
		slot_menu_exit();
	}
}

// public slots
void CPscDlg::slot_statusChanged(int status)
{
	qDebug() << Q_FUNC_INFO << StatusChanger::status2Str((StatusChanger::Status)status);

	foreach (QAction* pAction, m_pStatusMenu->actions())
	{
		if (status == pAction->data().toInt())
		{
			m_pStatusMenu->setActiveAction(pAction);
			ui->btnStatus->setPixmap(pAction->icon().pixmap(16, 16));
			ui->btnStatus->setToolTip(tr("Current Status: %1").arg(pAction->text()));
		}
	}
}

void CPscDlg::slot_menu_exit()
{
	// quit
	m_bClose = true;
	QTimer::singleShot(0, qPmApp, SLOT(quit()));
}

void CPscDlg::slot_menu_switch_company()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString switchId = action->data().toString();
	if (switchId.isEmpty())
		return;

	activateWindow();

	QString str = tr("Are you sure to switch to %1").arg(action->text());
	bool bResult = PMessageBox::question(0, tr("Switch Corporation"), str, QDialogButtonBox::Ok|QDialogButtonBox::Cancel) == QDialogButtonBox::Ok;
	if (bResult)
	{
		doSwitchCompany(switchId);
	}
}

void CPscDlg::slot_menu_logout()
{
	activateWindow();

	QString str = tr("Are you sure to log out");
	bool bResult = PMessageBox::question(0, tr("Log out"), str, QDialogButtonBox::Yes|QDialogButtonBox::No) == QDialogButtonBox::Yes;
	if (bResult)
	{
		doLogout();
	}
}

void CPscDlg::slot_modify_passwd()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't modify password. Please try when online"));
		return;
	}

	if (!m_pPasswdModifyDlg)
	{
		m_pPasswdModifyDlg = new PasswdModifyDlg();
	}

	WidgetManager::showActivateRaiseWindow(m_pPasswdModifyDlg.data());
}

void CPscDlg::slot_receiveKick()
{
	m_bKick = true;
	qPmApp->GetLoginMgr()->logout();

	if (m_bKick)
	{
		activateWindow();
		PMessageBox::information(this, tr("Tip"), tr("Your account logined other place"));
	}	
}

void CPscDlg::addFlickering(const QString &id, bean::MessageType msgType)
{
	// my phone and subscription do not flick
	if (msgType == bean::Message_Chat &&
		(id == Account::instance()->phoneFullId() || id == QString(SUBSCRIPTION_ROSTER_ID)))
		return;

	m_pFlickerHelper->addFlickerItem(id, msgType);
}

void CPscDlg::removeFlickering(const QString &id, bean::MessageType msgType)
{
	m_pFlickerHelper->removeFlickerItem(id, msgType);
}

void CPscDlg::clearFlickering()
{
	m_pFlickerHelper->clearFlickerItems();
}

void CPscDlg::setSignature()
{
	bean::DetailItem *pItem = qPmApp->getModelManager()->detailItem(Account::instance()->id());
	if (!pItem)
	{
		ui->leditSignature->clearFocus();
		return;
	}

	// if not logined, revert to original signature
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		QString orignalSignature = pItem->message();
		ui->leditSignature->setText(orignalSignature);
		ui->leditSignature->clearFocus();
		return;
	}

	// send modify signature request
	QString changedSignature = ui->leditSignature->wholeText();
	if (pItem && changedSignature != qPmApp->getModelManager()->getSignature(pItem))
	{
		pItem->setMessage(changedSignature);

		ModifyProcess *proc = new ModifyProcess(this);
		proc->initObject();
		QMap<int, QVariant> changeMap;
		changeMap[bean::DETAIL_MESSAGE] = changedSignature;
		if (!proc->sendModify(changeMap))
		{
			qDebug("change signature send modify failed.");
		}
	}
	ui->leditSignature->clearFocus();
}

void CPscDlg::viewContactInfo(const QString &id)
{
	if (id.isEmpty())
		return;

	if (id == Account::instance()->phoneFullId()) // my phone do not show material
		return;

	if (id == Account::instance()->id())
	{
		// self info
		on_avatarWidget_clicked();
	}
	else
	{
		// other's info
		m_pContactInfoManager->openContactInfo(id);
	}
}

void CPscDlg::viewContactHistory(const QString &id)
{
	MessageManagerDlg *msgManagerDialog = MessageManagerDlg::instance();
	msgManagerDialog->init(id, bean::Message_Chat);
	WidgetManager::showActivateRaiseWindow(msgManagerDialog);
}

void CPscDlg::viewGroupHistory(const QString &id)
{
	MessageManagerDlg *msgManagerDialog = MessageManagerDlg::instance();
	msgManagerDialog->init(id, bean::Message_GroupChat);
	WidgetManager::showActivateRaiseWindow(msgManagerDialog);
}

void CPscDlg::viewDiscussHistory(const QString &id)
{
	MessageManagerDlg *msgManagerDialog = MessageManagerDlg::instance();
	msgManagerDialog->init(id, bean::Message_DiscussChat);
	WidgetManager::showActivateRaiseWindow(msgManagerDialog);
}

void CPscDlg::sendMail(const QString &id)
{
	bean::DetailItem *pItem = qPmApp->getModelManager()->detailItem(id);
	if (!pItem)
		return;

	QString mailAddress = pItem->email();
	if (mailAddress.isEmpty())
	{
		QString name = qPmApp->getModelManager()->userName(id);
		PMessageBox::information(0, tr("Tip"), tr("%1 hasn't set mail address").arg(name));
		return;
	}

	QString mailToAddress = QString("mailto:%1").arg(mailAddress);
	QDesktopServices::openUrl(QUrl(mailToAddress));
}

void CPscDlg::msgMultiSend(const QStringList &members, const QString &id)
{
	// remove self
	QString selfId = Account::instance()->id();
	QStringList sendMemebers = members;
	if (sendMemebers.contains(selfId))
	{
		sendMemebers.removeAll(selfId);
	}

	// check if member is empty
	if (sendMemebers.isEmpty())
	{
		return;
	}

	if (sendMemebers.count() > KMaxSendCount)
	{
		PMessageBox::information(this, tr("Tip"), tr("Multi-send message only supports at most %1 members").arg(KMaxSendCount));
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	LastContactModel *lastContactModel = modelManager->lastContactModel();

	QString newId = id;
	bool newCreated = false;
	if (newId.isEmpty())
	{
		newId = lastContactModel->multiSendMsgId(sendMemebers, newCreated);
	}

	MsgMultiSendDlg *dlg = qPmApp->getBuddyMgr()->openMsgMultiSend(newId, sendMemebers, newCreated);
	if (dlg)
	{
		connect(dlg, SIGNAL(doScreenshot()), this, SLOT(screenshot()), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotOk(QString)), dlg, SLOT(slot_screenshot_ok(QString)), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotCancel()), dlg, SLOT(slot_screenshot_cancel()), Qt::UniqueConnection);
		connect(dlg, SIGNAL(chat(QString)), this, SLOT(openChat(QString)), Qt::UniqueConnection);
		connect(dlg, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)), Qt::UniqueConnection);
		connect(dlg, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
		connect(dlg, SIGNAL(memberChanged(QString, QString, QStringList)), lastContactModel, SLOT(onMultiSendMemberChanged(QString, QString, QStringList)), Qt::UniqueConnection);
		connect(dlg, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)), Qt::UniqueConnection);
		connect(dlg, SIGNAL(removeMultiSend(QString)), lastContactModel, SLOT(onRemoveMultiSend(QString)), Qt::UniqueConnection);

		WidgetManager::showActivateRaiseWindow(dlg);
	}
}

void CPscDlg::addFriendRequest(const QString &id, const QString &name)
{
	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	if (!rosterModel)
		return;

	if (id == Account::instance()->id())
	{
		PMessageBox::information(0, tr("Tip"), tr("Can't add self as a friend"));
		return;
	}

	if (rosterModel->isFriend(id))
	{
		QString name = rosterModel->rosterName(id);
		PMessageBox::information(0, tr("Tip"), tr("You and %1 are already friends").arg(name));
		return;
	}

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(0, tr("Tip"), tr("You are offline, can't add friends"));
		return;
	}

	AddFriendRequestDlg *addFriendRequestDlg = AddFriendRequestDlg::getAddFriendRequestDlg(id, name); 
	connect(addFriendRequestDlg, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(addFriendRequestDlg);
}

void CPscDlg::onAddFriendOK(const QString &id, const QString &name, const QString &group)
{
	if (id.isEmpty() || name.isEmpty() || group.isEmpty())
		return;

	if (id == Account::instance()->id())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(0, tr("Tip"), tr("You are offline, can't add friends"));
		return;
	}

	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	if (!rosterModel)
		return;

	if (!rosterModel->containsRoster(id))
	{
		rosterModel->appendRoster(id, name);
	}
}

void CPscDlg::openSubscriptionLastMsgDialog()
{
	// remove this message
	qPmApp->getUnreadMsgModel()->takeMsg(QString(SUBSCRIPTION_ROSTER_ID), bean::Message_Chat);

	SubscriptionLastMsgDialog *dialog = SubscriptionLastMsgDialog::getDialog();
	connect(dialog, SIGNAL(openSubscriptionMsg(QString)), this, SLOT(openSubscriptionMsg(QString)), Qt::UniqueConnection);
	connect(dialog, SIGNAL(openSubscriptionDetail(QString)), this, SLOT(openSubscriptionDetail(QString)), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(dialog);
}

void CPscDlg::openSubscriptionMsg(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	// remove this message
	qPmApp->getUnreadMsgModel()->takeMsg(QString(SUBSCRIPTION_ROSTER_ID), bean::Message_Chat);

	// open subscription message dialog
	qPmApp->getSubscriptionMsgManager()->openSubscriptionMsgDialog(subscriptionId);
}

void CPscDlg::openSubscriptionDetail(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	qPmApp->getSubscriptionDetailAndHistoryManager()->openSubscriptionDetail(subscriptionId);
}

void CPscDlg::openSubscriptionHistory(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	qPmApp->getSubscriptionDetailAndHistoryManager()->openSubscriptionHistory(subscriptionId);
}

void CPscDlg::onCompanyLoginFailed(const QString &desc)
{
	// stop reconnect
	qPmApp->setLogout(true);

	// tip and logout
	PMessageBox::information(0, tr("Login Error"), desc);
	doLogout();
}

void CPscDlg::onMyInfoChanged(const QString &id)
{
	if (id == Account::instance()->id())
	{
		// set avatar
		StatusChanger *pStatusChanger = qPmApp->getStatusChanger();
		setAvatar(pStatusChanger->curStatus());

		// set name
		QString sName = qPmApp->getModelManager()->userName(qPmApp->getAccount()->id());
		ui->labName->setText(sName);
		
		// set signature
		bean::DetailItem *pItem = qPmApp->getModelManager()->detailItem(id);
		ui->leditSignature->setText(qPmApp->getModelManager()->getSignature(pItem));
	}
}

void CPscDlg::onDetailChanged(const QString &id)
{
	// check roster's name
	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	if (rosterModel->allRosterIds().contains(id))
	{
		QString oldName = rosterModel->rosterName(id);
		QString newName = modelManager->userName(id);
		if (oldName != newName)
		{
			// change roster name in model
			rosterModel->setRosterName(id, newName);

			// send modify name roster request
			QString groupName = RosterModel::defaultGroupName();
			qPmApp->getRosterManager()->changeRosterName(id, oldName, newName, groupName);
		}
	}

	// check last contact's name
	LastContactModel *lastContactModel = modelManager->lastContactModel();
	if (lastContactModel->containsContact(id))
	{
		LastContactItem *item = lastContactModel->contact(id);
		if (item)
		{
			QString oldName = item->itemName();
			QString newName = modelManager->userName(id);
			if (oldName != newName)
			{
				lastContactModel->changeItemName(LastContactItem::LastContactTypeContact, id, newName);
			}
		}
	}
}

void CPscDlg::slotQuitAct()
{
	//quit(false);
}

void CPscDlg::slotLogoutAct()
{
	//quit(false, true);
}

void CPscDlg::slotRestoreAct()
{
	show();
	raise();
}

void CPscDlg::slot_open_help()
{
	PMessageBox::information(this, tr("Tip"), tr("Function is under development"));
}

void CPscDlg::slot_open_setting()
{
	PMessageBox::information(this, tr("Tip"), tr("Function is under development"));
}

void CPscDlg::slot_open_about()
{
	if (!m_pAboutDialog)
	{
		m_pAboutDialog = new CAboutDialog();
	}

	WidgetManager::showActivateRaiseWindow(m_pAboutDialog.data());
}

void CPscDlg::slot_open_update()
{
	// do not have this function right now
}

void CPscDlg::slot_system_mute_toggled()
{
	QAction* action = static_cast<QAction*>(sender());
	if (!action)
		return;

	// self settings
	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
	{
		accountSettings->setMute(action->isChecked());
		PlayBeep::setMute(action->isChecked());
	}
}

void CPscDlg::slotMainMenuHide()
{
	QPoint cursorPos = QCursor::pos();
	QPoint btnMenuPos = ui->btnMenu->pos();
	QPoint globalBtnMenuPos = ((QWidget*)(ui->btnMenu->parent()))->mapToGlobal(btnMenuPos);
	QRect btnMenuRect(globalBtnMenuPos, ui->btnMenu->size());
	btnMenuRect.adjust(-1, -1, 1, 1);
	if (!btnMenuRect.contains(cursorPos))
		ui->btnMenu->setChecked(false);
}

void CPscDlg::on_avatarWidget_clicked()
{
	MyInfoDialog *myInfoDialog = MyInfoDialog::instance();
	WidgetManager::showActivateRaiseWindow(myInfoDialog);
}

void CPscDlg::on_btnStatus_clicked()
{
	QPoint pos(0,0);
	pos.ry() += ui->btnStatus->height();

	m_pStatusMenu->exec(ui->btnStatus->mapToGlobal(pos));
}

void CPscDlg::on_btnMenu_toggled(bool checked)
{
	if (checked)
	{
		QPoint pos = this->geometry().bottomLeft();
		pos.setX(pos.x() + 9);
		int offset = 27*m_mainMenu.actions().count();
		pos.setY(pos.y() - ui->bottomBar->height()/2 - offset);
		m_mainMenu.exec(pos);
	}
	else
	{
		m_mainMenu.close();
	}
}

void CPscDlg::on_btnSearch_clicked()
{
	SearchDialog *searchDialog = SearchDialog::getInstance();
	connect(searchDialog, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)), Qt::UniqueConnection);
	connect(searchDialog, SIGNAL(showMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
	connect(searchDialog, SIGNAL(subscriptionSubscribed(SubscriptionDetail, SubscriptionMsg)), 
		this, SLOT(onSubscriptionSubscribed(SubscriptionDetail, SubscriptionMsg)), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(searchDialog);
}

void CPscDlg::searchPeople()
{
	on_btnSearch_clicked();

	SearchDialog *searchDialog = SearchDialog::getInstance();
	searchDialog->setSearchType(SearchDialog::SearchPeople);
}

void CPscDlg::searchSubscription()
{
	on_btnSearch_clicked();

	SearchDialog *searchDialog = SearchDialog::getInstance();
	searchDialog->setSearchType(SearchDialog::SearchSubscription);
}

void CPscDlg::on_btnSystemSettings_clicked()
{
	QString shotKey = Account::settings()->getScreenshotKey();
	bool shotKeyOK = m_pScreenshotShortcut->setShortcut(QKeySequence(shotKey));

	QString takeMsgKey = Account::settings()->getTakeMsgKey();
	bool takeMsgKeyOK = m_pTakeMsgShortcut->setShortcut(QKeySequence(takeMsgKey));
	
	SystemSettingsDialog *settingsDialog = SystemSettingsDialog::getInstance();
	settingsDialog->setShotKeyConflict(!shotKeyOK);
	settingsDialog->setTakeMsgKeyConflict(!takeMsgKeyOK);
	connect(settingsDialog, SIGNAL(mainPanelTopmost(bool)), this, SLOT(setTopmost(bool)), Qt::UniqueConnection);
	connect(settingsDialog, SIGNAL(shortcutKeyApplied()), this, SLOT(setShortcutKey()), Qt::UniqueConnection);
	connect(settingsDialog, SIGNAL(shortcutKeyApplied()), qPmApp, SIGNAL(shortcutKeyChanged()), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(settingsDialog);
}

void CPscDlg::on_btnMsgManager_clicked()
{
	MessageManagerDlg *msgManagerDialog = MessageManagerDlg::instance();
	msgManagerDialog->init();
	WidgetManager::showActivateRaiseWindow(msgManagerDialog);
}

void CPscDlg::on_btnAddFriendList_clicked()
{
	// clear unread add friend message first
	qPmApp->getUnreadMsgModel()->takeMsg(QString(ROSTER_ADD_MESSAGE_ID), bean::Message_Chat);

	// show dialog
	AddFriendListDlg *addFriendListDlg = AddFriendListDlg::instance();
	addFriendListDlg->refreshList();
	connect(addFriendListDlg, SIGNAL(addFriendOK(QString, QString, QString)), this, SLOT(onAddFriendOK(QString, QString, QString)), Qt::UniqueConnection);
	connect(addFriendListDlg, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
	connect(addFriendListDlg, SIGNAL(setUnhandleFlag(bool)), this, SLOT(setAddFriendUnhandleFlag(bool)), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(addFriendListDlg);
}

void CPscDlg::on_btnInterphone_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't create interphone"));
		return;
	}

	if (qPmApp->hasSession())
	{
		PMessageBox::information(this, tr("Tip"), tr("There is audio/video session in progress, please try later"));
		return;
	}

	if (InterphoneDialog::hasInterphoneDialog())
	{
		PMessageBox::information(this, tr("Tip"), tr("You must quit current interphone to create a new one"));
		return;
	}

	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Interphone"), 
		tr("Create interphone needs to create discuss, do you want to continue"), QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret != QDialogButtonBox::Yes)
	{
		return;
	}

	CreateDiscussDialog *pDlg = new CreateDiscussDialog(CreateDiscussDialog::Type_CreateInterphone);
	connect(pDlg, SIGNAL(createInterphone(QString, QStringList)), this, SLOT(createInterphone(QString, QStringList)));
	connect(pDlg, SIGNAL(openDiscuss(QString)), this, SLOT(openDiscussChat(QString)));
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void CPscDlg::on_btnAppManage_clicked()
{
	AppManageDialog *appManageDialog = AppManageDialog::getDialog();
	WidgetManager::showActivateRaiseWindow(appManageDialog);
	connect(appManageDialog, SIGNAL(appChanged()), this, SLOT(setAppButtons()));
}

void CPscDlg::on_btnFileManager_clicked()
{
	FileManagerDlg *fileManagerDialog = FileManagerDlg::instance();
	WidgetManager::showActivateRaiseWindow(fileManagerDialog);
}

void CPscDlg::slot_statusMenu_triggered(QAction* action)
{
	StatusChanger *pStatusChanger = qPmApp->getStatusChanger();	
	pStatusChanger->setStatus(action->data().toInt());
}

void CPscDlg::setAvatar(int status)
{
	Q_UNUSED(status);
	QPixmap pix = qPmApp->getModelManager()->getUserAvatar(Account::instance()->id());
	ui->avatarWidget->setPixmap(pix);
}

void CPscDlg::removeGroupChat(const QString &groupId)
{
	// remove from last contact
	qPmApp->getModelManager()->lastContactModel()->onRemoveGroupChat(groupId);

	// remove all group unread message
	qPmApp->getUnreadMsgModel()->takeMsg(groupId, bean::Message_GroupChat);
}

void CPscDlg::removeDiscussChat(const QString &discussId)
{
	// remove from last contact
	qPmApp->getModelManager()->lastContactModel()->onRemoveDiscuss(discussId);

	// remove all discuss unread message
	qPmApp->getUnreadMsgModel()->takeMsg(discussId, bean::Message_DiscussChat);
}

void CPscDlg::editFilterChanged(const QString &filterText)
{
	QString searchString = filterText;
	if (searchString.isEmpty())
	{
		if (ui->mainStackedWidget->currentIndex() != 0)
			ui->mainStackedWidget->setCurrentIndex(0);
	}
	else
	{
		if (ui->mainStackedWidget->currentIndex() != 1)
			ui->mainStackedWidget->setCurrentIndex(1);
	}

	// first check if open pm console
	if (searchString == QString("##lt##"))
	{
		CDebugDlg::getDebugDlg()->slot_show();
		ui->leditFilter->clear();
		return;
	}

	ui->pageSearch->editFilterChanged(filterText);
}

void CPscDlg::editFilterSelectItem(const QString &id, int source, const QString &wid)
{
	if (ui->mainStackedWidget->currentIndex() != 0)
		ui->mainStackedWidget->setCurrentIndex(0);

	ModelManager *modelManager = qPmApp->getModelManager();
	if (source == SELECT_SOURCE_ROSTER) // roster
	{
		RosterModel *rosterModel = modelManager->rosterModel();
		RosterProxyModel *rosterProxyModel = rosterModel->proxyModel();
		if (rosterModel->containsRoster(id))
		{
			QStandardItem *rosterItem = rosterModel->rosterItem(id);

			// scroll to index and select it
			ui->pageWidget->setCurrentIndex(kPageIndexRoster);
			QModelIndex sourceIndex = rosterItem->index();
			QModelIndex proxyIndex = rosterProxyModel->mapFromSource(sourceIndex);
			m_rosterView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);

			m_rosterView->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect);
		}
	}
	else if (source == SELECT_SOURCE_OS) // organization structure
	{
		OrgStructModel *osModel = modelManager->orgStructModel();
		if (osModel->containsContactByWid(wid))
		{
			OrgStructContactItem *contactItem = osModel->contactByWid(wid);
			if (contactItem)
			{
				// scroll to index and select it
				ui->pageWidget->setCurrentIndex(kPageIndexOs);
				QModelIndex sourceIndex = contactItem->index();
				m_orgView->scrollTo(sourceIndex, QAbstractItemView::PositionAtTop);

				m_orgView->selectionModel()->select(sourceIndex, QItemSelectionModel::ClearAndSelect);
			}
		}
	}
	else if (source == SELECT_SOURCE_GROUP)
	{
		GroupModel *groupModel = modelManager->groupModel();
		CSortFilterProxyModel *groupProxyModel = groupModel->proxyModel();
		MucGroupItem *groupItem = groupModel->getGroup(id);
		if (groupItem)
		{
			// scroll to index and select it
			ui->pageWidget->setCurrentIndex(kPageIndexGroup);
			m_groupPanel->activeGroupPage();

			QModelIndex sourceIndex = groupItem->index();
			QModelIndex proxyIndex = groupProxyModel->mapFromSource(sourceIndex);
			m_groupView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);

			m_groupView->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect);
		}
	}
	else if (source == SELECT_SOURCE_DISCUSS)
	{
		DiscussModel *discussModel = modelManager->discussModel();
		DiscussProxyModel *discussProxyModel = discussModel->proxyModel();
		DiscussItem *discussItem = discussModel->getDiscuss(id);
		if (discussItem)
		{
			// scroll to index and select it
			ui->pageWidget->setCurrentIndex(kPageIndexGroup);
			m_groupPanel->activeDiscussPage();

			QModelIndex sourceIndex = discussItem->index();
			QModelIndex proxyIndex = discussProxyModel->mapFromSource(sourceIndex);
			m_discussView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);

			m_discussView->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect);
		}
	}
	else if (source == SELECT_SOURCE_SUBSCRIPTION)
	{
		RosterModel *rosterModel = modelManager->rosterModel();
		RosterProxyModel *rosterProxyModel = rosterModel->proxyModel();
		QStandardItem *rosterItem = rosterModel->rosterItem(QString(SUBSCRIPTION_ROSTER_ID));
		if (rosterItem)
		{
			// scroll to index and select it
			ui->pageWidget->setCurrentIndex(kPageIndexRoster);
			QModelIndex sourceIndex = rosterItem->index();
			QModelIndex proxyIndex = rosterProxyModel->mapFromSource(sourceIndex);
			m_rosterView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);

			m_rosterView->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect);

			openSubscriptionDialog();
			SubscriptionDialog::getDialog()->setCurrent(id);
		}
	}
}

void CPscDlg::mainPageChanged(int index)
{
	if (index == 1)
	{
		ui->pageSearch->addEditFilterCompleter();
	}
}

void CPscDlg::editFilterGainFocus()
{
	QString searchString = ui->leditFilter->text();
	if (!searchString.isEmpty())
	{
		editFilterChanged(searchString);
	}
}

void CPscDlg::showContactCard(const QString &id, int posY)
{
	if (sender() != m_rosterView && sender() != m_lastContactView && sender() != m_orgView)
		return;

	if (id == Account::instance()->phoneFullId()) // my phone do not show card
		return;

	if (id == QString(SUBSCRIPTION_ROSTER_ID)) // subscription do not show card
		return;

	if (m_pContactCard.data()->isCardShowing(id))
		return;

	QRect rc = frameGeometry();
	QRect rcAvailable = QApplication::desktop()->availableGeometry();
	QPoint pos(0, posY);
	if (sender() == m_rosterView)
		pos = m_rosterView->mapToGlobal(pos);
	else if (sender() == m_lastContactView)
		pos = m_lastContactView->mapToGlobal(pos);
	else if (sender() == m_orgView)
		pos = m_orgView->mapToGlobal(pos);

	if (rc.center().x() > rcAvailable.center().x())
	{
		pos.setX(rc.left() - m_pContactCard.data()->width());
	}
	else
	{
		pos.setX(rc.right());
	}
	if (pos.y() < 0)
	{
		pos.setY(0);
	}
	if (pos.y()+m_pContactCard.data()->height() > rcAvailable.bottom())
	{
		pos.setY(rcAvailable.bottom() - m_pContactCard.data()->height());
	}

	m_pContactCard.data()->preShowCard(id, pos);
}

void CPscDlg::hideContactCard()
{
	m_pContactCard.data()->preHide();
}

void CPscDlg::setTopmost(bool topmost)
{
	int flags = this->windowFlags();
	qPmApp->getLogger()->debug(QString("psc dialog windows flag: %1").arg(flags));
	if (topmost)
	{
		qPmApp->getLogger()->debug("set topmost");
		if ((windowFlags() & Qt::WindowStaysOnTopHint) == 0)
		{
			qPmApp->getLogger()->debug("not has top hint");
			bool originalVisible = this->isVisible();
			this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
			this->setVisible(originalVisible);
			flags = this->windowFlags();
			qPmApp->getLogger()->debug(QString("later windows flag: %1").arg(flags));
		}
	}
	else
	{
		qPmApp->getLogger()->debug("not topmost");
		if ((windowFlags() & Qt::WindowStaysOnTopHint) != 0)
		{
			qPmApp->getLogger()->debug("has top hint");
			bool originalVisible = this->isVisible();
			this->setWindowFlags(windowFlags() & (~Qt::WindowStaysOnTopHint));
			this->setVisible(originalVisible);
			flags = this->windowFlags();
			qPmApp->getLogger()->debug(QString("later windows flag: %1").arg(flags));
		}
	}
}

void CPscDlg::screenshot()
{
	// check if need to hide chat widget to screen shot
	QWidget *activeWidget = qApp->activeWindow();
	if (activeWidget)
	{
		AccountSettings *accountSettings = Account::settings();
		if (accountSettings->hideToScreenshot())
		{
			QWidget *chatDialog = qPmApp->getBuddyMgr()->unionChatDialog();
			if (chatDialog == activeWidget)
			{
				activeWidget->showMinimized();
				activeWidget->activateWindow();
			}
		}
	}

	m_pScreenshot->shot();
}

void CPscDlg::onScreenshotOk(const QString &imagePath)
{
	emit screenshotOk(imagePath);
}

void CPscDlg::onScreenshotCancel()
{
	emit screenshotCancel();
}

void CPscDlg::onChatDialogCreated(CChatDialog *chatDialog)
{
	if (chatDialog)
	{
		connect(chatDialog, SIGNAL(viewContactInfo(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
		connect(chatDialog, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)), Qt::UniqueConnection);
		connect(chatDialog, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
		connect(chatDialog, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)), Qt::UniqueConnection);
		connect(chatDialog, SIGNAL(doScreenshot()), this, SLOT(screenshot()), Qt::UniqueConnection);
		connect(chatDialog, SIGNAL(removeBlack(QString)), this, SLOT(removeBlack(QString)), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotOk(QString)), chatDialog, SLOT(slot_screenshot_ok(QString)), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotCancel()), chatDialog, SLOT(slot_screenshot_cancel()), Qt::UniqueConnection);
		connect(this, SIGNAL(blackListChanged()), chatDialog, SLOT(onBlackListChanged()), Qt::UniqueConnection);
		connect(chatDialog, SIGNAL(createDiscuss(QStringList)),this, SLOT(createDiscussDialog(QStringList)));
	}
}

void CPscDlg::onGroupDialogCreated(CGroupDialog *groupDialog)
{
	if (groupDialog)
	{
		connect(groupDialog, SIGNAL(doScreenshot()), this, SLOT(screenshot()), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotOk(QString)), groupDialog, SLOT(slot_screenshot_ok(QString)), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotCancel()), groupDialog, SLOT(slot_screenshot_cancel()), Qt::UniqueConnection);
		connect(groupDialog, SIGNAL(chat(QString)), this, SLOT(openChat(QString)), Qt::UniqueConnection);
		connect(groupDialog, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)), Qt::UniqueConnection);
		connect(groupDialog, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
		connect(groupDialog, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)), Qt::UniqueConnection);
		connect(groupDialog, SIGNAL(removeGroupChat(QString)), this, SLOT(removeGroupChat(QString)), Qt::UniqueConnection);
	}
}

void CPscDlg::onDiscussDialogCreated(DiscussDialog *discussDialog)
{
	if (discussDialog)
	{
		connect(discussDialog, SIGNAL(doScreenshot()), this, SLOT(screenshot()), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotOk(QString)), discussDialog, SLOT(slot_screenshot_ok(QString)), Qt::UniqueConnection);
		connect(this, SIGNAL(screenshotCancel()), discussDialog, SLOT(slot_screenshot_cancel()), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(chat(QString)), this, SLOT(openChat(QString)), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(sendMail(QString)), this, SLOT(sendMail(QString)), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(addFriendRequest(QString, QString)), this, SLOT(addFriendRequest(QString, QString)), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(quitDiscuss(QString, bool)), this, SLOT(exitDiscuss(QString, bool)), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(addDiscussMembers(QString, QStringList)), this, SLOT(addDiscussMembers(QString, QStringList)), Qt::UniqueConnection);
		connect(discussDialog, SIGNAL(removeDiscussChat(QString)), this, SLOT(removeDiscussChat(QString)), Qt::UniqueConnection);
	}
}

void CPscDlg::onUnreadMsgCountChanged()
{
	if (m_lastContactView)
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		LastContactModel *lastContactModel = modelManager->lastContactModel();
		if (lastContactModel)
		{
			lastContactModel->onUnreadItemCountChanged();
			if (lastContactModel->hasUnreadMsg())
			{
				ui->pageWidget->setTabButtonIcon(m_lastContactView, QString(":/images/Icon_54_emphasis.png"), QString(":/images/Icon_54_on.png"), "");
			}
			else
			{
				ui->pageWidget->setTabButtonIcon(m_lastContactView, QString(":/images/Icon_54_normal.png"), QString(":/images/Icon_54_on.png"), QString(":/images/Icon_54_hover.png"));
			}
		}
	}
}

void CPscDlg::onUnreadMsgPreIgnoreAll()
{
	UnreadMsgItem *unreadMsgItem = qPmApp->getUnreadMsgModel()->peekUnreadMsg(QString(ROSTER_ADD_MESSAGE_ID), bean::Message_Chat);
	if (unreadMsgItem)
	{
		// has request need to set read
		QList<bean::MessageBody> msgs = unreadMsgItem->msgs();
		foreach (bean::MessageBody msg, msgs)
		{
			bean::MessageExt ext = msg.ext();
			QString addType = ext.data("addtype").toString();
			if (addType == "request")
			{
				QString sId = ext.data("sid").toString();
				qPmApp->getAddFriendManager()->addFriendRead(sId);
			}
		}
	}
}

void CPscDlg::modifyRosterFailed(const QString &errMsg, int actionType, 
								 const QStringList &ids, const QStringList &names, 
								 const QStringList &groups, const QList<int> &modifies)
{
	// show error message
	QString tipMsg;
	if (actionType == RosterManager::ActionAddRoster)
	{
		tipMsg = tr("Add friend failed: %1").arg(errMsg);
	}
	else if (actionType == RosterManager::ActionRemoveRoster)
	{
		tipMsg = tr("Delete friend failed: %1").arg(errMsg);
	}
	else if (actionType == RosterManager::ActionChangeGroup)
	{
		tipMsg = tr("Move friend failed: %1").arg(errMsg);
	}
	if (!tipMsg.isEmpty())
	{
		PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, tipMsg, QDialogButtonBox::Ok, tr("Error"));
		WidgetManager::showActivateRaiseWindow(pMB);
	}
	
	// recover from error
	QString id;
	QString name;
	QString group;
	RosterManager::ModifyType modifyType;
	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	for (int i = 0; i < ids.count(); i++)
	{
		id = ids[i];
		name = names[i];
		group = groups[i];
		modifyType = (RosterManager::ModifyType)modifies[i];
		if (modifyType == RosterManager::ModifyAdd)
		{
			rosterModel->appendRoster(id, name);
		}
		else if (modifyType == RosterManager::ModifyDelete)
		{
			rosterModel->removeRoster(id);
		}
	}
}

void CPscDlg::rosterModified(const QStringList &ids, const QStringList &names, const QStringList &groups, const QList<int> &modifies)
{
	// sync with these modifies
	QString id;
	QString name;
	QString group;
	RosterManager::ModifyType modifyType;
	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	for (int i = 0; i < ids.count(); i++)
	{
		id = ids[i];
		name = names[i];
		group = groups[i];
		modifyType = (RosterManager::ModifyType)modifies[i];
		if (modifyType == RosterManager::ModifyAdd)
		{
			rosterModel->appendRoster(id, name);
		}
		else if (modifyType == RosterManager::ModifyDelete)
		{
			rosterModel->removeRoster(id);
		}
	}
}

void CPscDlg::onDeleteFriendOK(const QString &id)
{
	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	if (!rosterModel->containsRoster(id))
		return;

	// remove from roster model
	rosterModel->removeRoster(id);

	/*
	// remove from last contact
	qPmApp->getModelManager()->lastContactModel()->onRemoveChat(id);

	// remove all unread message
	qPmApp->getUnreadMsgModel()->takeMsg(id, bean::Message_Chat);

	// close chat dialog
	qPmApp->getBuddyMgr()->closeChat(bean::Message_Chat, id);
	*/
}

void CPscDlg::onDeleteFriendFailed(const QString &id, const QString &desc)
{
	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	if (!rosterModel->containsRoster(id))
		return;

	// show error message
	QString tipMsg = tr("Delete friend failed: %1").arg(desc);
	PMessageBox *pMB = new PMessageBox(PMessageBox::Warning, tipMsg, QDialogButtonBox::Ok, tr("Error"));
	WidgetManager::showActivateRaiseWindow(pMB);
}

void CPscDlg::onUserDeleted(const QString &id)
{
	if (id == Account::instance()->id())
	{
		// off-line first
		StatusChanger* pStatusChanger = qPmApp->getStatusChanger();	
		pStatusChanger->setStatus(StatusChanger::Status_Offline);

		// tip and logout
		QString companyName = Account::instance()->companyName();
		PMessageBox::information(0, tr("Tip"), 
			tr("You are deleted from %1, please contact the administrator of corporation if has any question").arg(companyName));
		doLogout();
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		RosterModel *rosterModel = modelManager->rosterModel();
		if (rosterModel->containsRoster(id))
		{
			// remove from roster model
			rosterModel->removeRoster(id);
		}

		// update contact detail
		modelManager->syncDetail(id);
	}
}

void CPscDlg::onUserFrozen(const QString &id)
{
	if (id == Account::instance()->id())
	{
		// off-line first
		StatusChanger* pStatusChanger = qPmApp->getStatusChanger();	
		pStatusChanger->setStatus(StatusChanger::Status_Offline);
		
		// tip and logout
		QString companyName = Account::instance()->companyName();
		PMessageBox::information(0, tr("Tip"), tr("You are frozen in %1, please contact the manager").arg(companyName));
		doLogout();
	}
}

void CPscDlg::createDiscussDialog(const QStringList &preAddUids /*= QStringList()*/)
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't create discuss"));
		return;
	}

	CreateDiscussDialog *pDlg = new CreateDiscussDialog(CreateDiscussDialog::Type_Create, QString(), QStringList(), preAddUids);
	connect(pDlg, SIGNAL(createDiscuss(QString, QStringList)), this, SLOT(createDiscuss(QString, QStringList)));
	connect(pDlg, SIGNAL(openDiscuss(QString)), this, SLOT(openDiscussChat(QString)));
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void CPscDlg::createInterphone(const QString &name, const QStringList &uids)
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(0, tr("Tip"), tr("You are offline, can't create interphone"));
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	QString memberName;
	QString selfId = Account::instance()->id();
	QString discussName = name;
	if (discussName.isEmpty())
	{
		int count = 0;
		foreach (QString uid, uids)
		{
			if (uid != selfId)
			{
				memberName = modelManager->userName(uid);
				discussName.append(memberName);
				discussName.append(tr(","));

				if (++count >= 2)
					break;
			}
		}
		if (!discussName.isEmpty())
		{
			discussName.remove(discussName.length()-1, 1);
		}
	}

	// create discuss
	DiscussManager *dm = qPmApp->getDiscussManager();
	int handle = dm->createDiscuss(discussName, uids);
	m_discussHandles << handle;
	m_interphoneDiscussHandles << handle;
}

void CPscDlg::createDiscuss( const QString &name, const QStringList &uids )
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(0, tr("Tip"), tr("You are offline, can't create discuss"));
		return;
	}

	// create discuss
	DiscussManager* dm = qPmApp->getDiscussManager();
	m_discussHandles << dm->createDiscuss(name, uids);
}

void CPscDlg::exitDiscuss( const QString &id, bool force /*= false*/ )
{
	ModelManager *modelManager = qPmApp->getModelManager();
	DiscussItem *discussItem = modelManager->discussModel()->getDiscuss(id);
	if (!discussItem)
		return;

	DiscussManager *dm = qPmApp->getDiscussManager();
	QString discussName = discussItem->itemName();
	QString creator = discussItem->creator();
	if (force) // quit directly
	{
		if (creator != Account::instance()->id())
		{
			// 不是创建者退出讨论组
			m_quitHandles << dm->quitDiscuss(id, discussName, Account::instance()->id());
		}
		else
		{
			// 创建者则解散讨论组
			dm->disband(id, creator);
		}
		
		return;
	}

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't quit discuss"));
		return;
	}

	QDialogButtonBox::StandardButton sb = QDialogButtonBox::No;
	if (creator != Account::instance()->id())
	{
		sb = PMessageBox::question(this, tr("Quit Discuss"), 
			tr("Are you sure to quit discuss"), 
			QDialogButtonBox::Yes | QDialogButtonBox::No);
	}
	else
	{
		sb = PMessageBox::question(this, tr("Quit Discuss"), 
			tr("You are creator of discuss, quit will make discuss dissolved, do you want to continue"), 
			QDialogButtonBox::Yes | QDialogButtonBox::No);
	}
	if (QDialogButtonBox::Yes == sb)
	{
		if (creator != Account::instance()->id())
		{
			// 不是创建者退出讨论组
			m_quitHandles << dm->quitDiscuss(id, discussName, Account::instance()->id());
		}
		else
		{
			// 创建者则解散讨论组
			dm->disband(id, creator);
		}
	}
}

void CPscDlg::disbandDiscuss(const QString &id)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	DiscussItem *discussItem = modelManager->discussModel()->getDiscuss(id);
	QString discussName = discussItem->itemName();
	QString creator = discussItem->creator();

	if (!discussItem)
		return;

	DiscussManager *dm = qPmApp->getDiscussManager();
	QDialogButtonBox::StandardButton sb = QDialogButtonBox::No;
	sb = PMessageBox::question(this, tr("Information"), 
		tr("Are you sure to disband discuss?"), 
		QDialogButtonBox::Yes | QDialogButtonBox::No);

	if (QDialogButtonBox::Yes == sb)
	{
		//解散讨论组
		dm->disband(id, creator);
	}
}

void CPscDlg::changeDiscussName(const QString &id)
{
	if (id.isEmpty())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasDiscussItem(id))
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(0, tr("Tip"), tr("You are offline, can't modify name of discuss"));
		return;
	}

	QString oldName = modelManager->discussName(id);
	PlainTextLineInput changeNameInput(this);
	changeNameInput.setWindowModality(Qt::WindowModal);
	changeNameInput.init(tr("Modify name"), tr("Please input new name"), 
		GuiConstants::kMaxDiscussNameLength, PlainTextLineInput::ModeUnicode, oldName);
	if (QDialog::Rejected == changeNameInput.exec())
		return;

	QString newName = changeNameInput.getInputText().trimmed();
	if (newName.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(0, tr("Tip"), tr("You are offline, please try when online"));
		return;
	}

	if (!modelManager->hasDiscussItem(id))
		return;

	DiscussManager *dm = qPmApp->getDiscussManager();
	dm->changeName(id, newName);
}

void CPscDlg::addDiscussMembers(const QString &id, const QStringList &members)
{
	QWidget *senderWidget = qobject_cast<QWidget *>(sender());
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(senderWidget, tr("Tip"), tr("You are offline, can't invite member of discuss"));
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	QString discussName = modelManager->discussName(id);
	DiscussManager *dm = qPmApp->getDiscussManager();
	m_addMemberHandles << dm->addMembers(id, discussName, members);
}

void CPscDlg::onCreatedDiscuss(int handle, const QString &id)
{
	if (m_discussHandles.contains(handle))
	{
		m_discussHandles.removeAll(handle);

		m_newDiscussIds << id;

		if (m_interphoneDiscussHandles.contains(handle))
		{
			m_interphoneDiscussHandles.removeAll(handle);

			m_interphoneDiscussId << id;
		}
	}
}

void CPscDlg::onAddedMembers(int handle, const QString &id)
{
	Q_UNUSED(id);
	if (m_addMemberHandles.contains(handle))
	{
		m_addMemberHandles.removeAll(handle);
	}
}

void CPscDlg::onQuitedDiscuss(int handle, const QString &id)
{
	if (m_quitHandles.contains(handle))
	{
		m_quitHandles.removeAll(handle);

		onQuitedDiscuss(id);
	}
}

void CPscDlg::onQuitedDiscuss(const QString &id)
{
	if (qPmApp->getModelManager()->hasDiscussItem(id))
	{
		QString name = qPmApp->getModelManager()->discussName(id);
		PMessageBox* pMB = new PMessageBox(PMessageBox::Information, 
			tr("You have quited discuss: %1").arg(name), QDialogButtonBox::Ok, tr("Tip"));
		WidgetManager::showActivateRaiseWindow(pMB);

		// do stuff if discuss is removed
		onDiscussRemoved(id);
	}
}

void CPscDlg::onDiscussNameChanged(int handle, const QString &id, const QString &name)
{
	Q_UNUSED(handle);
	Q_UNUSED(id);
	Q_UNUSED(name);

	// there is discuss notification, process there
}

void CPscDlg::onDiscussError( int handle, int type, const QString &errmsg, const QString &discussId, const QString &discussName, const QStringList &members )
{
	Q_UNUSED(type);
	Q_UNUSED(errmsg);
	if (m_discussHandles.contains(handle))
	{
		m_discussHandles.removeAll(handle);

		QStringList baseUids;
		baseUids << Account::instance()->id();
		CreateDiscussDialog *pDlg = 0;
		if (!m_interphoneDiscussHandles.contains(handle))
		{
			pDlg = new CreateDiscussDialog(CreateDiscussDialog::Type_Create, discussName, baseUids, members, true);
			connect(pDlg, SIGNAL(createDiscuss(QString, QStringList)), this, SLOT(createDiscuss(QString, QStringList)));
			connect(pDlg, SIGNAL(openDiscuss(QString)), this, SLOT(openDiscussChat(QString)));
		}
		else
		{
			m_interphoneDiscussHandles.removeAll(handle);

			pDlg = new CreateDiscussDialog(CreateDiscussDialog::Type_CreateInterphone, discussName, baseUids, members, true);
			connect(pDlg, SIGNAL(createInterphone(QString, QStringList)), this, SLOT(createInterphone(QString, QStringList)));
		}
		WidgetManager::showActivateRaiseWindow(pDlg);

		PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
			tr("Create discuss '%1' failed, please try again").arg(discussName), 
			QDialogButtonBox::Ok, tr("Error"), pDlg);
		WidgetManager::showActivateRaiseWindow(pMB);
	}

	if (m_addMemberHandles.contains(handle))
	{
		m_addMemberHandles.removeAll(handle);

		ModelManager *modelManager = qPmApp->getModelManager();
		GroupItemListModel *discussMemberModel = modelManager->discussItemsModel(discussId);
		if (discussMemberModel)
		{
			QStringList baseUids = discussMemberModel->allMemberIds();
			QStringList preAddUids = baseUids;
			preAddUids << members;
			preAddUids.removeDuplicates();
			CreateDiscussDialog *pDlg = new CreateDiscussDialog(CreateDiscussDialog::Type_Add, discussName, baseUids, preAddUids, true);
			pDlg->setDiscussId(discussId);
			connect(pDlg, SIGNAL(addMembers(QString, QStringList)), this, SLOT(addDiscussMembers(QString, QStringList)));
			WidgetManager::showActivateRaiseWindow(pDlg);

			PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
				tr("Invite member of discuss failed, please try again"), 
				QDialogButtonBox::Ok, tr("Error"), pDlg);
			WidgetManager::showActivateRaiseWindow(pMB);
		}
	}

	if (m_quitHandles.contains(handle))
	{
		m_quitHandles.removeAll(handle);

		PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
			tr("Quit discuss '%1' failed, please try again").arg(discussName), 
			QDialogButtonBox::Ok, tr("Error"));
		WidgetManager::showActivateRaiseWindow(pMB);
	}

	if (type == DiscussManager::ChangeName)
	{
		PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
			tr("Modify name of discuss failed, please try again"), 
			QDialogButtonBox::Ok, tr("Error"));
		WidgetManager::showActivateRaiseWindow(pMB);
	}

	if (type == DiscussManager::ChangeCardName)
	{
		PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
			tr("Modify card name in discuss '%1' failed, please try again")
			.arg(discussName), 
			QDialogButtonBox::Ok, tr("Error"));
		WidgetManager::showActivateRaiseWindow(pMB);
	}
}

void CPscDlg::onNotifyDiscussChanged( const QString &id )
{
	if (m_newDiscussIds.contains(id))
	{
		m_newDiscussIds.removeAll(id);
		DiscussDialog *discussDialog = qPmApp->getBuddyMgr()->openDiscussChat(id);

		if (m_interphoneDiscussId.contains(id))
		{
			m_interphoneDiscussId.removeAll(id);

			discussDialog->createInterphone();
		}
	}
}

void CPscDlg::onDiscussNewAdded(const QString &id)
{
	QString interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_DiscussChat, id);
	qPmApp->getInterphoneManager()->syncInterphoneMember(interphoneId);
}

void CPscDlg::onDiscussChangeNotice(const QString &param)
{
	DiscussManager *dm = qPmApp->getDiscussManager();
	if (param.isEmpty()) // refresh all
	{
		dm->syncDiscuss();
	}
	else
	{
		QStringList params = param.split(":");
		if (params.count() != 2)
			return;

		QString discussId = QString::fromUtf8(QByteArray::fromBase64(params[0].toLatin1()));
		QString changeType = params[1];
		if (changeType == "delete")
		{
			// do stuff if discuss is removed
			if (qPmApp->getModelManager()->hasDiscussItem(discussId))
			{
				QString discussName = qPmApp->getModelManager()->discussName(discussId);
				PMessageBox* pMB = new PMessageBox(PMessageBox::Information, 
					tr("Discuss '%1' has been dissolved").arg(discussName), 
					QDialogButtonBox::Ok, tr("Tip"), 0);
				WidgetManager::showActivateRaiseWindow(pMB);

				onDiscussRemoved(discussId);
			}
		}
		else if (changeType == "kick")
		{
			// do stuff if you are kicked
			if (qPmApp->getModelManager()->hasDiscussItem(discussId))
			{
				QString discussName = qPmApp->getModelManager()->discussName(discussId);
				PMessageBox* pMB = new PMessageBox(PMessageBox::Information, 
					tr("You are removed from discuss '%1'").arg(discussName), 
					QDialogButtonBox::Ok, tr("Tip"), 0);
				WidgetManager::showActivateRaiseWindow(pMB);

				onDiscussRemoved(discussId);
			}
		}
		else // "add" or "change"
		{
			dm->syncDiscuss(discussId);
		}
	}
}

void CPscDlg::onDiscussRemoved(const QString &discussId)
{
	// get discuss name first
	QString discussName = qPmApp->getModelManager()->discussName(discussId);

	// delete from model
	qPmApp->getModelManager()->delDiscuss(discussId);

	// remove from last contact
	removeDiscussChat(discussId);

	// reset discuss msg settings
	Account::settings()->setDiscussMsgSetting(discussId, AccountSettings::Tip);

	// if is in interphone, need quit interphone
	bool quitInterphone = false;
	QString discussInterphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_DiscussChat, discussId);
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
		if (dlg->interphoneId() == discussInterphoneId)
		{
			dlg->quitAndClose();
			quitInterphone = true;
		}
	}

	// if has interphone item, need to remove
	if (!quitInterphone)
	{
		qPmApp->getInterphoneManager()->removeInterphone(discussInterphoneId);
	}

	// close discuss dialog
	qPmApp->getBuddyMgr()->closeChat(bean::Message_DiscussChat, discussId);
}

void CPscDlg::onDiscussKickFailed(const QString &discussId, const QString &uid, const QString &errMsg)
{
	if (discussId.isEmpty() || uid.isEmpty())
		return;

	QString discussName = qPmApp->getModelManager()->discussName(discussId);
	QString memberName = qPmApp->getModelManager()->userName(uid);
	PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
		tr("Remove %2 from discuss '%1' failed (%3), please try again")
		.arg(discussName).arg(memberName).arg(errMsg), 
		QDialogButtonBox::Ok, tr("Error"));
	WidgetManager::showActivateRaiseWindow(pMB);
}

void CPscDlg::onDiscussKickOK(const QString &discussId, const QString &uid)
{
	Q_UNUSED(discussId);
	Q_UNUSED(uid);
}

void CPscDlg::onDiscussDisbandFailed(const QString &discussId, const QString &errMsg)
{
	if (discussId.isEmpty())
		return;

	QString discussName = qPmApp->getModelManager()->discussName(discussId);
	PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, 
		tr("Delete discuss '%1' failed (%2), please try again")
		.arg(discussName).arg(errMsg), 
		QDialogButtonBox::Ok, tr("Error"));
	WidgetManager::showActivateRaiseWindow(pMB);
}

void CPscDlg::onDiscussDisbandOK(const QString &discussId)
{
	if (qPmApp->getModelManager()->hasDiscussItem(discussId))
	{
		onDiscussRemoved(discussId);
	}
}

void CPscDlg::onGroupChangeNotice(const QString &param)
{
	GroupManager *groupManager = qPmApp->getGroupManager();
	if (param.isEmpty()) // refresh all
	{
		groupManager->syncGroups();
	}
	else
	{
		QStringList params = param.split(":");
		if (params.count() != 2)
			return;

		QString groupId = QString::fromUtf8(QByteArray::fromBase64(params[0].toLatin1()));
		QString changeType = params[1];
		if (changeType == "delete")
		{
			// remove from model
			qPmApp->getModelManager()->delGroup(groupId);

			// remove group chat
			removeGroupChat(groupId);

			// close discuss dialog
			qPmApp->getBuddyMgr()->checkChatDlgValid(true);
		}
		else if (changeType == "change") // "change"
		{
			if (qPmApp->getModelManager()->hasGroupItem(groupId))
				groupManager->syncGroupMembers(groupId);
			else
				groupManager->syncGroups();
		}
		else // "add"
		{
			groupManager->syncGroups();
		}
	}
}

void CPscDlg::setShortcutKey()
{
	int failedCount = 0;
	QString shotKey = Account::settings()->getScreenshotKey();
	if (!m_pScreenshotShortcut->setShortcut(QKeySequence(shotKey)))
	{
		++failedCount;
	}

	QString takeMsgKey = Account::settings()->getTakeMsgKey();
	if (!m_pTakeMsgShortcut->setShortcut(QKeySequence(takeMsgKey)))
	{
		++failedCount;
	}

	if (failedCount > 0 && GlobalSettings::isShortcutConflickTipOn())
	{
		showShortcutConflictDlg(failedCount);
	}
	else
	{
		hideShortcutConflictDlg();
	}
}

void CPscDlg::showShortcutConflictDlg(int failedCount)
{
	ShortcutConflictDlg *shortcutConflictDlg = ShortcutConflictDlg::instance(failedCount);
	connect(shortcutConflictDlg, SIGNAL(modifyShortcutKey()), this, SLOT(modifyShortcutKey()), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(shortcutConflictDlg);
}

void CPscDlg::hideShortcutConflictDlg()
{
	ShortcutConflictDlg *shortcutConflictDlg = ShortcutConflictDlg::instance(0);
	shortcutConflictDlg->hide();
}

void CPscDlg::modifyShortcutKey()
{
	on_btnSystemSettings_clicked();

	SystemSettingsDialog *settingsDialog = SystemSettingsDialog::getInstance();
	settingsDialog->setSettingIndex(SystemSettingsDialog::IndexShortcut);
}

void CPscDlg::dockIn()
{
	if (m_dockInAnimation->state() == QPropertyAnimation::Running)
	{
		return;
	}

	if (m_dockState || !m_dockReady)
	{
		return;
	}

	QRect frameRect = frameGeometry();
	QRect checkRect = frameRect.adjusted(-5, -5, 5, 5);
	if (checkRect.contains(QCursor::pos()))
	{
		return;
	}

	QRect rect = frameGeometry();
	QRect availableRect = QApplication::desktop()->availableGeometry();
	int x = availableRect.right() - 1;
	QPoint endPoint = rect.topLeft();
	endPoint.setX(x);
	m_dockInAnimation->setEndValue(endPoint);
	m_dockInAnimation->start();
}

void CPscDlg::dockOut(bool checkCursor /*= true*/)
{
	if (m_dockOutAnimation->state() == QPropertyAnimation::Running)
	{
		return;
	}

	if (!m_dockState)
	{
		return;
	}

	QRect frameRect = frameGeometry();
	if (checkCursor && !frameRect.contains(QCursor::pos()))
	{
		return;
	}

	QRect rect = frameGeometry();
	QRect availableRect = QApplication::desktop()->availableGeometry();
	int x = availableRect.right() - rect.width();
	QPoint endPoint = rect.topLeft();
	endPoint.setX(x);
	m_dockOutAnimation->setEndValue(endPoint);
	m_dockOutAnimation->start();

	if (!checkCursor)
	{
		m_dockCheckTimer->stop();
		QTimer::singleShot(3000, m_dockCheckTimer, SLOT(start()));
	}
}

void CPscDlg::onDockInFinished()
{
	m_dockState = true;
	setResizeable(false);
	setTopmost(true);
}

void CPscDlg::onDockOutFinished()
{
	m_dockState = false;
	setResizeable(true);
}

void CPscDlg::checkDock()
{
	if (!Account::settings()->isPscEdgeHide())
	{
		return;
	}

	if (m_dockInAnimation->state() == QPropertyAnimation::Running ||
		m_dockOutAnimation->state() == QPropertyAnimation::Running)
	{
		return;
	}

	if (!m_dockState && m_dockReady)
	{
		dockIn();
	}
	else if (m_dockState && !m_leftKeyPressed)
	{
		dockOut();
	}
}

void CPscDlg::checkToClearAllMessages()
{
	// deal with clear message 
	bool needPromptDeleteMsg = Account::settings()->clearMsgWhenClose();
	if (needPromptDeleteMsg)
	{
		QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Delete history message"), 
			tr("You set auto delete history message when logout.\nDo you want to delete now"), 
			QDialogButtonBox::Yes|QDialogButtonBox::No);
		if (ret == QDialogButtonBox::Yes)
		{
			// delete all message records
			DB::ComponentMessageDB::clearMessages();

			// set last message's content empty
			QScopedPointer<DB::LastContactDB> lastContactDB;
			lastContactDB.reset(new DB::LastContactDB());
			lastContactDB->clearLastBody();
		}
	}
}

void CPscDlg::prepareForQuit()
{
	m_bClose = true;

	checkToClearAllMessages();
}

void CPscDlg::onPasswdModified()
{
	// clear password of this account
	Account::instance()->clearPassword();

	// off-line first
	StatusChanger* pStatusChanger = qPmApp->getStatusChanger();	
	pStatusChanger->setStatus(StatusChanger::Status_Offline);

	// tip and logout
	PMessageBox::information(0, tr("Offline notice"), tr("Password has been modified.\nPlease re-login for safety"));
	doLogout();
}

void CPscDlg::onValidateFailed()
{
	// clear password of this account
	Account::instance()->clearPassword();

	// tip and logout
	PMessageBox::information(0, tr("Re-login"), tr("Your password may be modified other place.\nPlease re-login for safety"));
	doLogout();
}

void CPscDlg::onRosterAddNotice(int action, const QString &param)
{
	// base64(fromId):base64(fromName):base64(toId):base64(toName):base64(group):base64(message):sessionId
	if (param.isEmpty())
		return;

	QStringList params = param.split(":");
	if (params.count() != 7)
	{
		qDebug() << Q_FUNC_INFO << "param is not correct: " << param;
		return;
	}

	// request the add friend list
	if (qPmApp->GetLoginMgr()->isLogined())
	{
		QString fromId = QString::fromUtf8(QByteArray::fromBase64(params[0].toLatin1()));
		QString fromName = QString::fromUtf8(QByteArray::fromBase64(params[1].toLatin1()));
		QString toId = QString::fromUtf8(QByteArray::fromBase64(params[2].toLatin1()));
		QString toName = QString::fromUtf8(QByteArray::fromBase64(params[3].toLatin1()));
		QString group = QString::fromUtf8(QByteArray::fromBase64(params[4].toLatin1()));
		QString message = QString::fromUtf8(QByteArray::fromBase64(params[5].toLatin1()));
		QString sId = params[6];

		if (action == AddFriendManager::Request &&
			toId == Account::instance()->id() &&
			qPmApp->getModelManager()->isInBlackList(fromId))
		{
			qDebug() << "receive add friend request from black: " << fromId;
			return;
		}

		onRosterAddNotice(action, fromId, fromName, toId, toName, sId, group, message);

		// if list is open, need to refresh
		if (AddFriendListDlg::instance()->isVisible())
		{
			qPmApp->getAddFriendManager()->requestAddFriendList();
		}
	}
}

void CPscDlg::onRosterAddNotice(int action, const QString &fromId, const QString &fromName,
	                            const QString &toId, const QString &toName, const QString &sId, 
								const QString &group, const QString &message, bool read /*= false*/)
{
	QString selfId = Account::instance()->id();
	QString otherId = fromId;
	QString otherName = fromName;
	if (otherId == selfId)
	{
		otherId = toId;
		otherName = toName;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	bool isFriend = rosterModel->isFriend(otherId);
	if (action == AddFriendManager::Request)
	{
		if (isFriend)
		{
			// accept directly
			QString group1 = RosterModel::defaultGroupName();
			qPmApp->getAddFriendManager()->addFriendAction(AddFriendManager::Accept, fromId, toId, sId, message, group, group1);
		}
		else
		{
			// show un-handle flag
			setAddFriendUnhandleFlag(true);

			if (!AddFriendListDlg::instance()->isVisible() && !read) // only show tip when list dialog is not visible
			{
				bool addMsg = true;
				UnreadMsgItem *unreadMsgItem = qPmApp->getUnreadMsgModel()->peekUnreadMsg(QString(ROSTER_ADD_MESSAGE_ID), bean::Message_Chat);
				if (unreadMsgItem)
				{
					// same request only add once
					QList<bean::MessageBody> msgs = unreadMsgItem->msgs();
					foreach (bean::MessageBody msg, msgs)
					{
						bean::MessageExt ext = msg.ext();
						QString other = ext.data("other").toString();
						QString addType = ext.data("addtype").toString();
						if (other == otherId && addType == "request")
						{
							addMsg = false;
							break;
						}
					}
				}

				if (addMsg)
				{
					// add a message
					bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
					msgBody.setSend(false);
					msgBody.setFrom(selfId);
					msgBody.setTo(QString(ROSTER_ADD_MESSAGE_ID));
					msgBody.setTime(CDateTime::currentDateTimeUtcString());
					msgBody.setBody(tr("He wants to add you as a friend: %1").arg(message));
					bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
					ext.setData("level", "info");
					ext.setData("other", otherId);
					ext.setData("addtype", "request");
					ext.setData("sid", sId);
					msgBody.setExt(ext);
					qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
				}
			}
		}
	}
	else if (action == AddFriendManager::Accept)
	{
		// confirm first
		qPmApp->getAddFriendManager()->addFriendConfirm(sId);

		// set read
		qPmApp->getAddFriendManager()->addFriendRead(sId);

		if (!isFriend)
		{
			// add to roster
			onAddFriendOK(otherId, otherName, group);
		}

		// add a message to chat dialog
		bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
		msgBody.setSend(false);
		msgBody.setFrom(selfId);
		msgBody.setTo(otherId);
		msgBody.setToName(otherName);
		msgBody.setTime(CDateTime::currentDateTimeUtcString());
		msgBody.setBody(tr("We are friends now"));
		bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
		ext.setData("level", "info");
		ext.setData(bean::EXT_DATA_LASTCONTACT_NAME, true);
		ext.setData(bean::EXT_DATA_HISTORY_NAME, true);
		msgBody.setExt(ext);
		qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
	}
	else if (action == AddFriendManager::Refuse)
	{
		// confirm first
		qPmApp->getAddFriendManager()->addFriendConfirm(sId);

		// set read
		qPmApp->getAddFriendManager()->addFriendRead(sId);

		if (!isFriend)
		{
			if (!AddFriendListDlg::instance()->isVisible()) // only show tip when list dialog is not visible
			{
				// add a message
				bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
				msgBody.setSend(false);
				msgBody.setFrom(selfId);
				msgBody.setTo(QString(ROSTER_ADD_MESSAGE_ID));
				msgBody.setTime(CDateTime::currentDateTimeUtcString());
				msgBody.setBody(tr("He refused you: %1").arg(message));
				bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
				ext.setData("level", "info");
				ext.setData("other", otherId);
				ext.setData("addtype", "refuse");
				msgBody.setExt(ext);
				qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
			}
		}
	}
}

void CPscDlg::onRosterAddList()
{
	AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
	QList<AddFriendManager::Item> origItems = addFriendManager->refreshItems();
	QList<AddFriendManager::Item> items;

	// do data analyze, only keep request from others or which is solved
	foreach (AddFriendManager::Item origItem, origItems)
	{
		bool add = true;
		if (origItem.m_fromId == Account::instance()->id() && origItem.m_action == AddFriendManager::Request)
		{
			add = false;
		}
		else
		{
			for (int k = 0; k < items.count(); k++)
			{
				AddFriendManager::Item item = items[k];
				if (origItem.m_sId == item.m_sId)
				{
					if (origItem.m_action != AddFriendManager::Request && item.m_action == AddFriendManager::Request)
					{
						items[k] = origItem;
					}
					add = false;
					break;
				}
				else // different session id
				{
					// previous request do not show
					if (((origItem.m_fromId == item.m_fromId && origItem.m_toId == item.m_toId) || (origItem.m_fromId == item.m_toId && origItem.m_toId == item.m_fromId))
						&& origItem.m_action == AddFriendManager::Request)
					{
						add = false;
						break;
					}
				}
			}
		}

		if (add)
		{
			items.append(origItem);
		}
	}

	// remove request from black list
	QList<int> blackIds;
	int index = 0;
	for (index = 0; index < items.count(); index++)
	{
		AddFriendManager::Item item = items[index];
		if (item.m_action == AddFriendManager::Request && qPmApp->getModelManager()->isInBlackList(item.m_fromId))
			blackIds.insert(0, index);
	}
	for (index = 0; index < blackIds.count(); index++)
	{
		int blackId = blackIds[index];
		items.removeAt(blackId);
	}

	// check if need to show un-handle flag
	for (; index < items.count(); index++)
	{
		AddFriendManager::Item item = items[index];
		if (item.m_action == AddFriendManager::Request)
		{
			break;
		}
	}

	if (index >= items.count())
	{
		setAddFriendUnhandleFlag(false);
	}
	else
	{
		setAddFriendUnhandleFlag(true);
	}

	if (AddFriendListDlg::instance()->isVisible())
		return;

	// clear unread add friend message first
	qPmApp->getUnreadMsgModel()->takeMsg(QString(ROSTER_ADD_MESSAGE_ID), bean::Message_Chat);

	// show the result
	QString selfId = Account::instance()->id();
	foreach (AddFriendManager::Item item, items)
	{
		if ((item.m_action == AddFriendManager::Request) ||
			(item.m_action == AddFriendManager::Accept && item.m_fromId == selfId && item.m_status != 1) ||
			(item.m_action == AddFriendManager::Refuse && item.m_fromId == selfId && item.m_status != 1))
		{
			onRosterAddNotice((int)item.m_action, item.m_fromId, item.m_fromName, item.m_toId, item.m_toName,
				item.m_sId, item.m_group, item.m_message, item.m_read);
		}
	}
}

void CPscDlg::setAddFriendUnhandleFlag(bool hasUnhandle)
{
	if (!hasUnhandle)
	{
		ui->btnAddFriendList->setState(StyleToolButton::State1st);
	}
	else
	{
		ui->btnAddFriendList->setState(StyleToolButton::State2nd);
	}
}

void CPscDlg::onRosterAddResponded(const QString &param)
{
	qPmApp->getAddFriendManager()->requestAddFriendList();

	/*
	同意:
	accept:sId:base64(fromId):base64(fromName):base64(group1)

	拒绝
	refuse:sId
	*/

	if (param.startsWith("accept"))
	{
		QStringList parts = param.split(":");
		if (parts.count() != 5)
			return;

		QString addId = QString::fromUtf8(QByteArray::fromBase64(parts[2].toLatin1()));
		QString addName = QString::fromUtf8(QByteArray::fromBase64(parts[3].toLatin1()));
		QString addGroup = QString::fromUtf8(QByteArray::fromBase64(parts[4].toLatin1()));
		onAddFriendOK(addId, addName, addGroup);
	}
}

void CPscDlg::onSubscriptionSubscribed(const SubscriptionDetail &subscription, const SubscriptionMsg &msg)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	if (modelManager->hasSubscriptionItem(subscription.id()))
		return;

	// insert to model
	SubscriptionModel *subscriptionModel = qPmApp->getModelManager()->subscriptionModel();
	if (subscriptionModel)
	{
		subscriptionModel->addSubscription(subscription);
	}

	// recv message
	SubscriptionMsgManager *subscriptionMsgManager = qPmApp->getSubscriptionMsgManager();
	if (subscriptionMsgManager)
	{
		subscriptionMsgManager->recvMsg(msg);
	}

	// get menu
	qPmApp->getSubscriptionManager()->getMenu(subscription.id());
}

void CPscDlg::onSubscriptionUnsubscribed(bool ok, const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasSubscriptionItem(subscriptionId))
		return;

	if (ok) 
	{
		removeSubscription(subscriptionId);
		qPmApp->getSubscriptionDetailAndHistoryManager()->onSubscriptionUnsubscribed(ok, subscriptionId);
	}
	else
	{
		PMessageBox* pMB = new PMessageBox(PMessageBox::Warning, tr("Unfollow subscription failed, please try again"), 
			QDialogButtonBox::Ok, tr("Error"), 0);
		WidgetManager::showActivateRaiseWindow(pMB);
	}
}

void CPscDlg::onSubscriptionSubscribed(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	// if this subscription has not been subscribed, get the whole list
	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasSubscriptionItem(subscriptionId))
	{
		SubscriptionManager *subscriptionManager = qPmApp->getSubscriptionManager();
		subscriptionManager->getSubscriptionList(Account::instance()->id());
	}
}

void CPscDlg::onSubscriptionUnsubscribed(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	// if this subscription has not been un-subscribed, remove this subscription
	ModelManager *modelManager = qPmApp->getModelManager();
	if (modelManager->hasSubscriptionItem(subscriptionId))
	{
		onSubscriptionUnsubscribed(true, subscriptionId);
	}
}

void CPscDlg::removeSubscription(const QString &subscriptionId)
{	
	if (subscriptionId.isEmpty())
		return;

	// close message dialog
	SubscriptionMsgManager *subscriptionMsgManager = qPmApp->getSubscriptionMsgManager();
	subscriptionMsgManager->closeSubscriptionMsgDialog(subscriptionId);
	subscriptionMsgManager->setUnreadMsgCount(subscriptionId, 0);
	subscriptionMsgManager->removeMessageOfSubscription(subscriptionId);

	// remove last message
	SubscriptionLastMsgModel *subscriptionLastMsgModel = qPmApp->getModelManager()->subscriptionLastMsgModel();
	subscriptionLastMsgModel->removeLastMsg(subscriptionId);
	if (subscriptionLastMsgModel->rowCount() <= 0)
	{
		LastContactModel *lastContactModel = qPmApp->getModelManager()->lastContactModel();
		lastContactModel->onRemoveChat(QString(SUBSCRIPTION_ROSTER_ID));
	}

	// remove from subscription model
	SubscriptionModel *subscriptionModel = qPmApp->getModelManager()->subscriptionModel();
	subscriptionModel->removeSubscription(subscriptionId);
}

void CPscDlg::onSubscriptionUnreadMsgChanged(const QString &subscriptionId, int count)
{
	Q_UNUSED(count);
	if (subscriptionId.isEmpty())
		return;

	SubscriptionModel *subscriptionModel = qPmApp->getModelManager()->subscriptionModel();
	if (!subscriptionModel->hasSubscription(subscriptionId))
		return;

	// update last contact message count
	LastContactModel *lastContactModel = qPmApp->getModelManager()->lastContactModel();
	if (lastContactModel->hasUnreadMsg())
	{
		ui->pageWidget->setTabButtonIcon(m_lastContactView, QString(":/images/Icon_54_emphasis.png"), QString(":/images/Icon_54_on.png"), "");
	}
	else
	{
		ui->pageWidget->setTabButtonIcon(m_lastContactView, QString(":/images/Icon_54_normal.png"), QString(":/images/Icon_54_on.png"), QString(":/images/Icon_54_hover.png"));
	}
}

void CPscDlg::openSubscriptionDialog()
{
	SubscriptionDialog *dlg = SubscriptionDialog::getDialog();
	connect(dlg, SIGNAL(openSubscriptionMsg(QString)), this, SLOT(openSubscriptionMsg(QString)), Qt::UniqueConnection);
	connect(dlg, SIGNAL(openSubscriptionDetail(QString)), this, SLOT(openSubscriptionDetail(QString)), Qt::UniqueConnection);
	connect(dlg, SIGNAL(openSubscriptionHistory(QString)), this, SLOT(openSubscriptionHistory(QString)), Qt::UniqueConnection);
	connect(dlg, SIGNAL(searchSubscription()), this, SLOT(searchSubscription()), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(dlg);
}

void CPscDlg::addBlack(const QString &id)
{
	if (id.isEmpty())
		return;

	// set to config
	ModelManager *modelManager = qPmApp->getModelManager();
	BlackListModel *model = modelManager->blackListModel();
	QStringList ids = model->allIds();
	ids.append(id);
	ids.removeDuplicates();
	ConfigManager *configManager = qPmApp->getConfigManager();
	configManager->setConfig2(ids);

	// update model
	modelManager->onGotBlackListIds(ids);

	emit blackListChanged();
}

void CPscDlg::removeBlack(const QString &id)
{
	if (id.isEmpty())
		return;

	// set to config
	ModelManager *modelManager = qPmApp->getModelManager();
	BlackListModel *model = modelManager->blackListModel();
	QStringList ids = model->allIds();
	ids.removeAll(id);
	ConfigManager *configManager = qPmApp->getConfigManager();
	configManager->setConfig2(ids);

	// update model
	modelManager->onGotBlackListIds(ids);

	emit blackListChanged();
}

void CPscDlg::manageBlack()
{
	BlackListDialog *blackListDialog = BlackListDialog::getBlackListDialog();
	connect(blackListDialog, SIGNAL(removeBlack(QString)), this, SLOT(removeBlack(QString)), Qt::UniqueConnection);
	connect(blackListDialog, SIGNAL(viewMaterial(QString)), this, SLOT(viewContactInfo(QString)), Qt::UniqueConnection);
	WidgetManager::showActivateRaiseWindow(blackListDialog);
}

void CPscDlg::onAppButtonClicked()
{
	QToolButton *btn = qobject_cast<QToolButton *>(sender());
	if (!btn)
		return;

	int index = m_appButtons.indexOf(btn);
	if (index == -1)
		return;

	QString path = m_appButtonPathes[index];
	QFileInfo fi(path);
	if (!fi.exists())
	{
		PMessageBox::warning(this, tr("Tip"), tr("App does not exist, please add again"));
		return;
	}

	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
	{
		PMessageBox::warning(this, tr("Tip"), tr("Can't find suitable program to open this app"));
		return;
	}
}

void CPscDlg::setAppButtons()
{
	// clear original buttons
	for (int i = 0; i < m_appButtons.count(); i++)
	{
		QToolButton *btn = m_appButtons[i];
		ui->horizontalLayoutApps->removeWidget(btn);
	}
	qDeleteAll(m_appButtons);
	m_appButtons.clear();
	m_appButtonPathes.clear();

	// add new buttons
	int index = 0;
	QList<AccountSettings::AppInfo> infos = Account::settings()->appInfos();
	foreach (AccountSettings::AppInfo info, infos)
	{
		QIcon icon = AppManageModel::appSmallIcon(info.path);
		QString name = info.name;
		QToolButton *btn = new QToolButton(this);
		btn->setIcon(icon);
		btn->setIconSize(QSize(20, 20));
		btn->setText(name);
		btn->setToolTip(name);
		ui->horizontalLayoutApps->insertWidget(index, btn);
		m_appButtons.append(btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(onAppButtonClicked()));
		m_appButtonPathes.append(info.path);
		++index;
	}
}

void CPscDlg::onLinkItemClicked()
{
	HttpImageToolButton *btn = qobject_cast<HttpImageToolButton *>(sender());
	if (btn)
	{
		QString linkUrl = btn->data("url").toString().trimmed();
		if (!linkUrl.isEmpty())
		{
			QDesktopServices::openUrl(QUrl::fromUserInput(linkUrl));
		}
	}
}

void CPscDlg::onConfigChanged(const QString &param)
{
	// conf2
	if (param.startsWith("conf"))
	{
		int configNum = param.right(1).toInt();
		qPmApp->getConfigManager()->getConfig(QList<int>() << configNum);
	}
}

void CPscDlg::setSilence(const QStringList &silenceList)
{
	Account::settings()->setSilenceList(silenceList);
	ui->pageWidget->currentWidget()->update();
}

void CPscDlg::getGroupLogoFinished(const QString &gid, int version, const QImage &logo)
{
	// save to file
	bool updateVersion = true;
	if (!logo.isNull())
	{
		QString logoPath = qPmApp->getGroupManager()->logoPath(gid);
		QFile::remove(logoPath); // remove existing
		if (!logo.save(logoPath, "jpg"))
		{
			qWarning() << Q_FUNC_INFO << logoPath << "save failed";
			updateVersion = false;
		}

		// update group view
		m_groupView->update();
	}

	// save group logo version
	if (updateVersion)
	{
		AccountSettings *accountSettings = Account::settings();
		accountSettings->setGroupLogoVersion(gid, version);
	}
}

void CPscDlg::changeGroupCardNameFailed(const QString &gid, const QString &errMsg)
{
	Q_UNUSED(errMsg);

	if (gid.isEmpty())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	if (!modelManager->hasGroupItem(gid))
		return;

	QString groupName = modelManager->groupName(gid);
	PMessageBox *pMB = new PMessageBox(PMessageBox::Warning, 
		tr("Modify my card name in group '%1' failed, please try again").arg(groupName), 
		QDialogButtonBox::Ok, tr("Error"));
	WidgetManager::showActivateRaiseWindow(pMB);
}

void CPscDlg::doLogout()
{
	qPmApp->setLogout(true);
	qPmApp->setLogoutMode(MODE_LOGOUT_COMPANY);
	qPmApp->setSwitchId("");

	// clear messages
	checkToClearAllMessages();

	QTimer::singleShot(0, qPmApp, SLOT(restart()));
}

void CPscDlg::doSwitchCompany(const QString &uid)
{
	qPmApp->setLogout(true);
	qPmApp->setLogoutMode(MODE_SWITCH_COMPANY);
	qPmApp->setSwitchId(uid);

	// clear messages
	checkToClearAllMessages();

	QTimer::singleShot(0, qPmApp, SLOT(restart()));
}

