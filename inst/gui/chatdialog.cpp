#include <QTime>
#include <QFileInfo>
#include <QDebug>
#include <QCloseEvent>
#include <QShortcut>
#include <QResizeEvent>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QApplication>

#include "chatdialog.h"
#include "ui_chatdialog.h"

#include "buddymgr.h"

#include "PmApp.h"

#include "bean/DetailItem.h"
#include "model/ModelManager.h"

#include "util/PlayBeep.h"

#include "manager/presencemanager.h"

#include "guiconstants.h"

#include "pmessagebox.h"
#include "widgetmanager.h"
#include "messagemanagerdlg.h"

#include "model/rostermodeldef.h"
#include "Account.h"

#include "settings/GlobalSettings.h"

#include "sessionvideomanager.h"
#include "chatinputbox.h"
#include "clickablelabel.h"
#include "message4js.h"

#include "manager/tipmanager.h"

#include "interphonemanager.h"
#include "interphonedialog.h"
#include "rtc/rtcsessionmanager.h"
#include "MessageDBStore.h"
#include "util/MsgEncryptionUtil.h"
#include "sessionvideodialog.h"
#include <QWebFrame>

const static unsigned int s_unMaxFileSize = -1;

CChatDialog::CChatDialog(const QString &rsUid, QWidget *parent)
	: ChatBaseDialog(parent)
	, ui(new Ui::CChatDialog)
	, m_fetchHistoryMsgId(-1)
{
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

	InitUI(rsUid);

	//resize(GuiConstants::WidgetSize::ChatMainWidth, GuiConstants::WidgetSize::ChatHeight);
	// setMinimumSize(GuiConstants::WidgetSize::ChatMainWidth, GuiConstants::WidgetSize::ChatHeight);
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

CChatDialog::~CChatDialog()
{
	qDebug() << Q_FUNC_INFO << ui->messagePannel->id();

	delete ui;
}

void CChatDialog::setSkin()
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

	// set button mail style
	info.urlNormal = QString(":/images/Icon_109.png");
	info.urlHover = QString(":/images/Icon_109_hover.png");
	info.urlPressed = QString(":/images/Icon_109_hover.png");
	info.urlDisabled = QString(":/images/Icon_109_disabled.png");
	info.tooltip = tr("Send Mail");
	ui->btnMail->setInfo(info);

	// set button add friend style
	info.urlNormal = QString(":/images/Icon_110.png");
	info.urlHover = QString(":/images/Icon_110_hover.png");
	info.urlPressed = QString(":/images/Icon_110_hover.png");
	info.urlDisabled = QString(":/images/Icon_110_disabled.png");
	info.tooltip = tr("Add Friends");
	ui->btnAddFriend->setInfo(info);

	// set button interphone style
	info.urlNormal = QString(":/interphone/create_normal.png");
	info.urlHover = QString(":/interphone/create_hover.png");
	info.urlPressed = QString(":/interphone/create_hover.png");
	info.urlDisabled = QString(":/interphone/create_disabled.png");
	info.tooltip = tr("Interphone");
	ui->btnInterphone->setInfo(info);

	// set button btnInviteToNewDiscuss style
	info.urlNormal = QString(":/images/Icon_113.png");
	info.urlHover = QString(":/images/Icon_113_hover.png");
	info.urlPressed = QString(":/images/Icon_113_hover.png");
	info.urlDisabled = QString(":/images/Icon_113_disabled.png");
	info.tooltip = tr("Create New Discuss Group");
	ui->btnInviteToNewDiscuss->setInfo(info);

	ui->onlineLabel->setStyleSheet("color: rgb(128, 128, 128);");

    ui->pageSession->setSkin();

	ui->messagePannel->setSkin();

	ui->stackInfoWidget->setSkin();
}

void CChatDialog::slot_screenshot_ok( const QString &imagePath )
{
	ui->messagePannel->slot_screenshot_ok(imagePath);
}

void CChatDialog::slot_screenshot_cancel()
{
	ui->messagePannel->slot_screenshot_cancel();
}

void CChatDialog::onBlackListChanged()
{
	QString uid = ui->messagePannel->id();
	bool isBlack = qPmApp->getModelManager()->isInBlackList(uid);
	if (isBlack)
	{
		// if has session, close session
		rtcsession::Session *s = qPmApp->getRtcSessionManager()->sessionWithUid(uid);
		if (s)
		{
			s->close(rtcsession::Session::CloseOther, tr("You are in black list"));
		}

		// if is in interphone, close interphone
		QString interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_Chat, uid);
		if (InterphoneDialog::hasInterphoneDialog())
		{
			InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
			if (dlg->interphoneId() == interphoneId) 
			{
				ui->messagePannel->quitInterphone();
			}
		}

		// ui
		ui->blackLabel->setVisible(true);
	}
	else
	{
		ui->blackLabel->setVisible(false);
	}
}

void CChatDialog::closeEvent(QCloseEvent *e)
{
	if (ui->pageSession->isVisible() || ui->sideTabWidget->indexOf(ui->pageSession) >= 0)
    {
        ui->pageSession->sessionClose();
        hideSessionWidget();
    }

	// no first offline message, need to clear offline
	if (!qPmApp->getBuddyMgr()->hasFirstOffline(bean::Message_Chat, ui->messagePannel->id()))
	{
		qPmApp->getBuddyMgr()->clearOffline(ui->messagePannel->type(), ui->messagePannel->id());
		qPmApp->getOfflineMsgManager()->clearOfflineItem(OfflineMsgManager::User, ui->messagePannel->id());
	}

	QWidget::closeEvent(e);
}

void CChatDialog::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		closeChat();
		return;
	}

	QWidget::keyPressEvent(event);
}

void CChatDialog::appendSendMessage(const bean::MessageBody &rBody)
{
	ui->messagePannel->onMessage(rBody, false, false, false);
}

void CChatDialog::onMessage(const bean::MessageBody &rBody, bool history /*= false*/, bool firstHistory /*= false*/)
{
	stopShowInputTip();

	this->addMessageCount();

	if ((!history && !rBody.sync()) || (history && firstHistory))
	{
		this->addUnreadMessageCount();
	}

	ui->messagePannel->onMessage(rBody, history, firstHistory);

	/*
	if (rBody.ext().type() == bean::MessageExt_Shake)
	{
		if (!history || (history && firstHistory))
		{
			startShake();
		}
	}
	*/
}

void CChatDialog::clearMessages()
{
	clearMessageCount();
	ui->messagePannel->clearMessages();
}

void CChatDialog::showAutoTip(const QString &tip)
{
	ui->messagePannel->showAutoTip(tip);
}

void CChatDialog::startVideo()
{
	if (ui->btnVideo->isEnabled())
	{
		ui->btnVideo->click();		
	}
}

void CChatDialog::addUnreadMessageCount(int addCount /*= 1*/)
{
	m_unreadMessageCount += addCount;
	int unreadMessageCount = this->unreadMessageCount();
	emit chatUnreadMessageCountChanged(unreadMessageCount);
}

int CChatDialog::unreadMessageCount() const
{
	int unreadMessageCount = m_unreadMessageCount;
	unreadMessageCount += qPmApp->getOfflineMsgManager()->offlineMsgCount(OfflineMsgManager::User, ui->messagePannel->id());
	return unreadMessageCount;
}

void CChatDialog::clearUnreadMessageCount()
{
	m_unreadMessageCount = 0;
	if (qPmApp->getBuddyMgr()->isOfflineReceived())
		qPmApp->getOfflineMsgManager()->clearOfflineMsgCount(OfflineMsgManager::User, ui->messagePannel->id());
}

void CChatDialog::setMaximizeState(bool maximizeState)
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

bool CChatDialog::isExpanded() const
{
	return ui->sideTabWidget->isVisible();
}

int CChatDialog::unExpandedWidth() const
{
	QSize size = this->size();
	if (isExpanded())
	{
		return size.width() - ui->sideTabWidget->width();
	}
	else
	{
		return size.width();
	}
}

bool CChatDialog::canClose()
{
	if (ui->pageSession->isVisible() || ui->sideTabWidget->indexOf(ui->pageSession) >= 0)
	{
		if (!(qPmApp->isShutingDown() || qPmApp->isLogout())) // if not logout or exit, need query to close session
		{
			if (isBlockingClose())
			{
				return false;
			}

			setBlockingClose(true);
			QDialogButtonBox::StandardButton sb = PMessageBox::question(this->window(), 
				ui->pageSession->title()+tr(" Call"), 
				tr("%1 call will be terminated, continue").arg(ui->pageSession->title()), 
				QDialogButtonBox::Yes|QDialogButtonBox::No);
			setBlockingClose(false);
			if (QDialogButtonBox::No == sb || QDialogButtonBox::Cancel == sb)
			{
				return false;
			}
		}
	}

	return true;
}

void CChatDialog::onUnionStateChanged()
{
	if (unionState() == ChatBaseDialog::Single)
	{
		ui->icon->setVisible(true);
	}
	else // Union
	{
		ui->icon->setVisible(false);
	}
}

void CChatDialog::showMoreMsgTip()
{
	ui->messagePannel->showMoreMsgTip();
}

void CChatDialog::closeMoreMsgTip()
{
	ui->messagePannel->closeMoreMsgTip();
}

void CChatDialog::onMoreMsgFinished()
{
	ui->messagePannel->onMoreMsgFinished();
}

void CChatDialog::showMoreHistoryMsgTip()
{
	ui->messagePannel->showMoreHistoryMsgTip();
}

void CChatDialog::appendHistorySeparator()
{
	ui->messagePannel->appendHistorySeparator();
}

void CChatDialog::focusToEdit()
{
	ui->messagePannel->chatInput()->setFocus();
}

void CChatDialog::onUserChanged(const QString &id)
{
	if (id == ui->messagePannel->id())
	{
		bean::DetailItem *pItem = qPmApp->getModelManager()->detailItem(id);
		setTitle(pItem);
		setIcon(pItem);
		setUserMsg(pItem);

		QString name = qPmApp->getModelManager()->userName(id);
		ui->messagePannel->setName(name);

		ui->stackInfoWidget->updateInfo(id);

		// notify changed
		emit chatIconChanged(*(ui->icon->pixmap()));
		emit chatNameChanged(ui->title->text());
	}

	if ((ui->messagePannel->id() != Account::instance()->phoneFullId()) &&
		(id == ui->messagePannel->id() || id == Account::instance()->id()))
	{
		ui->messagePannel->updateChatAvatar(id);
	}
}

void CChatDialog::onUserPresenceChanged(const QString &id)
{
	if (id == Account::instance()->phoneFullId())
	{
		ui->onlineLabel->clear();
	}
	else
	{
		if (ui->messagePannel->id() == id)
		{
			// to do
			PresenceManager *presenceManager = qPmApp->getPresenceManager();
			if (presenceManager->isAvailable(id))
			{
				ui->onlineLabel->setText(qPmApp->getModelManager()->getTerminalText(id));
			}
			else
			{
				ui->onlineLabel->setText(tr("offline"));
			}
		}
	}
}

void CChatDialog::onUserPresenceChanged()
{
	onUserPresenceChanged(ui->messagePannel->id());
}

void CChatDialog::on_icon_clicked()
{
	emit viewContactInfo(ui->messagePannel->id());
}

void CChatDialog::on_title_clicked()
{
	emit viewContactInfo(ui->messagePannel->id());
}

void CChatDialog::on_btnVideo_clicked()
{
	QString id = ui->messagePannel->id();
    if (ui->pageSession->isVisible())
    {
		ui->pageSession->sessionClose();
        hideSessionWidget();
    }

	QSysInfo::WinVersion winVer = QSysInfo::windowsVersion();
	if (winVer < QSysInfo::WV_VISTA)
	{
		ui->messagePannel->showAutoTip(tr("Windows XP below don't support audio/video call"));
		return;
	}

    if (!qPmApp->GetLoginMgr()->isLogined())
    {
        ui->messagePannel->showAutoTip(tr("You are offline, can't start video call, please try when online"));
        return;
    }

	if (qPmApp->getModelManager()->isInBlackList(id))
	{
		ui->messagePannel->showAutoTip(tr("He is blocked, please try when he is removed out of blocked list"));
		return;
	}
	
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	if (!presenceManager->isAvailable(id))
	{
		ui->messagePannel->showAutoTip(tr("He is offline, can't start video call"));
		return;
	}

	if (qPmApp->isTakeSelfPhoto())
	{
		ui->messagePannel->showAutoTip(tr("You are taking photo of yourself now, can't start video call"));
		return;
	}

    if (ui->pageSession->inviteVideo(id))
    {
        showSessionWidget();
    }
}

void CChatDialog::on_btnPtt_clicked()
{
	QString id = ui->messagePannel->id();

	if (ui->pageSession->isVisible())
    {
        ui->pageSession->sessionClose();
        hideSessionWidget();
    }

	QSysInfo::WinVersion winVer = QSysInfo::windowsVersion();
	if (winVer < QSysInfo::WV_VISTA)
	{
		ui->messagePannel->showAutoTip(tr("Windows XP below don't support audio/video call"));
		return;
	}

    if (!qPmApp->GetLoginMgr()->isLogined())
    {
        ui->messagePannel->showAutoTip(tr("Your are offline, can't start audio call, please try when online"));
        return;
    }

	if (qPmApp->getModelManager()->isInBlackList(id))
	{
		ui->messagePannel->showAutoTip(tr("He is blocked, please try when he is removed out of blocked list"));
		return;
	}
	
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	if (!presenceManager->isAvailable(id))
	{
		ui->messagePannel->showAutoTip(tr("He is offline, can't start audio call"));
		return;
	}

    if (ui->pageSession->inviteAudio(id))
    {
        showSessionWidget();
    }
}

void CChatDialog::on_btnInterphone_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		ui->messagePannel->showAutoTip(tr("Your are offline, can't start interphone, please try when online"));
		return;
	}

	ui->messagePannel->addInterphone();
}

void CChatDialog::onbtnHistoryMsgClicked(bool checked /*= true*/)
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
		connect(qPmApp->getBuddyMgr(), SIGNAL(sendSecretMessageRead(QString, QString)), ui->tabLocalHistory, 
			SLOT(onSendSecretMessageRead(QString, QString)), Qt::UniqueConnection);
		connect(qPmApp->getBuddyMgr(), SIGNAL(recvSecretMessageRead(QString, QString)), ui->tabLocalHistory, 
			SLOT(onRecvSecretMessageRead(QString, QString)), Qt::UniqueConnection);
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
		else
		{
			// has session tab, adjust window size
			setSideTabWidgetVisible(true);
		}
	}
	ui->messagePannel->historyButton()->setChecked(checked);
}

void CChatDialog::onbtnHistoryMsgToggled(bool checked)
{
	if (!checked)
	{
		ui->tabLocalHistory->hideSearchBar();
	}
}

void CChatDialog::shakingTimeout()
{
	m_shakingTimer->stop();
	QRect rect = m_shakingFrameBak;
	rect.translate(m_shakingPosList.at(m_shakingCount));
	setGeometry(rect);
	m_shakingCount++;
	if(m_shakingCount < m_shakingPosList.size())
	{
		m_shakingTimer->start(20);
	}
}

void CChatDialog::setSideTabWidgetVisible(bool visible, bool removeAllTabs /*= true*/)
{
	if (visible)
	{
		// deal with window size
		int miniWidth = 0;
        if (ui->sideTabWidget->indexOf(ui->pageHistory) != -1)
        {
            miniWidth = GuiConstants::WidgetSize::ChatHistoryWidth;
        }
        else if (ui->sideTabWidget->indexOf(ui->pageSession) != -1)
        {
            miniWidth = GuiConstants::WidgetSize::ChatSessionInviteWidth;
        }

		QSize size = this->size();
		if (minimumWidth() > GuiConstants::WidgetSize::GroupChatMainWidth + miniWidth)
		{
			setMinimumWidth(GuiConstants::WidgetSize::GroupChatMainWidth + miniWidth);

			if (unionChatDelegate())
			{
				unionChatDelegate()->chatMinimumWidthChanged(GuiConstants::WidgetSize::GroupChatMainWidth + miniWidth);
			}
		}

		if (unionChatDelegate() && !unionChatDelegate()->isMaximumState())
		{
			if (size.width() < (GuiConstants::WidgetSize::GroupChatMainWidth + miniWidth))
			{
				size.setWidth(size.width() + miniWidth - ui->sideStackedWidget->minimumWidth());
				unionChatDelegate()->chatWidthChanged(size.width());
			}
			else if (miniWidth < ui->sideStackedWidget->minimumWidth())
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

		/*
		setMinimumWidth(GuiConstants::WidgetSize::ChatMainWidth);
		if (unionChatDelegate())
		{
			unionChatDelegate()->chatMinimumWidthChanged(GuiConstants::WidgetSize::ChatMainWidth);
		}
		if (ui->sideTabWidget->isVisible())
		{
			if (unionChatDelegate() && !unionChatDelegate()->isMaximumState())
			{
				int miniWidth = ui->sideTabWidget->minimumWidth();
				QSize size = this->size();
				size.setWidth(size.width() - miniWidth);
				unionChatDelegate()->chatWidthChanged(size.width());
			}
		}
		ui->sideTabWidget->setVisible(false);
		*/

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

void CChatDialog::slotSideTabCloseRequest(int index)
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
    else if (closeWidget == ui->pageSession)
    {
        QDialogButtonBox::StandardButton sb = PMessageBox::question(this->window(), 
			ui->pageSession->title()+tr(" Call"), 
			tr("%1 call will be terminated, continue").arg(ui->pageSession->title()), 
			QDialogButtonBox::Yes|QDialogButtonBox::No);
		if (QDialogButtonBox::No == sb || QDialogButtonBox::Cancel == sb)
        {
            return;
        }

        ui->pageSession->sessionClose();
        hideSessionWidget();
    }
}

void CChatDialog::onMaximizeStateChanged(bool isMaximized)
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

void CChatDialog::sendMail()
{
	emit sendMail(ui->messagePannel->id());
}

void CChatDialog::inviteToNewDiscuss()
{
	QStringList preAddUids;
	preAddUids.append(ui->messagePannel->id());
	emit createDiscuss(preAddUids);
}

void CChatDialog::addFriend()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(ui->messagePannel->id());
	emit addFriendRequest(ui->messagePannel->id(), name);
}

void CChatDialog::onRosterChanged()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	QString id = ui->messagePannel->id();
	if (!rosterModel->isFriend(id))
	{
		ui->messagePannel->shakeButton()->setVisible(false);
		ui->btnAddFriend->setVisible(true);
	}
	else
	{
		ui->messagePannel->shakeButton()->setVisible(true);
		ui->btnAddFriend->setVisible(false);
	}
}

void CChatDialog::onShakeDialog()
{
	/*
	m_shakingFrameBak = frameGeometry();
	m_shakingCount = 0;
	m_shakingTimer->start(20);
	*/

	emit shaking();
}

void CChatDialog::onInputChanged()
{
	if (!m_inputChangeTimer->isActive())
	{
		// both are on-line and not in black list
		if (qPmApp->GetLoginMgr()->isLogined() &&
			qPmApp->getPresenceManager()->isAvailable(ui->messagePannel->id()) &&
			!qPmApp->getModelManager()->isInBlackList(ui->messagePannel->id()))
		{
			// notify input
			qPmApp->getTipManager()->sendInputTip(Account::instance()->id(), ui->messagePannel->id());

			// start input change timer
			m_inputChangeTimer->start();
		}
	}
}

void CChatDialog::onRecordStart()
{
	/*
	if (!m_inputChangeTimer->isActive())
	{
		// both are on-line and not in black list
		if (qPmApp->GetLoginMgr()->isLogined() &&
			qPmApp->getPresenceManager()->isRosterAvailable(ui->messagePannel->id()) &&
			!qPmApp->getModelManager()->isInBlackList(ui->messagePannel->id()))
		{
			// notify input
			qPmApp->getTipManager()->sendSpeakTip(Account::instance()->id(), ui->messagePannel->id());

			// start input change timer
			m_inputChangeTimer->start();
		}
	}
	*/
}

void CChatDialog::onMessageToSend()
{
	// stop input change timer
	m_inputChangeTimer->stop();
}

void CChatDialog::onInputShowTimeout()
{
	if (++m_inputShowIndex >= (9000/m_inputShowTimer->interval())) // 9s
	{
		stopShowInputTip();
	}
	else
	{
		QString baseText = ui->inputTip->toolTip();
		QString inputText = baseText;
		for (int i = 0; i < (m_inputShowIndex%4); i++)
		{
			inputText.append(".");
		}
		ui->inputTip->setText(inputText);
	}
}

void CChatDialog::onInputTipRecved(const QString &from, const QString &to, const QString &action)
{
	Q_UNUSED(action);
	Q_UNUSED(to);

	if (from != ui->messagePannel->id())
		return;

	if (qPmApp->getModelManager()->isInBlackList(ui->messagePannel->id()))
		return;

	startShowInputTip(tr("Inputing"));
}

void CChatDialog::onSpeakTipRecved(const QString &from, const QString &to, const QString &action)
{
	Q_UNUSED(action);
	Q_UNUSED(to);

	if (from != ui->messagePannel->id())
		return;

	if (qPmApp->getModelManager()->isInBlackList(ui->messagePannel->id()))
		return;

	startShowInputTip(tr("Talking"));
}

void CChatDialog::on_blackLabel_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, can't remove out of blocked list"));
		return;
	}

	QString id = ui->messagePannel->id();
	QString name = qPmApp->getModelManager()->userName(id);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Remove out of blocked list"), 
		tr("Are you sure to remove %1 out of blocked list").arg(name), 
		QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	if (ret == QDialogButtonBox::Cancel)
		return;

	emit removeBlack(id);
}

void CChatDialog::onLoadHistoryMessagesFinished(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs)
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
			if (ui->messagePannel->id() == Account::instance()->phoneFullId())
				loadHistory = true;
			if (loadHistory)
				this->showMoreMsgTip();
			else
				this->closeMoreMsgTip();
		}
		else
		{
			if (ui->messagePannel->id() == Account::instance()->phoneFullId())
			{
				this->closeMoreMsgTip();
			}
			else
			{
				if (curPage <= 1)
					this->closeMoreMsgTip();
				else
					this->showMoreHistoryMsgTip();
			}
		}

		if (origMsgCount <= 0 && curPage == maxPage)
		{
			ui->messagePannel->scrollMessageToBottom();
		}

		m_fetchHistoryMsgId = -1;
	}
}

void CChatDialog::onBtnMoreMessageClicked()
{
	MessageManagerDlg *pDlg = MessageManagerDlg::instance();
	pDlg->init(ui->messagePannel->id(), bean::Message_Chat);
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void CChatDialog::onHistoryTabWidgetCurrentChanged(int index)
{
	if (index == 1) // roaming message
	{
		ui->tabRoamingMsg->init(ui->messagePannel->id(), ui->messagePannel->type());
	}
}

void CChatDialog::InitUI( const QString &uid )
{
	// get use name
	QString name = qPmApp->getModelManager()->userName(uid);

	// init message panel
	ui->messagePannel->init(bean::Message_Chat, uid, name);
	if (uid != Account::instance()->phoneFullId())
	{
		ui->messagePannel->setSupportSecretMessage();
	}
	connect(ui->messagePannel, SIGNAL(doScreenshot()), this, SIGNAL(doScreenshot()));
	connect(ui->messagePannel, SIGNAL(sendMail(QString)), this, SIGNAL(sendMail(QString)));
	connect(ui->messagePannel, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->messagePannel, SIGNAL(doClearMessages()), this, SLOT(clearMessages()));
	connect(ui->messagePannel, SIGNAL(inputChanged()), this, SLOT(onInputChanged()));
	connect(ui->messagePannel, SIGNAL(recordStart()), this, SLOT(onRecordStart()));
	connect(ui->messagePannel, SIGNAL(messageToSend()), this, SLOT(onMessageToSend()));
	connect(ui->messagePannel, SIGNAL(closeRequest()), this, SLOT(closeChat()));
	connect(ui->messagePannel, SIGNAL(fetchHistoryMsg()), this, SLOT(fetchMoreMessages()));
	connect(ui->messagePannel, SIGNAL(openHistoryMsg()), this, SLOT(onbtnHistoryMsgClicked()));

	// init input
	initInputTimer();

	TipManager *tipManager = qPmApp->getTipManager();
	connect(tipManager, SIGNAL(inputTipRecved(QString, QString, QString)), this, SLOT(onInputTipRecved(QString, QString, QString)));
	connect(tipManager, SIGNAL(speakTipRecved(QString, QString, QString)), this, SLOT(onSpeakTipRecved(QString, QString, QString)));

	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onUserChanged(QString)));
	
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	connect(presenceManager, SIGNAL(presenceReceived(QString, int, int)), this, SLOT(onUserPresenceChanged(QString)));
	connect(presenceManager, SIGNAL(presenceCleared()), this, SLOT(onUserPresenceChanged()));

	// title
	bean::DetailItem* pItem = modelManager->detailItem(uid);
	ui->title->setFontAtt(QColor("#333333"), 13, false);
	if (uid != Account::instance()->phoneFullId())
		ui->title->setClickable(true);
	else
		ui->title->setClickable(false);
	setTitle(pItem);

	// icon
	ui->icon->setVisible(false);
	if (uid != Account::instance()->phoneFullId())
		ui->icon->setClickable(true);
	else
		ui->icon->setClickable(false);
	setIcon(pItem);

	// online info
	onUserPresenceChanged();

	// labUserMsg
	setUserMsg(pItem);

	// init shaking
	initShake();

	// init msg setting
	ui->messagePannel->msgSettingButton()->setVisible(false);

	// init side tab widget
	ui->sideTabWidget->setTabsClosable(true);
	connect(ui->sideTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotSideTabCloseRequest(int)));
	setSideTabWidgetVisible(false);

	// init history message button
	connect(ui->messagePannel->historyButton(), SIGNAL(clicked(bool)), this, SLOT(onbtnHistoryMsgClicked(bool)));
	connect(ui->messagePannel->historyButton(), SIGNAL(toggled(bool)), this, SLOT(onbtnHistoryMsgToggled(bool)));
	if (uid == Account::instance()->phoneFullId())
	{
		// my phone do not allow to view history message
		ui->messagePannel->historyButton()->setVisible(false);
	}

	connect(ui->historyTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onHistoryTabWidgetCurrentChanged(int)));

    // init session widget
	ui->pageSession->setChatDialog(this);
	connect(ui->pageSession, SIGNAL(sessionVideoSetup()), this, SLOT(onSessionVideoSetup()));
    connect(ui->pageSession, SIGNAL(sessionClosed()), this, SLOT(onSessionClosed()));

	// check if has video session on
	if (qPmApp->getSessionVideoManager()->hasVideoSession(uid))
	{
		ui->btnPtt->setEnabled(false);
		ui->btnVideo->setEnabled(false);

		SessionVideoDialog *videoDialog = qPmApp->getSessionVideoManager()->videoDialog(uid);
		connect(videoDialog, SIGNAL(destroyed()), this, SLOT(onSessionClosed()), Qt::UniqueConnection);
	}
	else
	{
		ui->btnPtt->setEnabled(true);
		ui->btnVideo->setEnabled(true);
	}
	bool enableAudio = !GlobalSettings::audioDisabled();
	if (enableAudio)
		ui->btnPtt->setVisible(true);
	else
		ui->btnPtt->setVisible(false);
	bool enableVideo = !GlobalSettings::videoDisabled();
	if (enableVideo)
		ui->btnVideo->setVisible(true);
	else
		ui->btnVideo->setVisible(false);

	// send mail button
	connect(ui->btnMail, SIGNAL(clicked()), this, SLOT(sendMail()));

	// add friend button
	onRosterChanged();
	connect(ui->btnAddFriend, SIGNAL(clicked()), this, SLOT(addFriend()));
	connect(qPmApp->getModelManager()->rosterModel(), SIGNAL(rosterChanged()), this, SLOT(onRosterChanged()));

	// configure roaming message
	if (ui->historyTabWidget->count() > 1)
	{
		if (GlobalSettings::roamingMsgDisabled())
		{
			ui->historyTabWidget->removeTab(1);
		}
	}

	if (modelManager->isInBlackList(uid))
	{
		ui->blackLabel->setVisible(true);
	}
	else
	{
		ui->blackLabel->setVisible(false);
	}

	// if is device phone
	if (uid == Account::instance()->phoneFullId())
	{
		ui->btnAddFriend->setVisible(false);
		ui->btnInterphone->setVisible(false);
		ui->btnMail->setVisible(false);
		ui->btnPtt->setVisible(false);
		ui->btnVideo->setVisible(false);
		ui->btnInviteToNewDiscuss->setVisible(false);
		ui->messagePannel->shakeButton()->setVisible(false);

		ui->historyTabWidget->setCornerWidget(0); // no more message shown
		if (ui->historyTabWidget->count() > 1)  // no roaming message shown
		{
			ui->historyTabWidget->removeTab(1);
		}
	}
	else
	{
		// config interphone function
		if (GlobalSettings::interphoneDisabled())
			ui->btnInterphone->setVisible(false);
		else
			ui->btnInterphone->setVisible(true);
	}

	// chat info
	ui->stackInfoWidget->updateInfo(uid);
	//invite roster to create a discuss group
	connect(ui->btnInviteToNewDiscuss, SIGNAL(clicked()), this, SLOT(inviteToNewDiscuss()));

	qDebug() << Q_FUNC_INFO << ui->messagePannel->id();
}

void CChatDialog::initShake()
{
	// shake time, every 20ms change a pos
	m_shakingTimer = new QTimer(this);
	m_shakingTimer->setInterval(20);

	// init position list
	m_shakingPosList.clear();
	m_shakingPosList.append(QPoint(5, 0));
	m_shakingPosList.append(QPoint(5, -5));
	m_shakingPosList.append(QPoint(-5, -5));
	m_shakingPosList.append(QPoint(-5, 5));
	m_shakingPosList.append(QPoint(5, 5));
	m_shakingPosList.append(QPoint(5, -5));
	m_shakingPosList.append(QPoint(-5, -5));
	m_shakingPosList.append(QPoint(-5, 5));
	m_shakingPosList.append(QPoint(5, 5));
	m_shakingPosList.append(QPoint(5, -5));
	m_shakingPosList.append(QPoint(-5, -5));
	m_shakingPosList.append(QPoint(-5, 5));
	m_shakingPosList.append(QPoint(5, 5));
	m_shakingPosList.append(QPoint(0, 0));

	// init signals & slots
	connect(ui->messagePannel, SIGNAL(shakeDialog()), this, SLOT(onShakeDialog()));
	connect(m_shakingTimer, SIGNAL(timeout()), SLOT(shakingTimeout()));
}

void CChatDialog::startShake()
{
	/*
	// show chat dialog directly
	WidgetManager::showActivateRaiseWindow(this);

	// start shake
	m_shakingFrameBak = frameGeometry();
	m_shakingCount = 0;
	if (isVisible())
		m_shakingTimer->start(20);
	*/

	emit shaking();
}


void CChatDialog::setTitle(bean::DetailItem* pItem)
{
	// title
	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(ui->messagePannel->id());
	QString nameLabelText = name;
	if (pItem->isDisabled())
		nameLabelText.append(tr("(invalid)"));
	ui->title->setText(nameLabelText);
	setWindowTitle(name);
}

void CChatDialog::setIcon(bean::DetailItem* pItem)
{
	// icon
	QPixmap pixWindowIcon = qPmApp->getModelManager()->getUserAvatar(pItem->uid());
	QPixmap pix = pixWindowIcon.scaled(QSize(90, 90), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->icon->setPixmap(pix);

	// reset window icon
	setWindowIcon(QIcon(pixWindowIcon));
}

void CChatDialog::setUserMsg(bean::DetailItem* pItem)
{
	QString sUserMsg = pItem->message();
	sUserMsg = sUserMsg.replace('\n', " ").trimmed();
	ui->labUserMsg->setText(sUserMsg);
	ui->labUserMsg->setToolTip(sUserMsg);
}

void CChatDialog::showSessionWidget()
{
	if (ui->sideTabWidget->indexOf(ui->pageSession) != -1)
	{
		ui->sideTabWidget->setCurrentIndex(ui->sideTabWidget->indexOf(ui->pageSession));
		return;
	}

	ui->sideTabWidget->addTab(ui->pageSession, ui->pageSession->title());
	ui->sideTabWidget->setCurrentIndex(ui->sideTabWidget->indexOf(ui->pageSession));
	setSideTabWidgetVisible(true);

    ui->btnPtt->setEnabled(false);
    ui->btnVideo->setEnabled(false);
}

void CChatDialog::hideSessionWidget(bool enableSession /*= true*/)
{
	if (ui->sideTabWidget->indexOf(ui->pageSession) != -1)
	{
		ui->sideTabWidget->removeTab(ui->sideTabWidget->indexOf(ui->pageSession));
	}
	if (ui->sideTabWidget->count() <= 0)
	{
		setSideTabWidgetVisible(false);
	}

	if (enableSession)
	{
		ui->btnPtt->setEnabled(true);
		ui->btnVideo->setEnabled(true);
	}
}

void CChatDialog::onSessionVideoSetup()
{
	hideSessionWidget(false);
}

void CChatDialog::onSessionClosed()
{
    hideSessionWidget();
}

void CChatDialog::insertMimeData(const QMimeData *source)
{
	ui->messagePannel->chatInput()->insertMimeData(source);
}

void CChatDialog::loadHistoryMessages(int count)
{
	Q_UNUSED(count);

	connect(qPmApp->getMessageDBStore(), SIGNAL(gotMessages(qint64, int, int, bean::MessageBodyList)), 
		this, SLOT(onLoadHistoryMessagesFinished(qint64, int, int, bean::MessageBodyList)));
	int pageIndex = this->historyMsgPageIndex();
	QString endMsgTime = this->historyMsgEndTime();
	m_fetchHistoryMsgId = qPmApp->getMessageDBStore()->getMessagesBeforeTime(
		ui->messagePannel->type(), ui->messagePannel->id(), pageIndex, kPageHistoryMessageCount, endMsgTime);
}

void CChatDialog::fetchHistoryMessages()
{
	bean::MessageType msgType = ui->messagePannel->type();
	QString id = ui->messagePannel->id();
	qPmApp->getBuddyMgr()->getHistoryMsg(msgType, id);
}

void CChatDialog::fetchMoreMessages()
{
	QString id = ui->messagePannel->id();
	OfflineMsgManager *offlineMsgManager = qPmApp->getOfflineMsgManager();
	if (offlineMsgManager->containOfflineItem(OfflineMsgManager::User, id)) // get offline history message
	{
		if (!qPmApp->GetLoginMgr()->isLogined()) // not logined
		{
			qWarning() << Q_FUNC_INFO << "was not logined, user: " << id;

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

void CChatDialog::onSessionInvite( const QString &sid )
{
    if (ui->pageSession->isVisible())
    {
        ui->pageSession->sessionClose();
        hideSessionWidget();
    }

	QSysInfo::WinVersion winVer = QSysInfo::windowsVersion();
	if (winVer < QSysInfo::WV_VISTA)
	{
		ui->messagePannel->showAutoTip(tr("Windows XP below don't support audio/video call"));

		// if has session, close session
		rtcsession::Session *s = qPmApp->getRtcSessionManager()->session(sid);
		if (s)
		{
			s->close(rtcsession::Session::CloseUnsupport, tr("XP platform"));
		}
		return;
	}

    ui->pageSession->onInvite(sid);

    showSessionWidget();

	WidgetManager::showActivateRaiseWindow(this);
}

void CChatDialog::initInputTimer()
{
	m_inputChangeTimer = new QTimer(this);
	m_inputChangeTimer->setSingleShot(true);
	m_inputChangeTimer->setInterval(14*1000); // 14s

	m_inputShowTimer = new QTimer(this);
	m_inputShowTimer->setSingleShot(false);
	m_inputShowTimer->setInterval(500);
	connect(m_inputShowTimer, SIGNAL(timeout()), this, SLOT(onInputShowTimeout()));
	m_inputShowIndex = 0;
}

void CChatDialog::startShowInputTip(const QString &tipText)
{
	m_inputShowIndex = 0;
	m_inputShowTimer->start();
	ui->inputTip->setVisible(true);
	ui->inputTip->setText(tipText);
	ui->inputTip->setToolTip(tipText);
}

void CChatDialog::stopShowInputTip()
{
	m_inputShowIndex = 0;
	m_inputShowTimer->stop();
	ui->inputTip->setVisible(false);
	ui->inputTip->setText("");
	ui->inputTip->setToolTip("");
}