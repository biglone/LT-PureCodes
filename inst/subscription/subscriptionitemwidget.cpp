#include "subscriptionitemwidget.h"
#include "ui_subscriptionitemwidget.h"
#include <QPixmap>
#include "util/MaskUtil.h"
#include "ModelManager.h"
#include "Account.h"

SubscriptionItemWidget::SubscriptionItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::SubscriptionItemWidget();
	ui->setupUi(this);

	ui->labelName->setStyleSheet("color: black;");
	ui->labelNum->setStyleSheet("color: black; font-size: 9pt;");
	ui->labelIntroduction->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
	ui->pushButtonAdd->setToolTip(tr("Follow"));

	this->setStyleSheet("QWidget#mainWidget {border: 1px solid rgb(219, 219, 219);}");
}

SubscriptionItemWidget::~SubscriptionItemWidget()
{
	delete ui;
}

void SubscriptionItemWidget::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
	ui->labelAvatar->setNetworkAccessManager(networkAccessManager);
}

void SubscriptionItemWidget::setSubscription(const SubscriptionDetail &subscription)
{
	m_subscription = subscription;

	// set icon
	QSize size = ui->labelAvatar->size();
	QIcon defaultIcon = ModelManager::subscriptionDefaultIcon();
	QPixmap pixmap = defaultIcon.pixmap(size);

	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, size);

	ui->labelAvatar->setMask(mask);
	ui->labelAvatar->setPixmap(pixmap);

	ui->labelAvatar->setCacheDir(Account::instance()->subscriptionPath());
	ui->labelAvatar->setPixmapSize(size);
	ui->labelAvatar->setHttpUrl(subscription.logo());

	// set name
	ui->labelName->setText(subscription.name());

	// set num
	ui->labelNum->setText(tr("ID: %1").arg(subscription.num()));

	// set introduction
	ui->labelIntroduction->setText(subscription.introduction());
}

SubscriptionDetail SubscriptionItemWidget::subscription() const
{
	return m_subscription;
}

void SubscriptionItemWidget::setSubscribeEnabled(bool enabled)
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

void SubscriptionItemWidget::on_pushButtonAdd_clicked()
{
	emit subscribe(m_subscription);
}

void SubscriptionItemWidget::onSucribeSucceed(const QString &subscriptionId)
{
	if (m_subscription.id() == subscriptionId)
	{
		setSubscribeEnabled(false);
	}
}
