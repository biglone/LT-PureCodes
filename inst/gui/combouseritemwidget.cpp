#include "combouseritemwidget.h"
#include "ui_combouseritemwidget.h"
#include "util/MaskUtil.h"

ComboUserItemWidget::ComboUserItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::ComboUserItemWidget();
	ui->setupUi(this);

	ui->avatarLabel->setVisible(false);

	ui->closeToolButton->setVisible(false);

	ui->nameLabel->setStyleSheet("color: black;");

	ui->backWidget->setStyleSheet("QWidget#backWidget {background: #FFFFFF;}");
}

ComboUserItemWidget::~ComboUserItemWidget()
{
	delete ui;
}

void ComboUserItemWidget::setAvatar(const QPixmap &avatar)
{
	QPixmap pixmap = avatar.scaled(ui->avatarLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->avatarLabel->setPixmap(pixmap);

	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, ui->avatarLabel->size());
	ui->avatarLabel->setMask(mask);

	ui->avatarLabel->setVisible(true);
}

void ComboUserItemWidget::setName(const QString &name)
{
	ui->nameLabel->setText(name);
}

void ComboUserItemWidget::enterEvent(QEvent *e)
{
	QWidget::enterEvent(e);

	ui->closeToolButton->setVisible(true);

	ui->backWidget->setStyleSheet("QWidget#backWidget {background: #3991D0;}");

	ui->nameLabel->setStyleSheet("color: white;");
}

void ComboUserItemWidget::leaveEvent(QEvent *e)
{
	ui->closeToolButton->setVisible(false);

	ui->nameLabel->setStyleSheet("color: black;");

	ui->backWidget->setStyleSheet("QWidget#backWidget {background: #FFFFFF;}");

	QWidget::leaveEvent(e);
}

void ComboUserItemWidget::on_closeToolButton_clicked()
{
	QString user = ui->nameLabel->text();
	emit deleteUser(user);
}
