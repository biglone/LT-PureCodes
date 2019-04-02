#include <QDebug>
#include <QTimer>
#include "session/VideoFrameCB.h"
#include "rtc/rtcsession.h"
#include "rtc/rtcsessionmanager.h"
#include "PmApp.h"
#include "bean/MessageBody.h"
#include "chatdialog.h"
#include "Account.h"
#include "sessionvideomanager.h"
#include "buddymgr.h"
#include "pmessagebox.h"
#include <QMovie>
#include "sessionwidget.h"
#include "ui_sessionwidget.h"
#include "microphonecontrolpanel.h"
#include "interphonedialog.h"
#include "rtc/volumecontrolpanel.h"

static const int kMaxVolume  =  10;
static const int kMinVolume  =  1;
static const int kMuteVolume =  0;

SessionWidget::SessionWidget(QWidget *parent)
    : QWidget(parent)
	, m_pChatDlg(0)
	, timerId(0)
	, elapsed(0)
	, m_currentVolume(kMaxVolume)
{
    ui = new Ui::SessionWidget();
    ui->setupUi(this);

	connect(ui->btnMic, SIGNAL(clicked(bool)), this, SLOT(onBtnMicClicked(bool)));
	connect(ui->btnVolume, SIGNAL(clicked(bool)), this, SLOT(onBtnVolumeClicked(bool)));
	connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));
	// connect(ui->btnVolume, SIGNAL(enter()), this, SLOT(onEnterVolumeBtn()));
	// connect(ui->btnVolume, SIGNAL(leave()), this, SLOT(onLeaveVolumeBtn()));
}

SessionWidget::~SessionWidget()
{
    delete ui;
}

void SessionWidget::setChatDialog( CChatDialog *pChatDialog )
{
	m_pChatDlg = pChatDialog;
}

bool SessionWidget::inviteAudio( const QString &uid )
{
    rtcsession::RtcSessionManager *pSM = qPmApp->getRtcSessionManager();
    Q_ASSERT(pSM != 0);

	m_pSession = pSM->createSession(rtcsession::Session::Peer_A, Account::instance()->id(), uid);
	if (!m_pSession)
	{
		qWarning() << Q_FUNC_INFO << "create session failed to" << uid;
		return false;
	}

    m_pSession->invite(rtcsession::ST_Audio);

    disconnect(m_pSession, SIGNAL(onOk()), this, SLOT(onSessionOnOk()));
	disconnect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	disconnect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	disconnect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));
	disconnect(m_pSession, SIGNAL(audioSendChanged(bool)), this, SLOT(onAudioSendChanged(bool)));
	
	connect(m_pSession, SIGNAL(onOk()), this, SLOT(onSessionOnOk()));
	connect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	connect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	connect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));
	connect(m_pSession, SIGNAL(audioSendChanged(bool)), this, SLOT(onAudioSendChanged(bool)));

    // ui
    initUI();
    ui->stackedWidget->setCurrentIndex(SS_Invite);

	// quit interphone
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog::getInterphoneDialog()->quitAndClose();
	}

    return true;
}

bool SessionWidget::inviteVideo( const QString &uid )
{
    rtcsession::RtcSessionManager *pSM = qPmApp->getRtcSessionManager();
    Q_ASSERT(pSM != 0);

	m_pSession = pSM->createSession(rtcsession::Session::Peer_A, Account::instance()->id(), uid);
	if (!m_pSession)
	{
		qWarning() << Q_FUNC_INFO << "create session failed to" << uid;
		return false;
	}

	m_pSession->invite(rtcsession::ST_AudioVideo);

    // init
	disconnect(m_pSession, SIGNAL(onOk()), this, SLOT(onSessionOnOk()));
	disconnect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	disconnect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	disconnect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));

	connect(m_pSession, SIGNAL(onOk()), this, SLOT(onSessionOnOk()));
	connect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	connect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	connect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));

    // ui
    initUI();
    ui->stackedWidget->setCurrentIndex(SS_Invite);

	// quit interphone
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog::getInterphoneDialog()->quitAndClose();
	}

    return true;
}

bool SessionWidget::onInvite( const QString &sid )
{
	// quit interphone
	if (InterphoneDialog::hasInterphoneDialog())
	{
		InterphoneDialog::getInterphoneDialog()->quitAndClose();
	}

    if (m_pSession)
    {
        m_pSession->close(rtcsession::Session::CloseNormal);
    }

    qWarning() << Q_FUNC_INFO << sid;
    rtcsession::RtcSessionManager *pSM = qPmApp->getRtcSessionManager();
    Q_ASSERT(pSM != 0);

    m_pSession = pSM->session(sid);
    Q_ASSERT(m_pSession != 0);

	disconnect(m_pSession, SIGNAL(onOk()), this, SLOT(onSessionOnOk()));
	disconnect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	disconnect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
    disconnect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));

	connect(m_pSession, SIGNAL(onOk()), this, SLOT(onSessionOnOk()));
	connect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	connect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	connect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));

    // ui
    initUI();
    ui->stackedWidget->setCurrentIndex(SS_OnInvite);

    return true;
}

QString SessionWidget::title() const
{
    if (!m_pSession.isNull())
    {
        switch (m_pSession->type())
        {
        case rtcsession::ST_Audio:
            return tr("Audio");
        case rtcsession::ST_AudioVideo:
            return tr("Video");
        }
    }
    return "";
}

int SessionWidget::state() const
{
    return ui->stackedWidget->currentIndex();
}

void SessionWidget::timerEvent( QTimerEvent * event )
{
    if (timerId != event->timerId())
    {
        return;
    }

    ++elapsed;
    QString str = QString("%1:%2:%3")
        .arg(elapsed/3600, 2, 10, QChar('0'))
        .arg((elapsed/60)%60, 2, 10, QChar('0'))
        .arg(elapsed%60, 2, 10, QChar('0'));

    ui->labTime->setText(str);
}

void SessionWidget::onSessionDestoryed( QObject *obj )
{
    Q_UNUSED(obj);

    emit sessionClosed();
}

void SessionWidget::onSessionError( int errCode )
{
	if (rtcsession::Session::Error_InviteTimeout == errCode)
	{
		// invite timeout
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("No one picks up, %1 request canceled.").arg(title()));
		}
	}
	if (rtcsession::Session::Error_RingingTimeout == errCode)
	{
		// ringing timeout
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("The other disconnected."));
		}
	}
	else if (rtcsession::Session::Error_InviteConflict == errCode)
	{
		// invite conflict
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("%1 request conflicts, please try again.").arg(title()));
		}
	}
	else if (rtcsession::Session::Error_NoCamera == errCode)
	{
		// find no camera
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("No camera detected, please plug camera in."));
		}
	}
	else if (rtcsession::Session::Error_CameraOpened == errCode)
	{
		// start camera failed
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("Camera start failed, maybe occupied."));
		}
	}
	else if (rtcsession::Session::Error_AckTimeout == errCode)
	{
		// ack time out
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("Connection failed, please try again."));
		}
	}
	else if (rtcsession::Session::Error_DataTimeout == errCode)
	{
		// data time out
		// add a error message
		bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
		msgBody.setSend(false);
		msgBody.setFrom(Account::instance()->id());
		msgBody.setTo(otherId());
		msgBody.setTime(CDateTime::currentDateTimeUtcString());
		msgBody.setBody(tr("The connection of other side lost."));
		bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
		ext.setData("level", "error");
		msgBody.setExt(ext);
		qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
	}
	else if (rtcsession::Session::Error_AudioDeviceError == errCode)
	{
		// audio device error
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("No audio device detected, the audio can't be used."));
		}
	}
	else if (rtcsession::Session::Error_PhoneHandle == errCode)
	{
		// phone handle
		// add a info message
		bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
		msgBody.setSend(false);
		msgBody.setFrom(Account::instance()->id());
		msgBody.setTo(otherId());
		msgBody.setTime(CDateTime::currentDateTimeUtcString());
		msgBody.setBody(tr("%1 is handled by other device.").arg(title()));
		bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
		ext.setData("level", "info");
		msgBody.setExt(ext);
		qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
	}
	else if (rtcsession::Session::Error_RtcServiceError == errCode)
	{
		// rtc service error
		if (m_pChatDlg)
		{
			m_pChatDlg->showAutoTip(tr("Audio/Video service error, please restart audio/video session"));
		}
	}
}

void SessionWidget::onSessionStateChanged( int ctrlPeer, int curState, int oldState )
{
	qDebug() << Q_FUNC_INFO 
		<< " PEER " << ctrlPeer 
		<< " cur " << curState 
		<< " old " << oldState;

	int peer = m_pSession->peer();

	if (rtcsession::Session::State_Connect == curState)
	{
		if (peer == rtcsession::Session::Peer_A)
			ui->labInfo1->setText(tr("Setup connection..."));
		else
			ui->labInfo2->setText(tr("Setup connection..."));
	}

	if (rtcsession::Session::State_End == curState)
	{
		switch (oldState)
		{
		case rtcsession::Session::State_Invite:
		case rtcsession::Session::State_Ring:
		case rtcsession::Session::State_Connect:
			{
				rtcsession::Session::SessionError sessionErr = m_pSession->sessionError();
				if (sessionErr == rtcsession::Session::Error_InviteTimeout ||
					sessionErr == rtcsession::Session::Error_RingingTimeout ||
					sessionErr == rtcsession::Session::Error_InviteConflict ||
					sessionErr == rtcsession::Session::Error_AckTimeout ||
					sessionErr == rtcsession::Session::Error_PhoneHandle)
					break;

				if (peer == ctrlPeer)
				{
					if (peer == rtcsession::Session::Peer_A)
					{
						QString str = tr("%1 request is canceled.").arg(title());

						if (m_pChatDlg)
						{
							m_pChatDlg->showAutoTip(str);
						}
					}
				}
				else
				{
					QString str = "";
					switch (peer)
					{
					case rtcsession::Session::Peer_A:
						{
							rtcsession::Session::CloseType closeType = m_pSession->otherCloseType();
							if (closeType == rtcsession::Session::CloseBusy)
							{
								str = tr("The other is busy");
							}
							else if (closeType == rtcsession::Session::CloseUnsupport)
							{
								str = tr("The other does not support");
							}
							else
							{
								str = tr("The other side refused your %1 request.").arg(title());
							}
						}
						break;
					case rtcsession::Session::Peer_B:
						str = tr("The other side canceled %1 request.").arg(title());
						break;
					}

					if (m_pChatDlg)
					{
						m_pChatDlg->showAutoTip(str);
					}
				}
			}
			break;
		case rtcsession::Session::State_Setup:
			{
				int hour = elapsed;
				int second = hour % 60;
				hour /= 60;
				int minute = hour % 60;
				hour /= 60;

				QString str = "";
				if (hour > 0)
				{
					str += tr("%1 hour(s) ").arg(hour);
				}

				if (minute > 0)
				{
					str += tr("%1 minute(s) ").arg(minute);
				}

				str += tr("%1 second(s)").arg(second);

				QString msgText;
				if (peer == ctrlPeer)
				{
					msgText = tr("You stopped %1, last %2.").arg(title()).arg(str);
				}
				else 
				{
					msgText = tr("The other stopped %1, last %2.").arg(title()).arg(str);
				}

				QString otherSideId = otherId();
				qPmApp->getBuddyMgr()->appendSessionMessage(otherSideId, msgText);
			}
			break;
		default:
			break;
		}
	}	
}

void SessionWidget::onSessionOnOk()
{
    if (!m_pSession) // close
    {
        return;
    }

	// audio setup
	if (m_pSession->type() == rtcsession::ST_Audio)
	{
		ui->stackedWidget->setCurrentIndex(SS_Audio);
		startElapsedTimer();
		return;
	}

	// video setup
	QString otherSideUid = otherId();
	qPmApp->getSessionVideoManager()->startVideo(otherSideUid, m_pSession.data());

	// disconnect unnecessary notification
	disconnect(m_pSession, SIGNAL(stateChanged(int, int, int)), this, SLOT(onSessionStateChanged(int, int, int)));
	disconnect(m_pSession, SIGNAL(error(int)), this, SLOT(onSessionError(int)));
	// disconnect(m_pSession, SIGNAL(destroyed(QObject*)), this, SLOT(onSessionDestoryed(QObject*)));

	emit sessionVideoSetup();
}

void SessionWidget::on_btnBye_clicked()
{
    if (m_pSession.isNull())
    {
        // close
        return;
    }

    m_pSession->bye(rtcsession::Session::CloseNormal);
}

void SessionWidget::on_btnOk_clicked()
{
    if (m_pSession.isNull())
    {
        // close
        return;
    }

	if (m_pSession->type() == rtcsession::ST_AudioVideo)
	{
		if (qPmApp->isTakeSelfPhoto())
		{
			if (m_pChatDlg)
			{
				m_pChatDlg->showAutoTip(tr("You are taking photo of yourself, can't start video call."));
			}
			return;
		}
	}

	if (m_pSession->sessionError() != rtcsession::Session::Error_NoError)
	{
		if (m_pSession->type() == rtcsession::ST_Audio)
		{
			PMessageBox::warning(this, tr("Audio Call"), tr("No audio device detected, can't start audio call."));
			m_pSession->close(rtcsession::Session::CloseError, tr("No audio device"));
			return;
		}
		else
		{
			onSessionError(m_pSession->sessionError());
		}
	}

    m_pSession->ok();

	ui->btnOk->setEnabled(false);
	ui->btnReject->setEnabled(false);
}

void SessionWidget::on_btnReject_clicked()
{
	if (m_pSession)
	{
		// reject this session, session close
		m_pSession->reject(rtcsession::Session::CloseNormal);
	}
}

void SessionWidget::on_btnAudioBye_clicked()
{
	if (m_pSession)
	{
		// close this session
		m_pSession->close(rtcsession::Session::CloseNormal);
	}
}

void SessionWidget::onBtnMicClicked( bool checked )
{
    m_pSession->setAudioSendEnable(!checked);
    if (checked)
    {
        ui->btnMic->setToolTip(tr("Recover Microphone"));
    }
    else
    {
        ui->btnMic->setToolTip(tr("Mute Microphone"));
    }
}

void SessionWidget::onBtnVolumeClicked(bool checked)
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

/*
void SessionWidget::onEnterVolumeBtn()
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

void SessionWidget::onLeaveVolumeBtn()
{
	// hide the volume control panel
	VolumeControlPanel *panel = volumeControlPanel();
	if (panel->isVisible())
	{
		panel->preHide();
	}
}
*/

void SessionWidget::onAudioSendChanged(bool send)
{
	disconnect(ui->btnMic, SIGNAL(clicked(bool)), this, SLOT(onBtnMicClicked(bool)));
	ui->btnMic->setChecked(!send);
	if (!send)
	{
		ui->btnMic->setToolTip(tr("Recover Microphone"));
	}
	else
	{
		ui->btnMic->setToolTip(tr("Mute Microphone"));
	}
	connect(ui->btnMic, SIGNAL(clicked(bool)), this, SLOT(onBtnMicClicked(bool)));
}

void SessionWidget::onVolumeChanged(int vol)
{
	if (vol == m_currentVolume)
		return;

	m_currentVolume = vol;
	m_pSession->setAudioVolume(m_currentVolume);
	ui->sliderVolume->setToolTip(tr("Current Volume %1").arg(vol));
}

void SessionWidget::startElapsedTimer()
{
    elapsed = 0;
    timerId = startTimer(1000); // 1s
    ui->labTime->setText("00:00:00");
}

void SessionWidget::stopElapsedTimer()
{
    if (timerId <= 0)
        return;

    killTimer(timerId);
    timerId = 0;
    ui->labTime->setText("00:00:00");
}

void SessionWidget::sessionClose()
{
	if (!m_pSession.isNull())
	{
		m_pSession->close(rtcsession::Session::CloseNormal);
	}
}

void SessionWidget::setSkin()
{
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

	{
		QIcon icon;
		icon.addPixmap(QPixmap(":/session/lvoice2.png"));
		icon.addPixmap(QPixmap(":/session/lvoice2_r.png"), QIcon::Active, QIcon::On);
		ui->btnMic->setIcon(icon);
	}

	{
		QIcon icon;
		icon.addPixmap(QPixmap(":/session/lvoice-.png"));
		icon.addPixmap(QPixmap(":/session/lvoice_r.png"), QIcon::Active, QIcon::On);
		ui->btnVolume->setIcon(icon);
	}

	StylePushButton::Info acceptInfo;
	acceptInfo.urlNormal = QString(":/session/accept_normal.png");
	acceptInfo.urlHover = QString(":/session/accept_hover.png");
	acceptInfo.urlPressed = QString(":/session/accept_pressed.png");

	StylePushButton::Info rejectInfo;
	rejectInfo.urlNormal = QString(":/session/reject_normal.png");
	rejectInfo.urlHover = QString(":/session/reject_hover.png");
	rejectInfo.urlPressed = QString(":/session/reject_pressed.png");

	ui->btnBye->setInfo(rejectInfo);
	ui->btnOk->setInfo(acceptInfo);
	ui->btnReject->setInfo(rejectInfo);
	ui->btnAudioBye->setInfo(rejectInfo);
}

void SessionWidget::initUI()
{
    switch (m_pSession->type())
    {
    case rtcsession::ST_Audio:
        {
			QMovie *audioMovie = new QMovie(":/session/voice.gif");
            ui->toAvatar1->setMovie(audioMovie);
            ui->toAvatar2->setMovie(audioMovie);
			audioMovie->setParent(this);
			audioMovie->start();
            ui->labInfo2->setText(tr("The other is inviting audio..."));
        }
        break;
    case rtcsession::ST_AudioVideo:
        {
			QMovie *videoMovie = new QMovie(":/session/camera.gif");
            ui->toAvatar1->setMovie(videoMovie);
            ui->toAvatar2->setMovie(videoMovie);
			videoMovie->setParent(this);
			videoMovie->start();
            ui->labInfo2->setText(tr("The other is inviting video..."));
        }
    default:
        // error
        break;
    }

    // ui
	ui->labInfo1->setText(tr("Wait for acceptance..."));
    ui->btnMic->setChecked(false);
	ui->btnMic->setToolTip(tr("Mute Microphone"));
	ui->btnVolume->setChecked(false);
	ui->btnVolume->setToolTip(tr("Mute Volume"));
	ui->btnOk->setEnabled(true);
	ui->btnReject->setEnabled(true);

	if (!m_pSession.isNull())
	{
		rtcsession::SessionMediaParam param = m_pSession->selfMediaParam();
		if (!param.audioSend)
		{
			ui->btnMic->setChecked(true);
			ui->btnMic->setToolTip(tr("Recover Microphone"));
		}
	}

	// slider volume
	ui->sliderVolume->setMinimum(kMinVolume);
	ui->sliderVolume->setMaximum(kMaxVolume);
	ui->sliderVolume->setSingleStep(1);
	ui->sliderVolume->setPageStep(1);
	ui->sliderVolume->setTickInterval(1);
	ui->sliderVolume->setValue(kMaxVolume);
	ui->sliderVolume->setToolTip(tr("Current Volume %1").arg(kMaxVolume));
}

QString SessionWidget::otherId() const
{
	QString otherSideId;
	if (m_pSession->from() == Account::instance()->id())
	{
		otherSideId = m_pSession->to();
	}
	else
	{
		otherSideId = m_pSession->from();
	}
	return otherSideId;
}

/*
VolumeControlPanel *SessionWidget::volumeControlPanel()
{
	if (m_volumeControlPanel.isNull())
	{
		m_volumeControlPanel.reset(new VolumeControlPanel(kMinVolume, kMaxVolume));
		connect(m_volumeControlPanel.data(), SIGNAL(volumeChanged(int)), this, SLOT(onVolumeChanged(int)));
	}
	return m_volumeControlPanel.data();
}
*/




