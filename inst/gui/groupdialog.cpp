#include <QtGlobal>
#include <QtCore>
#include <QtGui>
#include <QDesktopWidget>

#include "sortfilterproxymodel.h"

#include "model/ModelManager.h"
#include "model/groupitemlistmodeldef.h"
#include "model/groupmodeldef.h"
#include "model/groupitemdef.h"

#include "groupdialog.h"
#include "ui_groupdialog.h"

#include "common/datetime.h"

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
#include "util/FileDialog.h"
#include "chatinputbox.h"
#include "settings/GlobalSettings.h"
#include "util/AmrPlayUtil.h"
#include "chatinputbox.h"
#include "clickablelabel.h"
#include "message4js.h"
#include "MessageDBStore.h"
#include "util/MsgEncryptionUtil.h"
#include <QWebFrame>
#include "groupmanager.h"
#include "groupsmembermanager.h"
#include "configmanager.h"
#include "iospushmanager.h"

const static unsigned int s_unMaxFileSize = -1;

CGroupDialog::CGroupDialog(const QString& rsGid, QWidget *parent /* = 0 */)
	: ChatBaseDialog(parent)
	, m_groupSettingMenu(this)
	, m_fetchHistoryMsgId(-1)
{
	ui = new Ui::CGroupDialog();
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

	InitUI(rsGid);

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

CGroupDialog::~CGroupDialog()
{
	qDebug() << Q_FUNC_INFO << ui->messagePannel->id();

	delete ui;
}

QString CGroupDialog::groupName() const
{
	return ui->messagePannel->name();
}

void CGroupDialog::setSkin()
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

	// set button interphone style
	info.urlNormal = QString(":/interphone/create_normal.png");
	info.urlHover = QString(":/interphone/create_hover.png");
	info.urlPressed = QString(":/interphone/create_hover.png");
	info.urlDisabled = QString(":/interphone/create_disabled.png");
	info.tooltip = tr("Interphone");
	ui->btnInterphone->setInfo(info);
	
	ui->messagePannel->setSkin();
}

void CGroupDialog::slot_screenshot_ok(const QString &imagePath)
{
	ui->messagePannel->slot_screenshot_ok(imagePath);
}

void CGroupDialog::slot_screenshot_cancel()
{
	ui->messagePannel->slot_screenshot_cancel();
}

void CGroupDialog::insertMimeData(const QMimeData *source)
{
	ui->messagePannel->chatInput()->insertMimeData(source);
}

void CGroupDialog::loadHistoryMessages(int count)
{
	Q_UNUSED(count);

	connect(qPmApp->getMessageDBStore(), SIGNAL(gotMessages(qint64, int, int, bean::MessageBodyList)), 
		this, SLOT(onLoadHistoryMessagesFinished(qint64, int, int, bean::MessageBodyList)), Qt::UniqueConnection);
	int pageIndex = this->historyMsgPageIndex();
	QString endMsgTime = this->historyMsgEndTime();
	m_fetchHistoryMsgId = qPmApp->getMessageDBStore()->getMessagesBeforeTime(
		ui->messagePannel->type(), ui->messagePannel->id(), pageIndex, kPageHistoryMessageCount, endMsgTime);
}

void CGroupDialog::fetchHistoryMessages()
{
	bean::MessageType msgType = ui->messagePannel->type();
	QString id = ui->messagePannel->id();
	qPmApp->getBuddyMgr()->getHistoryMsg(msgType, id);
}

void CGroupDialog::fetchMoreMessages()
{
	QString id = ui->messagePannel->id();
	OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
	if (offlineMsgManager->containOfflineItem(OfflineMsgManager::Group, id)) // get offline history message
	{
		if (!qPmApp->GetLoginMgr()->isLogined()) // not logined
		{
			qWarning() << Q_FUNC_INFO << "was not logined, group: " << id;

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

void CGroupDialog::closeEvent(QCloseEvent *e)
{
	// no first offline message, need to clear offline
	if (!qPmApp->getBuddyMgr()->hasFirstOffline(bean::Message_GroupChat, ui->messagePannel->id()))
	{
		qPmApp->getBuddyMgr()->clearOffline(ui->messagePannel->type(), ui->messagePannel->id());
		qPmApp->getOfflineMsgManager()->clearOfflineItem(OfflineMsgManager::Group, ui->messagePannel->id());
	}

	QWidget::closeEvent(e);
}

void CGroupDialog::keyPressEvent (QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		closeChat();
		return;
	}

	QWidget::keyPressEvent(event);
}

void CGroupDialog::appendSendMessage(const bean::MessageBody &rBody)
{
	ui->messagePannel->onMessage(rBody, false, false, false);
}

void CGroupDialog::onMessage(const bean::MessageBody &rBody, bool history /*= false*/, bool firstHistory /*= false*/)
{
	this->addMessageCount();

	if ((!history && !rBody.sync()) || (history && firstHistory))
	{
		this->addUnreadMessageCount();
	}

	ui->messagePannel->onMessage(rBody, history, firstHistory);
}

void CGroupDialog::clearMessages()
{
	clearMessageCount();
	ui->messagePannel->clearMessages();
}

void CGroupDialog::showAutoTip(const QString &tip)
{
	ui->messagePannel->showAutoTip(tip);
}

void CGroupDialog::addUnreadMessageCount(int addCount /*= 1*/)
{
	m_unreadMessageCount += addCount;
	int unreadMessageCount = this->unreadMessageCount();
	emit chatUnreadMessageCountChanged(unreadMessageCount);
}

int CGroupDialog::unreadMessageCount() const
{
	int unreadMessageCount = m_unreadMessageCount;
	unreadMessageCount += qPmApp->getOfflineMsgManager()->offlineMsgCount(OfflineMsgManager::Group, ui->messagePannel->id());
	return unreadMessageCount;
}

void CGroupDialog::clearUnreadMessageCount()
{
	m_unreadMessageCount = 0;
	if (qPmApp->getBuddyMgr()->isOfflineReceived())
		qPmApp->getOfflineMsgManager()->clearOfflineMsgCount(OfflineMsgManager::Group, ui->messagePannel->id());
}

void CGroupDialog::setMaximizeState(bool maximizeState)
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

bool CGroupDialog::isExpanded() const
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

int CGroupDialog::unExpandedWidth() const
{
	return size().width() - ui->sideStackedWidget->width() + GuiConstants::WidgetSize::GroupChatMemberWidth;
}

void CGroupDialog::onUnionStateChanged()
{
	if (unionState()== ChatBaseDialog::Single)
	{
		ui->icon->setVisible(true);
	}
	else
	{
		ui->icon->setVisible(false);
	}
}

void CGroupDialog::showMoreMsgTip()
{
	ui->messagePannel->showMoreMsgTip();
}

void CGroupDialog::closeMoreMsgTip()
{
	ui->messagePannel->closeMoreMsgTip();
}

void CGroupDialog::onMoreMsgFinished()
{
	ui->messagePannel->onMoreMsgFinished();
}

void CGroupDialog::showMoreHistoryMsgTip()
{
	ui->messagePannel->showMoreHistoryMsgTip();
}

void CGroupDialog::appendHistorySeparator()
{
	ui->messagePannel->appendHistorySeparator();
}

void CGroupDialog::focusToEdit()
{
	ui->messagePannel->chatInput()->setFocus();
}

void CGroupDialog::onbtnHistoryMsgClicked(bool checked /*= true*/)
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

void CGroupDialog::onbtnHistoryMsgToggled(bool checked)
{
	if (!checked)
	{
		ui->tabLocalHistory->hideSearchBar();
	}
}

void CGroupDialog::onBtnGroupSettingClicked()
{
	QMenu *msgSettingMenu = &m_groupSettingMenu;

	QPoint pos;
	pos.setX(6);
	pos.setY(-msgSettingMenu->sizeHint().height()-1);

	msgSettingMenu->setGeometry(QRect(ui->messagePannel->msgSettingButton()->mapToGlobal(pos), msgSettingMenu->size()));
	msgSettingMenu->exec(ui->messagePannel->msgSettingButton()->mapToGlobal(pos));
}

void CGroupDialog::onMsgSettingsMenuAboutToShow()
{
	AccountSettings::GroupMsgSettingType setting = Account::settings()->groupMsgSetting(ui->messagePannel->id());
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

void CGroupDialog::onBtnMoreMessageClicked()
{
	MessageManagerDlg *pDlg = MessageManagerDlg::instance();
	pDlg->init(ui->messagePannel->id(), bean::Message_GroupChat);
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void CGroupDialog::onHistoryTabWidgetCurrentChanged(int index)
{
	if (index == 1) // roaming message
	{
		ui->tabRoamingMsg->init(ui->messagePannel->id(), ui->messagePannel->type());
	}
}

void CGroupDialog::groupSetting(QAction *action)
{
	if (!action)
		return;
	int silence = 0;
	AccountSettings::GroupMsgSettingType setting = AccountSettings::Tip;
	if (action == m_groupUntip)
	{
		setting = AccountSettings::UnTip;
		silence = 1;
	}
	else
	{
		setting = AccountSettings::Tip;
	}

	if (setting != Account::settings()->groupMsgSetting(ui->messagePannel->id()))
	{
		// set group message setting
		Account::settings()->setGroupMsgSetting(ui->messagePannel->id(), setting);

		// set conf3
		QStringList silenceList = Account::settings()->silenceList();
		ConfigManager *configManager = qPmApp->getConfigManager();
		configManager->setConfig3(silenceList);

		// ios push
		IOSPushManager *pIOSPushManager = qPmApp->getIOSPushManager();
		pIOSPushManager->pushForIOS("group", ui->messagePannel->id(), silence);
	}

	ui->messagePannel->msgSettingButton()->setToolTip(action->text());
}

void CGroupDialog::on_btnInterphone_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		ui->messagePannel->showAutoTip(tr("You are offline, can't start interphone, please try when online"));
		return;
	}

	ui->messagePannel->addInterphone();
}

void CGroupDialog::onLoadHistoryMessagesFinished(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs)
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

void CGroupDialog::onUserChanged(const QString &uid)
{
	GroupItemListModel *groupMemberModel = ui->listView->groupMemberModel();
	if (groupMemberModel)
	{
		QStringList memberIds = groupMemberModel->allMemberIds();
		if (memberIds.contains(uid))
			ui->messagePannel->updateChatAvatar(uid);
	}
}

void CGroupDialog::onMemberChanged()
{
	// update at completer
	initAtCompleter();

	// update chat name
	ui->messagePannel->updateChatName();

	// update member count
	setMemberCount();
}

void CGroupDialog::changeCardName(const QString &uid, const QString &cardName)
{
	GroupManager *groupManager = qPmApp->getGroupManager();
	groupManager->changeCardName(ui->messagePannel->id(), uid, cardName);
}

void CGroupDialog::InitUI( const QString &id )
{
	ModelManager* pMM = qPmApp->getModelManager();

	// set window title
	QString name = "";
	QString desc = "";

	name = pMM->groupName(id);
	desc = pMM->groupDescription(id);

	GroupItemListModel *pModel = pMM->groupItemsModel(id);
	if (pModel)
	{
		ui->listView->setGroupMemberModel(pModel);
		connect(pModel, SIGNAL(memberChanged(QString, QStringList, QStringList)), this, SLOT(onMemberChanged()));
		connect(pModel, SIGNAL(memberInited(QString, QStringList)), this, SLOT(onMemberChanged()));
	}

	GroupModel *groupModel = pMM->groupModel();
	QString oldVersion = groupModel->groupVersion(id);
	GroupsMemberManager *groupsMemberManager = qPmApp->getGroupsMemberManager();
	QString newVersion = groupsMemberManager->groupNewVersion(id);
	if (oldVersion != newVersion)
	{
		groupsMemberManager->fetchGroupMembers(id);
	}

	if (name.trimmed().isEmpty())
		name = id;
	QString title = name;

	ui->title->setText(title);
	ui->labDesc->setText(desc);

	setWindowTitle(title);

	// set window icon
	setWindowIcon(QIcon(pMM->getGroupLogo(id)));
	ui->icon->setVisible(false);
	ui->icon->setPixmap(pMM->getGroupLogo(id));
	ui->icon->setClickable(false);

	// init message panel
	ui->messagePannel->init(bean::Message_GroupChat, id, name);

	connect(ui->messagePannel, SIGNAL(doScreenshot()), this, SIGNAL(doScreenshot()));
	connect(ui->messagePannel, SIGNAL(sendMail(QString)), this, SIGNAL(sendMail(QString)));
	connect(ui->messagePannel, SIGNAL(chat(QString)), this, SIGNAL(chat(QString)));
	connect(ui->messagePannel, SIGNAL(multiMail()), this, SIGNAL(multiMail()));
	connect(ui->messagePannel, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->messagePannel, SIGNAL(doClearMessages()), this, SLOT(clearMessages()));
	connect(ui->messagePannel, SIGNAL(closeRequest()), this, SLOT(closeChat()));
	connect(ui->messagePannel, SIGNAL(fetchHistoryMsg()), this, SLOT(fetchMoreMessages()));
	connect(ui->messagePannel, SIGNAL(openHistoryMsg()), this, SLOT(onbtnHistoryMsgClicked()));

	// init msg setting
	ui->messagePannel->shakeButton()->setVisible(false);

	ui->leditGroupFind->setVisible(false);
	ui->btnVideo->setEnabled(false);
	ui->btnPtt->setEnabled(false);
	ui->btnVideo->setVisible(false);
	ui->btnPtt->setVisible(false);

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
	connect(pMM, SIGNAL(detailChanged(QString)), this, SLOT(onUserChanged(QString)));

	// init side tab widget
	ui->sideTabWidget->setTabsClosable(true);
	connect(ui->sideTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotSideTabCloseRequest(int)));
	setSideTabWidgetVisible(false);

	connect(ui->historyTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onHistoryTabWidgetCurrentChanged(int)));

	// init group setting actions
	initMsgSettingActions();

	// announcement
	MucGroupItem *groupItem = groupModel->getGroup(id);
	if (groupItem)
	{
		QString annt = groupItem->itemAnnt();
		ui->leditAnnt->clear();
		if (!annt.isEmpty())
			ui->leditAnnt->insertPlainText(annt);
		else
			ui->leditAnnt->insertPlainText(tr("No Announcement"));
		ui->leditAnnt->moveCursor(QTextCursor::Start);
	}

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

void CGroupDialog::initMsgSettingActions()
{
	m_groupActionGroup = new QActionGroup(this);
	m_groupTip = new QAction(tr("Tip normal"), m_groupActionGroup);
	m_groupTip->setCheckable(true);
	m_groupUntip = new QAction(tr("No tip"), m_groupActionGroup);
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
	AccountSettings::GroupMsgSettingType setting = Account::settings()->groupMsgSetting(ui->messagePannel->id());
	QList<QAction *> actions = m_groupActionGroup->actions();
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

void CGroupDialog::initAtCompleter()
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

void CGroupDialog::setMemberCount()
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

void CGroupDialog::setSideTabWidgetVisible(bool visible, bool removeAllTabs /*= true*/)
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

void CGroupDialog::slotSideTabCloseRequest(int index)
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

void CGroupDialog::onMaximizeStateChanged(bool isMaximized)
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