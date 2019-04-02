#include "interphonememberitemwidget.h"
#include "ui_interphonememberitemwidget.h"

InterphoneMemberItemWidget::InterphoneMemberItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::InterphoneMemberItemWidget();
	ui->setupUi(this);

	ui->labelName->setStyleSheet("font: 9pt;");
}

InterphoneMemberItemWidget::~InterphoneMemberItemWidget()
{
	delete ui;
}

void InterphoneMemberItemWidget::setId(const QString &id)
{
	m_id = id;
}

void InterphoneMemberItemWidget::setAvatar(const QPixmap &avatar)
{
	if (avatar.isNull())
		return;

	QPixmap pixmap = avatar;
	if (pixmap.size() != ui->labelAvatar->size())
		pixmap = pixmap.scaled(ui->labelAvatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->labelAvatar->setPixmap(pixmap);
}

void InterphoneMemberItemWidget::setName(const QString &name)
{
	if (name.isEmpty())
		return;

	ui->labelName->setText(name);
	ui->labelName->setToolTip(name);
	ui->labelAvatar->setToolTip(name);
}

void InterphoneMemberItemWidget::on_labelAvatar_clicked()
{
	emit chat(m_id);
}
