#include <QtGlobal>
#include <QtCore>
#include <QtGui>
#include <QDesktopWidget>

#include "common/datetime.h"

#include "sortfilterproxymodel.h"

#include "model/ModelManager.h"
#include "model/groupitemlistmodeldef.h"
#include "model/DiscussItemdef.h"
#include "model/DiscussModeldef.h"

#include "login/Account.h"
#include "util/PlayBeep.h"
#include "buddymgr.h"
#include "PmApp.h"
#include "presencemanager.h"
#include "minisplitter.h"
#include "guiconstants.h"
#include "pmessagebox.h"
#include "widgetmanager.h"
#include "messagemanagerdlg.h"
#include "CreateDiscussDialog.h"

#include "settings/GlobalSettings.h"

#include "DiscussDialog.h"
#include "ui_DiscussDialog.h"
#include "chatinputbox.h"
#include "clickablelabel.h"
#include "message4js.h"
#include "MessageDBStore.h"
#include "DiscussManager.h"
#include "util/MsgEncryptionUtil.h"
#include <QWebFrame>
#include "groupsmembermanager.h"
#include "configmanager.h"
#include "iospushmanager.h"

const static int s_minTitleEditLength = 100;
const static int s_maxTitleEditLength = 272;

DiscussDialog::DiscussDialog( const QString &id, QWidget *parent /*= 0*/ )
: ChatBaseDialog(parent)
, m_groupSettingMenu(this)
, m_quitHandle(0)
, m_fetchHistoryMsgId(-1)
{
	ui = new Ui::DiscussDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	/*
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setMainLayout(ui->verticalLayoutMain);
	setResizeable(true);
	setMaximizeable(true);
	*/

	InitUI(id);

	resize(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth, GuiConstants::WidgetSize::ChatHeight);
	setMinimumSize(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth, GuiConstants::WidgetSize::ChatHeight);
	ui->sideStackedWidget->setMinimumWidth(GuiConstants::WidgetSize::GroupChatMemberWidth);
	ui->sideStackedWidget->setFixedWidth(GuiConstants::WidgetSize::GroupChatMemberWidth);

	setSkin();

	connect(ui->btnClose2, SIGNAL(clicked()), this, SIGNAL(requestClose()));
	connect(ui->btnMinimize2, SIGNAL(clicked()), this, SIGNAL(requestMinimized()));
	connect(ui->btnMaximize2, SIGNAL(clicked()), this, SIGNAL(requestMaximized()));

	ui->messagePannel->layoutPannel();
}

DiscussDialog::~DiscussDialog()
{
	qDebug() << Q_FUNC_INFO << ui->messagePannel->id();

	delete ui;
}

QString DiscussDialog::id() const
{
	return ui->messagePannel->id();
}

void DiscussDialog::setSkin()
{
	/*
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_4.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 95;
	setBG(bgPixmap, bgSizes);
	*/

	// set qss file
	QFile qssFile(":/qss/chatdlg.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		setStyleSheet(qss);
		qssFile.close();
	}

	// set button voice style
	StyleToolButton::Info info;
	info.urlNormal = QString(":/images/Icon_85.png");
	info.urlHover = QString(":/images/Icon_85_hover.png");
	info.urlPressed = QString(":/images/Icon_85_hover.png");
	info.urlDisabled = QString(":/images/Icon_85_disabled.png");
	info.tooltip = tr("Start Audio Call");
	ui->btnPtt->setInfo(info);

	// set button video style
	info.urlNormal = QString(":/images/Icon_86.png");
	info.urlHover = QString(":/images/Icon_86_hover.png");
	info.urlPressed = QString(":/images/Icon_86_hover.png");
	info.urlDisabled = QString(":/images/Icon_86_disabled.png");
	info.tooltip = tr("Start Video Call");
	ui->btnVideo->setInfo(info);

	// set button add members
	info.urlNormal = QString(":/images/Icon_113.png");
	info.urlHover = QString(":/images/Icon_113_hover.png");
	info.urlPressed = QString(":/images/Icon_113_hover.png");
	info.urlDisabled = QString(":/images/Icon_113_disabled.png");
	info.tooltip = tr("Invite member of discuss");
	ui->btnAddMembers->setInfo(info);

	// set button quit discuss
	info.urlNormal = QString(":/images/Icon_114.png");
	info.urlHover = QString(":/images/Icon_114_hover.png");
	info.urlPressed = QString(":/images/Icon_114_hover.png");
	info.urlDisabled = QString(":/images/Icon_114_disabled.png");
	info.tooltip = tr("Quit Discuss");
	ui->btnQuitDiscuss->setInfo(info);

	// set button interphone style
	info.urlNormal = QString(":/interphone/create_normal.png");
	info.urlHover = QString(":/interphone/create_hover.png");
	info.urlPressed = QString(":/interphone/create_hover.png");
	info.urlDisabled = QString(":/interphone/create_disabled.png");
	info.tooltip = tr("Interphone");
	ui->btnInterphone->setInfo(info);

	ui->labelCreator->setFontAtt(QColor("#666666"), 10, false);

	ui->title->setTextColor(QColor("#333333"), QColor("#333333"));
	ui->title->setBorderColor(QColor("#D9D8DD"));

	ui->messagePannel->setSkin();
}

void DiscussDialog::slot_screenshot_ok(const QString &imagePath)
{
	ui->messagePannel->slot_screenshot_ok(imagePath);
}

void DiscussDialog::slot_screenshot_cancel()
{
	ui->messagePannel->slot_screenshot_cancel();
}

void DiscussDialog::insertMimeData(const QMimeData *source)
{
	ui->messagePannel->chatInput()->insertMimeData(source);
}

void DiscussDialog::loadHistoryMessages(int count)
{
	Q_UNUSED(count);

	connect(qPmApp->getMessageDBStore(), SIGNAL(gotMessages(qint64, int, int, bean::MessageBodyList)), 
		this, SLOT(onLoadHistoryMessagesFinished(qint64, int, int, bean::MessageBodyList)), Qt::UniqueConnection);
	int pageIndex = this->historyMsgPageIndex();
	QString endMsgTime = this->historyMsgEndTime();
	m_fetchHistoryMsgId = qPmApp->getMessageDBStore()->getMessagesBeforeTime(
		ui->messagePannel->type(), ui->messagePannel->id(), pageIndex, kPageHistoryMessageCount, endMsgTime);
}

void DiscussDialog::fetchHistoryMessages()
{
	bean::MessageType msgType = ui->messagePannel->type();
	QString id = ui->messagePannel->id();
	qPmApp->getBuddyMgr()->getHistoryMsg(msgType, id);
}

void DiscussDialog::fetchMoreMessages()
{
	QString id = ui->messagePannel->id();
	OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
	if (offlineMsgManager->containOfflineItem(OfflineMsgManager::Discuss, id)) // get offline history message
	{
		if (!qPmApp->GetLoginMgr()->isLogined()) // not logined
		{
			qWarning() << Q_FUNC_INFO << "was not logined, discuss: " << id;

			// deal with more message tip
			this->onMoreMsgFinished();
			this->closeMoreMsgTip();
			this->showAutoTip(tr("You are offline, can't check more messages"));
			return;
		}

		fetchHistoryMessages();
	}
	else
	{
		AccountSettings *accountSettings = Account::settings();
		bool loadHistory = accountSettings->chatLoadHistory();

		bean::MessageBodyList moreMsgs = this->moreMessages();
		int msgCount = moreMsgs.count();
		if (msgCount > 0)
		{
			// max load one page
			if (msgCount > kPageHistoryMessageCount)
				msgCount = kPageHistoryMessageCount;
			int moreCount = moreMsgs.count() - msgCount;

			QString lastMsgTime;
			for (int i = 0; i < msgCount; ++i)
			{
				bean::MessageBody msg = moreMsgs.takeLast();
				lastMsgTime = msg.time();
				this->onMessage(msg, true, false);
			}

			// update more count
			this->setMoreMessages(moreMsgs);
			this->setMoreCount(moreCount);

			// if more message is empty, set history time
			if (moreMsgs.isEmpty())
			{
				this->setHistoryMsgEndTime(lastMsgTime);
				this->setHistoryMsgPageIndex(INT_MAX);
			}

			// deal with more message tip
			this->onMoreMsgFinished();
			
			if (moreCount <= 0)
			{
				if (loadHistory)
				{
					this->appendHistorySeparator();
					this->showMoreMsgTip();
				}
				else
					this->closeMoreMsgTip();
			}
			else
			{
				this->showMoreMsgTip();
			}
		}
		else
		{
			// load local history message
			int pageIndex = this->historyMsgPageIndex();
			this->setHistoryMsgPageIndex(pageIndex-1);
			loadHistoryMessages(kPageHistoryMessageCount);
		}
	}
}

void DiscussDialog::createInterphone()
{
	on_btnInterphone_clicked();
}

QString DiscussDialog::discussName() const
{
	return ui->messagePannel->name();
}

void DiscussDialog::closeEvent(QCloseEvent *e)
{
	// no first offline message, need to clear offline
	if (!qPmApp->getBuddyMgr()->hasFirstOffline(bean::Message_DiscussChat, ui->messagePannel->id()))
	{
		qPmApp->getBuddyMgr()->clearOffline(ui->messagePannel->type(), ui->messagePannel->id());
		qPmApp->getOfflineMsgManager()->clearOfflineItem(OfflineMsgManager::Discuss, ui->messagePannel->id());
	}

	QWidget::closeEvent(e);
}

void DiscussDialog::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		closeChat();
		return;
	}

	QWidget::keyPressEvent(event);
}

void DiscussDialog::mousePressEvent(QMouseEvent *e)
{
	if (ui->title->hasFocus())
	{
		changeDiscussName();
	}

	QWidget::mousePressEvent(e);
}

void DiscussDialog::appendSendMessage(const bean::MessageBody &rBody)
{
	ui->messagePannel->onMessage(rBody, false, false, false);
}

void DiscussDialog::onMessage(const bean::MessageBody &rBody, bool history /*= false*/, bool firstHistory /*= false*/)
{
	this->addMessageCount();

	if ((!history && !rBody.sync()) || (history && firstHistory))
	{
		this->addUnreadMessageCount();
	}

	ui->messagePannel->onMessage(rBody, history, firstHistory);
}

void DiscussDialog::clearMessages()
{
	clearMessageCount();
	ui->messagePannel->clearMessages();
}

void DiscussDialog::showAutoTip(const QString &tip)
{
	ui->messagePannel->showAutoTip(tip);
}

void DiscussDialog::addUnreadMessageCount(int addCount /*= 1*/)
{
	m_unreadMessageCount += addCount;
	int unreadMessageCount = this->unreadMessageCount();
	emit chatUnreadMessageCountChanged(unreadMessageCount);
}

int DiscussDialog::unreadMessageCount() const
{
	int unreadMessageCount = m_unreadMessageCount;
	unreadMessageCount += qPmApp->getOfflineMsgManager()->offlineMsgCount(OfflineMsgManager::Discuss, ui->messagePannel->id());
	return unreadMessageCount;
}

void DiscussDialog::clearUnreadMessageCount()
{
	m_unreadMessageCount = 0;
	if (qPmApp->getBuddyMgr()->isOfflineReceived())
		qPmApp->getOfflineMsgManager()->clearOfflineMsgCount(OfflineMsgManager::Discuss, ui->messagePannel->id());
}

void DiscussDialog::setMaximizeState(bool maximizeState)
{
	if (maximizeState)
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

bool DiscussDialog::isExpanded() const
{
	if (ui->sideStackedWidget->currentIndex() > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int DiscussDialog::unExpandedWidth() const
{
	return size().width() - ui->sideStackedWidget->width() + GuiConstants::WidgetSize::GroupChatMemberWidth;
}

void DiscussDialog::onUnionStateChanged()
{
	if (unionState() == ChatBaseDialog::Single)
	{
		ui->icon->setVisible(true);
	}
	else
	{
		ui->icon->setVisible(false);
	}
}

void DiscussDialog::showMoreMsgTip()
{
	ui->messagePannel->showMoreMsgTip();
}

void DiscussDialog::closeMoreMsgTip()
{
	ui->messagePannel->closeMoreMsgTip();
}

void DiscussDialog::onMoreMsgFinished()
{
	ui->messagePannel->onMoreMsgFinished();
}

void DiscussDialog::showMoreHistoryMsgTip()
{
	ui->messagePannel->showMoreHistoryMsgTip();
}

void DiscussDialog::appendHistorySeparator()
{
	ui->messagePannel->appendHistorySeparator();
}

void DiscussDialog::focusToEdit()
{
	ui->messagePannel->chatInput()->setFocus();
}

void DiscussDialog::appendMenuItem(QMenu *menu, const QString &uid)
{
	if (menu && !uid.isEmpty())
	{
		DiscussItem *discussItem = qPmApp->getModelManager()->discussModel()->getDiscuss(ui->messagePannel->id());
		if (discussItem)
		{
			QString creator = discussItem->creator();
			if (creator == Account::instance()->id() && creator != uid)
			{
				menu->addSeparator();
				m_kickMember->setData(uid);
				menu->addAction(m_kickMember);
			}
		}
	}
}

void DiscussDialog::onbtnHistoryMsgClicked(bool checked /*= true*/)
{
	if (checked)
	{
		if (ui->sideTabWidget->currentWidget() == ui->pageHistory)
			return;

		if (ui->sideTabWidget->indexOf(ui->pageHistory) == -1)
			ui->sideTabWidget->addTab(ui->pageHistory, tr("Chat History"));
		ui->sideTabWidget->setCurrentIndex(ui->sideTabWidget->indexOf(ui->pageHistory));
		setSideTabWidgetVisible(true);

		ui->historyTabWidget->setCurrentIndex(0);
		ui->tabLocalHistory->init(ui->messagePannel->id(), ui->messagePannel->type());
		connect(ui->tabLocalHistory->message4Js(), SIGNAL(showMessageTip(QString)), ui->messagePannel, 
			SLOT(showTip(QString)), Qt::UniqueConnection);
	}
	else
	{
		if (ui->sideTabWidget->indexOf(ui->pageHistory) != -1)
		{
			ui->sideTabWidget->removeTab(ui->sideTabWidget->indexOf(ui->pageHistory));
		}
		if (ui->sideTabWidget->count() <= 0)
		{
			setSideTabWidgetVisible(false);
		}
	}
	ui->messagePannel->historyButton()->setChecked(checked);
}

void DiscussDialog::onbtnHistoryMsgToggled(bool check)
{
	if (!check)
	{
		ui->tabLocalHistory->hideSearchBar();
	}
}

void DiscussDialog::onBtnGroupSettingClicked()
{
	QMenu *msgSettingMenu = &m_groupSettingMenu;

	QPoint pos;
	pos.setX(6);
	pos.setY(-msgSettingMenu->sizeHint().height()-1);

	msgSettingMenu->setGeometry(QRect(ui->messagePannel->msgSettingButton()->mapToGlobal(pos), msgSettingMenu->size()));
	msgSettingMenu->exec(ui->messagePannel->msgSettingButton()->mapToGlobal(pos));
}

void DiscussDialog::onMsgSettingsMenuAboutToShow()
{
	AccountSettings::GroupMsgSettingType setting = Account::settings()->discussMsgSetting(ui->messagePannel->id());
	if (setting == AccountSettings::Tip)
	{
		m_groupTip->setChecked(true);
		ui->messagePannel->msgSettingButton()->setToolTip(m_groupTip->text());
	}
	else
	{
		m_groupUntip->setChecked(true);
		ui->messagePannel->msgSettingButton()->setToolTip(m_groupUntip->text());
	}
}

void DiscussDialog::onBtnMoreMessageClicked()
{
	MessageManagerDlg *pDlg = MessageManagerDlg::instance();
	pDlg->init(ui->messagePannel->id(), bean::Message_DiscussChat);
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void DiscussDialog::onHistoryTabWidgetCurrentChanged(int index)
{
	if (index == 1) // roaming message
	{
		ui->tabRoamingMsg->init(ui->messagePannel->id(), ui->messagePannel->type());
	}
}

void DiscussDialog::groupSetting(QAction *action)
{
	if (!action)
		return;
	int silence = 0;
	AccountSettings::GroupMsgSettingType setting = AccountSettings::Tip;
	if (action == m_groupTip)
	{
		setting = AccountSettings::Tip;
	}
	else
	{
		setting = AccountSettings::UnTip;
		silence = 1;
	}

	if (Account::settings()->discussMsgSetting(ui->messagePannel->id()) != setting)
	{
		// set discuss message setting
		Account::settings()->setDiscussMsgSetting(ui->messagePannel->id(), setting);

		// set conf3
		QStringList silenceList = Account::settings()->silenceList();
		ConfigManager *configManager = qPmApp->getConfigManager();
		configManager->setConfig3(silenceList);

		// ios push
		IOSPushManager *pIOSPushManager = qPmApp->getIOSPushManager();
		pIOSPushManager->pushForIOS("discuss", ui->messagePannel->id(), silence);
	}
	
	ui->messagePannel->msgSettingButton()->setToolTip(action->text());
}

void DiscussDialog::InitUI(const QString &id)
{
	ModelManager *modelManager = qPmApp->getModelManager();

	QPixmap logo = modelManager->discussLogo(id);
	setWindowIcon(logo);
	ui->icon->setPixmap(logo);
	ui->icon->setVisible(false);
	ui->icon->setClickable(false);

	// set discuss info
	setDiscussInfo(id);
	connect(modelManager->discussModel(), SIGNAL(discussInfoChanged(QString)), this, SLOT(onDiscussInfoChanged(QString)));

	QString title = ui->title->wholeText().trimmed();
	ui->title->setMaxLength(GuiConstants::kMaxDiscussNameLength);
	selfAdaptiveTitleEditLength();
	connect(ui->title, SIGNAL(editingFinished()), this, SLOT(changeDiscussName()));
	connect(ui->title, SIGNAL(editingFinished()), this, SLOT(selfAdaptiveTitleEditLength()));
	connect(ui->title, SIGNAL(textEdited(QString)), this, SLOT(selfAdaptiveTitleEditLength()));

	// init message panel
	ui->messagePannel->init(bean::Message_DiscussChat, id, ui->title->text());

	// set discuss member
	m_kickMember = new QAction(tr("Remove from this discuss"), this);
	connect(m_kickMember, SIGNAL(triggered()), this, SLOT(kickMember()));
	ui->listView->setMenuDelegate(this);
	GroupItemListModel *pModel = modelManager->discussItemsModel(id);
	if (pModel)
	{
		ui->listView->setGroupMemberModel(pModel);
		connect(pModel, SIGNAL(memberChanged(QString, QStringList, QStringList)), this, SLOT(onMemberChanged()));
		connect(pModel, SIGNAL(memberInited(QString, QStringList)), this, SLOT(onMemberChanged()));
	}

	QString oldVersion = modelManager->discussModel()->discussVersion(id);
	GroupsMemberManager *groupsMemberManager = qPmApp->getGroupsMemberManager();
	QString newVersion = groupsMemberManager->discussNewVersion(id);
	if (oldVersion != newVersion)
	{
		groupsMemberManager->fetchDiscussMembers(id);
	}

	connect(ui->messagePannel, SIGNAL(doScreenshot()), this, SIGNAL(doScreenshot()));
	connect(ui->messagePannel, SIGNAL(sendMail(QString)), this, SIGNAL(sendMail(QString)));
	connect(ui->messagePannel, SIGNAL(chat(QString)), this, SIGNAL(chat(QString)));
	connect(ui->messagePannel, SIGNAL(multiMail()), this, SIGNAL(multiMail()));
	connect(ui->messagePannel, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->messagePannel, SIGNAL(doClearMessages()), this, SLOT(clearMessages()));
	connect(ui->messagePannel, SIGNAL(closeRequest()), this, SLOT(closeChat()));
	connect(ui->messagePannel, SIGNAL(fetchHistoryMsg()), this, SLOT(fetchMoreMessages()));
	connect(ui->messagePannel, SIGNAL(openHistoryMsg()), this, SLOT(onbtnHistoryMsgClicked()));

	ui->leditGroupFind->setVisible(false);
	ui->btnVideo->setEnabled(false);
	ui->btnPtt->setEnabled(false);
	ui->btnVideo->setVisible(false);
	ui->btnPtt->setVisible(false);

	// init msg setting
	ui->messagePannel->shakeButton()->setVisible(false);

	// init history message button
	connect(ui->messagePannel->historyButton(), SIGNAL(clicked(bool)), this, SLOT(onbtnHistoryMsgClicked(bool)));
	connect(ui->messagePannel->historyButton(), SIGNAL(toggled(bool)), this, SLOT(onbtnHistoryMsgToggled(bool)));

	// member clicked
	connect(ui->listView, SIGNAL(chat(QString)), qPmApp->getBuddyMgr(), SLOT(openChat(QString)));
	connect(ui->listView, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->listView, SIGNAL(sendMail(QString)), this, SIGNAL(sendMail(QString)));
	connect(ui->listView, SIGNAL(addFriendRequest(QString, QString)), this, SIGNAL(addFriendRequest(QString, QString)));
	connect(ui->listView, SIGNAL(atTA(QString)), ui->messagePannel->chatInput(), SLOT(addAtText(QString)));
	connect(ui->listView, SIGNAL(changeCardName(QString, QString)), this, SLOT(changeCardName(QString, QString)));

	// show group member count
	setMemberCount();

	// detail changed
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onUserChanged(QString)));

	// init side tab widget
	ui->sideTabWidget->setTabsClosable(true);
	connect(ui->sideTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotSideTabCloseRequest(int)));
	setSideTabWidgetVisible(false);

	connect(ui->historyTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onHistoryTabWidgetCurrentChanged(int)));

	// init discuss setting actions
	initMsgSettingActions();

	initAtCompleter();

	// configure roaming message
	if (ui->historyTabWidget->count() > 1)
	{
		if (GlobalSettings::roamingMsgDisabled())
		{
			ui->historyTabWidget->removeTab(1);
		}
	}

	// config interphone function
	if (GlobalSettings::interphoneDisabled())
		ui->btnInterphone->setVisible(false);
	else
		ui->btnInterphone->setVisible(true);

	qDebug() << Q_FUNC_INFO << ui->messagePannel->id();
}

void DiscussDialog::setDiscussInfo(const QString &id)
{
	// get discuss title
	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->discussName(id);
	if (name.trimmed().isEmpty())
		name = id;
	QString title = name;

	ui->title->setText(title);
	selfAdaptiveTitleEditLength();
	setWindowTitle(title);
	ui->messagePannel->setName(title);

	// create info
	setDiscussCreateInfo(id);
}

void DiscussDialog::setDiscussCreateInfo(const QString &id)
{
	// show creator and create time
	ModelManager *modelManager = qPmApp->getModelManager();
	QString creator;
	QString createTime;
	DiscussItem *discussItem = modelManager->discussModel()->getDiscuss(id);
	if (discussItem)
	{
		creator = discussItem->creator();
		creator = modelManager->memberNameInDiscuss(id, creator);
		createTime = discussItem->time();
		if (!createTime.isEmpty())
		{
			createTime = CDateTime::localDateTimeStringFromUtcString(createTime);
		}
		int dateIndex = createTime.indexOf(" ");
		if (dateIndex != -1)
		{
			createTime = createTime.left(dateIndex);
		}
		if (!createTime.isEmpty())
		{
			ui->labelCreateTime->setVisible(true);
			ui->labelTime->setVisible(true);
		}
		else
		{
			ui->labelCreateTime->setVisible(false);
			ui->labelTime->setVisible(false);
		}
	}
	ui->labelCreator->setText(creator);
	ui->labelTime->setText(createTime);
}

void DiscussDialog::setDiscussLogo(const QPixmap &logo)
{
	setWindowIcon(QIcon(logo));
	ui->icon->setPixmap(logo);
	emit chatIconChanged(logo);
}

void DiscussDialog::initMsgSettingActions()
{
	m_groupActionGroup = new QActionGroup(this);
	m_groupTip = new QAction(tr("Tip normal"), m_groupActionGroup);
	m_groupTip->setCheckable(true);
	m_groupUntip = new QAction(tr("Not tip"), m_groupActionGroup);
	m_groupUntip->setCheckable(true);
	m_groupActionGroup->addAction(m_groupTip);
	m_groupActionGroup->addAction(m_groupUntip);
	m_groupActionGroup->setExclusive(true);
	m_groupTip->setChecked(true);
	connect(m_groupActionGroup, SIGNAL(triggered(QAction *)), SLOT(groupSetting(QAction *)));

	QMenu *msgSettingMenu = &m_groupSettingMenu;
	msgSettingMenu->addAction(m_groupTip);
	msgSettingMenu->addAction(m_groupUntip);

	QToolButton *msgSettingsButton = ui->messagePannel->msgSettingButton();
	AccountSettings::GroupMsgSettingType setting = Account::settings()->discussMsgSetting(ui->messagePannel->id());
	if (setting == AccountSettings::Tip)
	{
		m_groupTip->setChecked(true);
		msgSettingsButton->setToolTip(m_groupTip->text());
	}
	else
	{
		m_groupUntip->setChecked(true);
		msgSettingsButton->setToolTip(m_groupUntip->text());
	}
	
	// init 
	msgSettingsButton->setMenu(msgSettingMenu);
	connect(msgSettingsButton, SIGNAL(clicked()), this, SLOT(onBtnGroupSettingClicked()));
	connect(msgSettingMenu, SIGNAL(aboutToShow()), this, SLOT(onMsgSettingsMenuAboutToShow()));
}

void DiscussDialog::initAtCompleter()
{
	QStringList atItems;
	GroupItemListModel *groupMemberModel = ui->listView->groupMemberModel();
	if (groupMemberModel)
	{
		QStringList memberIds = groupMemberModel->allMemberIds();
		foreach (QString memberId, memberIds)
		{
			if (memberId != Account::instance()->id())
			{
				QString memberName = groupMemberModel->memberNameInGroup(memberId);
				QString item = QString("%1(%2)").arg(memberName).arg(memberId);
				atItems.append(item);
			}
		}
	}
	ui->messagePannel->chatInput()->setCompleteStrings(atItems);
}

void DiscussDialog::setMemberCount()
{
	int memberCount = 0;
	GroupItemListModel *groupMemberModel = ui->listView->groupMemberModel();
	if (groupMemberModel)
	{
		memberCount = groupMemberModel->memberCount();
	}
	QString sCount = tr("(%1)").arg(memberCount);
	ui->labCount->setText(sCount);
}

void DiscussDialog::setSideTabWidgetVisible(bool visible, bool removeAllTabs /*= true*/)
{
	if (visible)
	{
		// deal with window size
		int miniWidth = 0;
		if (ui->sideTabWidget->indexOf(ui->pageHistory) != -1)
		{
			miniWidth = GuiConstants::WidgetSize::ChatHistoryWidth;
		}
		else
		{
			miniWidth = GuiConstants::WidgetSize::GroupChatMemberWidth;
		}

		QSize size = this->size();
		if (ui->sideStackedWidget->isVisible())
		{
			if (unionChatDelegate() && size.width() < (GuiConstants::WidgetSize::GroupChatMainWidth + miniWidth))
			{
				size.setWidth(size.width() + miniWidth - ui->sideStackedWidget->minimumWidth());
				unionChatDelegate()->chatWidthChanged(size.width());
			}
		}

		ui->sideStackedWidget->setCurrentIndex(1);
		ui->sideStackedWidget->setMinimumWidth(miniWidth);
		ui->sideStackedWidget->setFixedWidth(miniWidth);
		setMinimumWidth(GuiConstants::WidgetSize::GroupChatMainWidth + ui->sideStackedWidget->minimumWidth());
		if (unionChatDelegate())
		{
			unionChatDelegate()->chatMinimumWidthChanged(GuiConstants::WidgetSize::GroupChatMainWidth + ui->sideStackedWidget->minimumWidth());
		}

		// set history message button state
		if (ui->sideTabWidget->indexOf(ui->pageHistory) != -1)
		{
			ui->messagePannel->historyButton()->setChecked(true);
		}
		else
		{
			ui->messagePannel->historyButton()->setChecked(false);
		}
	}
	else
	{
		// deal with window size
		if (removeAllTabs)
		{
			while (ui->sideTabWidget->count() > 0)
				ui->sideTabWidget->removeTab(0);
		}

		QSize size = this->size();
		int miniWidth = ui->sideStackedWidget->minimumWidth();

		ui->sideStackedWidget->setCurrentIndex(0);
		ui->sideStackedWidget->setMinimumWidth(GuiConstants::WidgetSize::GroupChatMemberWidth);
		ui->sideStackedWidget->setFixedWidth(GuiConstants::WidgetSize::GroupChatMemberWidth);
		setMinimumWidth(GuiConstants::WidgetSize::GroupChatMainWidth + GuiConstants::WidgetSize::GroupChatMemberWidth);
		if (unionChatDelegate())
		{
			unionChatDelegate()->chatMinimumWidthChanged(GuiConstants::WidgetSize::GroupChatMainWidth + GuiConstants::WidgetSize::GroupChatMemberWidth);
		}

		if (ui->sideStackedWidget->isVisible())
		{
			if (unionChatDelegate() && !unionChatDelegate()->isMaximumState())
			{
				size.setWidth(size.width() - miniWidth + GuiConstants::WidgetSize::GroupChatMemberWidth);
				unionChatDelegate()->chatWidthChanged(size.width());
			}
		}

		// set history message button state
		if (ui->sideTabWidget->indexOf(ui->pageHistory) != -1)
		{
			ui->messagePannel->historyButton()->setChecked(true);
		}
		else
		{
			ui->messagePannel->historyButton()->setChecked(false);
		}
	}
}

void DiscussDialog::slotSideTabCloseRequest(int index)
{
	if (index < 0 || index >= ui->sideTabWidget->count())
		return;

	QWidget *closeWidget = ui->sideTabWidget->widget(index);
	if (!closeWidget)
		return;

	if (closeWidget == ui->pageHistory)
	{
		onbtnHistoryMsgClicked(false);
	}
}

void DiscussDialog::onMaximizeStateChanged(bool isMaximized)
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

void DiscussDialog::on_btnAddMembers_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't invite member of discuss"));
		return;
	}

	ModelManager* pMM = qPmApp->getModelManager();
	QString name = pMM->discussName(ui->messagePannel->id());

	QStringList uids;
	GroupItemListModel *pModel = pMM->discussItemsModel(ui->messagePannel->id());
	if (pModel)
	{
		uids = pModel->allMemberIds();
	}

	CreateDiscussDialog *pDlg = new CreateDiscussDialog(CreateDiscussDialog::Type_Add, name, uids, uids, false, this);
	pDlg->setDiscussId(ui->messagePannel->id());
	connect(pDlg, SIGNAL(addMembers(QString, QStringList)), this, SIGNAL(addDiscussMembers(QString, QStringList)));
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void DiscussDialog::on_btnQuitDiscuss_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't quit from discuss"));
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	DiscussItem *discussItem = modelManager->discussModel()->getDiscuss(ui->messagePannel->id());
	if (!discussItem)
		return;

	QString creator = discussItem->creator();
	QDialogButtonBox::StandardButton sb = QDialogButtonBox::No;
	if (creator != Account::instance()->id())
	{
		sb = PMessageBox::question(this, tr("Quit Discuss"), 
			tr("Are you sure to quit from this discuss"), 
			QDialogButtonBox::Yes|QDialogButtonBox::No);
	}
	else
	{
		sb = PMessageBox::question(this, tr("Quit Discuss"), 
			tr("You are creator of this discuss, quit will make this discuss dissolved, continue"), 
			QDialogButtonBox::Yes|QDialogButtonBox::No);
	}

	if (QDialogButtonBox::Yes == sb)
	{
		emit quitDiscuss(ui->messagePannel->id(), true);

		closeChat();
	}
}

void DiscussDialog::onMemberChanged()
{
	// update logo
	ModelManager *modelManage = qPmApp->getModelManager();
	QPixmap logo = modelManage->discussLogo(ui->messagePannel->id());
	setDiscussLogo(logo);

	// update member count
	setMemberCount();

	// update input at completer
	initAtCompleter();

	// create info
	setDiscussCreateInfo(ui->messagePannel->id());

	// update chat name
	ui->messagePannel->updateChatName();
}

void DiscussDialog::on_labelCreator_clicked()
{
	DiscussItem *discussItem = qPmApp->getModelManager()->discussModel()->getDiscuss(ui->messagePannel->id());
	if (discussItem)
	{
		QString creator = discussItem->creator();
		if (!creator.isEmpty())
		{
			emit viewMaterial(creator);
		}
	}
}

void DiscussDialog::onDiscussInfoChanged(const QString &discussId)
{
	if (discussId == ui->messagePannel->id())
	{
		setDiscussInfo(discussId);

		// notify change
		emit chatNameChanged(ui->title->text());
	}
}

void DiscussDialog::on_btnInterphone_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		ui->messagePannel->showAutoTip(tr("You are offline, can't start interphone, please try when online"));
		return;
	}

	ui->messagePannel->addInterphone();
}

void DiscussDialog::onLoadHistoryMessagesFinished(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs)
{
	if (m_fetchHistoryMsgId == seq)
	{
		int origMsgCount = this->allMessageCount();

		if (!msgs.isEmpty() && curPage == maxPage && origMsgCount <= 0)
		{
			ui->messagePannel->appendHistorySeparator();
		}

		for (int i = msgs.count()-1; i >= 0; --i)
		{
			bean::MessageBody msg = msgs[i];
			ui->messagePannel->onMessage(msg, true, false, false);
			this->addMessageCount();
		}

		this->setHistoryMsgPageIndex(curPage);

		this->onMoreMsgFinished();

		if (this->canFetchMore())
		{
			AccountSettings *accountSettings = Account::settings();
			bool loadHistory = accountSettings->chatLoadHistory();
			if (loadHistory)
				this->showMoreMsgTip();
			else
				this->closeMoreMsgTip();
		}
		else
		{
			if (curPage <= 1)
				this->closeMoreMsgTip();
			else
				this->showMoreHistoryMsgTip();
		}

		if (origMsgCount <= 0 && curPage == maxPage)
		{
			ui->messagePannel->scrollMessageToBottom();
		}

		m_fetchHistoryMsgId = -1;
	}
}

void DiscussDialog::kickMember()
{
	QString uid = m_kickMember->data().toString();
	if (uid.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't remove member"));
		return;
	}

	QString memberName = qPmApp->getModelManager()->memberNameInDiscuss(ui->messagePannel->id(), uid);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Remove Member"),
		tr("Do you want to remove %1 from this discuss").arg(memberName),
		QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret == QDialogButtonBox::Yes)
	{
		qPmApp->getDiscussManager()->kick(ui->messagePannel->id(), uid, Account::instance()->id());
	}
}

void DiscussDialog::onUserChanged(const QString &uid)
{
	// update chat avatar
	GroupItemListModel *groupMemberModel = ui->listView->groupMemberModel();
	if (groupMemberModel)
	{
		QStringList memberIds = groupMemberModel->allMemberIds();
		if (memberIds.contains(uid))
			ui->messagePannel->updateChatAvatar(uid);
	}

	// update logo
	ModelManager *modelManage = qPmApp->getModelManager();
	DiscussModel *discussModel = modelManage->discussModel();
	QString discussId = ui->messagePannel->id();
	if (discussModel->discussLogoIds(discussId).contains(uid))
	{
		QPixmap logo = modelManage->discussLogo(discussId);
		setDiscussLogo(logo);
	}
}

void DiscussDialog::changeCardName(const QString &uid, const QString &cardName)
{
	DiscussManager *discussManager = qPmApp->getDiscussManager();
	QString discussName = qPmApp->getModelManager()->discussName(ui->messagePannel->id());
	discussManager->changeCardName(ui->messagePannel->id(), discussName, uid, cardName);
}

void DiscussDialog::changeDiscussName()
{
	QString id = this->id();
	ModelManager *modelManager = qPmApp->getModelManager();

	QString oldName = modelManager->discussName(id);
	QString newName = ui->title->wholeText().trimmed();
	
	if (newName.isEmpty())
	{
		ui->title->setText(oldName);
		ui->title->clearFocus();
		return;
	}

	if (newName == oldName)
	{
		ui->title->clearFocus();
		return;
	}

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		ui->title->setText(oldName);
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't modify name of discuss"));
		return;
	}

	if (!modelManager->hasDiscussItem(id))
		return;

	DiscussManager *dm = qPmApp->getDiscussManager();
	dm->changeName(id, newName);
	disconnect(ui->title, SIGNAL(editingFinished()), this, SLOT(changeDiscussName()));
	ui->title->clearFocus();
	connect(ui->title, SIGNAL(editingFinished()), this, SLOT(changeDiscussName()));
}

void DiscussDialog::selfAdaptiveTitleEditLength()
{
	QString title = ui->title->wholeText().trimmed();
	if (title.isEmpty())
	{
		return;
	}

	QFont font = qApp->font();
	font.setPointSize(13);

	QFontMetrics fm(font);
	int width = fm.width(title);
	width += 10;

	if (width <= s_minTitleEditLength)
	{
		ui->title->setFixedWidth(s_minTitleEditLength);
	}
	else if (width <= s_maxTitleEditLength)
	{
		ui->title->setFixedWidth(width);
	}
	else
	{
		ui->title->setFixedWidth(s_maxTitleEditLength);
	}
}