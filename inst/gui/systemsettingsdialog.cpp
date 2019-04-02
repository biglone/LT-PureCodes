#include "systemsettingsdialog.h"
#include "ui_systemsettingsdialog.h"
#include <QStringListModel>
#include "commonlistitemdelegate.h"
#include "settings/AccountSettings.h"
#include "Account.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QItemSelectionModel>
#include <QTimer>
#include <QAudioDeviceInfo>
#include <QSound>
#include <QDebug>
#include "pmessagebox.h"
#include "plaintextinput.h"
#include "util/PlayBeep.h"
#include "util/FileDialog.h"

SystemSettingsDialog *SystemSettingsDialog::s_instance = 0;

SystemSettingsDialog *SystemSettingsDialog::getInstance()
{
	if (!s_instance)
		s_instance = new SystemSettingsDialog();
	return s_instance;
}

SystemSettingsDialog::SystemSettingsDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::SystemSettingsDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(524, 392);
	setFixedSize(524, 392);
	setResizeable(false);

	initUI();

	initSignals();

	setSkin();

	s_instance = this;
}

SystemSettingsDialog::~SystemSettingsDialog()
{
	s_instance = 0;

	delete ui;
}

void SystemSettingsDialog::setSettingIndex(SettingIndex index)
{
	if (index >= 0 && index < ui->stackedWidget->count())
	{
		ui->listViewPages->setCurrentIndex(ui->listViewPages->model()->index((int)index, 0));
		ui->stackedWidget->setCurrentIndex(index);
	}
}

void SystemSettingsDialog::setShotKeyConflict(bool conflict)
{
	if (conflict)
	{
		ui->tagShot->setStyleSheet("color: red;");
	}
	else
	{
		ui->tagShot->setStyleSheet("color: #333333;");
	}
}

void SystemSettingsDialog::setTakeMsgKeyConflict(bool conflict)
{
	if (conflict)
	{
		ui->tagTakeMsg->setStyleSheet("color: red;");
	}
	else
	{
		ui->tagTakeMsg->setStyleSheet("color: #333333;");
	}
}

void SystemSettingsDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_8.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 31;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// set list view style
	ui->listViewPages->setStyleSheet("QListView#listViewPages {background: transparent; border: none;}");
	ui->leftPanel->setStyleSheet("QWidget#leftPanel {background: rgb(232, 232, 232); border: none;}");

	// set font & color
	ui->frame1->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frame2->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frame3->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frame4->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frame5->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frame6->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frame7->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->frameSoundSetting->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");
	ui->labelUnionChatCloseTip->setStyleSheet("color: rgb(0, 120, 216);");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pushButtonApply->setStyleSheet(qss);
		ui->pushButtonOK->setStyleSheet(qss);
		ui->pushButtonClose->setStyleSheet(qss);
		qssFile.close();
	}

	QStringList fileNames;
	fileNames << ":/theme/qss/lineedit2_skin.qss";
	QString dlgQss;
	foreach (QString fileName, fileNames)
	{
		qssFile.setFileName(fileName);
		if (qssFile.open(QIODevice::ReadOnly))
		{
			QString qss = qssFile.readAll();
			dlgQss += qss;
			qssFile.close();
		}
	}
	setStyleSheet(dlgQss);
}

void SystemSettingsDialog::onPageChanged(const QModelIndex &index)
{
	int row = index.row();
	ui->stackedWidget->setCurrentIndex(row);

	if (row == IndexChatWindow)
	{
		ui->labelUnionChatCloseTip->setVisible(false);
	}
}

void SystemSettingsDialog::saveSettings()
{
	// apply all the settings
	applyNormalSettings();
	applyChatWindowSettings();
	applyFileSettings();
	applyShortcutSettings();
	applySoundSettings();
	// close settings dialog
	close();
}

void SystemSettingsDialog::applySettings()
{
	if (ui->stackedWidget->currentIndex() == IndexNormal)
	{
		applyNormalSettings();
	}
	else if (ui->stackedWidget->currentIndex() == IndexChatWindow)
	{
		applyChatWindowSettings();
	}
	else if (ui->stackedWidget->currentIndex() == IndexFile)
	{
		applyFileSettings();
	}
	else if (ui->stackedWidget->currentIndex() == IndexShortcut)
	{
		applyShortcutSettings();
	}
	else if (ui->stackedWidget->currentIndex() == IndexSound)
	{
		applySoundSettings();
	}
}

void SystemSettingsDialog::openFileSaveDir()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(ui->lineEditFileSave->text()));
}

void SystemSettingsDialog::changeFileSaveDir()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Dir"), ui->lineEditFileSave->text());
	if (!dir.isEmpty())
		ui->lineEditFileSave->setText(dir);
}

void SystemSettingsDialog::onShotKeyChanged(const QString &text)
{
	if (!text.isEmpty())
	{
		QString takeMsgKey = this->takeMsgKey();
		if (text == takeMsgKey)
		{
			PMessageBox::information(this, tr("Tip"), tr("The hot keys of screen shot and message taken conflict, please set another key."));
			ui->lineEditShotNew->clearKeySequence();
		}
	}
}

void SystemSettingsDialog::onTakeMsgKeyChanged(const QString &text)
{
	if (!text.isEmpty())
	{
		QString shotKey = this->shotKey();
		if (text == shotKey)
		{
			PMessageBox::information(this, tr("Tip"), tr("The hot keys of message taken and screen shot conflict, please set another key."));
			ui->lineEditTakeMsgNew->clearKeySequence();
		}
	}
}

void SystemSettingsDialog::onPushButtonRecoverChatCloseClicked()
{
	Account::settings()->setChatAlwaysCloseAll(false);
	ui->labelUnionChatCloseTip->setVisible(true);
	QTimer::singleShot(3000, ui->labelUnionChatCloseTip, SLOT(hide()));
}

void SystemSettingsDialog::onMuteStateChanged(int state)
{
	if (state)
	{
		ui->checkBoxBuddyBeepOn->setEnabled(false);
		ui->checkBoxSubscriptionBeepOn->setEnabled(false);
	}
	else
	{
		ui->checkBoxBuddyBeepOn->setEnabled(true);
		ui->checkBoxSubscriptionBeepOn->setEnabled(true);
	}
	PlayBeep::setMute(state);
}

void SystemSettingsDialog::onBuddyMuteStateChanged(int state)
{
	if (state)
	{
		PlayBeep::setBuddyMute(false);
	}
	else
	{
		PlayBeep::setBuddyMute(true);
	}
}

void SystemSettingsDialog::onSubscriptionMuteStateChanged(int state)
{
	if (state)
	{
		PlayBeep::setSubscriptionMute(false);
	}
	else
	{
		PlayBeep::setSubscriptionMute(true);
	}
}

void SystemSettingsDialog::onBuddyMsgAuditionClicked()
{
	audition(ui->lineEditBuddyMsgPromptFile->text());
}

void SystemSettingsDialog::onBuddyMsgChangerClicked()
{
	QString fileName;
	QString dirPath = QCoreApplication::applicationDirPath()+QString("/Misc/sounds");

	// filter
	QString filterTag = tr("Sound Files");
	wchar_t szFilter[256];
	memset(szFilter, 0, sizeof(szFilter));
	int len = filterTag.toWCharArray(szFilter);
	wchar_t szFilterSuffix[] = L"(*.wav)\0*.WAV\0\0";
	memcpy((char *)(szFilter+len), (char *)szFilterSuffix, sizeof(szFilterSuffix));

	fileName = FileDialog::getOpenFileName(this, tr("Change Sound"), dirPath, szFilter);
	if (fileName.isEmpty())
		return;
	
	fileName = QDir::toNativeSeparators(fileName);
	ui->lineEditBuddyMsgPromptFile->setText(fileName);
}

void SystemSettingsDialog::onSubscriptionMsgAudtionClicked()
{
	audition(ui->lineEditSubscriptionMsgPromptFile->text());
}

void SystemSettingsDialog::onSubscriptionMsgChangerClicked()
{
	QString fileName;
	QString dirPath = QCoreApplication::applicationDirPath()+QString("/Misc/sounds");

	// filter
	QString filterTag = tr("Sound Files");
	wchar_t szFilter[256];
	memset(szFilter, 0, sizeof(szFilter));
	int len = filterTag.toWCharArray(szFilter);
	wchar_t szFilterSuffix[] = L"(*.wav)\0*.WAV\0\0";
	memcpy((char *)(szFilter+len), (char *)szFilterSuffix, sizeof(szFilterSuffix));

	fileName = FileDialog::getOpenFileName(this, tr("Change Sound"), dirPath, szFilter);
	if (fileName.isEmpty())
		return;

	fileName = QDir::toNativeSeparators(fileName);
	ui->lineEditSubscriptionMsgPromptFile->setText(fileName);
}

void SystemSettingsDialog::initUI()
{
	// page list view
	ui->listViewPages->setEditTriggers(QListView::NoEditTriggers);

	QStringListModel *pagesListModel = new QStringListModel(ui->listViewPages);
	pagesListModel->setStringList(QStringList() << tr("General") << tr("Message Window") << tr("File Transfer") << tr("Hot Key") << tr("Sound"));
	ui->listViewPages->setModel(pagesListModel);

	CommonListItemDelegate *pagesListDelegate = new CommonListItemDelegate(ui->listViewPages);
	ui->listViewPages->setItemDelegate(pagesListDelegate);

	ui->listViewPages->setCurrentIndex(pagesListModel->index(0));
	ui->listViewPages->selectionModel()->select(pagesListModel->index(0), QItemSelectionModel::ClearAndSelect);
	ui->stackedWidget->setCurrentIndex(0);

	ui->lineEditShotNew->setPlaceholderText(tr("None"));
	ui->lineEditTakeMsgNew->setPlaceholderText(tr("None"));

	ui->pushButtonApply->setVisible(false);

	// load settings
	loadNormalSettings();
	loadChatWindowSettings();
	loadFileSettings();
	loadShortcutSettings();
	loadSoundSettings();
}

void SystemSettingsDialog::initSignals()
{
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(applySettings()));
	connect(ui->listViewPages, SIGNAL(clicked(QModelIndex)), this, SLOT(onPageChanged(QModelIndex)));
	connect(ui->pushButtonOpenDir, SIGNAL(clicked()), SLOT(openFileSaveDir()));
	connect(ui->pushButtonChangeDir, SIGNAL(clicked()), SLOT(changeFileSaveDir()));
	connect(ui->lineEditShotNew, SIGNAL(textChanged(QString)), SLOT(onShotKeyChanged(QString)));
	connect(ui->lineEditTakeMsgNew, SIGNAL(textChanged(QString)), SLOT(onTakeMsgKeyChanged(QString)));
	connect(ui->pushButtonRecoverChatClose, SIGNAL(clicked()), SLOT(onPushButtonRecoverChatCloseClicked()));
	connect(ui->pushButtonBuddyMsgAuditon, SIGNAL(clicked()), this, SLOT(onBuddyMsgAuditionClicked()));
	connect(ui->pushButtonBuddyMsgChanger, SIGNAL(clicked()), this, SLOT(onBuddyMsgChangerClicked()));
	connect(ui->pushButtonSubscriptionMsgAuditon, SIGNAL(clicked()), this, SLOT(onSubscriptionMsgAudtionClicked()));
	connect(ui->pushButtonSubscriptionMsgChanger, SIGNAL(clicked()), this, SLOT(onSubscriptionMsgChangerClicked()));
}

void SystemSettingsDialog::loadNormalSettings()
{
	AccountSettings *settings = Account::settings();
	bool isAutoRun = settings->isAutoRun();
	ui->checkBoxStart->setChecked(isAutoRun);

	bool bCloseHide = settings->isPscCloseHide();
	ui->radioButtonPanelHide->setChecked(bCloseHide);
	ui->radioButtonPanelExit->setChecked(!bCloseHide);

	bool bTopmost = settings->isPscTopmost();
	ui->checkBoxPanelTopmost->setChecked(bTopmost);

	bool bEdgeHide = settings->isPscEdgeHide();
	ui->checkBoxPanelEdgeHide->setChecked(bEdgeHide);

	bool bClearMsg = settings->clearMsgWhenClose();
	ui->checkBoxDeleteMsgs->setChecked(bClearMsg);
}

void SystemSettingsDialog::loadChatWindowSettings()
{
	AccountSettings *settings = Account::settings();
	bool isLoadHistory = settings->chatLoadHistory();
	ui->checkboxChatLoadHistory->setChecked(isLoadHistory);

	bool unreadBoxAutoShow = settings->unreadBoxAutoShow();
	ui->checkboxUnreadBoxAutoShow->setChecked(unreadBoxAutoShow);
}

void SystemSettingsDialog::loadFileSettings()
{
	QString downloadPath = Account::instance()->attachPath();
	downloadPath = QDir::toNativeSeparators(downloadPath);
	ui->lineEditFileSave->setText(downloadPath);
}

void SystemSettingsDialog::loadShortcutSettings()
{
	AccountSettings *settings = Account::settings();
	int sendType = settings->getSendType();
	if (sendType == 0)
	{
		ui->radioButtonChat1->setChecked(true);
		ui->radioButtonChat2->setChecked(false);
	}
	else
	{
		ui->radioButtonChat1->setChecked(false);
		ui->radioButtonChat2->setChecked(true);
	}

	QString shotkey = settings->getScreenshotKey();
	QString defaultShotkey = settings->defaultScreenshotKey();
	if (shotkey == defaultShotkey)
	{
		ui->radioButtonShotDefault->setChecked(true);
		ui->radioButtonShotNew->setChecked(false);
	}
	else
	{
		ui->radioButtonShotDefault->setChecked(false);
		ui->radioButtonShotNew->setChecked(true);
	}
	ui->lineEditShotNew->setText(shotkey);

	QString takeMsgkey = settings->getTakeMsgKey();
	QString defaultTakeMsgkey = settings->defaultTakeMsgKey();
	if (takeMsgkey == defaultTakeMsgkey)
	{
		ui->radioButtonTakeMsgDefault->setChecked(true);
		ui->radioButtonTakeMsgNew->setChecked(false);
	}
	else
	{
		ui->radioButtonTakeMsgDefault->setChecked(false);
		ui->radioButtonTakeMsgNew->setChecked(true);
	}
	ui->lineEditTakeMsgNew->setText(takeMsgkey);
}

void SystemSettingsDialog::loadSoundSettings()
{
	AccountSettings *settings = Account::settings();
	bool muteOn = settings->messagePromptMute();
	if (muteOn)
	{
		ui->checkBoxMute->setChecked(true);
		ui->checkBoxBuddyBeepOn->setEnabled(false);
		ui->checkBoxSubscriptionBeepOn->setEnabled(false);
	}
	else
	{
		ui->checkBoxMute->setChecked(false);
		ui->checkBoxBuddyBeepOn->setEnabled(true);
		ui->checkBoxSubscriptionBeepOn->setEnabled(true);
	}

	ui->checkBoxBuddyBeepOn->setChecked(!settings->buddyMsgMuteOn());
	ui->checkBoxSubscriptionBeepOn->setChecked(!settings->subscriptionMsgMuteOn());
	
	ui->lineEditBuddyMsgPromptFile->setText(settings->buddyMsgPromptFile());
	ui->lineEditSubscriptionMsgPromptFile->setText(settings->subscriptionMsgPromptFile());

	connect(ui->checkBoxMute, SIGNAL(stateChanged(int)), this, SLOT(onMuteStateChanged(int)));
	connect(ui->checkBoxBuddyBeepOn, SIGNAL(stateChanged(int)), this, SLOT(onBuddyMuteStateChanged(int)));
	connect(ui->checkBoxSubscriptionBeepOn, SIGNAL(stateChanged(int)), this, SLOT(onSubscriptionMuteStateChanged(int)));
}

void SystemSettingsDialog::applyNormalSettings()
{
	AccountSettings *settings = Account::settings();
	bool isAutoRun = ui->checkBoxStart->isChecked();
	settings->setAutoRun(isAutoRun);

	bool closeHide = ui->radioButtonPanelHide->isChecked();
	settings->setPscCloseHide(closeHide);

	bool topmost = ui->checkBoxPanelTopmost->isChecked();
	settings->setPscTopmost(topmost);
	emit mainPanelTopmost(topmost);

	bool edgeHide = ui->checkBoxPanelEdgeHide->isChecked();
	settings->setPscEdgeHide(edgeHide);

	bool clearMsg = ui->checkBoxDeleteMsgs->isChecked();
	settings->setClearMsgWhenClose(clearMsg);
}

void SystemSettingsDialog::applyChatWindowSettings()
{
	AccountSettings *settings = Account::settings();
	bool isLoadHistory = ui->checkboxChatLoadHistory->isChecked();
	settings->setChatLoadHistory(isLoadHistory);

	bool unreadBoxAutoShow = ui->checkboxUnreadBoxAutoShow->isChecked();
	settings->setUnreadBoxAutoShow(unreadBoxAutoShow);
}

void SystemSettingsDialog::applyFileSettings()
{
	AccountSettings *settings = Account::settings();
	QString downloadDir = ui->lineEditFileSave->text();
	settings->setDownloadDir(downloadDir);
}

void SystemSettingsDialog::applyShortcutSettings()
{
	AccountSettings *settings = Account::settings();
	int sendType = ui->radioButtonChat1->isChecked() ? 0 : 1;
	settings->setSendType(sendType);

	QString shotKey = this->shotKey();
	settings->setScreenshotKey(shotKey); 

	QString takeMsgKey = this->takeMsgKey();
	settings->setTakeMsgKey(takeMsgKey);

	emit shortcutKeyApplied();
}

void SystemSettingsDialog::applySoundSettings()
{
	AccountSettings *settings = Account::settings();
	settings->setMessagePromptMute(ui->checkBoxMute->isChecked());

	settings->setBuddyMsgMuteOn(!ui->checkBoxBuddyBeepOn->isChecked());
	settings->setSubscriptionMsgMuteOn(!ui->checkBoxSubscriptionBeepOn->isChecked());

	QString buddyMsgPromptFile = ui->lineEditBuddyMsgPromptFile->text();
	settings->setBuddyMsgPromptFile(buddyMsgPromptFile);
	PlayBeep::setBuddyFilePath(buddyMsgPromptFile);

	QString subscriptionMsgPromptFile = ui->lineEditSubscriptionMsgPromptFile->text();
	settings->setSubscriptionMsgPromptFile(subscriptionMsgPromptFile);
	PlayBeep::setSubscriptionFilePath(subscriptionMsgPromptFile);
}

QString SystemSettingsDialog::shotKey() const
{
	AccountSettings *settings = Account::settings();
	QString defaultShotkey = settings->defaultScreenshotKey();
	QString shotKey = defaultShotkey;
	if (ui->radioButtonShotNew->isChecked())
	{
		shotKey = ui->lineEditShotNew->text();
	}
	return shotKey;
}

QString SystemSettingsDialog::takeMsgKey() const
{
	AccountSettings *settings = Account::settings();
	QString defaultTakeMsgkey = settings->defaultTakeMsgKey();
	QString takeMsgKey = defaultTakeMsgkey;
	if (ui->radioButtonTakeMsgNew->isChecked())
	{
		takeMsgKey = ui->lineEditTakeMsgNew->text();
	}
	return takeMsgKey;
}

void SystemSettingsDialog::audition(const QString &file) const
{
	if (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "QSound is not available";
		return;
	}

	if (file.isEmpty() || !QFile::exists(file))
	{
		qWarning() << Q_FUNC_INFO << "QSound play file is not exists: " << file;
		return;
	}

	QSound::play(file);
}
