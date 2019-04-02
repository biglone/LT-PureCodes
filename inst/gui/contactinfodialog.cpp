#include "contactinfodialog.h"
#include "ui_contactinfodialog.h"
#include "PmApp.h"
#include "QStringListModel"
#include "commonlistitemdelegate.h"
#include "model/ModelManager.h"
#include "bean/DetailItem.h"
#include "model/orgstructmodeldef.h"
#include "model/orgstructitemdef.h"
#include "QRCodeUtil.h"
#include "model/rostermodeldef.h"
#include "settings/GlobalSettings.h"
#include "bigavatardialog.h"
#include "widgetmanager.h"

ContactInfoDialog::ContactInfoDialog(const QString &uid, QWidget *parent)
	: FramelessDialog(parent), m_uid(uid)
{
	ui = new Ui::ContactInfoDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(m_uid);
	QString titleText = name + tr(" Profile");
	setWindowIcon(qApp->windowIcon());
	ui->title->setText(titleText);
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);

	if (GlobalSettings::introductionViewType() == 0)
	{
		resize(471, 545);
		setFixedSize(471, 545);
	}
	else
	{
		resize(471, 505);
		setFixedSize(471, 505);
	}
	
	setResizeable(false);

	initUI();

	initSignals();

	setSkin();

	// sync detail every time
	modelManager->syncDetail(m_uid);
}

ContactInfoDialog::~ContactInfoDialog()
{
	delete ui;
}

void ContactInfoDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_7.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 131;
	bgSizes.bottomBarHeight = 32;
	bgSizes.leftBarWidth = 0;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("QLabel {color: white; font-size: 12pt;}");

	ui->labelName->setStyleSheet("QLabel {color: white; font-size: 11pt;}");

	ui->btnBasic->setStyleSheet(
		"QPushButton {"
		"padding-left: 0px;"
		"padding-right: 0px;"
		"border: none;"
		"border-image: none;"
		"background: transparent;"
		"border-bottom: 3px solid rgb(0, 120, 216);"
		"color: rgb(0, 120, 206);"
		"font-size: 11pt;"
		"}"
		"QPushButton:hover:!pressed {"
		"border-image: none;"
		"color: rgb(0, 120, 206);"
		"}"
		);
	ui->btnJob->setStyleSheet(
		"QPushButton {"
		"padding-left: 0px;"
		"padding-right: 0px;"
		"border: none;"
		"border-image: none;"
		"background: transparent;"
		"color: #333333;"
		"font-size: 11pt;"
		"}"
		"QPushButton:hover:!pressed {"
		"border-image: none;"
		"color: rgb(0, 120, 206);"
		"}"
		);

	ui->labelQRCode->setStyleSheet("background-color: white;");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pushButtonRefresh->setStyleSheet(qss);
		ui->pushButtonClose->setStyleSheet(qss);
		qssFile.close();
	}

	/*
	// edit style
	qssFile.setFileName(":/theme/qss/lineedit2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		setStyleSheet(qss);
		qssFile.close();
	}
	*/

	StyleToolButton::Info btnInfo;
	btnInfo.urlNormal = QString(":/images/addfriend_normal.png");
	btnInfo.urlHover = QString(":/images/addfriend_hover.png");
	btnInfo.urlPressed = QString(":/images/addfriend_pressed.png");
	btnInfo.tooltip = tr("Add Friends");
	ui->tBtnAddFriend->setInfo(btnInfo);

	btnInfo.urlNormal = QString(":/images/chat_normal.png");
	btnInfo.urlHover = QString(":/images/chat_hover.png");
	btnInfo.urlPressed = QString(":/images/chat_pressed.png");
	btnInfo.tooltip = tr("Send Message");
	ui->tBtnChat->setInfo(btnInfo);
}

void ContactInfoDialog::onDetailChanged(const QString &id)
{
	if (id == m_uid)
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QString name = modelManager->userName(m_uid);
		QString titleText = name + tr(" Profile");
		ui->title->setText(titleText);
		setWindowTitle(ui->title->text());

		setDetailInfo();
	}
}

void ContactInfoDialog::onRefresh()
{
	qPmApp->getModelManager()->syncDetail(m_uid);
}

void ContactInfoDialog::chat()
{
	emit chat(m_uid);
}

void ContactInfoDialog::addFriend()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(m_uid);
	emit addFriendRequest(m_uid, name);
}

void ContactInfoDialog::closeDialog()
{
	emit contactInfoClose(m_uid);

	close();
}

void ContactInfoDialog::on_btnBasic_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
}

void ContactInfoDialog::on_btnJob_clicked()
{
	ui->stackedWidget->setCurrentIndex(1);
}

void ContactInfoDialog::currentPageChanged(int index)
{
	if (index == 0)
	{
		ui->btnBasic->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"border-bottom: 3px solid rgb(0, 120, 216);"
			"color: rgb(0, 120, 206);"
			"font-size: 11pt;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
		ui->btnJob->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"color: #333333;"
			"font-size: 11pt;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
	}
	else if (index == 1)
	{
		ui->btnJob->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"border-bottom: 3px solid rgb(0, 120, 216);"
			"color: rgb(0, 120, 206);"
			"font-size: 11pt;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
		ui->btnBasic->setStyleSheet(
			"QPushButton {"
			"padding-left: 0px;"
			"padding-right: 0px;"
			"border: none;"
			"border-image: none;"
			"background: transparent;"
			"color: #333333;"
			"font-size: 11pt;"
			"}"
			"QPushButton:hover:!pressed {"
			"border-image: none;"
			"color: rgb(0, 120, 206);"
			"}"
			);
	}
}

void ContactInfoDialog::onRosterChanged()
{
	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	if (rosterModel->isFriend(m_uid))
	{
		ui->tBtnAddFriend->setVisible(false);
	}
	else
	{
		ui->tBtnAddFriend->setVisible(true);
	}
}

void ContactInfoDialog::viewBigAvatar()
{
	BigAvatarDialog *bigAvatarDialog = BigAvatarDialog::getDialog();
	bigAvatarDialog->getBigAvatar(m_uid);
	WidgetManager::showActivateRaiseWindow(bigAvatarDialog);
}

void ContactInfoDialog::initUI()
{
	ui->labelAvatar->setClickable(true);

	if (GlobalSettings::introductionViewType() == 0)
	{
		ui->labelIconGender->setVisible(false);
		ui->switchBar->setVisible(true);
		ui->stackedWidget->setCurrentIndex(0);
	}
	else
	{
		ui->labelIconGender->setVisible(true);
		ui->switchBar->setVisible(false);
		ui->stackedWidget->setCurrentIndex(2);
	}

	RosterModel *rosterModel = qPmApp->getModelManager()->rosterModel();
	if (rosterModel->isFriend(m_uid))
	{
		ui->tBtnAddFriend->setVisible(false);
	}
	else
	{
		ui->tBtnAddFriend->setVisible(true);
	}
	connect(rosterModel, SIGNAL(rosterChanged()), this, SLOT(onRosterChanged()));

	// hide organization
	ui->tagOrganization->setVisible(false);
	ui->labelOrganization->setVisible(false);

	// hide chat function
	ui->tBtnChat->setVisible(false);

	// set every detail info
	setDetailInfo();
}

void ContactInfoDialog::initSignals()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)));

	connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(currentPageChanged(int)));

	connect(ui->pushButtonRefresh, SIGNAL(clicked()), this, SLOT(onRefresh()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(closeDialog()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(closeDialog()));
	connect(ui->tBtnChat, SIGNAL(clicked()), this, SLOT(chat()));
	connect(ui->tBtnAddFriend, SIGNAL(clicked()), this, SLOT(addFriend()));
	connect(ui->labelAvatar, SIGNAL(clicked()), this, SLOT(viewBigAvatar()));
}

void ContactInfoDialog::setDetailInfo()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *index = modelManager->detailItem(m_uid);
	QString name = modelManager->userName(index->uid());
	QString deptName = index->depart();
	QString organization = index->organization();

	ui->labelAvatar->setPixmap(modelManager->getUserAvatar(m_uid));

	ui->labelName->setText(name);
	ui->labelDept->setText(deptName);
	ui->labelOrganization->setText(organization);

	if (!index->message().isEmpty())
	{
		ui->labelSignature->setText(index->message());
	}
	else
	{
		ui->labelSignature->setText(tr("No what's up"));
	}

	int sex = index->sex();
	if (sex < 0 && sex > 1)
	{
		sex = 9;
	}
	if (sex == 0)
	{
		ui->labelSex->setText(tr("Female"));
		ui->labelIconGender->setPixmap(QPixmap(":/images/Icon_95_white.png"));
	}
	else if (sex == 1)
	{
		ui->labelSex->setText(tr("Male"));
		ui->labelIconGender->setPixmap(QPixmap(":/images/Icon_94_white.png"));
	}
	else 
	{
		ui->labelSex->setText(tr("Secret"));
		ui->labelIconGender->setVisible(false);
	}

	ui->labelBirthday->setText(index->birthday());

	ui->labelPhone1->setText(index->phone1());
	// ui->labelPhone2->setText(index->phone2());
	ui->labelPhone3->setText(index->phone3());
	ui->labelMail->setText(index->email());

	ui->labelPosition->setText(index->duty());
	// ui->labelLocation->setText(index->area());
	if (!index->jobDesc().isEmpty())
	{
		ui->labelContent->setText(index->jobDesc());
	}
	else
	{
		ui->labelContent->setText(tr("No working content"));
	}

	// -- simplicity view
	ui->labelOrganization_2->setText(organization);
	ui->labelDept_2->setText(deptName);
	ui->labelPhone1_2->setText(index->phone1());
	ui->labelMail_2->setText(index->email());
	// -- simplicity view

	QString tel = ui->labelPhone1->text();
	QString mail = ui->labelMail->text();
	if (!name.isEmpty() && (!tel.isEmpty() || !mail.isEmpty()))
	{
		ui->labelQRCode->setVisible(true);
		QString vCard = QString("BEGIN:VCARD\n"
			"FN:%1\n"
			"TEL:%2\n"
			"EMAIL:%3\n"
			"END:VCARD").arg(name).arg(tel).arg(mail);
		QByteArray vCardData = vCard.toUtf8();
		QImage qrImage;
		if (QRCodeUtil::getQRImage(qrImage, vCardData, 2, QColor("#000000")))
		{
			ui->labelQRCode->setPixmap(QPixmap::fromImage(qrImage));
		}
	}
	else
	{
		ui->labelQRCode->setVisible(false);
	}
}