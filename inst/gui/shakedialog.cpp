#include "shakedialog.h"
#include "ui_shakedialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QDesktopWidget>
#include "util/MaskUtil.h"
#include "buddymgr.h"
#include "widgetmanager.h"

static const int kShakeDialogWidth  = 245;
static const int kShakeDialogHeight = 135;
static const int kCloseCount = 10;

QPointer<ShakeDialog> ShakeDialog::s_shakeDialog;

ShakeDialog::ShakeDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::ShakeDialog();
	ui->setupUi(this);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::WindowStaysOnTopHint;
	flags |= Qt::Popup;
	flags |= Qt::Dialog;
	setWindowFlags(flags);
	setWindowTitle(tr("Shake"));

	setAttribute(Qt::WA_DeleteOnClose, true);

	setMainLayout(ui->verticalLayoutMain);
	resize(kShakeDialogWidth, kShakeDialogHeight);
	setResizeable(false);
	setMoveAble(false);

	ui->labelTip->setFontAtt(QColor("#333333"), 10, false);
	ui->labelTip->setEnterUnderline(true);

	initShake();

	initCloseTimer();
	
	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), SLOT(close()));
	connect(ui->labelTip, SIGNAL(clicked()), SLOT(onTipClicked()));
	connect(this, SIGNAL(openChat(QString)), qPmApp->getBuddyMgr(), SLOT(openChat(QString)));
}

ShakeDialog::~ShakeDialog()
{
	delete ui;
}

void ShakeDialog::setUser(const QString &uid)
{
	m_uid = uid;

	ModelManager *modelManager = qPmApp->getModelManager();
	// set avatar
	QPixmap avatar = modelManager->getUserAvatar(uid);
	QSize avatarSize = ui->labelAvatar->size();
	avatar = avatar.scaled(avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 3;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, avatarSize);
	avatar.setMask(mask);
	ui->labelAvatar->setPixmap(avatar);

	// set tip
	QString name = modelManager->userName(uid);
	QString tip = tr("%1 send you a shake message").arg(name);
	ui->labelTip->setText(tip);
}

void ShakeDialog::startShake()
{
	// show chat dialog directly
	WidgetManager::showActivateRaiseWindow(this);

	// stop close
	ui->labelCloseTip->setVisible(false);
	m_closeCount = kCloseCount;
	m_closeTimer->stop();

	// start shake
	m_shakingFrameBak = frameGeometry();
	m_shakingCount = 0;
	if (isVisible())
		m_shakingTimer->start(20);
}

void ShakeDialog::shake(const QString &uid)
{
	if (uid.isEmpty())
		return;

	if (s_shakeDialog.isNull())
	{
		s_shakeDialog = new ShakeDialog();
	}

	s_shakeDialog.data()->setUser(uid);

	QRect rcAvailable =  QApplication::desktop()->availableGeometry();
	QRect geo = rcAvailable;
	geo.setLeft(geo.right() - kShakeDialogWidth);
	geo.setTop(geo.bottom() - kShakeDialogHeight);
	s_shakeDialog.data()->setGeometry(geo);
	s_shakeDialog.data()->startShake();
}

void ShakeDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_5.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 25;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->labelTitle->setStyleSheet("QLabel {font-size: 10pt; color: white;}");

	// set close tip style
	ui->labelCloseTip->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
}

void ShakeDialog::onTipClicked()
{
	emit openChat(m_uid);
	close();
}

void ShakeDialog::shakingTimeout()
{
	m_shakingTimer->stop();
	QRect rect = m_shakingFrameBak;
	rect.translate(m_shakingPosList.at(m_shakingCount));
	setGeometry(rect);
	m_shakingCount++;
	if (m_shakingCount < m_shakingPosList.size())
	{
		m_shakingTimer->start(20);
	}
	else
	{
		// shaking finished, need auto close
		ui->labelCloseTip->setVisible(true);
		ui->labelCloseTip->setText(tr("Close after %1 seconds").arg(m_closeCount));
		m_closeTimer->start();
	}
}

void ShakeDialog::closeTimeout()
{
	m_closeCount--;
	if (m_closeCount > 0)
	{
		ui->labelCloseTip->setText(tr("Close after %1 seconds").arg(m_closeCount));
	}
	else
	{
		close();
	}
}

void ShakeDialog::initShake()
{
	// shake time, every 20ms change a pos
	m_shakingTimer = new QTimer(this);
	m_shakingTimer->setInterval(20);

	// init position list
	const int kShakingLen = 3;
	m_shakingPosList.clear();
	m_shakingPosList.append(QPoint(kShakingLen, 0));
	m_shakingPosList.append(QPoint(kShakingLen, -kShakingLen));
	m_shakingPosList.append(QPoint(-kShakingLen, -kShakingLen));
	m_shakingPosList.append(QPoint(-kShakingLen, kShakingLen));
	m_shakingPosList.append(QPoint(kShakingLen, kShakingLen));
	m_shakingPosList.append(QPoint(kShakingLen, -kShakingLen));
	m_shakingPosList.append(QPoint(-kShakingLen, -kShakingLen));
	m_shakingPosList.append(QPoint(-kShakingLen, kShakingLen));
	m_shakingPosList.append(QPoint(kShakingLen, kShakingLen));
	m_shakingPosList.append(QPoint(kShakingLen, -kShakingLen));
	m_shakingPosList.append(QPoint(-kShakingLen, -kShakingLen));
	m_shakingPosList.append(QPoint(-kShakingLen, kShakingLen));
	m_shakingPosList.append(QPoint(kShakingLen, kShakingLen));
	m_shakingPosList.append(QPoint(0, 0));

	// init signals & slots
	connect(m_shakingTimer, SIGNAL(timeout()), SLOT(shakingTimeout()));
}

void ShakeDialog::initCloseTimer()
{
	m_closeTimer = new QTimer(this);
	m_closeTimer->setSingleShot(false);
	m_closeTimer->setInterval(1000);
	connect(m_closeTimer, SIGNAL(timeout()), this, SLOT(closeTimeout()));
	m_closeCount = kCloseCount;
}

