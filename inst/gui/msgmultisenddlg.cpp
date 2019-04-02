#include "msgmultisenddlg.h"
#include "ui_msgmultisenddlg.h"
#include "guiconstants.h"
#include "model/groupitemlistmodeldef.h"
#include "PmApp.h"
#include "model/ModelManager.h"
#include "model/lastcontactmodeldef.h"
#include <QCryptographicHash>
#include <QByteArray>
#include "manager/presencemanager.h"
#include <QDebug>
#include <QTimer>
#include "buddymgr.h"
#include "msgmultisendmemberdlg.h"
#include "pmessagebox.h"
#include "chatinputbox.h"
#include "loginmgr.h"
#include "Account.h"

MsgMultiSendDlg::MsgMultiSendDlg(const QStringList &members, const QString &id /*= QString()*/, QWidget *parent /*= 0*/)
	: ChatBaseDialog(parent), m_id(id), m_members(members)
{
	ui = new Ui::MsgMultiSendDlg();
	ui->setupUi(this);

	Q_ASSERT(!m_id.isEmpty());
	Q_ASSERT(!m_members.isEmpty());

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

	qSort(m_members);

	initMemberModel();
	initUI();

	resize(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth, GuiConstants::WidgetSize::ChatHeight);
	setMinimumSize(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth, GuiConstants::WidgetSize::ChatHeight);
	ui->rightPanel->setMinimumWidth(GuiConstants::WidgetSize::GroupChatMemberWidth);
	ui->rightPanel->setFixedWidth(GuiConstants::WidgetSize::GroupChatMemberWidth);

	setSkin();

	connect(ui->btnClose2, SIGNAL(clicked()), this, SIGNAL(requestClose()));
	connect(ui->btnMinimize2, SIGNAL(clicked()), this, SIGNAL(requestMinimized()));
	connect(ui->btnMaximize2, SIGNAL(clicked()), this, SIGNAL(requestMaximized()));

	ui->messagePannel->layoutPannel();
	ui->messagePannel->chatInput()->setFocus();
}

MsgMultiSendDlg::~MsgMultiSendDlg()
{
	delete m_memberModel;

	delete ui;
}

QString MsgMultiSendDlg::id() const
{
	return m_id;
}

bool MsgMultiSendDlg::hasSameMembers(const QStringList &members) const
{
	return isSameMember(m_members, members);
}

bool MsgMultiSendDlg::isSameMember(const QStringList &left, const QStringList &right)
{
	if (left.count() != right.count())
		return false;

	QStringList memberLeft = left;
	QStringList memberRight = right;
	qSort(memberLeft);
	qSort(memberRight);
	QString memberLeftStr = memberLeft.join(",");
	QString memberRightStr = memberRight.join(",");
	return memberLeftStr == memberRightStr;
}

void MsgMultiSendDlg::checkToAddMember()
{
	if (m_members.count() <= 1)
	{
		QTimer::singleShot(0, this, SLOT(doCheckToAddMember()));
	}
}

void MsgMultiSendDlg::sendMessage(const bean::MessageBody &msgBody)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QString selfId = Account::instance()->id();
	QString selfName = modelManager->userName(selfId);
	CBuddyMgr *buddyMgr = qPmApp->getBuddyMgr();

	// send this message to every member
	foreach (QString member, m_members)
	{
		bean::MessageBody body = msgBody;
		body.copy(); // detach from the original
		body.setTo(member);
		body.setToName(modelManager->userName(member));
		body.setFrom(selfId);
		body.setFromName(selfName);
		bean::MessageExt ext = body.ext();
		ext.setData(bean::EXT_DATA_HISTORY_NAME, false);     // not add to message history
		ext.setData(bean::EXT_DATA_LASTCONTACT_NAME, false); // not add to last contact
		ext.setData(bean::EXT_DATA_CHECKFAILED_NAME, false); // not do send check failed
		body.setExt(ext);

		buddyMgr->slot_sendMessage(body);
	}

	// add this record to last contact
	modelManager->lastContactModel()->appendMultiSendMsg(msgBody, m_id, m_members);
}

void MsgMultiSendDlg::setSkin()
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

	ui->rightPanel->setStyleSheet(QString(
		"QWidget#rightPanel {"
		"	background-color: white;"
		"	border: none;"
		"	border-left: 1px solid rgb(236, 236, 236);"
		"}"));

	// set button members style
	StyleToolButton::Info info;
	info.urlNormal = QString(":/images/Icon_115.png");
	info.urlHover = QString(":/images/Icon_115_hover.png");
	info.urlPressed = QString(":/images/Icon_115_hover.png");
	info.tooltip = tr("Edit Members");
	ui->btnMember->setInfo(info);

	// set button create discuss style
	info.urlNormal = QString(":/images/Icon_113.png");
	info.urlHover = QString(":/images/Icon_113_hover.png");
	info.urlPressed = QString(":/images/Icon_113_hover.png");
	info.urlDisabled = QString(":/images/Icon_113_disabled.png");
	info.tooltip = tr("Create Discuss");
	ui->btnDiscuss->setInfo(info);

	// set button leave style
	info.urlNormal = QString(":/images/Icon_114.png");
	info.urlHover = QString(":/images/Icon_114_hover.png");
	info.urlPressed = QString(":/images/Icon_114_hover.png");
	info.urlDisabled = QString(":/images/Icon_114_disabled.png");
	info.tooltip = tr("Quit Multi-Send Message");
	ui->btnLeave->setInfo(info);

	ui->messagePannel->setSkin();
}

void MsgMultiSendDlg::slot_screenshot_ok(const QString &imagePath)
{
	ui->messagePannel->slot_screenshot_ok(imagePath);
}

void MsgMultiSendDlg::slot_screenshot_cancel()
{
	ui->messagePannel->slot_screenshot_cancel();
}

void MsgMultiSendDlg::insertMimeData(const QMimeData *source)
{
	ui->messagePannel->chatInput()->insertMimeData(source);
}

void MsgMultiSendDlg::setMaximizeState(bool maximizeState)
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

bool MsgMultiSendDlg::isExpanded() const
{
	return false;
}

int MsgMultiSendDlg::unExpandedWidth() const
{
	return size().width();
}

void MsgMultiSendDlg::onUnionStateChanged()
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

void MsgMultiSendDlg::focusToEdit()
{
	ui->messagePannel->chatInput()->setFocus();
}

void MsgMultiSendDlg::onMaximizeStateChanged(bool isMaximized)
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

void MsgMultiSendDlg::setMemberCount()
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

void MsgMultiSendDlg::on_btnMember_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		ui->messagePannel->showAutoTip(tr("You are offline, can't edit member"));
		return;
	}

	MsgMultiSendMemberDlg dlg(m_members, this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QStringList newMembers = dlg.memberIds();
		if (!hasSameMembers(newMembers))
		{
			m_members = newMembers;
			qSort(m_members);
			onMemberChanged();
		}
	}
}

void MsgMultiSendDlg::on_btnLeave_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		ui->messagePannel->showAutoTip(tr("You are offline, can't quit multi-send message"));
		return;
	}

	QDialogButtonBox::StandardButton sb = PMessageBox::question(this, tr("Quit Multi-Send Message"), 
		tr("Are you sure to quit %1").arg(ui->title->text()), QDialogButtonBox::Yes | QDialogButtonBox::No);
	if (QDialogButtonBox::Yes == sb)
	{
		emit removeMultiSend(m_id);

		closeChat();
	}
}

void MsgMultiSendDlg::onMemberChanged()
{
	// update icon
	updateIcon();

	// update name
	updateName();

	// update message panel
	QString name = ui->title->text();
	ui->messagePannel->init(bean::Message_Chat, m_id, name);

	// update member model
	QStringList memberNames;
	foreach (QString id, m_members)
	{
		QString memberName = qPmApp->getModelManager()->userName(id);
		memberNames.append(memberName);
	}
	m_memberModel->setGroupItems(m_id, m_members, memberNames);

	setMemberCount();

	// issue this change
	emit memberChanged(m_id, name, m_members);

	// notify changed
	emit chatIconChanged(*(ui->icon->pixmap()));
	emit chatNameChanged(ui->title->text());
}

void MsgMultiSendDlg::doCheckToAddMember()
{
	MsgMultiSendMemberDlg dlg(m_members, this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QStringList newMembers = dlg.memberIds();
		m_members = newMembers;
		qSort(m_members);
		onMemberChanged();
	}
	else
	{
		closeChat();
	}
}

void MsgMultiSendDlg::initUI()
{
	// set window title
	updateName();
	ui->labDesc->setText(QString());

	// set window icon
	updateIcon();
	ui->icon->setVisible(false);
	ui->icon->setClickable(false);

	ui->btnDiscuss->setVisible(false);

	// init message panel
	QString name = ui->title->text();
	ui->messagePannel->init(bean::Message_Chat, m_id, name);
	ui->messagePannel->setMsgSendDelegate(this);
	connect(ui->messagePannel, SIGNAL(doScreenshot()), this, SIGNAL(doScreenshot()));
	connect(this, SIGNAL(cleanup()), ui->messagePannel, SIGNAL(cleanup()));
	connect(ui->messagePannel, SIGNAL(closeRequest()), this, SLOT(closeChat()));
	connect(ui->messagePannel, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));

	// init msg manipulation
	ui->messagePannel->shakeButton()->setVisible(false);
	ui->messagePannel->historyButton()->setVisible(false);
	ui->messagePannel->msgSettingButton()->setVisible(false);

	// member actions
	ui->listView->setContextMenuPolicy(Qt::NoContextMenu);
	connect(ui->listView, SIGNAL(chat(QString)), this, SIGNAL(chat(QString)));
	connect(ui->listView, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->listView, SIGNAL(sendMail(QString)), this, SIGNAL(sendMail(QString)));
	connect(ui->listView, SIGNAL(addFriendRequest(QString, QString)), this, SIGNAL(addFriendRequest(QString, QString)));

	// show group member
	setMemberCount();
}

void MsgMultiSendDlg::initMemberModel()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	m_memberModel = new GroupItemListModel(modelManager);
	QStringList memberNames;
	foreach (QString id, m_members)
	{
		QString memberName = qPmApp->getModelManager()->userName(id);
		memberNames.append(memberName);
	}
	m_memberModel->setGroupItems(m_id, m_members, memberNames);
	ui->listView->setGroupMemberModel(m_memberModel);
}

void MsgMultiSendDlg::updateIcon()
{
	QPixmap pixmap = generateIcon();
	setWindowIcon(QIcon(pixmap));
	ui->icon->setPixmap(pixmap);
}

void MsgMultiSendDlg::updateName()
{
	QString name = generateName();
	ui->title->setText(name);
	setWindowTitle(name);
}

QPixmap MsgMultiSendDlg::generateIcon() const
{
	if (m_members.isEmpty())
	{
		QPixmap icon(":/images/Icon_65.png");
		return icon;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	return modelManager->getMultiAvatar(110, m_members);
}

QString MsgMultiSendDlg::generateName() const
{
	if (m_members.isEmpty())
	{
		return tr("Multi-Send Message");
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	return modelManager->getMultiName(m_members);
}
