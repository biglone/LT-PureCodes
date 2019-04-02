#include "globalnotificationitemwidget.h"
#include "ui_globalnotificationitemwidget.h"
#include <QPixmap>
#include "util/MaskUtil.h"
#include "ModelManager.h"
#include "Account.h"

GlobalNotificationItemWidget::GlobalNotificationItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::GlobalNotificationItemWidget();
	ui->setupUi(this);

	ui->labelName->setStyleSheet("color: black;");
	ui->labelNum->setStyleSheet("color: black; font-size: 9pt;");
	ui->labelIntroduction->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
	ui->pushButtonAdd->setToolTip(tr("Follow"));

	this->setStyleSheet("QWidget#mainWidget {border: 1px solid rgb(219, 219, 219);}");
}

GlobalNotificationItemWidget::~GlobalNotificationItemWidget()
{
	delete ui;
}

void GlobalNotificationItemWidget::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
	ui->labelAvatar->setNetworkAccessManager(networkAccessManager);
}

void GlobalNotificationItemWidget::setGlobalNotification(const GlobalNotificationDetail &globalNotification)
{
	m_globalNotification = globalNotification;

	// set icon
	QSize size = ui->labelAvatar->size();
	QIcon defaultIcon = ModelManager::globalNotificationDefaultIcon();
	QPixmap pixmap = defaultIcon.pixmap(size);

	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, size);

	ui->labelAvatar->setMask(mask);
	ui->labelAvatar->setPixmap(pixmap);

	ui->labelAvatar->setCacheDir(Account::instance()->globalNotificationPath());
	ui->labelAvatar->setPixmapSize(size);
	ui->labelAvatar->setHttpUrl(globalNotification.logo());

	// set name
	ui->labelName->setText(globalNotification.name());

	// set num
	ui->labelNum->setText(tr("ID: %1").arg(globalNotification.num()));

	// set introduction
	ui->labelIntroduction->setText(globalNotification.introduction());
}

GlobalNotificationDetail GlobalNotificationItemWidget::globalNotification() const
{
	return m_globalNotification;
}

void GlobalNotificationItemWidget::setSubscribeEnabled(bool enabled)
{
	ui->pushButtonAdd->setEnabled(enabled);
	if (enabled)
	{
		ui->pushButtonAdd->setToolTip(tr("Follow"));
	}
	else
	{
		ui->pushButtonAdd->setToolTip(tr("Followed"));
	}
}

void GlobalNotificationItemWidget::on_pushButtonAdd_clicked()
{
	emit subscribe(m_globalNotification);
}

void GlobalNotificationItemWidget::onSucribeSucceed(const QString &globalNotificationId)
{
	if (m_globalNotification.id() == globalNotificationId)
	{
		setSubscribeEnabled(false);
	}
}
