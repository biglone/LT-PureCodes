#include "grouppanel.h"
#include "ui_grouppanel.h"
#include "grouplistview.h"
#include "discusstreeview.h"
#include "ModelManager.h"

GroupPanel::GroupPanel(GroupListView *groupListView, DiscussTreeView *discussTreeView, QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::GroupPanel();
	ui->setupUi(this);

	groupListView->setParent(this);
	discussTreeView->setParent(this);

	ui->stackedWidget->addWidget(groupListView);
	ui->stackedWidget->addWidget(discussTreeView);

	ui->tabDiscuss->setText(tr("Discusses"));
	ui->tabGroup->setText(tr("Groups"));
	ui->tbtnCreateDiscuss->setText(tr("Create Discuss"));

	connect(ui->tabGroup, SIGNAL(clicked()), this, SLOT(activeGroupPage()));
	connect(ui->tabDiscuss, SIGNAL(clicked()), this, SLOT(activeDiscussPage()));
	connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(currentPageChanged(int)));
	connect(ui->tbtnCreateDiscuss, SIGNAL(clicked()), this, SIGNAL(createDiscuss()));

	ui->stackedWidget->setCurrentIndex(0);

	ui->tbtnCreateDiscuss->setStyleSheet(
		"QToolButton:!checked:hover:!pressed{"
		"	border-image: none;"
		"	background-color: #e0eefb;"
		"}"
		"QToolButton:!checked:hover:pressed{"
		"	color: white;"
		"	border-image: none;"
		"	background-color: #0078d8;"
		"}"
		);

	ui->tabGroup->setStyleSheet(
		"QPushButton {"
		"padding-left: 0px;"
		"padding-right: 0px;"
		"border: none;"
		"border-image: none;"
		"background: transparent;"
		"border-bottom: 3px solid rgb(0, 120, 216);"
		"color: #333333;"
		"}"
		"QPushButton:hover:!pressed {"
		"border-image: none;"
		"color: rgb(0, 120, 206);"
		"}"
		);
	ui->tabDiscuss->setStyleSheet(
		"QPushButton {"
		"padding-left: 0px;"
		"padding-right: 0px;"
		"border: none;"
		"border-image: none;"
		"background: transparent;"
		"color: #333333;"
		"}"
		"QPushButton:hover:!pressed {"
		"border-image: none;"
		"color: rgb(0, 120, 206);"
		"}"
		);

	this->setStyleSheet(QString(
		"QWidget#widgetHead {"
		"background-color: rgb(255, 255, 255);"
		"border: none;"
		"border-bottom: 1px solid rgb(239, 235, 232);"
		"}"
		));
}

GroupPanel::~GroupPanel()
{
	delete ui;
}

void GroupPanel::activeGroupPage()
{
	if (ui->stackedWidget->currentIndex() != 0)
		ui->stackedWidget->setCurrentIndex(0);
}

void GroupPanel::activeDiscussPage()
{
	if (ui->stackedWidget->currentIndex() != 1)
		ui->stackedWidget->setCurrentIndex(1);
}

void GroupPanel::currentPageChanged(int index)
{
	if (index == 0)
	{
		ui->tabGroup->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"border-bottom: 3px solid rgb(0, 120, 216);"
			"color: #333333;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
		ui->tabDiscuss->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"color: #333333;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
	}
	else if (index == 1)
	{
		ui->tabDiscuss->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"border-bottom: 3px solid rgb(0, 120, 216);"
			"color: #333333;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
		ui->tabGroup->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"color: #333333;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
	}
}
