#include "selectcontactitemwidget.h"
#include "ui_selectcontactitemwidget.h"
#include <QStandardItem>
#include <QFile>
#include <QMovie>

QPixmap* SelectContactItemWidget::s_groupExpand = 0;
QPixmap* SelectContactItemWidget::s_groupCollapse = 0;
bool SelectContactItemWidget::s_inited = false;

SelectContactItemWidget::SelectContactItemWidget(QStandardItem *itemData, bool group, const QString &name, 
												 bool enableCheck /*= true*/, QWidget *parent /*= 0*/)
	: QWidget(parent), m_itemData(itemData)
{
	ui = new Ui::SelectContactItemWidget();
	ui->setupUi(this);

	if (!isIconInited())
	{
		initGroupPixmap();
		setIconInited(true);
	}

	if (group)
	{
		ui->labelGroup->setPixmap(*s_groupCollapse);
		ui->labelIcon->setVisible(false);
	}
	else
	{
		ui->labelGroup->setPixmap(QPixmap());
		ui->labelIcon->setVisible(true);
	}

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);
	ui->labelLoading->setVisible(false);

	ui->labelName->setText(name);
	ui->checkBox->setCheckState(m_itemData->checkState());
	ui->checkBox->setEnabled(enableCheck);

	connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckStateChanged(int)));
}

SelectContactItemWidget::~SelectContactItemWidget()
{
	delete ui;
}

void SelectContactItemWidget::setGroupExpanded(bool expanded)
{
	if (expanded)
	{
		ui->labelGroup->setPixmap(*s_groupExpand);
	}
	else
	{
		ui->labelGroup->setPixmap(*s_groupCollapse);
	}
}

void SelectContactItemWidget::setChecked(bool checked, bool partial /*= false*/)
{
	disconnect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckStateChanged(int)));
	if (partial)
	{
		ui->checkBox->setCheckState(Qt::PartiallyChecked);
	}
	else
	{
		if (checked)
		{
			ui->checkBox->setCheckState(Qt::Checked);		
		}
		else
		{
			ui->checkBox->setCheckState(Qt::Unchecked);
		}
	}
	connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckStateChanged(int)));
}

void SelectContactItemWidget::setCheckEnabled(bool enabled)
{
	ui->checkBox->setEnabled(enabled);
}

void SelectContactItemWidget::startLoading()
{
	ui->labelGroup->setVisible(false);
	ui->labelLoading->setVisible(true);
	ui->labelLoading->movie()->start();
}

void SelectContactItemWidget::stopLoading()
{
	ui->labelLoading->movie()->stop();
	ui->labelLoading->setVisible(false);
	ui->labelGroup->setVisible(true);
}

void SelectContactItemWidget::setTitle(const QString &title)
{
	ui->labelName->setText(title);
}

void SelectContactItemWidget::onCheckStateChanged(int state)
{
	if (state == Qt::Checked)
	{
		emit itemToggled(m_itemData, true);
	}
	else if (state == Qt::Unchecked)
	{
		emit itemToggled(m_itemData, false);
	}
}

void SelectContactItemWidget::initGroupPixmap()
{
	QPixmap tmpPixmap = QPixmap(":/images/Icon_42.png");
	s_groupExpand = new QPixmap(tmpPixmap);

	tmpPixmap = QPixmap(":/images/Icon_43.png");
	s_groupCollapse = new QPixmap(tmpPixmap);
}

bool SelectContactItemWidget::isIconInited()
{
	return s_inited;
}

void SelectContactItemWidget::setIconInited(bool inited)
{
	s_inited = inited;
}
