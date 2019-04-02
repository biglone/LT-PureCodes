#include "searchitemwidget.h"
#include "ui_searchitemwidget.h"
#include <QPixmap>
#include "util/MaskUtil.h"

SearchItemWidget::SearchItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::SearchItemWidget();
	ui->setupUi(this);

	ui->labelName->setStyleSheet("color: black;");
	ui->labelAvatar->setToolTip(tr("View Contact Profile"));
	ui->pushButtonAdd->setToolTip(tr("Add Friends"));
	
	ui->telTag->setPixmap(QPixmap(":/images/Icon_50_2.png"));
	this->setStyleSheet("QWidget#mainWidget {border: 1px solid rgb(219, 219, 219);}");
}

SearchItemWidget::~SearchItemWidget()
{
	delete ui;
}

void SearchItemWidget::setId(const QString &id)
{
	m_id = id;
}

QString SearchItemWidget::id() const
{
	return m_id;
}

void SearchItemWidget::setAvatar(const QPixmap &avatar)
{
	QSize avatarSize = ui->labelAvatar->size();
	QPixmap pixmapAvatar = avatar.scaled(avatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, avatarSize);
	
	ui->labelAvatar->setMask(mask);
	ui->labelAvatar->setPixmap(pixmapAvatar);
}

void SearchItemWidget::setName(const QString &name, const QColor &color /*= QColor(0, 0, 0)*/)
{
	ui->labelName->setText(name);
	ui->labelName->setStyleSheet(QString("color: %1;").arg(color.name()));
}

void SearchItemWidget::setSex(int sex)
{
	Q_UNUSED(sex);
	/*
	if (sex == 0)
	{
		ui->labelSex->setPixmap(QPixmap(":/images/Icon_95.png"));
	}
	else if (sex == 1)
	{
		ui->labelSex->setPixmap(QPixmap(":/images/Icon_94.png"));
	}
	else
	{
		ui->labelSex->setPixmap(QPixmap());
	}
	*/
}

void SearchItemWidget::setDepart(const QString &depart)
{
	ui->labelDepart->setText(depart);
}

void SearchItemWidget::setPhone(const QString &phone)
{
	ui->labelPhone->setText(phone);
}

void SearchItemWidget::setAddEnabled(bool enabled)
{
	ui->pushButtonAdd->setEnabled(enabled);
}

void SearchItemWidget::on_pushButtonAdd_clicked()
{
	emit addFriend(m_id);
}

void SearchItemWidget::on_labelAvatar_clicked()
{
	emit showMaterial(m_id);
}