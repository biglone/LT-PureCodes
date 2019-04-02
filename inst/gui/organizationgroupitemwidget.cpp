#include "organizationgroupitemwidget.h"
#include "ui_organizationgroupitemwidget.h"
#include <QMovie>

QPixmap* OrganizationGroupItemWidget::s_groupExpand = 0;
QPixmap* OrganizationGroupItemWidget::s_groupCollapse = 0;
bool OrganizationGroupItemWidget::s_inited = false;

OrganizationGroupItemWidget::OrganizationGroupItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::OrganizationGroupItemWidget();
	ui->setupUi(this);

	if (!isIconInited())
	{
		initGroupPixmap();
		setIconInited(true);
	}

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);
	ui->labelLoading->setVisible(false);
	ui->labelGroup->setPixmap(*s_groupCollapse);

	this->setStyleSheet("font-size: 10.5pt; color: black;");
}

OrganizationGroupItemWidget::~OrganizationGroupItemWidget()
{
	delete ui;
}

void OrganizationGroupItemWidget::setGroupExpanded(bool expanded)
{
	if (expanded)
	{
		ui->labelGroup->setPixmap(*s_groupExpand);
		startLoading();
	}
	else
	{
		ui->labelGroup->setPixmap(*s_groupCollapse);
		stopLoading();
	}
}

void OrganizationGroupItemWidget::setGroupName(const QString &name)
{
	ui->labelName->setText(name);
}

void OrganizationGroupItemWidget::startLoading()
{
	ui->labelLoading->movie()->start();
	ui->labelLoading->setVisible(true);
	ui->labelGroup->setVisible(false);
}

void OrganizationGroupItemWidget::stopLoading()
{
	ui->labelLoading->movie()->stop();
	ui->labelLoading->setVisible(false);
	ui->labelGroup->setVisible(true);
}

void OrganizationGroupItemWidget::initGroupPixmap()
{
	QPixmap tmpPixmap = QPixmap(":/images/Icon_42.png");
	s_groupExpand = new QPixmap(tmpPixmap);

	tmpPixmap = QPixmap(":/images/Icon_43.png");
	s_groupCollapse = new QPixmap(tmpPixmap);
}

bool OrganizationGroupItemWidget::isIconInited()
{
	return s_inited;
}

void OrganizationGroupItemWidget::setIconInited(bool inited)
{
	s_inited = inited;
}


