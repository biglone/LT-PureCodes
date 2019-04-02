#include "editfiltergroupitemwidget.h"
#include "ui_editfiltergroupitemwidget.h"

EditFilterGroupItemWidget::EditFilterGroupItemWidget(QWidget *parent)
	: QWidget(parent), m_type(Roster)
{
	ui = new Ui::EditFilterGroupItemWidget();
	ui->setupUi(this);

	ui->groupNameLabel->setStyleSheet("color: rgb(128, 128, 128);");
	ui->viewAllLabel->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->viewAllLabel->setVisible(false);
}

EditFilterGroupItemWidget::~EditFilterGroupItemWidget()
{
	delete ui;
}

void EditFilterGroupItemWidget::setType(ItemType itemType)
{
	m_type = itemType;
}

void EditFilterGroupItemWidget::setGroupText(const QString &groupText)
{
	ui->groupNameLabel->setText(groupText);
}

void EditFilterGroupItemWidget::setViewAllVisible(bool visible)
{
	ui->viewAllLabel->setVisible(visible);
}

void EditFilterGroupItemWidget::on_viewAllLabel_clicked()
{
	emit viewAll((int)m_type);
}

