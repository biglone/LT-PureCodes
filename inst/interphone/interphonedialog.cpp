#include "interphonedialog.h"
#include "ui_interphonedialog.h"
#include <QPainter>
#include "PmApp.h"
#include "ModelManager.h"
#include "interphonememberitemwidget.h"
#include "interphonemanager.h"
#include "pmessagebox.h"
#include "Account.h"
#include <QDebug>
#include <QSound>
#include <QAudioDeviceInfo>
#include "widgetmanager.h"
#include "interphonesession.h"
#include "settings/GlobalSettings.h"
#include "microphonecontrolpanel.h"
#include "buddymgr.h"

static const int kSoundWaveCount = 8;

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS InterphoneDialog

QPointer<InterphoneDialog> InterphoneDialog::s_interphoneDialog;

InterphoneDialog::InterphoneDialog(QWidget *parent)
	: FramelessDialog(parent), m_speakState(SpeakStop), m_closing(false), m_tickSeconds(0)
{
	ui = new Ui::InterphoneDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	ModelManager *modelManager = qPmApp->getModelManager();
	QString selfName = modelManager->userName(Account::instance()->id());
	QString title = tr("Interphone - ") + selfName;
	setWindowIcon(qApp->windowIcon());
	setWindowTitle(title);
	ui->title->setText(title);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(458, 587);
	setResizeable(false);

	initSoundWave();

	initUI();

	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
	bool connectOK = false;
	connectOK = connect(interphoneManager, SIGNAL(interphoneChanged(QString, int, QString)), 
		this, SLOT(onInterphoneChanged(QString, int, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(interphoneManager, SIGNAL(syncInterphoneMemberFinished(bool, QString)),
		this, SLOT(onSyncInterphoneMemberFinished(bool, QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(interphoneManager, SIGNAL(prepareSpeakOK(QString)),
		this, SLOT(onPrepareSpeakOK(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(interphoneManager, SIGNAL(prepareSpeakFailed(QString)),
		this, SLOT(onPrepareSpeakFailed(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(interphoneManager, SIGNAL(stopSpeakOK(QString)),
		this, SLOT(onStopSpeakOK(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(interphoneManager, SIGNAL(stopSpeakFailed(QString)),
		this, SLOT(onStopSpeakFailed(QString)));
	Q_ASSERT(connectOK);

	/*
	connectOK = connect(interphoneManager, SIGNAL(quitInterphoneFinished(bool, QString)),
		this, SLOT(onQuitInterphoneFinished(bool, QString)));
	Q_ASSERT(connectOK);
	*/

	connectOK = connect(interphoneManager, SIGNAL(interphonesCleared()), 
		this, SLOT(onInterphoneCleared()));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	Q_ASSERT(connectOK);

	setSkin();
}

InterphoneDialog::~InterphoneDialog()
{
	delete ui;
}

bool InterphoneDialog::hasInterphoneDialog()
{
	return !s_interphoneDialog.isNull();
}

InterphoneDialog * InterphoneDialog::getInterphoneDialog()
{
	if (s_interphoneDialog.isNull())
	{
		s_interphoneDialog = new InterphoneDialog();
	}
	return s_interphoneDialog.data();
}

void InterphoneDialog::setInterphoneId(const QString &id)
{
	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();

	m_interphoneId = id;
	m_interphone = interphoneManager->interphone(m_interphoneId);

	if (!m_interphone.speakerId().isEmpty())
	{
		// other is talking
		startTick();

		playSound(InterphoneDialog::OtherReady);

		// set speaker
		setSpeaker(m_interphone.speakerId());

		// set tip
		QString tip = configTipText(m_interphone.speakerId(), m_interphone.memberCount());
		setTip(tip);
	}
	else
	{
		// no speaker
		setSpeakState(SpeakStop);
	}	

	setMemberCount(m_interphone.memberCount());

	setMembers(m_interphone.memberIds());
}

QString InterphoneDialog::interphoneId() const
{
	return m_interphoneId;
}

bool InterphoneDialog::openChannel()
{
	if (m_session.isNull())
	{
		InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
		QString addr = interphoneManager->interphoneAudioAddr();
		if (addr.isEmpty())
		{
			PMessageBox msgBox(PMessageBox::Warning, tr("Interphone address is empty"), QDialogButtonBox::Ok, tr("Interphone"), this);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();

			// quit this interphone
			quitAndClose();
			return false;
		}

		QString selfId = Account::instance()->id();
		m_session.reset(new interphone::InterphoneSession(m_interphoneId, Account::instance()->id(), addr));
		interphone::InterphoneSession::StartType ret = m_session->start();
		if (ret != interphone::InterphoneSession::StartOK)
		{
			QString errTip;
			if (ret == interphone::InterphoneSession::InputError)
				errTip = tr("Start microphone failed, can't start interphone");
			else
				errTip = tr("Play sound failed, please check audio device");
			PMessageBox msgBox(PMessageBox::Warning, errTip, QDialogButtonBox::Ok, tr("Interphone"), this);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();

			// quit this interphone
			quitAndClose();
			return false;
		}

		bool connectOK = false;
		connectOK = connect(m_session.data(), SIGNAL(recvChannelFailed()), this, SLOT(onRecvChannelFailed()));
		Q_ASSERT(connectOK);
	}

	return true;
}

void InterphoneDialog::closeChannel()
{
	if (m_session)
	{
		m_session->stop();
	}
}

void InterphoneDialog::quitAndClose()
{
	stopSpeak();

	quitInterphone();

	m_closing = true;
	close();
}

void InterphoneDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/interphone/bg.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 6;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	ui->memberWidget->setStyleSheet("QWidget#memberWidget {background: white;}");

	ui->labelSpeakerName->setStyleSheet("color: white; font: bold 10pt;");

	ui->labelTime->setStyleSheet("color: rgb(31, 188, 107); font: bold 12pt;");

	ui->labelNumber->setStyleSheet("color: #3ABD8B; font: bold 14pt;");

	ui->lineSeperate->setStyleSheet("color: rgb(217, 217, 219);");

	ui->labelMemberInOutTip->setStyleSheet("color: #B1C4D7; font: bold 10pt;");

	ui->tableWidget->setStyleSheet("QTableWidget#tableWidget {"
		"background: transparent;" 
		"border: none;"
		"}"
		"QTableWidget#tableWidget::item {"
		"background: transparent;" 
		"border: none;"
		"}"
		);
}

void InterphoneDialog::paintEvent(QPaintEvent *ev)
{
	FramelessDialog::paintEvent(ev);

	QPainter painter(this);

	const int middleLine = 267;
	QRect rt = rect();

	// draw sound wave
	QPixmap soundWave = m_soundWavePixmaps[m_soundWaveIndex];
	QSize soundWaveSize = soundWave.size();
	int x = rt.left() + (rt.width() - soundWaveSize.width())/2;
	int y = rt.top() + middleLine - soundWaveSize.height();
	painter.drawPixmap(QPoint(x, y), soundWave);

	// draw sound wave shadow
	QPixmap soundWaveShadow = m_soundWavePixmaps[kSoundWaveCount];
	y = rt.top() + middleLine;
	painter.drawPixmap(QPoint(x, y), soundWaveShadow);
}

void InterphoneDialog::closeEvent(QCloseEvent *ev)
{
	if (!(qPmApp->isShutingDown() || qPmApp->isLogout()) && !m_closing) // if not logout or exit, need query to close
	{
		PMessageBox msgBox(PMessageBox::Question, tr("If you quit, you can't receive audio, continue"),
			QDialogButtonBox::Yes|QDialogButtonBox::No, tr("Interphone"), this);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		if (msgBox.clickedButton() == QDialogButtonBox::Yes)
		{
			quitInterphone();
			ev->accept();
		}
		else
		{
			ev->ignore();
		}

		return;
	}

	FramelessDialog::closeEvent(ev);
}

void InterphoneDialog::keyPressEvent(QKeyEvent *ev)
{
	if (ev->key() == Qt::Key_Space && !ev->isAutoRepeat())
	{
		if (!ui->speakButton->isPressed())
		{
			ui->speakButton->setPressed(true);
			return;
		}
	}
	else if (ev->key() == Qt::Key_Escape)
	{
		close();
		return;
	}

	FramelessDialog::keyPressEvent(ev);
}

void InterphoneDialog::keyReleaseEvent(QKeyEvent *ev)
{
	if (ev->key() == Qt::Key_Space && !ev->isAutoRepeat())
	{
		if (ui->speakButton->isPressed())
		{
			ui->speakButton->setPressed(false);
			return;
		}
	}

	FramelessDialog::keyReleaseEvent(ev);
}

void InterphoneDialog::changeEvent(QEvent *ev)
{
	if (ev->type() == QEvent::ActivationChange)
	{
		if (!isActiveWindow())
		{
			if (ui->speakButton->isPressed())
			{
				ui->speakButton->setPressed(false);
			}
		}
	}

	FramelessDialog::changeEvent(ev);
}

void InterphoneDialog::onSoundWaveTimeout()
{
	++m_soundWaveIndex;
	m_soundWaveIndex = (m_soundWaveIndex%kSoundWaveCount);
	update();
}

void InterphoneDialog::onSpeakButtonToogled(bool pressed)
{
	if (m_speakPrepareTimer.isActive())
		m_speakPrepareTimer.stop();
	
	if (pressed)
	{
		playSound(InterphoneDialog::Press);

		setSpeakState(SpeakPrepareing);
	}
	else
	{
		playSound(InterphoneDialog::Release);

		stopTalk();

		stopSpeak();

		if (m_speakState == Speaking)
		{
			m_interphone.setSpeakerId(QString());
		}
		setSpeakState(SpeakStop);
	}
}

void InterphoneDialog::onInterphoneChanged(const QString &interphoneId, int /*attchType*/, const QString & /*attachId*/)
{
	if (m_interphoneId == interphoneId)
	{
		InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();

		InterphoneInfo newInfo = interphoneManager->interphone(interphoneId);
		if (newInfo.memberCount() != m_interphone.memberCount())
		{
			// set member count
			setMemberCount(newInfo.memberCount());

			// sync new members
			if (!m_closing)
			{
				interphoneManager->syncInterphoneMember(interphoneId);
			}
		}

		QString newSpeaker = newInfo.speakerId();
		QString oldSpeaker = m_interphone.speakerId();
		m_interphone = newInfo;
		if (newSpeaker.isEmpty())
		{
			stopTick();
		}
		else if (newSpeaker != oldSpeaker)
		{
			startTick();

			QString selfId = Account::instance()->id();
			if (newSpeaker != selfId)
			{
				playSound(InterphoneDialog::OtherReady);
			}
			else if (m_speakState == SpeakPrepared)
			{
				playSound(InterphoneDialog::Ready);

				startTalk();

				setSpeakState(Speaking);

				return;
			}
		}

		// set speaker
		setSpeaker(m_interphone.speakerId());

		// set tip
		QString tip = configTipText(m_interphone.speakerId(), m_interphone.memberCount());
		setTip(tip);
	}
}

void InterphoneDialog::onSyncInterphoneMemberFinished(bool OK, const QString &interphoneId)
{
	if (m_interphoneId == interphoneId)
	{
		if (OK)
		{
			InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
			InterphoneInfo newInfo = interphoneManager->interphone(interphoneId);
			m_interphone = newInfo;

			// check if self in this interphone
			QString selfId = Account::instance()->id();
			if (!m_interphone.memberIds().contains(selfId))
			{
				quitAndClose();
				return;
			}

			// set members
			setMembers(m_interphone.memberIds());

			// set member count
			setMemberCount(m_interphone.memberCount());

			// set speaker
			setSpeaker(m_interphone.speakerId());

			// set tip
			QString tip = configTipText(m_interphone.speakerId(), m_interphone.memberCount());
			setTip(tip);
		}
	}
}

void InterphoneDialog::onPrepareSpeakOK(const QString &interphoneId)
{
	if (m_interphoneId == interphoneId)
	{
		/* // there should be notification, do nothing here
		if (m_speakState == SpeakPrepared)
		{
			playSound(InterphoneDialog::Ready);

			startTalk();

			QString selfId = Account::instance()->id();
			m_interphone.setSpeakerId(selfId);

			setSpeakState(Speaking);
		}
		*/
	}
}

void InterphoneDialog::onPrepareSpeakFailed(const QString &interphoneId)
{
	if (m_interphoneId == interphoneId)
	{
		if (m_speakState == SpeakPrepared)
		{
			playSound(InterphoneDialog::Conflict);

			setSpeakState(SpeakConflict);
		}
	}
}

void InterphoneDialog::onStopSpeakOK(const QString &interphoneId)
{
	if (m_interphoneId == interphoneId)
	{
		// there should be notification, do nothing here
	}
}

void InterphoneDialog::onStopSpeakFailed(const QString &interphoneId)
{
	if (m_interphoneId == interphoneId)
	{
		// do nothing here, can't do any more to recover
	}
}

/*
void InterphoneDialog::onQuitInterphoneFinished(bool ok, const QString &interphoneId)
{
	if (m_interphoneId == interphoneId)
	{
		if (!ok)
		{
			WidgetManager::showActivateRaiseWindow(this);

			PMessageBox msgBox(PMessageBox::Information, tr("Quit interphone failed, please try again."),
				QDialogButtonBox::Ok, tr("Interphone"), this);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			m_closing = false;
			return;
		}

		// close this dialog
		m_closing = true;
		close();
	}
}
*/

void InterphoneDialog::onInterphoneCleared()
{
	ui->speakButton->setPressed(false);
	ui->speakButton->setEnabled(false);
	setSpeaker(QString());
	setTip(tr("Disconnected, restart when online"), true);
	m_closing = true;
	closeChannel();
}

void InterphoneDialog::onSpeakPrepareTimeout()
{
	prepareSpeak();
}

void InterphoneDialog::onRecvChannelFailed()
{
	PMessageBox msgBox(PMessageBox::Warning, tr("Connection failed, please try again"), QDialogButtonBox::Ok, tr("Interphone"), this);
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.exec();

	// quit this interphone
	quitAndClose();
}

void InterphoneDialog::onTickTimeout()
{
	++m_tickSeconds;
	setTickText();
}

void InterphoneDialog::onEnterBtnStopAudio()
{
	// show the microphone control panel
	MicrophoneControlPanel *panel = microphoneControlPanel();
	QRect micBtnRect = ui->btnMicrophone->frameGeometry();
	int x = micBtnRect.left() + micBtnRect.width()/2 - panel->frameSize().width()/2;
	int y = micBtnRect.top() - panel->frameSize().height() - 2;
	QPoint showPt = ((QWidget *)ui->btnMicrophone->parent())->mapToGlobal(QPoint(x, y));
	panel->move(showPt);
	panel->show();
}

void InterphoneDialog::onLeaveBtnStopAudio()
{
	MicrophoneControlPanel *panel = microphoneControlPanel();
	if (panel->isVisible())
	{
		panel->preHide();
	}
}

void InterphoneDialog::chat(const QString &uid)
{
	qPmApp->getBuddyMgr()->openChat(uid);
}

void InterphoneDialog::on_btnOpenChatDialog_clicked()
{
	bean::MessageType msgType = bean::Message_Invalid;
	QString msgId;
	InterphoneManager::interphoneId2AttachTypeId(m_interphoneId, msgType, msgId);
	if (msgType == bean::Message_Chat)
	{
		qPmApp->getBuddyMgr()->openChat(msgId);
	}
	else if (msgType == bean::Message_GroupChat)
	{
		qPmApp->getBuddyMgr()->openGroupChat(msgId);
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		qPmApp->getBuddyMgr()->openDiscussChat(msgId);
	}
	else
	{
		qDebug() << Q_FUNC_INFO << "interphone id is invalid: " << m_interphoneId;
	}
}

void InterphoneDialog::onDetailChanged(const QString &id)
{
	if (m_memberWidgets.contains(id))
	{
		InterphoneMemberItemWidget *widget = m_memberWidgets[id];
		if (widget)
		{
			ModelManager *modelManager = qPmApp->getModelManager();
			QPixmap avatar = modelManager->getUserAvatar(id);
			widget->setAvatar(avatar);
			widget->setName(modelManager->userName(id));
		}
	}
}

void InterphoneDialog::startPlaySoundWave()
{
	m_isPlayingSoundWave = true;
	m_soundWaveIndex = 0;
	m_soundWaveTimer.start();
}

void InterphoneDialog::stopPlaySoundWave()
{
	m_soundWaveTimer.stop();
	m_isPlayingSoundWave = false;
	m_soundWaveIndex = 0;
	update();
}

void InterphoneDialog::initSoundWave()
{
	m_isPlayingSoundWave = false;
	m_soundWaveIndex = 0;
	m_soundWaveTimer.setSingleShot(false);
	m_soundWaveTimer.setInterval(300);
	connect(&m_soundWaveTimer, SIGNAL(timeout()), this, SLOT(onSoundWaveTimeout()));

	for (int i = 1; i <= kSoundWaveCount; i++)
	{
		QPixmap soundWavePixmap(QString(":/interphone/sound%1.png").arg(i));
		m_soundWavePixmaps.append(soundWavePixmap);
	}

	QPixmap soundWaveShadowPixmap(":/interphone/soundshadow.png");
	m_soundWavePixmaps.append(soundWaveShadowPixmap);
}

void InterphoneDialog::initUI()
{
	ui->labelTime->clear();
	m_tickTimer.setSingleShot(false);
	m_tickTimer.setInterval(1000);
	connect(&m_tickTimer, SIGNAL(timeout()), this, SLOT(onTickTimeout()));

	m_speakPrepareTimer.setInterval(1000);
	m_speakPrepareTimer.setSingleShot(true);
	connect(&m_speakPrepareTimer, SIGNAL(timeout()), this, SLOT(onSpeakPrepareTimeout()));

	ui->labelMemberInOutTip->clear();
	m_memberInOutTipTimer.setSingleShot(true);
	m_memberInOutTipTimer.setInterval(3000);
	connect(&m_memberInOutTipTimer, SIGNAL(timeout()), ui->labelMemberInOutTip, SLOT(hide()));

	setSpeaker(QString());

	setTip(QString());

	setMemberCount(0);

	connect(ui->speakButton, SIGNAL(toggled(bool)), this, SLOT(onSpeakButtonToogled(bool)));

	ui->tableWidget->setFocusPolicy(Qt::NoFocus);
	ui->tableWidget->setShowGrid(false);
	ui->tableWidget->setSelectionBehavior(QTableView::SelectItems);
	ui->tableWidget->setSelectionMode(QTableView::SingleSelection);
	ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
	ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	ui->tableWidget->horizontalHeader()->setVisible(false);
	ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	setMembers(QStringList());

	QIcon icon(":/session/lvoice2.png");
	ui->btnMicrophone->setFocusPolicy(Qt::NoFocus);
	ui->btnMicrophone->setIcon(icon);
	ui->btnMicrophone->setToolTip(tr("Microphone Adjustment"));
	connect(ui->btnMicrophone, SIGNAL(enter()), this, SLOT(onEnterBtnStopAudio()));
	connect(ui->btnMicrophone, SIGNAL(leave()), this, SLOT(onLeaveBtnStopAudio()));

	icon = QIcon(":/session/lmessage_blue.png");
	ui->btnOpenChatDialog->setFocusPolicy(Qt::NoFocus);
	ui->btnOpenChatDialog->setIcon(icon);
	ui->btnOpenChatDialog->setToolTip(tr("Open Chat"));

	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)));
}

void InterphoneDialog::setSpeaker(const QString &id)
{
	QPixmap avatar;
	QString name;
	
	if (!id.isEmpty())
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		avatar = modelManager->getUserAvatar(id);
		name = modelManager->userName(id);
	}

	ui->speakerAvatar->setAvatar(avatar);
	if (!name.isEmpty())
		ui->labelSpeakerName->setText(name);
	else
		ui->labelSpeakerName->clear();
}

void InterphoneDialog::setTip(const QString &tip, bool warning /*= false*/)
{
	if (!warning)
	{
		ui->labelTipInfo->setStyleSheet("color: rgb(31, 188, 107); font: bold 12pt;");
	}
	else
	{
		ui->labelTipInfo->setStyleSheet("color: rgb(255, 188, 0); font: bold 12pt;");
	}
	ui->labelTipInfo->setText(tip);
}

void InterphoneDialog::setMemberCount(int count)
{
	ui->labelNumber->setText(QString::number(count));
}

void InterphoneDialog::setMembers(const QStringList &memberIds)
{
	QStringList ids = memberIds;

	configMemberInOut(m_memberWidgets.keys(), ids);
	
	int count = ids.count();
	const int kCol = 7;
	int rows = count/kCol;
	if ((count%kCol))
		++rows;

	m_memberWidgets.clear();
	ui->tableWidget->clear();
	ui->tableWidget->setRowCount(rows);
	ui->tableWidget->setColumnCount(kCol);

	int row = 0;
	int col = 0;
	const int kRowHeight   = 80;
	const int kColumnWidth = 56;
	ModelManager *modelManager = qPmApp->getModelManager();
	for (int i = 0; i < count; i++)
	{
		QString id = ids[i];

		// insert row
		if ((col%kCol) == 0)
		{
			ui->tableWidget->setRowHeight(row, kRowHeight);
		}
		ui->tableWidget->setColumnWidth(col, kColumnWidth);

		InterphoneMemberItemWidget *widget = new InterphoneMemberItemWidget(ui->tableWidget);
		widget->setMaximumWidth(kColumnWidth);
		QPixmap avatar = modelManager->getUserAvatar(id);
		widget->setId(id);
		widget->setAvatar(avatar);
		widget->setName(modelManager->userName(id));
		connect(widget, SIGNAL(chat(QString)), this, SLOT(chat(QString)));
		ui->tableWidget->setCellWidget(row, col, widget);
		m_memberWidgets.insert(id, widget);

		++col;
		if ((col%kCol) == 0)
		{
			++row;
			col = 0;
		}
	}
	ui->tableWidget->resizeColumnsToContents();
}

QString InterphoneDialog::configTipText(const QString &speaker, int memberCount)
{
	QString tip;
	if (speaker.isEmpty())
	{
		if (memberCount == 1)
			tip = tr("Waiting others come in...");
	}
	else
	{
		tip = tr("Speaking...");
	}
	return tip;
}

void InterphoneDialog::prepareSpeak()
{
	if (m_speakState == SpeakPrepareing)
	{
		InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
		interphoneManager->prepareSpeak(m_interphoneId);

		setSpeakState(SpeakPrepared);
	}
}

void InterphoneDialog::stopSpeak()
{
	if (m_speakState == SpeakPrepared || m_speakState == Speaking)
	{
		InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
		interphoneManager->stopSpeak(m_interphoneId);
	}
}

void InterphoneDialog::setSpeakState(SpeakState state)
{
	SpeakState oldState = m_speakState;
	m_speakState = state;
	onSpeakStateChanged(oldState, m_speakState);
}

void InterphoneDialog::onSpeakStateChanged(SpeakState /*oldState*/, SpeakState newState)
{
	if (newState == SpeakPrepareing)
	{
		m_speakPrepareTimer.start();
		ui->speakButton->setSignalState(SpeakButton::Prepare);
		setTip(tr("Preparing..."));
	}
	else if (newState == SpeakPrepared)
	{
		// do nothing
	}
	else if (newState == Speaking)
	{
		// set speaker
		setSpeaker(m_interphone.speakerId());

		// set tip
		QString tip = configTipText(m_interphone.speakerId(), m_interphone.memberCount());
		setTip(tip);

		// update ui
		startPlaySoundWave();

		// ready state
		ui->speakButton->setSignalState(SpeakButton::Ready);
	}
	else if (newState == SpeakConflict)
	{
		// set tip
		QString tip = tr("Someone is speaking");
		setTip(tip, true);

		// update ui
		ui->speakButton->setSignalState(SpeakButton::Busy);
	}
	else if (newState == SpeakStop)
	{
		// set speaker
		setSpeaker(m_interphone.speakerId());

		// set tip
		QString tip = configTipText(m_interphone.speakerId(), m_interphone.memberCount());
		setTip(tip);

		// update ui
		stopPlaySoundWave();

		// normal state
		ui->speakButton->setSignalState(SpeakButton::Normal);
	}
}

void InterphoneDialog::playSound(SoundType soundType)
{
	if (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "QSound is not available";
		return;
	}

	QString fileName = QCoreApplication::applicationDirPath() + "/Misc/sounds/";
	switch (soundType)
	{
	case Press:
		fileName.append("press.wav");
		break;
	case Release:
		fileName.append("release.wav");
		break;
	case Ready:
		fileName.append("ready.wav");
		break;
	case OtherReady:
		fileName.append("otherready.wav");
		break;
	case Conflict:
		fileName.append("conflict.wav");
		break;
	default:
		fileName = "";
		break;
	}

	if (!fileName.isEmpty() && QFile::exists(fileName))
	{
		QSound::play(fileName);
	}
}

void InterphoneDialog::startTalk()
{
	if (m_session)
	{
		m_session->startTalk();
	}
}

void InterphoneDialog::stopTalk()
{
	if (m_session)
	{
		m_session->stopTalk();
	}
}

void InterphoneDialog::startTick()
{
	m_tickSeconds = 0;
	setTickText();
	m_tickTimer.start();
}

void InterphoneDialog::stopTick()
{
	m_tickTimer.stop();
	m_tickSeconds = 0;
	ui->labelTime->clear();
}

void InterphoneDialog::setTickText()
{
	int minute = m_tickSeconds/60;
	int second = m_tickSeconds%60;
	QString timeText = QString("%1:%2").arg(minute, 2, 10, QChar('0')).arg(second, 2, 10, QChar('0'));
	ui->labelTime->setText(timeText);
}

void InterphoneDialog::quitInterphone()
{
	QString selfId = Account::instance()->id();
	InterphoneManager *interphoneManager = qPmApp->getInterphoneManager();
	interphoneManager->quitInterphone(m_interphoneId, selfId);
}

MicrophoneControlPanel *InterphoneDialog::microphoneControlPanel()
{
	if (m_pMicrophoneControlPanel.isNull())
	{
		m_pMicrophoneControlPanel.reset(new MicrophoneControlPanel());
	}
	return m_pMicrophoneControlPanel.data();
}

void InterphoneDialog::showMemberInOutTip(const QString &tip)
{
	if (tip.isEmpty())
		return;

	ui->labelMemberInOutTip->setText(tip);
	ui->labelMemberInOutTip->setVisible(true);
	m_memberInOutTipTimer.start();
}

void InterphoneDialog::configMemberInOut(const QStringList &origMembers, const QStringList &newMembers)
{
	if (origMembers.isEmpty() || newMembers.isEmpty())
		return;

	QString selfId = Account::instance()->id();
	if (!origMembers.contains(selfId) || !newMembers.contains(selfId))
		return;

	QSet<QString> origIdSet = origMembers.toSet();
	QSet<QString> newIdSet = newMembers.toSet();
	QSet<QString> delIds = origIdSet - newIdSet;
	QSet<QString> addIds = newIdSet - origIdSet;

	ModelManager *modelManager = qPmApp->getModelManager();
	if (addIds.size() > 0)
	{
		QStringList addIdList = addIds.toList();
		foreach (QString addId, addIdList)
		{
			QString name = modelManager->userName(addId);
			QString tip = tr("%1 enter").arg(name);
			showMemberInOutTip(tip);
		}
	}

	if (delIds.size() > 0)
	{
		QStringList delIdList = delIds.toList();
		foreach (QString delId, delIdList)
		{
			QString name = modelManager->userName(delId);
			QString tip = tr("%1 quit").arg(name);
			showMemberInOutTip(tip);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SpeakerAvatar
SpeakerAvatar::SpeakerAvatar(QWidget *parent /*= 0*/) : QWidget(parent)
{
	m_border = QPixmap(":/interphone/border.png");
}

SpeakerAvatar::~SpeakerAvatar()
{

}

void SpeakerAvatar::setAvatar(const QPixmap &avatar)
{
	m_avatar = avatar;
	update();
}

void SpeakerAvatar::paintEvent(QPaintEvent *ev)
{
	if (m_avatar.isNull())
	{
		QWidget::paintEvent(ev);
		return;
	}

	QPainter painter(this);
	QRect rt = rect();
	QRect avatarRt = rt.adjusted(2, 2, -2, -2);
	QRegion region(avatarRt, QRegion::Ellipse);
	painter.setClipRegion(region);
	painter.setClipping(true);

	// draw avatar
	if (m_avatar.size() != avatarRt.size())
	{
		m_avatar = m_avatar.scaled(avatarRt.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	painter.drawPixmap(avatarRt.topLeft(), m_avatar);

	// draw border
	painter.setClipping(false);
	painter.drawPixmap(rt.topLeft(), m_border);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SpeakButton
SpeakButton::SpeakButton(QWidget *parent /*= 0*/)
: QWidget(parent), m_pressed(false), m_signalState(Normal), m_enabled(true)
{
	m_normalPixmap = QPixmap(":/interphone/button_normal.png");
	m_pressedPixmap = QPixmap(":/interphone/button_pressed.png");
	m_disabledPixmap = QPixmap(":/interphone/button_disabled.png");

	m_prepareSignalPixmap = QPixmap(":/interphone/talk_pressed.png");
	m_readySignalPixmap = QPixmap(":/interphone/talk_normal.png");
	m_busySignalPixmap = QPixmap(":/interphone/talk_busy.png");
}

SpeakButton::~SpeakButton()
{

}

void SpeakButton::setPressed(bool pressed)
{
	if (!m_enabled)
		return;

	bool oldPress = m_pressed;
	m_pressed = pressed;
	if (m_pressed != oldPress)
	{
		emit toggled(m_pressed);

		update();
	}
}

bool SpeakButton::isPressed() const
{
	return m_pressed;
}

void SpeakButton::setEnabled(bool enabled)
{
	m_enabled = enabled;
	update();
}

SpeakButton::SignalState SpeakButton::signalState() const
{
	return m_signalState;
}

void SpeakButton::setSignalState(SignalState state)
{
	m_signalState = state;
	update();
}

void SpeakButton::paintEvent(QPaintEvent * /*ev*/)
{
	QPainter painter(this);
	QRect rt = rect();

	// draw background
	if (!m_enabled)
	{
		painter.drawPixmap(rt.topLeft(), m_disabledPixmap);
		return;
	}

	if (!m_pressed)
	{
		painter.drawPixmap(rt.topLeft(), m_normalPixmap);
	}
	else
	{
		painter.drawPixmap(rt.topLeft(), m_pressedPixmap);
	}

	// draw signal
	QPoint signalPt = rt.topLeft();
	signalPt.setX(rt.right() - 74);
	signalPt.setY(rt.top() + 48);
	QPixmap signalPixmap;
	if (m_signalState == Prepare)
	{
		signalPixmap = m_prepareSignalPixmap;
	}
	else if (m_signalState == Ready)
	{
		signalPixmap = m_readySignalPixmap;
	}
	else if (m_signalState == Busy)
	{
		signalPixmap = m_busySignalPixmap;
	}

	if (!signalPixmap.isNull())
	{
		painter.drawPixmap(signalPt, signalPixmap);
	}

	// draw text
	QPen pen(QColor(152, 152, 152));
	pen.setWidth(8);
	painter.setPen(pen);
	QFont font = painter.font();
	font.setBold(true);
	painter.setBrush(Qt::NoBrush);
	if (!m_pressed)
	{
		font.setPixelSize(24);
		painter.setFont(font);

		QRect topRt = rt;
		topRt.setBottom(rt.bottom() - rt.height()/2 - 4);
		painter.drawText(topRt, Qt::AlignHCenter|Qt::AlignBottom, tr("Press speak"));

		font.setPixelSize(17);
		painter.setFont(font);
		QRect bottomRt = rt;
		bottomRt.setTop(rt.top() + rt.height()/2 + 4);
		painter.drawText(bottomRt, Qt::AlignHCenter|Qt::AlignTop, tr("or space speak"));
	}
}

void SpeakButton::mousePressEvent(QMouseEvent *ev)
{
	setPressed(true);

	QWidget::mousePressEvent(ev);
}

void SpeakButton::mouseReleaseEvent(QMouseEvent *ev)
{
	setPressed(false);

	QWidget::mouseReleaseEvent(ev);
}

void SpeakButton::mouseMoveEvent(QMouseEvent *ev)
{
	QWidget::mouseMoveEvent(ev);

	QRect rt = rect();
	if (!rt.contains(ev->pos()))
	{
		setPressed(false);
	}

	ev->accept();
}
