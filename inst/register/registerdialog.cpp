#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "countrycodedlg.h"
#include "settings/GlobalSettings.h"
#include "pmessagebox.h"
#include "PmApp.h"
#include "companyregistermanager.h"
#include "joincompanylistdialog.h"
#include "customercompanylistdialog.h"

RegisterDialog::RegisterDialog(QWidget *parent)
	: FramelessDialog(parent), m_countryCode("86")
{
	ui = new Ui::RegisterDialog();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(410, 464);
	setResizeable(false);

	initUI();

	setSkin();
}

RegisterDialog::~RegisterDialog()
{
	delete ui;
}

void RegisterDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");
	ui->widgetJoin->setStyleSheet("QWidget#widgetJoin {border: 1px solid rgb(220, 220, 220); background-color: white; }");
	ui->widgetFrom->setStyleSheet("QWidget#widgetFrom {border: 1px solid rgb(220, 220, 220); background-color: white; }");
	ui->widgetCountryCode->setStyleSheet("QWidget#widgetCountryCode {border: 1px solid rgb(220, 220, 220); background-color: white; }");
	ui->labelStatus->setStyleSheet("color: #470010;");

	ui->labelAsterisk->setStyleSheet("QLabel {color: red;}");
	ui->labelAsterisk1->setStyleSheet("QLabel {color: red;}");
	ui->labelAsterisk2->setStyleSheet("QLabel {color: red;}");
	ui->labelAsterisk3->setStyleSheet("QLabel {color: red;}");
	ui->labelAsterisk4->setStyleSheet("QLabel {color: red;}");

	ui->labelJoin->setFontAtt(QColor("#333333"), 10, false);
	ui->labelJoin->setEnterUnderline(false);

	ui->labelFrom->setFontAtt(QColor("#333333"), 10, false);
	ui->labelFrom->setEnterUnderline(false);

	ui->labelCountryCode->setFontAtt(QColor("#333333"), 10, false);
	ui->labelCountryCode->setEnterUnderline(false);

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->btnOK->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}

	// set style
	StyleToolButton::Info btnStyle;
	btnStyle.urlNormal = QString(":/images/right_arrow_normal.png");
	btnStyle.urlHover = QString(":/images/right_arrow_hover.png");
	btnStyle.urlPressed = QString(":/images/right_arrow_hover.png");
	btnStyle.tooltip = tr("More");
	ui->tBtnMoreCountry->setInfo(btnStyle);
	ui->tBtnMoreJoin->setInfo(btnStyle);
	ui->tBtnMoreFrom->setInfo(btnStyle);
}

void RegisterDialog::on_tBtnMoreJoin_clicked()
{
	JoinCompanyListDialog dlg;
	if (dlg.exec())
	{
		m_toJoinCompanyId = dlg.selCompanyId();
		m_toJoinCompanyName = dlg.selCompanyName();
		ui->labelJoin->setText(m_toJoinCompanyName);
	}
}

void RegisterDialog::on_tBtnMoreFrom_clicked()
{
	CustomerCompanyListDialog dlg;
	if (dlg.exec())
	{
		QString fromCompanyName = dlg.selCompanyName();
		ui->labelFrom->setText(fromCompanyName);
	}
}

void RegisterDialog::on_tBtnMoreCountry_clicked()
{
	CountryCodeDlg countryCodeDlg;
	countryCodeDlg.setWindowFlags(countryCodeDlg.windowFlags() | Qt::WindowStaysOnTopHint);
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		countryCodeDlg.showChineseCountryCode();
	else
		countryCodeDlg.showEnglishCountryCode();
	if (countryCodeDlg.exec())
	{
		QString countryCode = countryCodeDlg.selectedCountryCode();
		if (!countryCode.isEmpty())
		{
			m_countryCode = countryCode;
			ui->labelCountryCode->setText(QString("+%1").arg(m_countryCode));
		}
	}
}

void RegisterDialog::onOKClicked()
{
	if (m_toJoinCompanyId.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("You have to choose a company to join"));
		return;
	}
	
	QString fromCompanyName = ui->labelFrom->text().trimmed();
	if (fromCompanyName.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("You have to enter your company"));
		return;
	}

	// get name
	QString firstName = ui->leditFirstName->text().trimmed();
	QString lastName = ui->leditLastName->text().trimmed();
	if (firstName.isEmpty() && lastName.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("You have to enter your name"));
		return;
	}

	QString name = firstName;
	if (name.isEmpty())
	{
		name = lastName;
	}
	else
	{
		if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
			name.append(lastName);
		else
			name.append(QString(" ")+lastName);
	}

	// get phone
	QString phone = ui->leditMobile->text().trimmed();
	if (phone.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("You have enter your phone"));
		return;
	}

	if (m_countryCode != QString("86"))
		phone = QString("00") + m_countryCode + phone;

	// email
	QRegExp re("^[^@]+@[^@]+$");
	QString email = ui->leditEmail->text().trimmed();
	if (!email.isEmpty() && !re.exactMatch(email))
	{
		PMessageBox::information(this, tr("Tip"), tr("Email format is not right"));
		return;
	}

	// job title
	QString jobTitle = ui->leditJotTitle->text().trimmed();

	// memo
	QString memo = ui->leditMemo->text().trimmed();

	qPmApp->getCompanyRegisterManager()->doRegister(name, phone, fromCompanyName, 
		m_toJoinCompanyId, email, jobTitle, memo, "");
	beginRegister();
}

void RegisterDialog::onRegisterOK()
{
	endRegister();

	ui->labelStatus->setText(tr("Register information have been delivered OK"));

	ui->btnOK->setEnabled(false);
}

void RegisterDialog::onRegisterFailed(int retCode, const QString &desc)
{
	endRegister();

	ui->labelStatus->setText(tr("Register failed: %1(%2)").arg(desc).arg(retCode));
}

void RegisterDialog::onTextEdited()
{
	ui->labelStatus->clear();
}

void RegisterDialog::initUI()
{
	ui->labelStatus->clear();
	ui->labelCountryCode->setText(QString("+%1").arg(m_countryCode));
	ui->labelJoin->setText(m_toJoinCompanyName);
	ui->labelFrom->setText(tr("Individual"));

	ui->leditFirstName->setMaxLength(9);
	ui->leditLastName->setMaxLength(10);
	ui->leditMobile->setMaxLength(16);
	ui->leditEmail->setMaxLength(50);
	ui->leditJotTitle->setMaxLength(32);
	ui->leditMemo->setMaxLength(100);

	CompanyRegisterManager *companyRegisterManager = qPmApp->getCompanyRegisterManager();
	connect(companyRegisterManager, SIGNAL(registerOK()), this, SLOT(onRegisterOK()));
	connect(companyRegisterManager, SIGNAL(registerFailed(int, QString)), this, SLOT(onRegisterFailed(int, QString)));

	connect(ui->labelJoin, SIGNAL(clicked()), this, SLOT(on_tBtnMoreJoin_clicked()));
	connect(ui->labelFrom, SIGNAL(clicked()), this, SLOT(on_tBtnMoreFrom_clicked()));
	connect(ui->labelCountryCode, SIGNAL(clicked()), this, SLOT(on_tBtnMoreCountry_clicked()));
	
	connect(ui->leditFirstName, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->leditLastName, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->leditMobile, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->leditEmail, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->leditJotTitle, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));
	connect(ui->leditMemo, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void RegisterDialog::beginRegister()
{
	ui->labelJoin->setEnabled(false);
	ui->labelFrom->setEnabled(false);
	ui->tBtnMoreJoin->setEnabled(false);
	ui->tBtnMoreFrom->setEnabled(false);
	ui->leditFirstName->setEnabled(false);
	ui->leditLastName->setEnabled(false);
	ui->tBtnMoreCountry->setEnabled(false);
	ui->leditMobile->setEnabled(false);
	ui->leditJotTitle->setEnabled(false);
	ui->leditMemo->setEnabled(false);
	ui->btnOK->setEnabled(false);
	ui->btnClose->setEnabled(false);
}

void RegisterDialog::endRegister()
{
	ui->labelJoin->setEnabled(true);
	ui->labelFrom->setEnabled(true);
	ui->tBtnMoreJoin->setEnabled(true);
	ui->tBtnMoreFrom->setEnabled(true);
	ui->leditFirstName->setEnabled(true);
	ui->leditLastName->setEnabled(true);
	ui->tBtnMoreCountry->setEnabled(true);
	ui->leditMobile->setEnabled(true);
	ui->leditJotTitle->setEnabled(true);
	ui->leditMemo->setEnabled(true);
	ui->btnOK->setEnabled(true);
	ui->btnClose->setEnabled(true);
}