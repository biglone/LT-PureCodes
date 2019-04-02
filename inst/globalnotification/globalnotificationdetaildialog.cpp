#include "globalnotificationdetaildialog.h"
#include "ui_globalnotificationdetaildialog.h"
#include "globalnotificationdetail.h"
#include "PmApp.h"
#include "globalnotificationmanager.h"
#include "Account.h"
#include "common/datetime.h"
#include "ModelManager.h"
#include "globalnotificationmodel.h"
#include "pmessagebox.h"
#include <QPainter>
#include "loginmgr.h"
#include "widgetborder.h"
#include <QBitmap>
#include "util/MaskUtil.h"
#include "util/TextUtil.h"
#include <QToolTip>

GlobalNotificationDetailDialog::GlobalNotificationDetailDialog(const QString &globalNotificationId, QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_globalNotificationId(globalNotificationId)
{
	ui = new Ui::GlobalNotificationDetailDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(290, 442);
	setResizeable(false);

	initUI();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	setSkin();
}

GlobalNotificationDetailDialog::~GlobalNotificationDetailDialog()
{
	delete ui;
}

void GlobalNotificationDetailDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	StylePushButton::Info pushButtonInfo;
	pushButtonInfo.urlNormal = QString(":/subscription/unfollow.png");
	pushButtonInfo.urlHover = QString(":/subscription/unfollow_down.png");
	pushButtonInfo.urlPressed = QString(":/subscription/unfollow_down.png");
	ui->pushButtonUnfollow->setInfo(pushButtonInfo);

	pushButtonInfo.urlNormal = QString(":/subscription/history.png");
	pushButtonInfo.urlHover = QString(":/subscription/history_down.png");
	pushButtonInfo.urlPressed = QString(":/subscription/history_down.png");
	ui->pushButtonHistory->setInfo(pushButtonInfo);

	pushButtonInfo.urlNormal = QString(":/subscription/enter.png");
	pushButtonInfo.urlHover = QString(":/subscription/enter_down.png");
	pushButtonInfo.urlPressed = QString(":/subscription/enter_down.png");
	ui->pushButtonMsg->setInfo(pushButtonInfo);

	ui->pushButtonUnfollow->setTextWithColor(tr("Unfollow"), QColor(255, 87, 34).name(),
		QColor(255, 255, 255).name(), QColor(255, 255, 255).name());
	ui->pushButtonHistory->setTextWithColor(tr("Chat History"), QColor(72, 167, 243).name(),
		QColor(255, 255, 255).name(), QColor(255, 255, 255).name());
	ui->pushButtonMsg->setTextWithColor(tr("Enter Subscription"), QColor(50, 177, 108).name(),
		QColor(255, 255, 255).name(), QColor(255, 255, 255).name());

	ui->labelNum->setStyleSheet("color: #999999;");
	ui->labelIntroduction->setStyleSheet("color: #999999; font-size: 9pt;");
}

void GlobalNotificationDetailDialog::onUnsubscribeFailed()
{
	GlobalNotificationModel *globalNotificationModel = qPmApp->getModelManager()->globalNotificationModel();
	GlobalNotificationDetail globalNotification = globalNotificationModel->globalNotification(m_globalNotificationId);
	PMessageBox::warning(this, tr("Tip"), tr("Unfollow %1 failed, please try again").arg(globalNotification.name()));
}

void GlobalNotificationDetailDialog::paintEvent(QPaintEvent *ev)
{
	FramelessDialog::paintEvent(ev);

	QPainter painter(this);
	QRect rt = rect();
	QPoint pt = rt.topLeft();
	pt += QPoint(5, 5);
	QPixmap pixmap(":/subscription/detail_background.png");
	painter.drawPixmap(pt, pixmap);
}

void GlobalNotificationDetailDialog::on_pushButtonUnfollow_clicked()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, please try when online"));
		return;
	}

	GlobalNotificationModel *globalNotificationModel = qPmApp->getModelManager()->globalNotificationModel();
	GlobalNotificationDetail globalNotification = globalNotificationModel->globalNotification(m_globalNotificationId);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Unfollow"), 
		tr("Do you want to unfollow %1").arg(globalNotification.name()), QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret == QDialogButtonBox::Yes)
	{
		GlobalNotificationManager *globalNotificationManager = qPmApp->getGlobalNotificationManager();
		globalNotificationManager->unsubscribe(m_globalNotificationId, Account::instance()->id(), CDateTime::currentDateTimeUtcString());
	}
}

void GlobalNotificationDetailDialog::on_pushButtonHistory_clicked()
{
	emit openGlobalNotificationHistory(m_globalNotificationId);
}

void GlobalNotificationDetailDialog::on_pushButtonMsg_clicked()
{
	emit openGlobalNotificationMsg(m_globalNotificationId);
}

void GlobalNotificationDetailDialog::onLogoChanged(const QString &globalNotificationId)
{
	if (globalNotificationId == m_globalNotificationId)
	{
		// icon
		ModelManager *modelManager = qPmApp->getModelManager();
		QPixmap logo = modelManager->globalNotificationLogo(m_globalNotificationId);
		setWindowIcon(QIcon(logo));

		// label icon
		setLogoLabel();
	}
}

void GlobalNotificationDetailDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	GlobalNotificationModel *globalNotificationModel = modelManager->globalNotificationModel();
	GlobalNotificationDetail globalNotification = globalNotificationModel->globalNotification(m_globalNotificationId);

	QPixmap logo = modelManager->globalNotificationLogo(m_globalNotificationId);
	setLogoLabel();
	setWindowIcon(QIcon(logo));
	setWindowTitle(globalNotification.name() + tr("'s Detail"));
	ui->labelName->setText(globalNotification.name());
	ui->labelNum->setText(tr("ID: %1").arg(globalNotification.num()));

	// introduction
	QString fullIntroduction = tr("Function: ") + globalNotification.introduction();
	QFont font = ui->labelIntroduction->font();
	font.setPointSize(9);
	QFontMetrics fm(font);
	QString introduction = fm.elidedText(fullIntroduction, Qt::ElideRight, 660);
	ui->labelIntroduction->setText(introduction);
	if (fullIntroduction != introduction)
		ui->labelIntroduction->setToolTip(TextUtil::wrapText(QToolTip::font(), fullIntroduction, 180));

	if (globalNotification.special())
	{
		ui->pushButtonUnfollow->setVisible(false);
		setFixedSize(290, 398);
	}

	connect(globalNotificationModel, SIGNAL(globalNotificationLogoChanged(QString)), this, SLOT(onLogoChanged(QString)));
}

void GlobalNotificationDetailDialog::setLogoLabel()
{
	QSize logoSize(66, 66);
	ModelManager *modelManager = qPmApp->getModelManager();
	QPixmap logo = modelManager->globalNotificationLogo(m_globalNotificationId);
	logo = logo.scaled(logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, logoSize);
	ui->labelLogo->setMask(mask);
	ui->labelLogo->setPixmap(logo);
}