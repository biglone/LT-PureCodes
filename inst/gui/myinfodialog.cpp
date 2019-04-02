#include "myinfodialog.h"
#include "ui_myinfodialog.h"
#include "PmApp.h"
#include "Account.h"
#include "QStringListModel"
#include "commonlistitemdelegate.h"
#include "model/ModelManager.h"
#include "bean/DetailItem.h"
#include "QxtTimer.h"
#include "ModifyProcess.h"
#include "DialogAvatarEditor.h"
#include "model/orgstructmodeldef.h"
#include "model/orgstructitemdef.h"
#include "pmessagebox.h"
#include "QRCodeUtil.h"
#include "calendarwidget.h"
#include "util/AvatarUtil.h"
#include "detailphotomanager.h"
#include "settings/GlobalSettings.h"

const int SIGNATURE_MAX_LEN    = 100;
const int JOB_CONTENT_MAX_LEN  = 100;

MyInfoDialog *MyInfoDialog::s_instance = 0;

MyInfoDialog::MyInfoDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::MyInfoDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);

	if (GlobalSettings::introductionViewType() == 0)
	{
		resize(471, 535);
		setFixedSize(471, 535);
	}
	else
	{
		resize(471, 505);
		setFixedSize(471, 505);
	}

	setResizeable(false);

	m_uid = qPmApp->getAccount()->id();

	initUI();

	initSignals();

	setSkin();
}

MyInfoDialog::~MyInfoDialog()
{
	s_instance = 0;

	delete ui;
}

MyInfoDialog *MyInfoDialog::instance()
{
	if (!s_instance)
		s_instance = new MyInfoDialog();
	return s_instance;
}

void MyInfoDialog::setSkin()
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
		ui->pushButtonSave->setStyleSheet(qss);
		ui->pushButtonCancel->setStyleSheet(qss);
		qssFile.close();
	}

	// edit style
	QStringList fileNames;
	fileNames << ":/theme/qss/lineedit2_skin.qss" << ":/theme/qss/combobox2_skin.qss" << ":/theme/qss/dateedit2_skin.qss";
	QString dlgQss;
	foreach (QString fileName, fileNames)
	{
		qssFile.setFileName(fileName);
		if (qssFile.open(QIODevice::ReadOnly))
		{
			QString qss = qssFile.readAll();
			dlgQss += qss;
			qssFile.close();
		}
	}
	setStyleSheet(dlgQss);
}

void MyInfoDialog::onDetailChanged(const QString &id)
{
	if (id == m_uid)
	{
		disconnectContentChanged();
		setDetailInfo();
		connectContentChanged();
	}
}

void MyInfoDialog::onRefresh()
{
	m_mapSaveItem.clear();
	ui->pushButtonSave->setEnabled(false);

	qPmApp->getModelManager()->syncDetail(m_uid);
}

void MyInfoDialog::onSave()
{
	applyContent();
}

void MyInfoDialog::onContentChanged()
{
	if (sender() == ui->textEditSignature)
	{
		QString text = ui->textEditSignature->toPlainText();
		if (text.length() > SIGNATURE_MAX_LEN)
		{
			text = text.left(SIGNATURE_MAX_LEN);
			ui->textEditSignature->setPlainText(text);
			ui->textEditSignature->moveCursor(QTextCursor::End);
		}

		contentChanged(bean::DETAIL_MESSAGE, ui->textEditSignature->toPlainText());
	}
	else if (sender() == ui->comboBoxSex)
	{
		contentChanged(bean::DETAIL_SEX, QString("%1").arg(ui->comboBoxSex->itemData(ui->comboBoxSex->currentIndex()).toInt()));
	}
	else if (sender() == ui->dateEditBirthday)
	{
		contentChanged(bean::DETAIL_BIRTHDAY, ui->dateEditBirthday->date().toString("yyyy-MM-dd"));
	}
	else if (sender() == ui->lineEditPhone1)
	{
		contentChanged(bean::DETAIL_PHONE1, ui->lineEditPhone1->text());
	}
	/*
	else if (sender() == ui->lineEditPhone2)
	{
		contentChanged(bean::DETAIL_PHONE2, ui->lineEditPhone2->text());
	}
	*/
	else if (sender() == ui->lineEditPhone3)
	{
		contentChanged(bean::DETAIL_PHONE3, ui->lineEditPhone3->text());
	}
	else if (sender() == ui->lineEditMail)
	{
		contentChanged(bean::DETAIL_EMAIL, ui->lineEditMail->text());
	}
	else if (sender() == ui->lineEditPosition)
	{
		contentChanged(bean::DETAIL_DUTY, ui->lineEditPosition->text());
	}
	/*
	else if (sender() == ui->lineEditLocation)
	{
		contentChanged(bean::DETAIL_AREA, ui->lineEditLocation->text());
	}
	*/
	else if (sender() == ui->textEditContent)
	{
		contentChanged(bean::DETAIL_JOBDESC, ui->textEditContent->toPlainText());
	}
	else if (sender() == ui->lineEditPhone1_2)
	{
		contentChanged(bean::DETAIL_PHONE1, ui->lineEditPhone1_2->text());
	}
	else if (sender() == ui->lineEditMail_2)
	{
		contentChanged(bean::DETAIL_EMAIL, ui->lineEditMail_2->text());
	}
}

void MyInfoDialog::onFinish()
{
	setCursor(Qt::ArrowCursor);
	setEnabled(true);

	// update detail
	Account* pAccout = Account::instance();
	QString avatarPath = pAccout->avatarPath();
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *detailItem = modelManager->detailItem(m_uid);
	bean::DetailItem *newItem = detailItem->clone();
	foreach (int role, m_mapSaveItem.keys())
	{
		if (role == (int)(bean::DETAIL_PHOTO))
		{
			// via http upload
			/*
			QImage avatarImage = qvariant_cast<QImage>(m_mapSaveItem.value(bean::DETAIL_PHOTO));
			qPmApp->getDetailPhotoManager()->setAvatar(m_uid, avatarImage);
			*/
		}
		else
		{
			newItem->setData((bean::DetailDataRole)role, m_mapSaveItem[role]);
		}
	}
	qPmApp->getDetailPhotoManager()->setDetail(m_uid, newItem);

	m_mapSaveItem.clear();
	ui->pushButtonSave->setEnabled(false);

	PMessageBox::information(this, tr("Tip"), tr("Profile modified successfully"));

	QObject* obj = sender();
	if (obj)
	{
		obj->deleteLater();
	}
}

void MyInfoDialog::onError(const QString &errMsg)
{
	setCursor(Qt::ArrowCursor);
	setEnabled(true);

	PMessageBox::information(this, tr("Profile modify failed"), errMsg);

	QObject* obj = sender();
	if (obj)
	{
		obj->deleteLater();
	}
}

void MyInfoDialog::modifyAvatar()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QPixmap avatarPixmap = modelManager->getUserAvatar(m_uid);
	if (m_mapSaveItem.contains(bean::DETAIL_PHOTO))
	{
		QImage avatarImage = qvariant_cast<QImage>(m_mapSaveItem.value(bean::DETAIL_PHOTO));
		avatarPixmap = QPixmap::fromImage(avatarImage);
	}
	
	DialogAvatarEditor dlg(m_uid, this);
	dlg.setOrignalAvatar(avatarPixmap);
	dlg.setWindowModality(Qt::WindowModal);
	if (dlg.exec())
	{
		if (dlg.isModified())
		{
			QPixmap newPixmap = dlg.getAvatar();
			newPixmap = newPixmap.scaled(QSize(180, 180), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QImage newImage = newPixmap.toImage();

			// update avatar label
			ui->labelAvatar->setPixmap(newPixmap);

			// save to file
			qPmApp->getDetailPhotoManager()->setAvatar(m_uid, newImage);
		}
	}
}

void MyInfoDialog::on_btnBasic_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
}

void MyInfoDialog::on_btnJob_clicked()
{
	ui->stackedWidget->setCurrentIndex(1);
}

void MyInfoDialog::currentPageChanged(int index)
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

void MyInfoDialog::initUI()
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

	// sex settings
	ui->comboBoxSex->addItem(tr("Female"), 0);
	ui->comboBoxSex->addItem(tr("Male"), 1);
	ui->comboBoxSex->addItem(tr("Secret"), 9);

	// phone validator
	/*
	QRegExpValidator *phoneValidator = new QRegExpValidator(QRegExp("((\\d{11})|^((\\d{7,8})|(\\d{4}|\\d{3})-(\\d{7,8})|(\\d{4}|\\d{3})-(\\d{7,8})-(\\d{4}|\\d{3}|\\d{2}|\\d{1})|(\\d{7,8})-(\\d{4}|\\d{3}|\\d{2}|\\d{1}))$)"), this);
	ui->lineEditPhone1->setValidator(phoneValidator);
	ui->lineEditPhone2->setValidator(phoneValidator);
	ui->lineEditPhone3->setValidator(phoneValidator);
	*/
	ui->lineEditPhone1->setMaxLength(20);
	ui->lineEditPhone1_2->setMaxLength(20);
	// ui->lineEditPhone2->setMaxLength(20);
	ui->lineEditPhone3->setMaxLength(20);

	ui->lineEditPosition->setMaxLength(20);
	// ui->lineEditLocation->setMaxLength(60);

	// set email max length
	ui->lineEditMail->setMaxLength(60);
	ui->lineEditMail_2->setMaxLength(60);

	// birthday edit
	ui->dateEditBirthday->setDateRange(ui->dateEditBirthday->minimumDate(), QDate::currentDate());
	ui->dateEditBirthday->setCalendarPopup(true);
	CalendarWidget *calendarWidget = new CalendarWidget(ui->dateEditBirthday);
	ui->dateEditBirthday->setCalendarWidget(calendarWidget);

	// save button
	ui->pushButtonSave->setEnabled(false);

	// hide organization
	ui->tagOrganization->setVisible(false);
	ui->labelOrganization->setVisible(false);

	// set every detail info
	setDetailInfo();
}

void MyInfoDialog::initSignals()
{
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	connect(ui->labelAvatar, SIGNAL(clicked()), this, SLOT(modifyAvatar()));

	connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(currentPageChanged(int)));

	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)));

	connect(ui->pushButtonRefresh, SIGNAL(clicked()), this, SLOT(onRefresh()));
	connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
	connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));

	connectContentChanged();
}

void MyInfoDialog::setDetailInfo()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *index = modelManager->detailItem(m_uid);
	QString name = modelManager->userName(m_uid);
	QString deptName = index->depart();
	QString organization = index->organization();

	ui->labelAvatar->setPixmap(modelManager->getUserAvatar(m_uid));

	ui->labelName->setText(name);
	ui->labelDept->setText(deptName);
	ui->labelOrganization->setText(organization);

	ui->textEditSignature->setText(index->message());
	ui->textEditSignature->moveCursor(QTextCursor::Start);

	int sex = index->sex();
	if (sex < 0 && sex > 1)
	{
		sex = 9;
	}
	ui->comboBoxSex->setCurrentIndex(ui->comboBoxSex->findData(sex));

	if (sex == 0)
	{
		ui->labelIconGender->setPixmap(QPixmap(":/images/Icon_95_white.png"));
	}
	else if (sex == 1)
	{
		ui->labelIconGender->setPixmap(QPixmap(":/images/Icon_94_white.png"));
	}
	else 
	{
		ui->labelIconGender->setVisible(false);
	}

	QDate dateBirthday = QDate::fromString(index->birthday(), "yyyy-MM-dd");
	ui->dateEditBirthday->setDate(dateBirthday);

	ui->lineEditPhone1->setText(index->phone1());
	// ui->lineEditPhone2->setText(index->phone2());
	ui->lineEditPhone3->setText(index->phone3());
	ui->lineEditMail->setText(index->email());

	ui->lineEditPosition->setText(index->duty());
	// ui->lineEditLocation->setText(index->area());
	ui->textEditContent->clear();
	ui->textEditContent->insertPlainText(index->jobDesc());
	ui->textEditContent->moveCursor(QTextCursor::Start);

	// -- simplicity view
	ui->labelOrganization_2->setText(organization);
	ui->labelDept_2->setText(deptName);
	ui->lineEditPhone1_2->setText(index->phone1());
	ui->lineEditMail_2->setText(index->email());
	// -- simplicity view

	QString tel = ui->lineEditPhone1->text();
	QString mail = ui->lineEditMail->text();
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

void MyInfoDialog::contentChanged(int role, const QString &val)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *pItem = modelManager->detailItem(m_uid);
	if (!pItem)
		return;

	if (pItem->data((bean::DetailDataRole)role) == val)
	{
		m_mapSaveItem.remove(role);
		return;
	}

	m_mapSaveItem[role] = val;
	if (!m_mapSaveItem.isEmpty())
	{
		ui->pushButtonSave->setEnabled(true);
	}
}

void MyInfoDialog::applyContent()
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, please save when online"));
		return;
	}

	QRegExp re("^[^@]+@[^@]+$");
	QString email = ui->lineEditMail->text();
	if (!email.isEmpty() && !re.exactMatch(email))
	{
		PMessageBox::information(this, tr("Input Error"), tr("Email format is not correct, please input again"));
		return;
	}

	if (m_mapSaveItem.value(bean::DETAIL_JOBDESC).toString().length() > JOB_CONTENT_MAX_LEN)
	{
		PMessageBox::information(this, tr("Input Error"), tr("Job duties should be less than 100 characters"));
		return;
	}

	if (m_mapSaveItem.value(bean::DETAIL_MESSAGE).toString().length() > SIGNATURE_MAX_LEN)
	{
		PMessageBox::information(this, tr("Input Error"), tr("What's up should be less than 100 charaters"));
		return;
	}

 	setEnabled(false);
	setCursor(Qt::WaitCursor);

	ModifyProcess *proc = new ModifyProcess(this);
	proc->initObject();
	connect(proc, SIGNAL(finish()), SLOT(onFinish()));
	connect(proc, SIGNAL(error(QString)), SLOT(onError(QString)));

	if (!proc->sendModify(m_mapSaveItem))
	{
		QxtTimer::singleShot(0, this, SLOT(onError(QString)), tr("You are offline, please save when online"));
	}
}

void MyInfoDialog::connectContentChanged()
{
	connect(ui->textEditSignature, SIGNAL(textChanged()), this, SLOT(onContentChanged()));
	connect(ui->comboBoxSex, SIGNAL(currentIndexChanged(int)), this, SLOT(onContentChanged()));
	connect(ui->dateEditBirthday, SIGNAL(dateChanged(QDate)), this, SLOT(onContentChanged()));
	connect(ui->lineEditPhone1, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	// connect(ui->lineEditPhone2, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	connect(ui->lineEditPhone3, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	connect(ui->lineEditMail, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	connect(ui->lineEditPosition, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	// connect(ui->lineEditLocation, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	connect(ui->textEditContent, SIGNAL(textChanged()), this, SLOT(onContentChanged()));

	connect(ui->lineEditPhone1_2, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	connect(ui->lineEditMail_2, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
}

void MyInfoDialog::disconnectContentChanged()
{
	disconnect(ui->textEditSignature, SIGNAL(textChanged()), this, SLOT(onContentChanged()));
	disconnect(ui->comboBoxSex, SIGNAL(currentIndexChanged(int)), this, SLOT(onContentChanged()));
	disconnect(ui->dateEditBirthday, SIGNAL(dateChanged(QDate)), this, SLOT(onContentChanged()));
	disconnect(ui->lineEditPhone1, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	// disconnect(ui->lineEditPhone2, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	disconnect(ui->lineEditPhone3, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	disconnect(ui->lineEditMail, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	disconnect(ui->lineEditPosition, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	// disconnect(ui->lineEditLocation, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	disconnect(ui->textEditContent, SIGNAL(textChanged()), this, SLOT(onContentChanged()));

	disconnect(ui->lineEditPhone1_2, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
	disconnect(ui->lineEditMail_2, SIGNAL(textChanged(QString)), this, SLOT(onContentChanged()));
}
