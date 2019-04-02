#include <QDebug>
#include <QFontDatabase>
#include <QWebPage>
#include <QWebFrame>
#include <QStandardPaths>
#include <QUuid>
#include <QMimeData>

#include "MessagePannel.h"
#include "ui_MessagePannel.h"

#include "webview.h"

#include "common/datetime.h"

#include "message4js.h"
#include "login/Account.h"
#include "buddymgr.h"
#include "PmApp.h"
#include "model/ModelManager.h"
#include "minisplitter.h"
#include "pmessagebox.h"
#include "widgetmanager.h"
#include "util/FileDialog.h"
#include "util/AmrPlayUtil.h"
#include "manager/presencemanager.h"
#include "wave/amrrecord.h"
#include "interphonemanager.h"
#include "interphonedialog.h"
#include <QWebElement>
#include <QAction>
#include "util/FileUtil.h"
#include "util/ImageUtil.h"
#include <QClipboard>
#include "secretswitch.h"
#include "chatwebpage.h"
#include <QFileDialog>
#include "messagewithdrawmanager.h"
#include "pmessagelinetip.h"
#include "MessageDBStore.h"
#include "selectchatdialog.h"

const static unsigned int s_unMaxFileSize = -1;

MessagePannel::MessagePannel(QWidget *parent)
	: QWidget(parent)
	, m_pMessage4js(0)
	, m_msgSendDelegate(0)
	, m_recordId(0)
	, m_messageMode(Normal)
	, m_secretSwitchTip(0)
	, m_chatWebPage(0)
{
	ui = new Ui::MessagePannel();
	ui->setupUi(this);

	initUI();

	AmrRecord *amrRecord = qPmApp->getAmrRecord();
	bool connectOK = false;
	connectOK = connect(amrRecord, SIGNAL(finished(int, QString)), this, SLOT(onRecordFinished(int, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(amrRecord, SIGNAL(canceled(int)), this, SLOT(onRecordCanceled(int)));
	Q_ASSERT(connectOK);

	connectOK = connect(amrRecord, SIGNAL(startError(int, QString)), this, SLOT(onRecordStartError(int, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(amrRecord, SIGNAL(timeElapsed(int, int)), this, SLOT(onRecordTimeElapsed(int, int)));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->recordBar, SIGNAL(sendRecord()), this, SLOT(onSendRecord()));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->recordBar, SIGNAL(cancelRecord()), this, SLOT(onCancelRecord()));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->textEdit, SIGNAL(textChanged()), this, SIGNAL(inputChanged()));
	Q_ASSERT(connectOK);

	m_copyAction = new QAction(tr("Copy"), this);
	connectOK = connect(m_copyAction, SIGNAL(triggered()), this, SLOT(onCopyActionTriggered()));
	Q_ASSERT(connectOK);

	m_saveAction = new QAction(tr("Save as..."), this);
	connectOK = connect(m_saveAction, SIGNAL(triggered()), this, SLOT(onSaveActionTriggered()));
	Q_ASSERT(connectOK);

	m_addFavoriteEmotion = new QAction(tr("Favorite"), this);
	connectOK = connect(m_addFavoriteEmotion, SIGNAL(triggered()), this, SLOT(addFavoriteEmotion()));
	Q_ASSERT(connectOK);

	m_messageWithdrawAction = new QAction(tr("Withdraw"), this);
	connectOK = connect(m_messageWithdrawAction, SIGNAL(triggered()), this, SLOT(withdrawMessage()));
	Q_ASSERT(connectOK);

	m_messageCopyAction = new QAction(tr("Copy"), this);
	connectOK = connect(m_messageCopyAction, SIGNAL(triggered()), this, SLOT(copyMessage()));
	Q_ASSERT(connectOK);

	m_messageForwardAction = new QAction(tr("Forward"), this);
	connectOK = connect(m_messageForwardAction, SIGNAL(triggered()), this, SLOT(forwardMessage()));
	Q_ASSERT(connectOK);

	connectOK = connect(qPmApp->getBuddyMgr(), SIGNAL(sendSecretMessageRead(QString, QString)),
		this, SLOT(onSendSecretMessageRead(QString, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(qPmApp->getBuddyMgr(), SIGNAL(recvSecretMessageRead(QString, QString)),
		this, SLOT(onRecvSecretMessageRead(QString, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(qPmApp->getBuddyMgr(), SIGNAL(messageSent(bean::MessageType, QString, QString, QString)),
		this, SLOT(onMessageSent(bean::MessageType, QString, QString, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(qPmApp->getBuddyMgr(), 
		SIGNAL(messageWithdrawOK(bean::MessageType, QString, QString, QString, QString, bean::MessageBody)),
		this, SLOT(onWithdrawOK(bean::MessageType, QString, QString, QString, QString, bean::MessageBody)));
	Q_ASSERT(connectOK);

	connectOK = connect(qPmApp->getBuddyMgr(), SIGNAL(messageWithdrawFailed(bean::MessageType, QString, QString, QString)),
		this, SLOT(onWithdrawFailed(bean::MessageType, QString, QString, QString)));
	Q_ASSERT(connectOK);

	MessageDBStore *messageDB = qPmApp->getMessageDBStore();
	connectOK = connect(messageDB, SIGNAL(gotMessageOfStamp(qint64, QString, bean::MessageBody)), 
		this, SLOT(onGotMessageOfStamp(qint64, QString, bean::MessageBody)));
	Q_ASSERT(connectOK);

	setSkin();
}

MessagePannel::~MessagePannel()
{
	AmrRecord *amrRecord = qPmApp->getAmrRecord();
	if (amrRecord && amrRecord->isRecording())
	{
		if (m_recordId == amrRecord->currentRecordId())
		{
			onCancelRecord();
		}
	}

	if (m_secretSwitchTip)
	{
		delete m_secretSwitchTip;
		m_secretSwitchTip = 0;
	}

	delete ui;
}

void MessagePannel::init( bean::MessageType type, const QString &id, const QString &name )
{
	m_type = type;
	m_id = id;
	m_name = name;

	checkIfInInterphone();
}

bean::MessageType MessagePannel::type() const
{
	return m_type;
}

QString MessagePannel::id() const
{
	return m_id;
}

QString MessagePannel::name() const
{
	return m_name;
}

void MessagePannel::setName(const QString &name)
{
	m_name = name;
}

void MessagePannel::setMsgSendDelegate(MessageSendDelegate *msgSendDelegate)
{
	m_msgSendDelegate = msgSendDelegate;
}

void MessagePannel::layoutPannel()
{
	m_chatSplitter->setSizes(QList<int>() << 263 << 140);
}

void MessagePannel::setSupportSecretMessage()
{
	ui->switchSecret->setVisible(true);
	if (!m_secretSwitchTip)
		m_secretSwitchTip = new SecretSwitchTip();
}

void MessagePannel::scrollMessageToBottom()
{
	emit initUIComplete();
}

void MessagePannel::updateChatAvatar(const QString &uid)
{
	if (m_chatWebPage)
		m_chatWebPage->onAvatarChanged(uid);
}

void MessagePannel::updateChatName()
{
	emit m_pMessage4js->updateChatName();
}

void MessagePannel::insertWidgetToToolbar( int index, QWidget *widget )
{
	ui->midToolbarLayout->insertWidget(index, widget);
}

QToolButton * MessagePannel::historyButton() const
{
	return ui->btnHistoryMsg;
}

QToolButton * MessagePannel::msgSettingButton() const
{
	return ui->btnMsgSetting;
}

QToolButton * MessagePannel::shakeButton() const
{
	return ui->btnShake;
}

CChatInputBox * MessagePannel::chatInput() const
{
	return ui->textEdit;
}

void MessagePannel::addMenuAction(QMenu *menu, const QWebElement &webElement)
{
	if (!menu)
		return;

	menu->clear();

	bool checkToAddWithdraw = false;
	bool isImage = false;
	if (webElement.hasClass("img_autodisplay")) // image
	{
		checkToAddWithdraw = true;
		isImage = true;
	}
	else if (webElement.hasClass("bodyContext")) // text
	{
		checkToAddWithdraw = true;
	}
	else if (webElement.hasClass("face-img")) // emotion
	{
		checkToAddWithdraw = true;
	}
	else if (webElement.hasClass("secret")) // check secret
	{
		checkToAddWithdraw = true;
	}
	else if (webElement.hasClass("msgBodyContainer-send")) // bubble-send
	{
		checkToAddWithdraw = true;
	}
	else if (webElement.hasClass("msgBodyContainer-receive")) // bubble-receive
	{
		checkToAddWithdraw = true;
	}
	else if (webElement.hasClass("autodownload") || 
			    webElement.hasClass("audioplay") || 
				webElement.hasClass("audioplayinfo")) // audio message
	{
		checkToAddWithdraw = true;
	}
	else if (webElement.hasClass("attachContainer") ||
			    webElement.hasClass("fileFocueOut") ||
				webElement.hasClass("process") ||
				webElement.hasClass("attachImage") ||
				webElement.hasClass("attachInfoText") ||
				webElement.hasClass("attachNameText") ||
				webElement.hasClass("attachSizeText")) // attach message
	{
		checkToAddWithdraw = true;
	}

	if (checkToAddWithdraw)
	{
		QWebElement ele = webElement;
		do {
			if (ele.hasClass("msgBodyContainer-send") || ele.hasClass("msgBodyContainer-receive"))
			{
				QString stamp = ele.attribute("stamp", "");
				if (!stamp.isEmpty())
				{
					if (!ele.findFirst("div.autodownload").isNull())
					{
						// voice can't copy and forward
						// do nothing...
					}
					else
					{
 						QString secretFlag = ele.attribute("secret", "0");
 						if (secretFlag != "1")
 						{
							m_messageCopyAction->setData(stamp);
							m_messageForwardAction->setData(stamp);
							if (isImage)
							{
								QString pathUrl = webElement.attribute("origsrc", "");
								m_copyAction->setData(pathUrl);
								menu->addAction(m_copyAction);
							}
							else
							{
								menu->addAction(m_messageCopyAction);
							}
							menu->addAction(m_messageForwardAction);
							menu->addSeparator();
						}
					}

					if (ele.hasClass("msgBodyContainer-send"))
					{
						m_messageWithdrawAction->setData(stamp);
						menu->addAction(m_messageWithdrawAction);
						menu->addSeparator();
					}
				}
				break;
			}
			ele = ele.parent();
		} while (!ele.isNull());
	}

	if (isImage)
	{
		QString secretFlag = webElement.attribute("secret", "0");
		if (secretFlag != "1")
		{
			QString pathUrl = webElement.attribute("origsrc", "");
			QUrl url = QUrl::fromEncoded(pathUrl.toUtf8());
			QFileInfo info(url.toLocalFile());
			QString path = info.absoluteFilePath();
			m_saveAction->setData(pathUrl);
			m_addFavoriteEmotion->setData(path);
			menu->addAction(m_saveAction);
			menu->addAction(m_addFavoriteEmotion);
			menu->addSeparator();
		}
	}

	if (menu->isEmpty() && !ui->webView->selectedText().isEmpty())
	{
		menu->addAction(tr("Copy"), ui->webView, SLOT(slot_copy_action_trigger()));
	}
	menu->addAction(tr("Clear Messages"), this, SLOT(on_tBtnCleanup_clicked()));
}

bool MessagePannel::canDragImage(const QWebElement &imageElement)
{
	QString secretFlag = imageElement.attribute("secret", "0");
	if (secretFlag != "1") // secret message can't drag
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool MessagePannel::resendMessageOfSequence(const QString &seq)
{
	return resendFailedMessage(seq);
}

QObject *MessagePannel::instance()
{
	return this;
}

QWidget *MessagePannel::instanceWindow()
{
	return this->window();
}

void MessagePannel::onEmotionSelected(bool defaultEmotion, const QString &emotionId)
{
	if (defaultEmotion)
	{
		bool ok = false;
		int idx = emotionId.toInt(&ok);
		if (!ok)
			return;

		ui->textEdit->insertFace(idx);
	}
	else
	{
		QString emotionPath = EmotionUtil::instance().favoriteEmotionFilePath(emotionId);
		if (emotionPath.isEmpty())
			return;

		ui->textEdit->insertImage(emotionPath);
	}
	ui->textEdit->setFocus();
}

void MessagePannel::emotionClosed()
{
	QPoint cursorPos = QCursor::pos();
	QPoint globalBtnPos = ((QWidget*)(ui->btnFace->parent()))->mapToGlobal(ui->btnFace->pos());
	QRect btnRect(globalBtnPos, ui->btnFace->size());
	btnRect.adjust(-1, -1, 1, 1);
	if (!btnRect.contains(cursorPos))
		ui->btnFace->setChecked(false);
}

void MessagePannel::setSkin()
{
	QString popupToolButtonStyleSheet = QString(
		"QToolButton[popupMode=\"1\"] {" 
		"padding-right: 8px;" 
		"}"
		"QToolButton::menu-button {"
		"border-image: url(%1);"
		"width: 10px;"
		"}"
		"QToolButton::menu-button:hover {"
		"border-image: url(%2);"
		"}"
		"QToolButton::menu-button:pressed {"
		"border-image: url(%3);"
		"}"
		"QToolButton::menu-arrow {"
		"image: url(%4);"
		"}"
		"QToolButton::menu-arrow:open {"
		"top: 1px; left: 1px;"
		"}")
		.arg(":/images/tbtn_menu_button_normal.png")
		.arg(":/images/tbtn_menu_button_hover.png")
		.arg(":/images/tbtn_menu_button_pressed.png")
		.arg(":/images/Icon_88_gray.png");
	ui->tBtnScreenshot->setStyleSheet(popupToolButtonStyleSheet);
	ui->btnAttach->setStyleSheet(popupToolButtonStyleSheet);
	ui->btnMsgSetting->setStyleSheet(popupToolButtonStyleSheet);

	ui->msgTopWidget->setStyleSheet("QWidget#msgTopWidget {background: rgb(255, 255, 255); border: none; border-bottom: 1px solid rgb(219, 219, 219);}");
	ui->midToolBar->setStyleSheet("QWidget#midToolBar {background: rgb(255, 255, 255); border: none;}");
	ui->sendBar->setStyleSheet("QWidget#sendBar {background: rgb(255, 255, 255); border: none;}");
}

void MessagePannel::keyPressEvent(QKeyEvent *event)
{
	QWidget::keyPressEvent(event);
}

void MessagePannel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	ui->webView->onContentsSizeChanged();
}

void MessagePannel::onMessage(const bean::MessageBody &rBody, bool history /*= false*/, bool firstHistory /*= false*/, bool showAt /*= true*/)
{
	if (!history || (history && firstHistory))
	{
		m_pMessage4js->appendMessage(rBody);

		if (showAt && needShowAtMsg(rBody))
		{
			showAtMsg(rBody, true);
		}
	}
	else
	{
		m_pMessage4js->insertMessageToTop(rBody);

		if (showAt && needShowAtMsg(rBody))
		{
			showAtMsg(rBody, false);
		}
	}
}

void MessagePannel::clearMessages()
{
	// clear all the content in web view
	emit cleanup();

	// clear all at messages, and hide at widget
	ui->atWidget->clearAtMsg();
}

void MessagePannel::showMoreMsgTip()
{
	m_pMessage4js->moreMsgTipShow();
}

void MessagePannel::closeMoreMsgTip()
{
	m_pMessage4js->moreMsgTipClose();
}

void MessagePannel::onMoreMsgFinished()
{
	m_pMessage4js->moreMsgFinished();
}

void MessagePannel::showMoreHistoryMsgTip()
{
	m_pMessage4js->moreHistoryMsgTipShow();
}

void MessagePannel::showAutoTip(const QString &tip)
{
	showTip(tip);
}

void MessagePannel::on_switchSecret_toggled(bool checked)
{
	if (checked)
		m_messageMode = Secret;
	else
		m_messageMode = Normal;
	ui->btnShake->setEnabled(!checked);
	ui->btnAttach->setEnabled(!checked);

	if (m_secretSwitchTip)
	{
		QPoint pt;
		pt = ((QWidget *)ui->textEdit->parent())->mapToGlobal(ui->textEdit->pos());
		pt.setX(pt.x() + (ui->textEdit->width() - m_secretSwitchTip->width())/2);
		pt.setY(pt.y() + (ui->textEdit->height() - m_secretSwitchTip->height())/2);
		if (checked)
		{
			m_secretSwitchTip->showOnTip(pt);
		}
		else
		{
			m_secretSwitchTip->showOffTip(pt);
		}
	}
}

void MessagePannel::on_btnFace_clicked(bool checked)
{
	if (checked)
	{
		QPoint pos(ui->btnFace->width()/2, 0);
		pos = ui->btnFace->mapToGlobal(pos);

		EmotionUtil::instance().showEmotion(this, pos);
	}
	else
	{
		EmotionUtil::instance().closeEmotion();
	}
}

void MessagePannel::on_btnAttach_clicked()
{
	insertAttachFiles();
}

void MessagePannel::insertAttachFiles()
{
	// self settings
	QString sDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		sDir = accountSettings->getCurDir();

	QStringList filePaths = FileDialog::getOpenFileNames(this, tr("Choose Attach"), sDir);
	if (!filePaths.isEmpty())
	{
		QString filePath;
		foreach(filePath, filePaths)
		{
			ui->textEdit->insertFile(filePath);
		}

		ui->textEdit->setFocus();

		// self settings
		if (!filePath.isEmpty())
		{
			QFileInfo fi(filePath);
			if (accountSettings)
				accountSettings->setCurDir(fi.absoluteDir().absolutePath());
		}
	}
}

void MessagePannel::insertAttachDirs()
{
	// self settings
	QString sDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		sDir = accountSettings->getCurDir();

	QString dirPath = QFileDialog::getExistingDirectory(this, tr("Choose Dir"), sDir);
	if (!dirPath.isEmpty())
	{
		ui->textEdit->insertDir(dirPath);
		
		ui->textEdit->setFocus();

		// self settings
		if (!dirPath.isEmpty())
		{
			QFileInfo fi(dirPath);
			if (accountSettings)
				accountSettings->setCurDir(fi.absoluteDir().absolutePath());
		}
	}
}

void MessagePannel::on_btnImage_clicked()
{
	// self settings
	QString sDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		sDir = accountSettings->getCurDir();

	// get image file
	// filter
	QString filterTag = tr("Image Files");
	wchar_t szFilter[256];
	memset(szFilter, 0, sizeof(szFilter));
	int len = filterTag.toWCharArray(szFilter);
	wchar_t szFilterSuffix[] = L"(*.bmp;*.jpg;*.jpeg;*.gif;*.png)\0*.BMP;*.JPG;*.JPEG;*.GIF;*.PNG\0\0";
	memcpy((char *)(szFilter+len), (char *)szFilterSuffix, sizeof(szFilterSuffix));

	QStringList filePaths = FileDialog::getOpenFileNames(this, tr("Choose Image"), sDir, szFilter);
	if (filePaths.count() > 10)
	{
		showTip(tr("At most 10 images one time"));
		return;
	}

	// insert image
	if (!filePaths.isEmpty())
	{
		QString filePath;
		foreach (filePath, filePaths)
		{
			ui->textEdit->insertImage(filePath);
		}

		ui->textEdit->setFocus();

		if (!filePath.isEmpty())
		{
			QFileInfo fileInfo(filePath);
			accountSettings->setCurDir(fileInfo.dir().absolutePath());
		}
	}
}

void MessagePannel::on_btnShake_clicked()
{
	QString id = m_id;
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	if (!qPmApp->GetLoginMgr()->isLogined() || !presenceManager->isAvailable(id))
	{
		showAutoTip(tr("He is not online, can't send shake message"));
		return;
	}

	if (!checkBlackList(tr("send shake message")))
		return;

	if (m_shakingIntervalTimer->isActive())
	{
		showAutoTip(tr("You send too frequently, please wait for a moment"));
		return;
	}

	// send shaking message
	bean::MessageBody body = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	QString selfId = Account::instance()->id();
	QString selfName = qPmApp->getModelManager()->userName(selfId);
	body.setTo(id);
	body.setToName(m_name);
	body.setFrom(selfId);
	body.setFromName(selfName);
	body.setTime("");
	body.setSend(true);
	body.setBody(tr("%1 send you a shake message.").arg(selfName));
	body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Shake));
	qPmApp->getBuddyMgr()->slot_sendMessage(body);

	// append shaking tip
	m_pMessage4js->appendMessage(body);

	emit messageToSend();

	// interval check
	m_shakingIntervalTimer->start();

	// shake self
	emit shakeDialog();
}

void MessagePannel::on_btnRecord_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		showTip(tr("You are offline, can't send audio message, please try when online"));
		return;
	}

	if (!checkBlackList(tr("send audio message")))
		return;

	AmrRecord *amrRecord = qPmApp->getAmrRecord();
	if (amrRecord->isRecording())
	{
		showTip(tr("At same time only one record program can run, please try later"));
		return;
	}

	QString desc;
	bool startOK = amrRecord->start(Account::instance()->audioPath(), m_recordId, desc);
	if (!startOK)
	{
		showTip(desc);
		return;
	}

	showRecordBar();
}

void MessagePannel::on_btnSend_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		showTip(tr("You are offline, can't send message, please try when online"));
		return;
	}

	if (!checkBlackList(tr("send message")))
		return;
		
	Account *pAccount = Account::instance();
	QString uid = pAccount->id();
	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(uid);

	// 获取数据
	QString sBody = ui->textEdit->msgText();
	QStringList msgPieces = ui->textEdit->msgPieces();
	QStringList atIds = ui->textEdit->atIds();
	QStringList atIdNames;
	foreach (QString atId, atIds)
	{
		atId = atId.trimmed();
		if (atId.isEmpty())
		{
			atIdNames.append(atId);
		}
		else
		{
			QString atName;
			if (m_type == bean::Message_GroupChat)
			{
				atName = modelManager->memberNameInGroup(m_id, atId);
			}
			else if (m_type == bean::Message_DiscussChat)
			{
				atName = modelManager->memberNameInDiscuss(m_id, atId);
			}
			else
			{
				atName = modelManager->userName(atId);
			}
			QString atBase64Name = QString::fromLatin1(atName.toUtf8().toBase64().constData());
			atIdNames.append(QString("%1:%2").arg(atId).arg(atBase64Name));
		}
	}

	// 文件附件
	QStringList lstFileAttachs = ui->textEdit->msgFiles();

	// 图片附件
	QList<bean::AttachItem> lstImageAttachs;
	QMap<QString, QPair<QString, QSize>> mapImages = ui->textEdit->msgImages();
	foreach (QString sFile, mapImages.keys())
	{
		bean::AttachItem attachItem(sFile);
		attachItem.setMessageType(m_type);
		attachItem.setUuid(mapImages[sFile].first);
		attachItem.setTransferType(bean::AttachItem::Type_AutoDisplay);
		attachItem.setFrom(uid);
		attachItem.setPicWidth(mapImages[sFile].second.width());
		attachItem.setPicHeight(mapImages[sFile].second.height());
		lstImageAttachs.append(attachItem);
	}

	// 校验数据
	if (lstImageAttachs.isEmpty() && lstFileAttachs.isEmpty() && sBody.length() <= 0)
	{
		showTip( tr("Please input message content"));
		return;
	}

	if (lstImageAttachs.count() > 10)
	{
		showTip(tr("No more than 10 images in one message"));
		return;
	}

	/*
	if (sBody.length() > 1024)
	{
		showTip(tr("The message content is too long, please split it"));
		return;
	}
	*/

	// 准备发送
	ui->textEdit->setText("");
	ui->textEdit->clear();
	const int MAX_BODY_LENGTH = 1024;

	// 文本图片消息
	if (!lstImageAttachs.isEmpty() || !sBody.trimmed().isEmpty())
	{
		// 图文消息分开发送
		foreach (QString msgText, msgPieces)
		{
			lstImageAttachs.clear();
			if (msgText.startsWith("{") && msgText.endsWith("}"))
			{
				QString imageUuid = msgText.mid(1, msgText.length()-2);
				foreach (QString sFile, mapImages.keys())
				{
					if (mapImages[sFile].first == imageUuid)
					{
						// regenerate image uuid
						QString sUuid = QUuid::createUuid().toString();
						sUuid = sUuid.mid(1, sUuid.length()-2);
						imageUuid = sUuid;
						msgText = QString("{%1}").arg(imageUuid);

						bean::AttachItem attachItem(sFile);
						attachItem.setMessageType(m_type);
						attachItem.setUuid(imageUuid);
						attachItem.setTransferType(bean::AttachItem::Type_AutoDisplay);
						attachItem.setFrom(uid);
						attachItem.setPicWidth(mapImages[sFile].second.width());
						attachItem.setPicHeight(mapImages[sFile].second.height());
						lstImageAttachs.append(attachItem);
						break;
					}
				}
			}

			do // 超过1024长度的消息需要分条发送
			{
				if (msgText.isEmpty())
					break;

				QString piece = msgText;
				if (msgText.length() > MAX_BODY_LENGTH)
				{
					piece = msgText.left(MAX_BODY_LENGTH);
					msgText = msgText.mid(MAX_BODY_LENGTH);
				}
				else
				{
					msgText.clear();
				}

				bean::MessageBody body = bean::MessageBodyFactory::createMessage(m_type);
				body.setFrom(uid);
				body.setFromName(name);
				body.setTo(m_id);
				body.setToName(m_name);
				body.setSend(true);
				body.setTime("");
				body.setBody(piece);
				body.setAttachs(lstImageAttachs);

				if (!atIds.isEmpty() && lstImageAttachs.isEmpty())
				{
					// get all the @ at this message piece
					QStringList pieceAtIds;
					QStringList pieceAtIdNames;
					QString atText = piece;
					int atIndex = atText.indexOf("@");
					while (atIndex != -1)
					{
						if (atIds.isEmpty() || atIdNames.isEmpty())
							break;

						pieceAtIds.append(atIds.takeFirst());
						pieceAtIdNames.append(atIdNames.takeFirst());

						atText = atText.mid(atIndex+1);
						atIndex = atText.indexOf("@");
					}

					// check if @ is empty @
					bool emptyAt = true;
					foreach (QString atId, pieceAtIds)
					{
						if (!atId.isEmpty())
						{
							emptyAt = false;
							break;
						}
					}

					// add at ext
					if (!emptyAt)
					{
						bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_At);
						QString sUuid = QUuid::createUuid().toString();
						sUuid = sUuid.mid(1, sUuid.length()-2);
						ext.setData("at", pieceAtIdNames.join(","));
						ext.setData("atid", sUuid);
						body.setExt(ext);
					}
					else
					{
						body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Chat));
					}
				}
				else
				{
					if (m_messageMode == Normal)
						body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Chat));
					else
						body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Secret));
				}

				if (m_msgSendDelegate)
				{
					m_msgSendDelegate->sendMessage(body);
				}
				else
				{
					qPmApp->getBuddyMgr()->slot_sendMessage(body);
				}

				m_pMessage4js->appendMessage(body);
			} while (1);
		}

		emit messageToSend();
	}

	// 附件消息
	if (!lstFileAttachs.isEmpty())
	{
		sendAttachMessages(lstFileAttachs);
	}
}

void MessagePannel::on_tBtnScreenshot_clicked()
{
	// active this window
	QApplication::setActiveWindow(this->window());

	emit doScreenshot();
}

void MessagePannel::on_tBtnCleanup_clicked()
{
	emit doClearMessages();
}

void MessagePannel::on_btnSendSetting_clicked()
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

void MessagePannel::on_btnExit_clicked()
{
	emit closeRequest();
}

void MessagePannel::slot_sendshortkey_changed(QAction* action)
{
	int nSendType = m_sendShortcutMenu.actions().indexOf(action);
	ui->btnSend->setToolTip(action->data().toString());

	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		accountSettings->setSendType(nSendType);
}

void MessagePannel::slot_screenshot_ok(const QString &imagePath)
{
	if (this->window()->isActiveWindow() && this->isVisible())
	{
		if (this->window()->isMinimized())
		{
			WidgetManager::showActivateRaiseWindow(this->window());
			QApplication::setActiveWindow(this->window());
		}

		ui->textEdit->insertImage(imagePath);
		ui->textEdit->setFocus();
	}
}

void MessagePannel::slot_screenshot_cancel()
{
	if (this->window()->isActiveWindow())
	{
		if (this->window()->isMinimized())
		{
			WidgetManager::showActivateRaiseWindow(this->window());
			QApplication::setActiveWindow(this->window());
		}

		ui->textEdit->setFocus();
	}
}

void MessagePannel::appendTipMessage(const QString &timeStr, const QString &msgText, const QString &level, 
								     const QString &action /*= QString()*/, const QString &param /*= QString()*/)
{
	m_pMessage4js->appendTipMessage(timeStr, msgText, level, action, param);
}

void MessagePannel::anchorAtMsg(const QString &atId)
{
	QUrl msgUrl = ui->webView->url();
	QString msgUrlStr = msgUrl.toString();
	int anchorIndex = msgUrlStr.indexOf("#");
	if (anchorIndex == -1)
	{
		msgUrlStr += "#" + atId;
	}
	else
	{
		msgUrlStr = msgUrlStr.left(anchorIndex) + "#" + atId;
	}

	msgUrl = QUrl::fromUserInput(msgUrlStr);
	ui->webView->load(msgUrl);
}

void MessagePannel::showTip(const QString &tip)
{
	ui->tipWidget->setTipText(tip);
	ui->tipWidget->autoShow();
}

void MessagePannel::onScreenShotMenuAboutToShow()
{
	m_screenShotMenu.clear();
	AccountSettings *accountSettings = Account::settings();
	m_screenShotMenu.addAction(tr("Screen Shot ") + accountSettings->getScreenshotKey(), this, SLOT(on_tBtnScreenshot_clicked()));
	
	QAction *hideShotAction = new QAction(tr("Hide window to shot screen"), &m_screenShotMenu);
	hideShotAction->setCheckable(true);
	hideShotAction->setChecked(accountSettings->hideToScreenshot());
	connect(hideShotAction, SIGNAL(toggled(bool)), this, SLOT(hideToScreenShotToggled(bool)));
	m_screenShotMenu.addAction(hideShotAction);
}

void MessagePannel::hideToScreenShotToggled(bool checked)
{
	AccountSettings *accountSettings = Account::settings();
	accountSettings->setHideToScreenshot(checked);
}

void MessagePannel::onRecordFinished(int recordId, const QString &fileName)
{
	if (recordId == m_recordId)
	{
		int recordTimeInSec = (ui->recordBar->recordTime()+999)/1000;

		// hide record bar
		hideRecordBar();

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			showTip(tr("You are offline, can't send audio message, please try when online"));
			return;
		}

		if (!checkBlackList(tr("send audio message")))
			return;

		// send message
		Account* pAccount = Account::instance();
		Q_ASSERT_X(pAccount != NULL, Q_FUNC_INFO, "account is null");

		QString uid = pAccount->id();
		QString name = qPmApp->getModelManager()->userName(uid);

		QList<bean::AttachItem> lstAttachs;
		bean::AttachItem attachItem(fileName);
		attachItem.setMessageType(m_type);
		attachItem.setFrom(uid);
		attachItem.setTransferType(bean::AttachItem::Type_AutoDownload);
		attachItem.setTime(recordTimeInSec);
		lstAttachs.append(attachItem);

		bean::MessageBody body = bean::MessageBodyFactory::createMessage(m_type);
		body.setFrom(uid);
		body.setFromName(name);
		body.setTo(m_id);
		body.setToName(m_name);
		body.setSend(true);
		body.setTime("");
		body.setBody("");
		body.setAttachs(lstAttachs);
		if (m_messageMode == Normal)
			body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Chat));
		else
			body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Secret));
	
		if (m_msgSendDelegate)
		{
			m_msgSendDelegate->sendMessage(body);
		}
		else
		{
			qPmApp->getBuddyMgr()->slot_sendMessage(body);
		}

		m_pMessage4js->appendMessage(body);

		emit messageToSend();
	}
}

void MessagePannel::onRecordCanceled(int recordId)
{
	if (recordId == m_recordId)
	{
		// hide record bar
		hideRecordBar();
	}
}

void MessagePannel::onRecordStartError(int recordId, const QString &desc)
{
	if (recordId == m_recordId)
	{
		// hide record bar
		hideRecordBar();

		// show error tip
		showTip(desc);
	}
}

void MessagePannel::onRecordTimeElapsed(int recordId, int timeInMs)
{
	if (recordId == m_recordId)
	{
		if (ui->recordBar->recordTime() == 0)
		{
			emit recordStart();
		}

		ui->recordBar->setRecordTime(timeInMs);
	}
}

void MessagePannel::onSendRecord()
{
	// stop record
	AmrRecord *amrRecord = qPmApp->getAmrRecord();
	amrRecord->stop();

	// hide record bar
	hideRecordBar();
}

void MessagePannel::onCancelRecord()
{
	// cancel record
	AmrRecord *amrRecord = qPmApp->getAmrRecord();
	amrRecord->cancel();

	// hide record bar
	hideRecordBar();
}

void MessagePannel::addInterphone()
{
	if (qPmApp->hasSession())
	{
		showAutoTip(tr("There is a audio/video call in progress, please try later"));
		return;
	}

	if (!checkBlackList(tr("enter interphone")))
		return;

	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
	QString interphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
		if (dlg->interphoneId() != interphoneId)
		{
			PMessageBox msgBox(PMessageBox::Question, tr("You have to quit previous interphone before enter this, continue"),
				QDialogButtonBox::Yes|QDialogButtonBox::No, tr("Interphone"), this);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			if (msgBox.clickedButton() != QDialogButtonBox::Yes)
			{
				return;
			}

			// in case this dialog is closed at other please, need this check, do not delete
			if (InterphoneDialog::hasInterphoneDialog())
			{
				dlg = InterphoneDialog::getInterphoneDialog();
				dlg->quitAndClose();
			}
		}
		else
		{
			WidgetManager::showActivateRaiseWindow(dlg);
			return;
		}
	}

	QString selfId = Account::instance()->id();
	interphoneManager->addInterphone(interphoneId, selfId);
}

void MessagePannel::openInterphone()
{
	QString interphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
		if (dlg->interphoneId() == interphoneId)
		{
			WidgetManager::showActivateRaiseWindow(dlg);
		}
		else
		{
			qWarning() << Q_FUNC_INFO << "open interphone with different id: " << interphoneId << dlg->interphoneId();
		}
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "open interphone but without dialog" << interphoneId;
	}
}

void MessagePannel::quitInterphone()
{
	QString interphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
		if (dlg->interphoneId() != interphoneId)
		{
			qWarning() << Q_FUNC_INFO << "quit interphone with different id: " << interphoneId << dlg->interphoneId(); 
		}
		else
		{
			dlg->quitAndClose();
		}
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "quit interphone but without interphone dialog" << interphoneId; 
	}
}

void MessagePannel::appendHistorySeparator()
{
	bean::MessageBody sepMsg = bean::MessageBodyFactory::createMessage(m_type);
	sepMsg.setExt(bean::MessageExtFactory::create(bean::MessageExt_HistorySep));
	m_pMessage4js->insertMessageToTop(sepMsg);
}

void MessagePannel::onWithdrawOK(bean::MessageType chatType, 
								 const QString &toId, 
								 const QString &fromId, 
								 const QString &timeStamp,
								 const QString &tipText,
								 const bean::MessageBody &origMsg)
{
	Q_UNUSED(fromId);
	if (m_type == chatType && m_id == toId)
	{
		// check if @ message
		if (origMsg.ext().type() == bean::MessageExt_At)
		{
			QString atUid = origMsg.ext().data("atid").toString();
			ui->atWidget->removeAtMsg(atUid);
		}

		// check if voice is playing
		if (!origMsg.attachs().isEmpty())
		{
			bean::AttachItem attach = origMsg.attachs().at(0);
			if (attach.transferType() == bean::AttachItem::Type_AutoDownload)
			{
				AmrPlayUtil::instance().stop(attach.uuid());
			}
		}

		emit messageWithdrawed(timeStamp, tipText);
	}
}

void MessagePannel::onWithdrawFailed(bean::MessageType chatType, 
									 const QString &toId, 
									 const QString &fromId, 
									 const QString &timeStamp)
{
	Q_UNUSED(fromId);
	Q_UNUSED(timeStamp);
	if (m_type == chatType && m_id == toId)
	{
		showMessageWithdrawTip(tr("Message withdraw failed, please try again"));
	}
}

void MessagePannel::onInterphoneStarted(const QString &interphoneId)
{
	QString thisInterphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (thisInterphoneId == interphoneId)
	{
		checkIfInInterphone();
	}
}

void MessagePannel::onInterphoneFinished(const QString &interphoneId)
{
	QString thisInterphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (thisInterphoneId == interphoneId)
	{
		checkIfInInterphone();
	}
}

void MessagePannel::onInterphoneChanged(const QString &interphoneId, int /*attachType*/, const QString & /*attachId*/)
{
	QString thisInterphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (thisInterphoneId == interphoneId)
	{
		checkIfInInterphone();
	}
}

void MessagePannel::onAddInterphoneFinished(bool ok, const QString &interphoneId)
{
	QString currentId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (currentId == interphoneId)
	{
		if (!ok)
		{
			PMessageBox msgBox(PMessageBox::Information, tr("Enter interphone failed, please try again"),
				QDialogButtonBox::Ok, tr("Interphone"), this);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			return;
		}

		// update interphone widget
		checkIfInInterphone();

		// show interphone dialog
		InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
		dlg->setInterphoneId(interphoneId);
		WidgetManager::showActivateRaiseWindow(dlg);

		if (!dlg->openChannel())
		{
			return;
		}
	}
}

void MessagePannel::onInterphoneCleared()
{
	checkIfInInterphone();
}

/*
void MessagePannel::onQuitInterphoneFinished(bool ok, const QString &interphoneId)
{
	QString currentId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
	if (currentId == interphoneId)
	{
		if (!ok)
		{
			PMessageBox msgBox(PMessageBox::Information, tr("Quit interphone failed, please try again"),
				QDialogButtonBox::Ok, tr("Interphone"), this);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			return;
		}

		// update interphone widget
		checkIfInInterphone();

		// close interphone dialog
		if (InterphoneDialog::hasInterphoneDialog())
		{
			InterphoneDialog *dlg = InterphoneDialog::getInterphoneDialog();
			dlg->close();
		}
	}
}
*/

void MessagePannel::onCopyActionTriggered()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString pathUrl = action->data().toString();
	if (pathUrl.isEmpty())
		return;

	m_pMessage4js->copyImage(pathUrl);
}

void MessagePannel::onSaveActionTriggered()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString pathUrl = action->data().toString();
	if (pathUrl.isEmpty())
		return;

	m_pMessage4js->saveImage(pathUrl);
}

void MessagePannel::addFavoriteEmotion()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString fileName = action->data().toString();
	if (fileName.isEmpty())
		return;

	QWidget *window = this->window();
	if (EmotionUtil::instance().addFavoriteEmotion(fileName, window))
	{
		showAddFavoritEmotionOKTip();
	}
}

void MessagePannel::withdrawMessage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString stamp = action->data().toString();
	if (stamp.isEmpty())
		return;

	CBuddyMgr *buddyMgr = qPmApp->getBuddyMgr();
	buddyMgr->addWithdrawMessage(stamp);

	MessageDBStore *messageDB = qPmApp->getMessageDBStore();
	qint64 seq = messageDB->getMessageByStamp(m_type, m_id, stamp);
	m_withdrawStamps.insert(seq, stamp);
}

void MessagePannel::copyMessage()
{
	if (!ui->webView->selectedText().isEmpty())
	{
		ui->webView->slot_copy_action_trigger();
		return;
	}

	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString stamp = action->data().toString();
	if (stamp.isEmpty())
		return;

	MessageDBStore *messageDB = qPmApp->getMessageDBStore();
	qint64 seq = messageDB->getMessageByStamp(m_type, m_id, stamp);
	m_copyStamps.insert(seq, stamp);
}

void MessagePannel::forwardMessage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString stamp = action->data().toString();
	if (stamp.isEmpty())
		return;

	MessageDBStore *messageDB = qPmApp->getMessageDBStore();
	qint64 seq = messageDB->getMessageByStamp(m_type, m_id, stamp);
	m_forwardStamps.insert(seq, stamp);
}

void MessagePannel::onSendSecretMessageRead(const QString &uid, const QString &stamp)
{
	if (uid == m_id && m_type == bean::Message_Chat)
	{
		emit sendSecretMessageRead(stamp);
	}
}

void MessagePannel::onRecvSecretMessageRead(const QString &uid, const QString &stamp)
{
	if (uid == m_id && m_type == bean::Message_Chat)
	{
		emit recvSecretMessageRead(stamp);
	}
}

void MessagePannel::onMessageSent(bean::MessageType msgType, const QString &id, const QString &seq, const QString &stamp)
{
	if (m_type == msgType && m_id == id)
	{
		emit messageSent(seq, stamp);
	}
}

void MessagePannel::onShortcutKeyChanged()
{
	AccountSettings *accountSettings = Account::settings();
	ui->tBtnScreenshot->setToolTip(tr("Screen Shot ") + accountSettings->getScreenshotKey());
}

void MessagePannel::onGotMessageOfStamp(qint64 seq, const QString &stamp, const bean::MessageBody &msg)
{
	if (m_withdrawStamps.contains(seq))
	{
		m_withdrawStamps.remove(seq);
		doMessageWithdraw(stamp, msg);
	}

	if (m_copyStamps.contains(seq))
	{
		m_copyStamps.remove(seq);
		doMessageCopy(msg);
	}

	if (m_forwardStamps.contains(seq))
	{
		m_forwardStamps.remove(seq);
		doMessageForward(msg);
	}
}

bool MessagePannel::resendFailedMessage(const QString &seq)
{	
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		showTip(tr("You are offline, can't send message, please try when online"));
		return false;
	}

	if (!checkBlackList(tr("send message")))
		return false;

	// resend failed message
	bean::MessageBody msgBody;
	if (qPmApp->getBuddyMgr()->resendFailedMessage(seq, &msgBody))
	{
		m_pMessage4js->appendMessage(msgBody);
		return true;
	}

	return false;
}

void MessagePannel::initUI()
{
	// secret switch
	ui->switchSecret->setVisible(false);

	// splitter
	m_chatSplitter = new MiniSplitter(this);
	m_chatSplitter->setOrientation(Qt::Vertical);
	m_chatSplitter->addWidget(ui->msgTopWidget);
	m_chatSplitter->addWidget(ui->msgBottomWidget);
	m_chatSplitter->setStretchFactor(0, 1);
	m_chatSplitter->setStretchFactor(1, 0);
	m_chatSplitter->setSizes(QList<int>() << 263 << 140);
	m_chatSplitter->setChildrenCollapsible(false);

	QVBoxLayout* splitterLayout = new QVBoxLayout(this);
	splitterLayout->addWidget(m_chatSplitter);
	splitterLayout->setSpacing(0);
	splitterLayout->setContentsMargins(0, 0, 1, 0);

	// at least 10s to send a shake
	m_shakingIntervalTimer = new QTimer(this);
	m_shakingIntervalTimer->setInterval(10000);
	connect(m_shakingIntervalTimer, SIGNAL(timeout()), m_shakingIntervalTimer, SLOT(stop()));

	// web view related
	m_chatWebPage = new ChatWebPage(ui->webView);
	connect(m_chatWebPage, SIGNAL(avatarClicked(QString)), this, SIGNAL(viewMaterial(QString)));
	ui->webView->setBrowserPage(m_chatWebPage);
	QString tag = QString("chatmsg(%1)_webview").arg(m_id);
	ui->webView->setTag(tag);
	ui->webView->setMenuDelegate(this);
	ui->webView->setImageDragDelegate(this);

	ui->webView->setUrl(QUrl("qrc:/html/messagelist.html"));
	m_pMessage4js = ui->webView->message4Js();
	m_pMessage4js->setMessageResendDelegate(this);
	connect(m_pMessage4js, SIGNAL(fetchHistoryMessage()), this, SIGNAL(fetchHistoryMsg()));
	connect(this, SIGNAL(cleanup()), m_pMessage4js, SIGNAL(cleanup()));
	connect(this, SIGNAL(initUIComplete()), m_pMessage4js, SIGNAL(initUIComplete()));

	connect(m_pMessage4js, SIGNAL(chat(QString)), SIGNAL(chat(QString)));
	connect(m_pMessage4js, SIGNAL(sendMail(QString)), SIGNAL(sendMail(QString)));
	connect(m_pMessage4js, SIGNAL(multiMail()), SIGNAL(multiMail()));
	connect(m_pMessage4js, SIGNAL(viewMaterial(QString)), SIGNAL(viewMaterial(QString)));
	connect(m_pMessage4js, SIGNAL(atTextClicked(QString)), ui->textEdit, SLOT(addAtText(QString)));
	connect(m_pMessage4js, SIGNAL(atTA(QString)), ui->textEdit, SLOT(addAtText(QString)));
	connect(this, SIGNAL(sendSecretMessageRead(QString)), m_pMessage4js, SIGNAL(setSendSecretRead(QString)));
	connect(this, SIGNAL(recvSecretMessageRead(QString)), m_pMessage4js, SIGNAL(onRecvSecretRead(QString)));
	connect(this, SIGNAL(messageSent(QString, QString)), m_pMessage4js, SIGNAL(messageSent(QString, QString)));
	connect(m_pMessage4js, SIGNAL(openHistoryMsg()), this, SIGNAL(openHistoryMsg()));
	connect(m_pMessage4js, SIGNAL(showMessageTip(QString)), this, SLOT(showTip(QString)));
	connect(this, SIGNAL(messageWithdrawed(QString, QString)), m_pMessage4js, SIGNAL(messageWithdrawed(QString, QString)));
	connect(m_chatWebPage, SIGNAL(avatarContextMenu(QPoint, QString, QString, QString)), 
		m_pMessage4js, SLOT(avatarContextMenu(QPoint, QString, QString, QString)));

	// tip widget
	ui->tipWidget->setTipPixmap(QPixmap(":/images/Icon_76.png"));
	ui->tipWidget->stopShow();

	// at widget
	ui->atWidget->setVisible(false);
	connect(ui->atWidget, SIGNAL(anchorAtMsg(QString)), this, SLOT(anchorAtMsg(QString)));

	// send signal
	ui->textEdit->setFaceSource(EmotionUtil::instance().emotionCodeNames(), EmotionUtil::instance().emotionFilePathes());
	connect(ui->textEdit, SIGNAL(sendMessage()), SLOT(on_btnSend_clicked()));
	connect(ui->textEdit, SIGNAL(showInfoTip(QString)), SLOT(showTip(QString)));

	// screen shot menu
	m_screenShotMenu.setObjectName(QString::fromLatin1("ScreenShotMenu"));
	connect(&m_screenShotMenu, SIGNAL(aboutToShow()), this, SLOT(onScreenShotMenuAboutToShow()));
	ui->tBtnScreenshot->setMenu(&m_screenShotMenu);
	AccountSettings *accountSettings = Account::settings();
	ui->tBtnScreenshot->setToolTip(tr("Screen Shot ") + accountSettings->getScreenshotKey());
	connect(qPmApp, SIGNAL(shortcutKeyChanged()), this, SLOT(onShortcutKeyChanged()));

	// attach button menu
	m_attachMenu.setObjectName(QString::fromLatin1("AttachMenu"));
	m_attachMenu.addAction(tr("Add File"), this, SLOT(insertAttachFiles()));
	m_attachMenu.addAction(tr("Add Dir"), this, SLOT(insertAttachDirs()));
	ui->btnAttach->setMenu(&m_attachMenu);

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

	ui->recordBar->setVisible(false);

	// interphone widget
	ui->interphoneWidget->setVisible(false);
	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
	connect(ui->interphoneWidget, SIGNAL(addInterphone()), this, SLOT(addInterphone()));
	connect(ui->interphoneWidget, SIGNAL(openInterphone()), this, SLOT(openInterphone()));
	connect(ui->interphoneWidget, SIGNAL(quitInterphone()), this, SLOT(quitInterphone()));
	connect(qPmApp->getBuddyMgr(), SIGNAL(interphoneStarted(QString)), this, SLOT(onInterphoneStarted(QString)));
	connect(qPmApp->getBuddyMgr(), SIGNAL(interphoneFinished(QString)), this, SLOT(onInterphoneFinished(QString)));
	connect(interphoneManager, SIGNAL(interphoneChanged(QString, int, QString)), this, SLOT(onInterphoneChanged(QString, int, QString)));
	connect(interphoneManager, SIGNAL(addInterphoneFinished(bool, QString)), this, SLOT(onAddInterphoneFinished(bool, QString)));
	// connect(interphoneManager, SIGNAL(quitInterphoneFinished(bool, QString)), this, SLOT(onQuitInterphoneFinished(bool, QString)));
	connect(interphoneManager, SIGNAL(interphonesCleared()), this, SLOT(onInterphoneCleared()));

	qDebug() << Q_FUNC_INFO << " end ";
}

bool MessagePannel::needShowAtMsg(const bean::MessageBody &rBody) const
{
	if (rBody.ext().type() == bean::MessageExt_At)
	{
		QString selfId = Account::instance()->id();
		QString atText = rBody.ext().data("at").toString();
		QStringList atUids = atText.split(",");
		foreach (QString atUid, atUids)
		{
			atUid = atUid.trimmed();
			if (atUid.startsWith(selfId) && (atUid.indexOf(":") == selfId.length()))
			{
				return true;
			}
		}
	}
	return false;
}

void MessagePannel::showAtMsg(const bean::MessageBody &rBody, bool top)
{
	if (top)
	{
		ui->atWidget->setTopAtMsg(rBody);
	}
	else
	{
		ui->atWidget->setBottomAtMsg(rBody);
	}
	ui->atWidget->setVisible(true);
}

void MessagePannel::hideAtMsg()
{
	ui->atWidget->hide();
}

bool MessagePannel::isAtMsgShown() const
{
	return ui->atWidget->isVisible();
}

void MessagePannel::showRecordBar()
{
	if (!ui->recordBar->isVisible())
	{
		QList<int> splitterSizes = m_chatSplitter->sizes();
		ui->recordBar->show();
		ui->recordBar->setRunning(true);
		ui->recordBar->setRecordTime(0);
		splitterSizes[0] = splitterSizes[0] - ui->recordBar->height();
		splitterSizes[1] = splitterSizes[1] + ui->recordBar->height();
		m_chatSplitter->setSizes(splitterSizes);
	}
}

void MessagePannel::hideRecordBar()
{
	if (ui->recordBar->isRunning())
	{
		QList<int> splitterSizes = m_chatSplitter->sizes();
		ui->recordBar->hide();
		ui->recordBar->setRunning(false);
		ui->recordBar->setRecordTime(0);
		splitterSizes[0] = splitterSizes[0] + ui->recordBar->height();
		splitterSizes[1] = splitterSizes[1] - ui->recordBar->height();
		m_chatSplitter->setSizes(splitterSizes);
	}
}

void MessagePannel::checkIfInInterphone()
{
	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
	if (interphoneManager->hasInterphone(m_type, m_id))
	{
		ui->interphoneWidget->setVisible(true);

		QString interphoneId = InterphoneManager::attachTypeId2InterphoneId(m_type, m_id);
		InterphoneInfo interphone = interphoneManager->interphone(interphoneId);
		if (interphoneManager->isInInterphone(m_type, m_id))
		{
			QString tip = tr("You are in interphone");
			ui->interphoneWidget->setInInterphone(true, tip);
		}
		else
		{
			QString tip;
			if (m_type == bean::Message_Chat)
			{
				tip = tr("%1 is in interphone").arg(m_name);
			}
			else
			{
				tip = tr("%1 members is in interphone").arg(interphone.memberCount());
			}
			ui->interphoneWidget->setInInterphone(false, tip);
		}
	}
	else
	{
		ui->interphoneWidget->setVisible(false);
	}
}

bool MessagePannel::checkBlackList(const QString &op)
{
	if (!qPmApp->getModelManager()->isInBlackList(m_id))
		return true;

	showTip(tr("He is in your blocked list, can't %1").arg(op));
	return false;
}

void MessagePannel::sendAttachMessages(const QStringList &rsFiles)
{
	if (rsFiles.isEmpty())
		return;

	foreach (QString file, rsFiles)
	{
		// send message, one message for one attach
		Account* pAccount = Account::instance();
		Q_ASSERT_X(pAccount != NULL, Q_FUNC_INFO, "account is null");

		QString uid = pAccount->id();
		QString name = qPmApp->getModelManager()->userName(uid);

		QList<bean::AttachItem> lstAttachs;
		bean::AttachItem attachItem(file);
		attachItem.setMessageType(m_type);
		attachItem.setFrom(uid);
		lstAttachs.append(attachItem);

		bean::MessageBody body = bean::MessageBodyFactory::createMessage(m_type);
		body.setFrom(uid);
		body.setFromName(name);
		body.setTo(m_id);
		body.setToName(m_name);
		body.setSend(true);
		body.setTime("");
		body.setBody("");
		body.setAttachs(lstAttachs);
		body.setExt(bean::MessageExtFactory::create(bean::MessageExt_Chat));

		if (m_msgSendDelegate)
		{
			m_msgSendDelegate->sendMessage(body);
		}
		else
		{
			qPmApp->getBuddyMgr()->slot_sendMessage(body);
		}

		m_pMessage4js->appendMessage(body);

		emit messageToSend();
	}
}

void MessagePannel::showMessageWithdrawTip(const QString &tipText)
{
	PMessageLineTip *lineTip = new PMessageLineTip(PMessageLineTip::Information, tipText, this->window());
	QRect rt(QPoint(), ui->webView->size());
	QPoint center = rt.center();
	QPoint globalCenter = ui->webView->mapToGlobal(center);
	QSize sz = lineTip->sizeHint();
	QPoint pt;
	pt.setX(globalCenter.x()-sz.width()/2);
	pt.setY(globalCenter.y()-sz.height()/2);
	WidgetManager::showActivateRaiseWindow(lineTip);
	lineTip->move(pt);
}

void MessagePannel::doMessageWithdraw(const QString &stamp, const bean::MessageBody &msg)
{
	CBuddyMgr *buddyMgr = qPmApp->getBuddyMgr();
	if (buddyMgr->containWithdrawMessage(stamp))
	{
		if (!msg.isValid())
		{
			buddyMgr->removeWithdrawMessage(stamp);
			showMessageWithdrawTip(tr("Can't withdraw, maybe this message does not send OK"));
			return;
		}

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			buddyMgr->removeWithdrawMessage(stamp);
			showMessageWithdrawTip(tr("Can't withdraw, you're offline"));
			return;
		}

		QDateTime curDateTime = CDateTime::currentDateTimeUtc();
		QDateTime msgDateTime = CDateTime::QDateTimeFromString(msg.time());
		msgDateTime.setTimeSpec(Qt::UTC);
		if (qAbs(msgDateTime.secsTo(curDateTime)) > 120) // more than 2 minutes
		{
			buddyMgr->removeWithdrawMessage(stamp);
			showMessageWithdrawTip(tr("Can't withdraw, this message is 2 minutes ago"));
			return;
		}

		buddyMgr->addWithdrawMessage(stamp, msg);
		qPmApp->getMessageWithdrawManager()->withdraw(m_type, m_id, Account::instance()->id(), stamp);
	}
}

void MessagePannel::doMessageCopy(const bean::MessageBody &msg)
{
	if (!msg.isValid())
		return;

	QList<bean::AttachItem> attachs = msg.attachs();
	if (!attachs.isEmpty())
	{
		bean::AttachItem attach = attachs[0];
		if (attach.transferResult() != bean::AttachItem::Transfer_Successful)
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Attach was not transfer OK, can't copy"));
			return;
		}

		QString filePath = attach.filepath();
		if (!QFile::exists(filePath))
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Attach was deleted or moved to other place, can't copy"));
			return;
		}
	}

	QString title;
	QString attachPath;
	QString dirPath;
	QString imagePath;
	QString msgText = msg.body();
	if (!attachs.isEmpty())
	{
		msgText.clear();

		bean::AttachItem attach = attachs[0];
		if (attach.transferType() == bean::AttachItem::Type_AutoDisplay)
		{
			imagePath = attach.filepath();
		}
		else if (attach.transferType() == bean::AttachItem::Type_Default)
		{
			attachPath = attach.filepath();
		}
		else if (attach.transferType() == bean::AttachItem::Type_Dir)
		{
			dirPath = attach.filepath();
		}
	}

	QMimeData *mimeData = CChatInputBox::msgToCopyMimeData(title, msgText, imagePath, attachPath, dirPath);
	if (mimeData)
	{
		QApplication::clipboard()->setMimeData(mimeData);
	}
}

void MessagePannel::doMessageForward(const bean::MessageBody &msg)
{
	if (!msg.isValid())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::warning(this->window(), tr("Tip"), tr("Can't forward, you're offline"));
		return;
	}

	QList<bean::AttachItem> attachs = msg.attachs();
	if (!attachs.isEmpty())
	{
		bean::AttachItem attach = attachs[0];
		if (attach.transferResult() != bean::AttachItem::Transfer_Successful)
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Attach was not transfer OK, can't forward"));
			return;
		}

		QString filePath = attach.filepath();
		if (!QFile::exists(filePath))
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Attach was deleted or moved to other place, can't forward"));
			return;
		}
	}

	SelectChatDialog dlg(tr("Forward"), this);
	if (dlg.exec())
	{
		bean::MessageType msgType;
		QString id;
		dlg.getSelect(msgType, id);

		if (msgType == bean::Message_Invalid || id.isEmpty())
			return;

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Can't forward, you're offline"));
			return;
		}

		qPmApp->getBuddyMgr()->forwardMessage(msgType, id, msg);

		showMessageForwardTip(tr("Message has been forwarded"));
	}
}

void MessagePannel::showMessageForwardTip(const QString &tipText)
{
	PMessageLineTip *lineTip = new PMessageLineTip(PMessageLineTip::Success, tipText, this->window());
	lineTip->setTipTimeout(2000); // show 2 seconds
	QRect rt(QPoint(), ui->webView->size());
	QPoint center = rt.center();
	center.setY(60);
	QPoint globalCenter = ui->webView->mapToGlobal(center);
	QSize sz = lineTip->sizeHint();
	QPoint pt;
	pt.setX(globalCenter.x()-sz.width()/2);
	pt.setY(globalCenter.y()-sz.height()/2);
	WidgetManager::showActivateRaiseWindow(lineTip);
	lineTip->move(pt);
}

void MessagePannel::showAddFavoritEmotionOKTip()
{
	PMessageLineTip *lineTip = new PMessageLineTip(PMessageLineTip::Success, tr("Favorite OK"), this->window());
	lineTip->setTipTimeout(2000); // show 2 seconds
	QRect rt(QPoint(), ui->webView->size());
	QPoint center = rt.center();
	center.setY(rt.bottom() - 48);
	QPoint globalCenter = ui->webView->mapToGlobal(center);
	QSize sz = lineTip->sizeHint();
	QPoint pt;
	pt.setX(globalCenter.x()-sz.width()/2);
	pt.setY(globalCenter.y()-sz.height()/2);
	WidgetManager::showActivateRaiseWindow(lineTip);
	lineTip->move(pt);
}