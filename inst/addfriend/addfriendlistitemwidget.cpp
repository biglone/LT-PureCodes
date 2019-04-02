#include "addfriendlistitemwidget.h"
#include "ui_addfriendlistitemwidget.h"
#include "common/datetime.h"

AddFriendListItemWidget::AddFriendListItemWidget(QWidget *parent)
	: QWidget(parent), m_deletable(false), m_index(0)
{
	ui = new Ui::AddFriendListItemWidget();
	ui->setupUi(this);

	ui->labelAvatar->setClickable(true);
	ui->widgetMain->setStyleSheet("QWidget#widgetMain {background: white; border: solid 1px rgb(229, 229, 229);}");
	ui->labelName->setStyleSheet("font: bold 10.5pt; color: black;");
	ui->label->setStyleSheet("color: rgb(128, 128, 128);");
	ui->labelDate->setStyleSheet("color: rgb(128, 128, 128);");

	StylePushButton::Info info;
	info.urlNormal = QString(":/images/close_normal.png");
	info.urlHover = QString(":/images/close_pressed.png");
	info.urlPressed = QString(":/images/close_pressed.png");
	info.tooltip = tr("Delete record");
	ui->pushButtonDelete->setInfo(info);

	ui->pushButtonDelete->setVisible(false);
}

AddFriendListItemWidget::~AddFriendListItemWidget()
{
	delete ui;
}

void AddFriendListItemWidget::setIndex(int index)
{
	m_index = index;
}

void AddFriendListItemWidget::setAvatar(const QPixmap &avatar)
{
	QPixmap pixmap = avatar.scaled(QSize(ui->labelAvatar->size()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->labelAvatar->setPixmap(pixmap);
}

void AddFriendListItemWidget::setName(const QString &name)
{
	ui->labelName->setText(name);
}

void AddFriendListItemWidget::setSex(int sex)
{
	if (sex == 0)
	{
		ui->labelSex->setVisible(true);
		ui->labelSex->setPixmap(QPixmap(":/images/Icon_95.png"));
	}
	else if (sex == 1)
	{
		ui->labelSex->setVisible(true);
		ui->labelSex->setPixmap(QPixmap(":/images/Icon_94.png"));
	}
	else
	{
		ui->labelSex->setVisible(false);
		ui->labelSex->setPixmap(QPixmap());
	}
}

void AddFriendListItemWidget::setDepart(const QString &dept)
{
	ui->labelDept->setText(dept);
}

void AddFriendListItemWidget::setPhone(const QString &phone)
{
	ui->labelPhone->setText(phone);
}

void AddFriendListItemWidget::setDate(const QString &date)
{
	QDateTime dateTime = CDateTime::localDateTimeFromUtcString(date);
	QDateTime currentDateTime = CDateTime::currentDateTime();
	if (dateTime.daysTo(currentDateTime) == 0)
	{
		ui->labelDate->setText(dateTime.toString("hh:mm"));
	}
	else
	{
		ui->labelDate->setText(tr("%1.%2").arg(dateTime.date().month()).arg(dateTime.date().day()));
	}
}

void AddFriendListItemWidget::setAction(const QString &/*fromId*/, const QString &toId, const QString &selfId, AddFriendManager::Action action)
{
	ui->pushButtonAccept->setVisible(false);
	ui->pushButtonRefuse->setVisible(false);
	ui->labelResult->setVisible(true);
	ui->widgetMain->setStyleSheet("QWidget#widgetMain {background: rgb(245, 251, 247); border: solid 1px rgb(229, 229, 229);}");

	if (toId == selfId)
	{
		if (action == AddFriendManager::Request)
		{
			ui->pushButtonAccept->setVisible(true);
			ui->pushButtonRefuse->setVisible(true);
			ui->labelResult->setVisible(false);
			ui->labelResult->setText("");
			ui->widgetMain->setStyleSheet("QWidget#widgetMain {background: white; border: solid 1px rgb(229, 229, 229);}");
		}
		else if (action == AddFriendManager::Accept)
		{
			ui->labelResult->setText(tr("I agreed"));
		}
		else
		{
			ui->labelResult->setText(tr("I refused"));
		}
	}
	else
	{
		if (action == AddFriendManager::Request)
		{
			ui->labelResult->setText(tr("Waiting"));
		}
		else if (action == AddFriendManager::Accept)
		{
			ui->labelResult->setText(tr("Agreed"));
		}
		else
		{
			ui->labelResult->setText(tr("Refused"));
		}
	}
}

void AddFriendListItemWidget::setMessage(const QString &message)
{
	QString simplifiedMessage = message.simplified();
	ui->labelMessage->setText(simplifiedMessage);
}

void AddFriendListItemWidget::setDeletable(bool deletable)
{
	m_deletable = deletable;
}

void AddFriendListItemWidget::enterEvent(QEvent *e)
{
	QWidget::enterEvent(e);

	if (m_deletable)
		ui->pushButtonDelete->setVisible(true);
}

void AddFriendListItemWidget::leaveEvent(QEvent *e)
{
	ui->pushButtonDelete->setVisible(false);

	QWidget::leaveEvent(e);
}

void AddFriendListItemWidget::on_pushButtonAccept_clicked()
{
	emit accept(m_index);
}

void AddFriendListItemWidget::on_pushButtonRefuse_clicked()
{
	emit refuse(m_index);
}

void AddFriendListItemWidget::on_labelAvatar_clicked()
{
	emit viewMaterial(m_index);
}

void AddFriendListItemWidget::on_pushButtonDelete_clicked()
{
	emit deleteItem(m_index);
}
