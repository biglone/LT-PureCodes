#include "sessionvideodialog.h"
#include "ui_sessionvideodialog.h"
#include "rtc/rtcsession.h"
#include "session/VideoFrameCB.h"
#include <QTimer>
#include "pmessagebox.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QDebug>
#include "buddymgr.h"
#include "Account.h"
//#include "microphonecontrolpanel.h"
#include "chatdialog.h"
#include "rtc/rtcsessionmanager.h"
//#include "rtc/volumecontrolpanel.h"
#include <QDateTime>
#include <QStandardPaths>

static const int kMinVolume   = 1;
static const int kMaxVolume   = 10;
static const int kMuteVolume  = 0;

SessionVideoDialog::SessionVideoDialog(const QString &id, rtcsession::Session *pSession, QWidget *parent)
	: FramelessDialog(parent)
	, m_id(id)
	, m_pSession(pSession)
	, m_elapsedTimerId(0)
	, m_elapsedSeconds(0)
	, m_currentVolume(kMaxVolume)
{
	ui = new Ui::SessionVideoDialog();
	ui->setupUi(this); 

	setSupportTranslusent(false);
	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setMainLayout(ui->verticalLayoutMain);
	setResizeable(true);
	setMaximizeable(true);

	initUI();

	resize(556, 447);
	setMinimumSize(556, 447);

	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(this, SIGNAL(maximizeStateChanged(bool)), this, SLOT(onMaximizeStateChanged(bool)));
	connect(ui->btnVideoBye, SIGNAL(clicked()), this, SLOT(onBtnVideoByeClicked()));
	connect(ui->btnStopAudio, SIGNAL(clicked(bool)), this, SLOT(onBtnStopAudioClicked(bool)));
	connect(ui->btnVolume, SIGNAL(clicked(bool)), this, SLOT(onBtnVolumeClicked(bool)));
	connect(ui->btnStopVideo, SIGNAL(clicked(bool)), this, SLOT(onBtnStopVideoClicked(bool)));
	connect(ui->btnChat, SIGNAL(clicked()), this, SLOT(onBtnChatClicked()));
	connect(ui->btnFullScrean, SIGNAL(toggled(bool)), this, SLOT(onBtnFullScreanToggled(bool)));
	connect(ui->videoCanvas, SIGNAL(videoDoubleClicked()), this, SLOT(onBigVideoDoubleClicked()));
	connect(ui->videoCanvas, SIGNAL(videoHasEvent()), this, SLOT(onVideoHasEvent()));
	connect(ui->btnPhoto, SIGNAL(clicked()), this, SLOT(onBtnPhotoClicked()));
	connect(ui->btnShowMode, SIGNAL(clicked(bool)), this, SLOT(onBtnShowModeClicked(bool)));
	connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));

	connect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));
	connect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	connect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	connect(m_pSession, SIGNAL(audioSendChanged(bool)), this, SLOT(onAudioSendChanged(bool)));
	connect(m_pSession, SIGNAL(videoSendChanged(bool)), this, SLOT(onVideoSendChanged(bool)));
	connect(m_pSession, SIGNAL(otherAudioChanged(bool, bool)), this, SLOT(onOtherAudioChanged(bool, bool)));
	connect(m_pSession, SIGNAL(otherVideoChanged(bool, bool)), this, SLOT(onOtherVideoChanged(bool, bool)));
	connect(m_pSession, SIGNAL(packageStatistics(int, int)), this, SLOT(onPackageStatistics(int, int)));
	/*
	connect(ui->btnVolume, SIGNAL(enter()), this, SLOT(onEnterBtnVolume()));
	connect(ui->btnVolume, SIGNAL(leave()), this, SLOT(onLeaveBtnVolume()));
	*/
	// start elapsed timer
	startElapsedTimer();

	setFocus();
}

SessionVideoDialog::~SessionVideoDialog()
{
	delete ui;
}

void SessionVideoDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/session/session_bg.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 2;
	bgSizes.topBarHeight = 30;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {color: white;}");

	// set elapse time label style
	ui->labElapseTime->setStyleSheet("QLabel {color: white;}");

	ui->bottomBar->setStyleSheet("QWidget#bottomBar {background: #4c333333;}");

	ui->sliderVolume->setStyleSheet(
		"QSlider::groove:horizontal {"
		"	background: rgb(0, 120, 216);"
		"	height: 4px;"
		"}"
		"QSlider::handle:horizontal {"
		"	width: 10px;"
		"	border: 1px solid rgb(223, 223, 223);"
		"	background: white;"
		"	margin: -5px 0;"
		"	border-radius: 5px;"
		"}"
		"QSlider::sub-page:horizontal {"
		"	background: rgb(0, 120, 216);"
		"}"
		"QSlider::add-page:horizontal {"
		"	background: rgb(196, 196, 196);"
		"}"
		"QSlider::sub-page:horizontal:disabled {"
		"	background: rgb(196, 196, 196);"
		"}"
		);

	// stop audio button
	{
		QIcon icon;
		icon.addPixmap(QPixmap(":/session/lvoice2_white.png"));
		icon.addPixmap(QPixmap(":/session/lvoice2_r_white.png"), QIcon::Active, QIcon::On);
		ui->btnStopAudio->setIcon(icon);
	}

	// volume button
	{
		QIcon icon;
		icon.addPixmap(QPixmap(":/session/lvoice_white.png"));
		icon.addPixmap(QPixmap(":/session/lvoice_r_white.png"), QIcon::Active, QIcon::On);
		ui->btnVolume->setIcon(icon);
	}

	// stop video button
	{
		QIcon icon;
		icon.addPixmap(QPixmap(""));
		icon.addPixmap(QPixmap(":/session/lcamera_white.png"));
		icon.addPixmap(QPixmap(":/session/lcamera_r_white.png"), QIcon::Active, QIcon::On);
		ui->btnStopVideo->setIcon(icon);
	}

	// full screen button
	{
		QIcon icon;
		icon.addPixmap(QPixmap(""));
		icon.addPixmap(QPixmap(":/session/maximun_white.png"));
		icon.addPixmap(QPixmap(":/session/restore_white.png"), QIcon::Active, QIcon::On);
		ui->btnFullScrean->setIcon(icon);
	}

	// show mode button
	{
		QIcon icon;
		icon.addPixmap(QPixmap(""));
		icon.addPixmap(QPixmap(":/session/self_down.png"));
		icon.addPixmap(QPixmap(":/session/self_right.png"), QIcon::Active, QIcon::On);
		ui->btnShowMode->setIcon(icon);
	}

	QString checkedBtnStyle = QString(
		"QToolButton:checked{"
		"border-image: none;"
		"}"
		"QToolButton:checked:hover:!pressed{"
		"border-image: none;"
		"border: 1px solid rgb(173, 173, 173);"
		"}"
		"QToolButton:checked:hover:pressed{"
		"border-image: none;"
		"border: 1px solid rgb(173, 173, 173);"
		"}"
		);
	ui->btnStopAudio->setStyleSheet(checkedBtnStyle);
	ui->btnVolume->setStyleSheet(checkedBtnStyle);
	ui->btnStopVideo->setStyleSheet(checkedBtnStyle);
	ui->btnFullScrean->setStyleSheet(checkedBtnStyle);
	ui->btnShowMode->setStyleSheet(checkedBtnStyle);
}

void SessionVideoDialog::timerEvent(QTimerEvent *event)
{
	if (m_elapsedTimerId != event->timerId())
	{
		return;
	}

	++m_elapsedSeconds;
	QString str = QString("%1:%2:%3")
		.arg(m_elapsedSeconds/3600, 2, 10, QChar('0'))
		.arg((m_elapsedSeconds/60)%60, 2, 10, QChar('0'))
		.arg(m_elapsedSeconds%60, 2, 10, QChar('0'));

	ui->labElapseTime->setText(str);
}

void SessionVideoDialog::keyPressEvent( QKeyEvent *event )
{
	if (Qt::Key_Escape == event->key())
	{
		onFullScreenEscape();
		return;
	}
	QWidget::keyPressEvent(event);
}

void SessionVideoDialog::closeEvent(QCloseEvent *event)
{
	qDebug() << Q_FUNC_INFO << "begin: (" << ((void *)this) << ")"; 

	if (m_pSession.isNull())
	{
		event->accept();
		return;
	}

	if (!(qPmApp->isShutingDown() || qPmApp->isLogout())) // if not logout or exit, need query to close session
	{
		// query if close this video session
		QDialogButtonBox::StandardButton sb = PMessageBox::information(this, tr("Tip"), 
			tr("If you close window, %1 will be terminated, continue").arg(ui->title->text()), 
			QDialogButtonBox::Yes|QDialogButtonBox::No);
		if (QDialogButtonBox::No == sb || QDialogButtonBox::Cancel == sb)
		{
			event->ignore();
			return;
		}
	}

	// close session
	if (!m_pSession.isNull())
	{
		disconnect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));
	
		qDebug() << Q_FUNC_INFO << "close session: (" << ((void *)m_pSession.data()) << ")";

		m_pSession->close(rtcsession::Session::CloseNormal);
	}
	
	event->accept();

	qDebug() << Q_FUNC_INFO << "end: (" << ((void *)this) << ")"; 
}

void SessionVideoDialog::enterEvent(QEvent *e)
{
	m_bottomHideTimer.stop();
	ui->bottomBar->setVisible(true);
	ui->videoCanvas->showSelfBottomMargin();

	FramelessDialog::enterEvent(e);
}

void SessionVideoDialog::leaveEvent(QEvent *e)
{
	FramelessDialog::leaveEvent(e);

	m_bottomHideTimer.stop();
	m_bottomHideTimer.start();
}

void SessionVideoDialog::onBtnVideoByeClicked()
{
	closeSession();
}

void SessionVideoDialog::onBtnStopAudioClicked( bool checked )
{
	m_pSession->setAudioSendEnable(!checked);
	if (checked)
	{
		ui->btnStopAudio->setToolTip(tr("Recover Microphone"));
	}
	else
	{
		ui->btnStopAudio->setToolTip(tr("Mute Microphone"));
	}
}

void SessionVideoDialog::onBtnVolumeClicked(bool checked)
{
	if (checked)
	{
		m_pSession->setAudioVolume(kMuteVolume);
		ui->btnVolume->setToolTip(tr("Recover Volume"));
		/*
		if (m_volumeControlPanel.data() && m_volumeControlPanel.data()->isVisible())
			m_volumeControlPanel->setEnable(false);
			*/
		ui->sliderVolume->setEnabled(false);
	}
	else
	{
		m_pSession->setAudioVolume(m_currentVolume);
		ui->btnVolume->setToolTip(tr("Mute Volume"));
		/*
		if (m_volumeControlPanel.data() && m_volumeControlPanel.data()->isVisible())
			m_volumeControlPanel->setEnable(true);
			*/
		ui->sliderVolume->setEnabled(true);
	}
}

void SessionVideoDialog::onBtnStopVideoClicked( bool checked )
{
	m_pSession->setVideoSendEnable(!checked);
	if (checked)
	{
		ui->btnStopVideo->setToolTip(tr("Open Camera"));
		stopSmallVideo();
	}
	else
	{
		ui->btnStopVideo->setToolTip("Close Camera");
		startSmallVideo();
	}
}

void SessionVideoDialog::onBtnFullScreanToggled(bool checked)
{
	if (checked)
	{
		ui->btnFullScrean->setToolTip(tr("Quit Full Screen"));
	}
	else
	{
		ui->btnFullScrean->setToolTip(tr("Enter Full Screen"));
	}
	triggerMaximize();
}

void SessionVideoDialog::onFullScreenEscape()
{
	if (ui->btnFullScrean->isChecked())
	{
		ui->btnFullScrean->setChecked(false);
	}
}

void SessionVideoDialog::onBigVideoDoubleClicked()
{
	bool checked = ui->btnFullScrean->isChecked();
	ui->btnFullScrean->setChecked(!checked);
}

void SessionVideoDialog::onBtnChatClicked()
{
	CChatDialog *chatDialog = qPmApp->getBuddyMgr()->openChat(m_id);
	if (chatDialog)
	{
		connect(this, SIGNAL(destroyed()), chatDialog, SLOT(onSessionClosed()), Qt::UniqueConnection);
	}
}

void SessionVideoDialog::onBtnPhotoClicked()
{
	QImage frame = ui->videoCanvas->otherCurFrame();
	if (!frame.isNull())
	{
		QString defaultLoc = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-z.png");
		if (frame.save(defaultLoc+QString("/")+fileName, "PNG"))
		{
			showTip(tr("Photo save to desktop\\%1").arg(fileName));
		}
		else
		{
			showTip(tr("Save photo failed"));
		}
	}
	else
	{
		showTip(tr("Photo is empty"));
	}
}

void SessionVideoDialog::onBtnShowModeClicked(bool checked)
{
	if (checked)
	{
		ui->videoCanvas->setShowMode(SimpleVideoWidget::ModeLeftRight);
		ui->btnShowMode->setToolTip(tr("Left-right Mode"));
	}
	else
	{
		ui->videoCanvas->setShowMode(SimpleVideoWidget::ModeTopBottom);
		ui->btnShowMode->setToolTip(tr("Top-bottom Mode"));
	}
}

void SessionVideoDialog::onAudioSendChanged(bool send)
{
	disconnect(ui->btnStopAudio, SIGNAL(clicked(bool)), this, SLOT(onBtnStopAudioClicked(bool)));
	ui->btnStopAudio->setChecked(!send);
	if (!send)
	{
		ui->btnStopAudio->setToolTip(tr("Recover Microphone"));
	}
	else
	{
		ui->btnStopAudio->setToolTip(tr("Mute Microphone"));
	}
	connect(ui->btnStopAudio, SIGNAL(clicked(bool)), this, SLOT(onBtnStopAudioClicked(bool)));
}

void SessionVideoDialog::onVideoSendChanged(bool send)
{
	disconnect(ui->btnStopVideo, SIGNAL(clicked(bool)), this, SLOT(onBtnStopVideoClicked(bool)));
	ui->btnStopVideo->setChecked(!send);
	if (!send)
	{
		ui->btnStopVideo->setToolTip(tr("Open Camera"));
		stopSmallVideo();
	}
	else
	{
		ui->btnStopVideo->setToolTip(tr("Close Camera"));
		startSmallVideo();
	}
	connect(ui->btnStopVideo, SIGNAL(clicked(bool)), this, SLOT(onBtnStopVideoClicked(bool)));
}

void SessionVideoDialog::onSessionDestoryed(QObject *obj)
{
	Q_UNUSED(obj);

	close();
}

void SessionVideoDialog::onSessionStateChanged(int ctrlPeer, int curState, int oldState)
{
	qDebug() << Q_FUNC_INFO 
		<< " PEER " << ctrlPeer 
		<< " cur " << curState 
		<< " old " << oldState;

	if (m_pSession.isNull())
	{
		return;
	}

	int peer = m_pSession->peer();

	if (rtcsession::Session::State_End == curState)
	{
		if (oldState == rtcsession::Session::State_Setup)
		{
			int hour = m_elapsedSeconds;
			int second = hour % 60;
			hour /= 60;
			int minute = hour % 60;
			hour /= 60;

			QString str = "";
			if (hour > 0)
			{
				str += tr("%1 hour ").arg(hour);
			}

			if (minute > 0)
			{
				str += tr("%1 minute ").arg(minute);
			}

			str += tr("%1 second").arg(second);

			QString msgText;
			if (peer == ctrlPeer)
			{
				msgText = tr("You stopped video call, last %1.").arg(str);
			}
			else 
			{
				msgText = tr("The other stopped video call, last %1.").arg(str);
			}

			qPmApp->getBuddyMgr()->appendSessionMessage(m_id, msgText);
		}
	}	
}

void SessionVideoDialog::onSessionError(int errCode)
{
	if (rtcsession::Session::Error_NoCamera == errCode)
	{
		// find no camera
		showTip(tr("No camera detected, please plug camera in."));
	}
	else if (rtcsession::Session::Error_CameraOpened == errCode)
	{
		// start camera failed
		showTip(tr("Camera start failed, maybe occupied."));
	}
	else if (rtcsession::Session::Error_DataTimeout == errCode)
	{
		// data time out
		// add a error message
		bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
		msgBody.setSend(false);
		msgBody.setFrom(Account::instance()->id());
		msgBody.setTo(m_id);
		msgBody.setTime(CDateTime::currentDateTimeUtcString());
		msgBody.setBody(tr("The connection of other side lost."));
		bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
		ext.setData("level", "error");
		msgBody.setExt(ext);
		qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
	}
	else if (rtcsession::Session::Error_RtcServiceError == errCode)
	{
		// rtc service error
		showTip(tr("Audio/Video service error, please restart audio/video session"));
	}
}

void SessionVideoDialog::onMaximizeStateChanged(bool maximized)
{
	disconnect(ui->btnFullScrean, SIGNAL(toggled(bool)), this, SLOT(onBtnFullScreanToggled(bool)));
	if (maximized)
	{
		ui->titlebar->setVisible(false);
		ui->btnFullScrean->setChecked(true);
	}
	else
	{
		ui->titlebar->setVisible(true);
		ui->btnFullScrean->setChecked(false);
	}
	connect(ui->btnFullScrean, SIGNAL(toggled(bool)), this, SLOT(onBtnFullScreanToggled(bool)));
}
/*
void SessionVideoDialog::onEnterBtnVolume()
{
	// show the volume control panel
	VolumeControlPanel *panel = volumeControlPanel();
	panel->setVolume(m_currentVolume);
	if (ui->btnVolume->isChecked())
		panel->setEnable(false);
	else
		panel->setEnable(true);

	QRect btnRect = ui->btnVolume->frameGeometry();
	int x = btnRect.left() + btnRect.width()/2 - panel->frameSize().width()/2;
	int y = btnRect.top() - panel->frameSize().height() - 2;
	QPoint showPt = ((QWidget *)ui->btnVolume->parent())->mapToGlobal(QPoint(x, y));
	panel->move(showPt);
	panel->show();
}

void SessionVideoDialog::onLeaveBtnVolume()
{
	// hide the volume control panel
	VolumeControlPanel *panel = volumeControlPanel();
	if (panel->isVisible())
	{
		panel->preHide();
	}
}
*/
void SessionVideoDialog::onVolumeChanged(int vol)
{
	if (vol == m_currentVolume)
		return;

	m_currentVolume = vol;
	m_pSession->setAudioVolume(m_currentVolume);
	ui->sliderVolume->setToolTip(tr("Current Volume %1").arg(vol));
}

void SessionVideoDialog::onOtherAudioChanged(bool before, bool now)
{
	if (!before && now)
	{
		showTip(tr("Other opened the microphone"));
	}
	else if (before && !now)
	{
		showTip(tr("Other closed the microphone"));
	}
}

void SessionVideoDialog::onOtherVideoChanged(bool before, bool now)
{
	if (!before && now)
	{
		ui->videoCanvas->setOtherHide(false);
		ui->btnPhoto->setEnabled(true);
		showTip(tr("Other opened the camera"));
	}
	else if (before && !now)
	{
		ui->videoCanvas->setOtherHide(true);
		ui->btnPhoto->setEnabled(false);
		showTip(tr("Other closed the camera"));
	}
}

void SessionVideoDialog::onVideoHasEvent()
{
	if (!ui->bottomBar->isVisible())
	{
		ui->bottomBar->setVisible(true);
		ui->videoCanvas->showSelfBottomMargin();
	}

	m_bottomHideTimer.stop();
	m_bottomHideTimer.start();
}

void SessionVideoDialog::onUserDetailChanged(const QString &uid)
{
	if (uid == m_otherUid)
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QPixmap otherAvatar = modelManager->getUserAvatar(m_otherUid);
		QString otherName = modelManager->userName(m_otherUid);
		ui->videoCanvas->setOtherNameAvatar(otherName, otherAvatar);
	}
	else if (uid == Account::instance()->id())
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QString selfId = Account::instance()->id();
		QPixmap selfAvatar = modelManager->getUserAvatar(selfId);
		QString selfName = modelManager->userName(selfId);
		ui->videoCanvas->setSelfNameAvatar(selfName, selfAvatar);
	}
}

void SessionVideoDialog::onPackageStatistics(int packageLost, int packageReceived)
{
	if (packageReceived == 0)
	{
		ui->labelSignal->setPixmap(QPixmap(":/session/signal_0.png"));
		ui->labelSignal->setToolTip("");
		return;
	}

	float lostPercent = ((float)packageLost)/((float)packageReceived);
	if (lostPercent >= 0.3)
	{
		ui->labelSignal->setPixmap(QPixmap(":/session/signal_1.png"));
		ui->labelSignal->setToolTip(tr("Session quality: bad"));

		showTip(tr("Current network is busy"));
	}
	else if (lostPercent >= 0.2)
	{
		ui->labelSignal->setPixmap(QPixmap(":/session/signal_2.png"));
		ui->labelSignal->setToolTip(tr("Session quality: not good"));
	}
	else if (lostPercent >= 0.1)
	{
		ui->labelSignal->setPixmap(QPixmap(":/session/signal_3.png"));
		ui->labelSignal->setToolTip(tr("Session quality: normal"));
	}
	else
	{
		ui->labelSignal->setPixmap(QPixmap(":/session/signal_4.png"));
		ui->labelSignal->setToolTip(tr("Session quality: good"));
	}
}

void SessionVideoDialog::initUI()
{
	// ui
	ModelManager *modelManager = qPmApp->getModelManager();

	// set dialog icon
	QPixmap icon = qPmApp->getModelManager()->getUserAvatar(m_id);
	setWindowIcon(QIcon(icon));

	// set dialog title
	QString title = tr("Video call with %1").arg(modelManager->userName(m_id));
	ui->title->setText(title);
	setWindowTitle(title);

	// set video widget
	initVideoCanvas();

	// audio/video button state check
	ui->btnStopAudio->setChecked(false);
	ui->btnStopAudio->setToolTip(tr("Mute microphone"));

	ui->btnVolume->setChecked(false);
	ui->btnVolume->setToolTip(tr("Mute Volume"));

	ui->btnStopVideo->setChecked(false);
	ui->btnStopVideo->setToolTip(tr("Close Camera"));

	ui->btnShowMode->setChecked(false);
	ui->btnShowMode->setToolTip(tr("Top-bottom Mode"));

	if (m_pSession->sessionError() == rtcsession::Session::Error_AudioDeviceError)
	{
		ui->btnStopAudio->setEnabled(false);
	}

	rtcsession::SessionMediaParam param = m_pSession->selfMediaParam();
	if (!param.audioSend)
	{
		ui->btnStopAudio->setChecked(true);
		ui->btnStopAudio->setToolTip(tr("Recover Microphone"));
	}

	if (!param.videoSend)
	{
		ui->btnStopVideo->setChecked(true);
		ui->btnStopVideo->setToolTip(tr("Open Camera"));
		stopSmallVideo();
	}

	rtcsession::SessionMediaParam otherParam = m_pSession->otherMediaParam();
	if (!otherParam.videoSend)
	{
		ui->btnPhoto->setEnabled(false);
		ui->videoCanvas->setOtherHide(true);
	}
	else
	{
		ui->btnPhoto->setEnabled(true);
		ui->videoCanvas->setOtherHide(false);
	}
	
	ui->btnFullScrean->setToolTip(tr("Enter Full Screen"));

	// tip widget
	ui->tipWidget->setTipPixmap(QPixmap(":/images/Icon_76.png"));
	ui->tipWidget->setTipTextColor(QColor("#ffffffff"));
	ui->tipWidget->setBackgroundColor(QColor("#4c333333"));
	ui->tipWidget->setCanClose(false);
	ui->tipWidget->stopShow();

	ui->sliderVolume->setMinimum(kMinVolume);
	ui->sliderVolume->setMaximum(kMaxVolume);
	ui->sliderVolume->setSingleStep(1);
	ui->sliderVolume->setPageStep(1);
	ui->sliderVolume->setTickInterval(1);
	ui->sliderVolume->setValue(kMaxVolume);

	// hide timer
	m_bottomHideTimer.setInterval(3*1000);
	m_bottomHideTimer.setSingleShot(false);
	connect(&m_bottomHideTimer, SIGNAL(timeout()), ui->bottomBar, SLOT(hide()));
	connect(&m_bottomHideTimer, SIGNAL(timeout()), ui->videoCanvas, SLOT(hideSelfBottomMargin()));
	m_bottomHideTimer.start();

	onPackageStatistics(0, 0);
}

void SessionVideoDialog::startElapsedTimer()
{
	m_elapsedSeconds = 0;
	m_elapsedTimerId = startTimer(1000); // 1s
	ui->labElapseTime->setText("00:00:00");
}

void SessionVideoDialog::stopElapsedTimer()
{
	if (m_elapsedTimerId <= 0)
	{
		return;
	}

	killTimer(m_elapsedTimerId);
	m_elapsedTimerId = 0;
	m_elapsedSeconds = 0;
	ui->labElapseTime->setText("00:00:00");
}

void SessionVideoDialog::initVideoCanvas()
{
	if (m_pSession.isNull())
	{
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	QString selfId = Account::instance()->id();
	m_otherUid = m_pSession->from();
	if (m_pSession->from() == selfId)
		m_otherUid = m_pSession->to();
	QPixmap selfAvatar = modelManager->getUserAvatar(selfId);
	QString selfName = modelManager->userName(selfId);
	QPixmap otherAvatar = modelManager->getUserAvatar(m_otherUid);
	QString otherName = modelManager->userName(m_otherUid);
	ui->videoCanvas->setSelfNameAvatar(selfName, selfAvatar);
	ui->videoCanvas->setOtherNameAvatar(otherName, otherAvatar);
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onUserDetailChanged(QString)), Qt::UniqueConnection);

	session::VideoFrameCB *otherVideoFrameCB = m_pSession->otherVideoFrameCB();
	if (otherVideoFrameCB)
	{
		disconnect(otherVideoFrameCB, SIGNAL(updated(QImage)), ui->videoCanvas, SLOT(onOtherUpdated(QImage)));
		disconnect(otherVideoFrameCB, SIGNAL(sizeChanged(QSize)), ui->videoCanvas, SLOT(onOtherSizeChanged(QSize)));
		connect(otherVideoFrameCB, SIGNAL(updated(QImage)), ui->videoCanvas, SLOT(onOtherUpdated(QImage)));
		connect(otherVideoFrameCB, SIGNAL(sizeChanged(QSize)), ui->videoCanvas, SLOT(onSelfSizeChanged(QSize)));
		ui->videoCanvas->onOtherSizeChanged(otherVideoFrameCB->imageSize());
	}

	rtcsession::RtcSessionManager *rtcSessionManager = qPmApp->getRtcSessionManager();
	session::VideoFrameCB *selfVideoFrameCB = rtcSessionManager->selfVideoFrameCB();
	if (selfVideoFrameCB)
	{
		disconnect(selfVideoFrameCB, SIGNAL(updated(QImage)), ui->videoCanvas, SLOT(onSelfUpdated(QImage)));
		disconnect(selfVideoFrameCB, SIGNAL(sizeChanged(QSize)), ui->videoCanvas, SLOT(onSelfSizeChanged(QSize)));
		connect(selfVideoFrameCB, SIGNAL(updated(QImage)), ui->videoCanvas, SLOT(onSelfUpdated(QImage)));
		connect(selfVideoFrameCB, SIGNAL(sizeChanged(QSize)), ui->videoCanvas, SLOT(onSelfSizeChanged(QSize)));
		ui->videoCanvas->onSelfSizeChanged(selfVideoFrameCB->imageSize());
	}
}

void SessionVideoDialog::closeSession()
{
	if (!m_pSession.isNull())
	{
		m_pSession->close(rtcsession::Session::CloseNormal);
	}
}
/*
VolumeControlPanel *SessionVideoDialog::volumeControlPanel()
{
	if (m_volumeControlPanel.isNull())
	{
		m_volumeControlPanel.reset(new VolumeControlPanel(kMinVolume, kMaxVolume));
		connect(m_volumeControlPanel.data(), SIGNAL(volumeChanged(int)), this, SLOT(onVolumeChanged(int)));
	}
	return m_volumeControlPanel.data();
}
*/
void SessionVideoDialog::showTip(const QString &tip)
{
	ui->tipWidget->setTipText(tip);
	ui->tipWidget->autoShow();
}

void SessionVideoDialog::startSmallVideo()
{
	rtcsession::RtcSessionManager *rtcSessionManager = qPmApp->getRtcSessionManager();
	session::VideoFrameCB *selfVideoFrameCB = rtcSessionManager->selfVideoFrameCB();
	if (selfVideoFrameCB)
	{
		ui->videoCanvas->setSelfHide(false);
		connect(selfVideoFrameCB, SIGNAL(updated(QImage)), ui->videoCanvas, SLOT(onSelfUpdated(QImage)), Qt::UniqueConnection);
		connect(selfVideoFrameCB, SIGNAL(sizeChanged(QSize)), ui->videoCanvas, SLOT(onSelfSizeChanged(QSize)), Qt::UniqueConnection);
	}
}

void SessionVideoDialog::stopSmallVideo()
{
	rtcsession::RtcSessionManager *rtcSessionManager = qPmApp->getRtcSessionManager();
	session::VideoFrameCB *selfVideoFrameCB = rtcSessionManager->selfVideoFrameCB();
	if (selfVideoFrameCB)
	{
		// make black
		QImage image(selfVideoFrameCB->imageSize(), QImage::Format_RGB32);
		image.fill(Qt::black);
		ui->videoCanvas->onSelfUpdated(image);
		ui->videoCanvas->setSelfHide(true);

		disconnect(selfVideoFrameCB, SIGNAL(updated(QImage)), ui->videoCanvas, SLOT(onSelfUpdated(QImage)));
		disconnect(selfVideoFrameCB, SIGNAL(sizeChanged(QSize)), ui->videoCanvas, SLOT(onSelfSizeChanged(QSize)));
	}
}