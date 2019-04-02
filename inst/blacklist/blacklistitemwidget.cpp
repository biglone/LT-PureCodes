#include "blacklistitemwidget.h"
#include "ui_blacklistitemwidget.h"
#include "widgetborder.h"
#include "util/MaskUtil.h"

BlackListItemWidget::BlackListItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::BlackListItemWidget();
	ui->setupUi(this);

	ui->backWidget->setStyleSheet("QWidget#backWidget {background: #FFFFFF;}");
	ui->pushButtonRemove->setVisible(false);
}

BlackListItemWidget::~BlackListItemWidget()
{
	delete ui;
}

void BlackListItemWidget::setId(const QString &id)
{
	m_id = id;
}

void BlackListItemWidget::setAvatar(const QPixmap &avatar)
{
	QPixmap pixmap = avatar.scaled(ui->labelAvatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->labelAvatar->setPixmap(pixmap);

	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, ui->labelAvatar->size());
	ui->labelAvatar->setMask(mask);
}

void BlackListItemWidget::setNameText(const QString &text)
{
	ui->labelName->setText(text);
}

void BlackListItemWidget::enterEvent(QEvent *e)
{
	QWidget::enterEvent(e);

	ui->pushButtonRemove->setVisible(true);

	ui->backWidget->setStyleSheet("QWidget#backWidget {background: #DFEEFA;}");
}

void BlackListItemWidget::leaveEvent(QEvent *e)
{
	ui->pushButtonRemove->setVisible(false);

	ui->backWidget->setStyleSheet("QWidget#backWidget {background: #FFFFFF;}");

	QWidget::leaveEvent(e);
}

void BlackListItemWidget::on_pushButtonRemove_clicked()
{
	emit removeBlack(m_id);
}
