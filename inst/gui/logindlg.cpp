#include <QDebug>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QFocusEvent>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QDataWidgetMapper>
#include <QMenu>
#include <QMovie>
#include <QDesktopServices>

#include "tooltip.h"
#include "Constants.h"

#include "application/Logger.h"
using namespace application;

#include "statuschanger/StatusChanger.h"
#include "PmApp.h"
#include "login/Account.h"
#include "settings/GlobalSettings.h"
#include "db/UserDB.h"
#include "loginsettingdlg.h"
#include "PmApp.h"
#include "logger/logger.h"
#include "logindlg.h"
#include "ui_logindlg.h"
#include "pmessagebox.h"
#include "combouseritemdelegate.h"
#include "combouseritemwidget.h"
#include "userdeletedialog.h"
#include "util/FileUtil.h"
#include "util/AvatarUtil.h"
#include "ModelManager.h"
#include "debugdlg.h"
#include "companyloginmanager.h"
#include "proxysettingdlg.h"
#include <QNetworkProxy>
#include "countrycodedlg.h"
#include "qt-json/json.h"
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include "phonepasssetdlg.h"
#include "phoneactivedlg.h"
#include "registerdialog.h"

static const int kPagePhone         = 0;
static const int kPagePassword      = 1;
static const int kPageCompany       = 2;
static const int kPageError         = 3;

CLoginDlg::CLoginDlg(QWidget *parent) 
: FramelessDialog(parent)
, ui(new Ui::CLoginDlg())
, m_operation(OpLogin)
, m_loginState(None)
, m_countryCode("86")
{
    ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	setMainLayout(ui->verticalLayoutMain);

	initUI();

	getLoginInfo();

	setFixedSize(410, 343);
	setResizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), qPmApp, SLOT(quit()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(hide()));

	connect(qPmApp->GetLoginMgr(), SIGNAL(loginStatus(QString)), this, SLOT(slot_loginStatus(QString)));
	connect(qPmApp->GetLoginMgr(), SIGNAL(loginError(QString)), this, SLOT(slot_loginError(QString)));
	connect(qPmApp->GetLoginMgr(), SIGNAL(beLogined()), this, SLOT(slot_beLogined()));
	connect(qPmApp->GetLoginMgr(), SIGNAL(validateFailed()), this, SLOT(slot_validateFailed()));
	connect(qPmApp->GetLoginMgr(), SIGNAL(companyLogined()), this, SLOT(slot_gotCompany()));

	setSkin();
}

CLoginDlg::~CLoginDlg()
{
    delete ui;

	if (m_pToolTip)
	{
		delete m_pToolTip;
		m_pToolTip = 0;
	}
}

void CLoginDlg::loginWithId(const QString &uid)
{
	if (uid.isEmpty())
		return;

	CompanyLoginManager *companyLoginManager = qPmApp->getCompanyLoginManager();
	if (companyLoginManager->countryCode().isEmpty() ||
		companyLoginManager->phone().isEmpty() || 
		companyLoginManager->passwd().isEmpty() || 
		companyLoginManager->apiKey().isEmpty())
		return;

	m_switchId = uid;
	QString countryCode = companyLoginManager->countryCode();
	QString loginPhone = companyLoginManager->phone();
	QString phone = phoneFromLoginPhoneAndCountryCode(loginPhone, countryCode);
	QString passwd = companyLoginManager->passwd();
	m_countryCode = countryCode;
	QString country = countryShowName();
	ui->labelCountry->setText(country);
	ui->labelPhone->setText(country+QString(" ")+phone);
	ui->leditPhone->setText(phone);
	ui->leditPwd->setText(passwd);

	// make account
	Account *pAccount = qPmApp->getAccount();
	Q_ASSERT(pAccount != NULL);

	pAccount->setCountryCode(countryCode);
	pAccount->setLoginPhone(loginPhone);
	pAccount->setId("");

	qDebug() << Q_FUNC_INFO << loginPhone << passwd << uid;

	// change state & login
	m_operation = OpLogin;
	setLoginState(GotPasswordStatus);
	on_btnPwdNext_clicked();
}

QString CLoginDlg::loginPhoneFromCountryCodeAndPhone(const QString &countryCode, const QString &phone)
{
	if (countryCode == QString("86"))
	{
		return phone;
	}
	else
	{
		return QString("00") + countryCode + phone;
	}
}

QString CLoginDlg::phoneFromLoginPhoneAndCountryCode(const QString &loginPhone, const QString &countryCode)
{
	QString prefix = QString("00") + countryCode;
	if (loginPhone.startsWith(prefix))
	{
		return loginPhone.mid(prefix.length());
	}
	else
	{
		return loginPhone;
	}
}

void CLoginDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_2.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 130;
	setBG(bgPixmap, bgSizes);

	ui->labelErrorDesc->setStyleSheet("color: rgb(128, 128, 128);");

	ui->countryLine->setStyleSheet("QWidget#countryLine {background: rgb(255, 255, 255);}");
	ui->phoneLine->setStyleSheet("QWidget#phoneLine {background: rgb(255, 255, 255);}");
	ui->labelCountry->setStyleSheet("color: rgb(94, 103, 112); font-size: 11pt;");
	ui->labelPhone->setStyleSheet("color: rgb(94, 103, 112); font-size: 11pt;");

	// set qss file
	QStringList fileNames;
	fileNames << ":/qss/logindlg.qss";
	QString qssString;
	QFile qssFile;
	foreach (QString fileName, fileNames)
	{
		qssFile.setFileName(fileName);
		if (qssFile.open(QIODevice::ReadOnly))
		{
			QString qss = qssFile.readAll();
			qssString += qss;
			qssFile.close();
		}
	}
	setStyleSheet(qssString);

	// set style
	StyleToolButton::Info btnStyle;
	btnStyle.urlNormal = QString(":/images/right_arrow_normal.png");
	btnStyle.urlHover = QString(":/images/right_arrow_hover.png");
	btnStyle.urlPressed = QString(":/images/right_arrow_hover.png");
	btnStyle.tooltip = tr("More");
	ui->tBtnCountry->setInfo(btnStyle);
}

void CLoginDlg::slot_loginStatus(const QString &rsStatus)
{
	Q_UNUSED(rsStatus);
}

void CLoginDlg::slot_loginError(const QString &rsError)
{
	Q_UNUSED(rsError);
	m_switchId = "";
	ui->labelErrorDesc->setText(rsError);
	setLoginState(LoginError);
}

void CLoginDlg::slot_idLockedError()
{
	// dislogin first
	qPmApp->GetLoginMgr()->dislogin();

	// tip error
	QString tipText = tr("Your account has already logined on this PC, please don't login again");
	ui->labelErrorDesc->setText(tipText);
	setLoginState(LoginError);
}

void CLoginDlg::slot_beLogined()
{
	if (QDialogButtonBox::Yes == PMessageBox::question(this, tr("Tip"), tr("Your account has already logined other place, do you still want to login"), QDialogButtonBox::Yes|QDialogButtonBox::No))
	{
		qPmApp->getAccount()->setVoilent(true);
		doLogin();
	}
	else
	{
		setLoginState(None);
	}
}

void CLoginDlg::slot_validateFailed()
{
	QString tipText = tr("Phone or password error, please retry again");
	ui->labelErrorDesc->setText(tipText);
	setLoginState(LoginError);
}

void CLoginDlg::slot_gotCompany()
{
	if (m_loginState != GetingCompany)
		return;

	// store logined phone
	Account *pAccount = Account::instance();
	pAccount->storeLoginPhone();

	// add all companies
	ui->comboBoxCompany->clear();
	QList<CompanyInfo> companyInfos = pAccount->companyInfos();
	foreach (CompanyInfo companyInfo, companyInfos)
	{
		QString companyName = companyInfo.companyName;
		if (companyInfo.userState == 0)
		{
			companyName.prepend(tr("[Unactive] "));
		}
		else if (companyInfo.frozen)
		{
			companyName.prepend(tr("[Frozen] "));
		}
		ui->comboBoxCompany->addItem(companyName, companyInfo.uid);
	}

	// find default company
	int defaultIndex = 0;
	if (companyInfos.count() > 1)
	{
		DB::UserDB &userDB = pAccount->getUserDBref();
		QString defaultId = m_switchId;
		if (defaultId.isEmpty())
			defaultId = userDB.defaultUserId(pAccount->loginPhone());
		if (!defaultId.isEmpty())
		{
			for (int k = 0; k < ui->comboBoxCompany->count(); ++k)
			{
				QString loginId = ui->comboBoxCompany->itemData(k).toString();
				if (loginId == defaultId)
				{
					defaultIndex = k;
					break;
				}
			}
		}
	}
	ui->comboBoxCompany->setCurrentIndex(defaultIndex);

	setLoginState(GotCompany);
	
	// only one company && active already, login directly
	if (companyInfos.count() == 1 && companyInfos[defaultIndex].userState == 1)
	{
		on_btnEnterCompany_clicked();
	}
	else if (!m_switchId.isEmpty())
	{
		m_switchId = "";
		on_btnEnterCompany_clicked();
	}
}

void CLoginDlg::on_tBtnCountry_clicked()
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
			ui->labelCountry->setText(countryShowName());
		}
	}
}

void CLoginDlg::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        return;
    case Qt::Key_Return:
	case Qt::Key_Enter:
		{
			if (m_loginState == None || m_loginState == GetingPasswordStatus)
			{
				on_btnPhoneNext_clicked();
			}
			else if (m_loginState == GotPasswordStatus || m_loginState == GetingCompany)
			{
				on_btnPwdNext_clicked();
			}
			else if (m_loginState == GotCompany || m_loginState == LoginingCompany)
			{
				on_btnEnterCompany_clicked();
			}
			else if (m_loginState == LoginError)
			{
				on_btnErrorBack_clicked();
			}
		}
        break;
    default:
        break;
    }

    return QWidget::keyPressEvent(e);
}

void CLoginDlg::focusInEvent(QFocusEvent *e)
{
	QDialog::focusInEvent(e);
}

void CLoginDlg::on_btnSetting_clicked()
{
	ProxySettingDlg dlg(this);
	dlg.setProxyType((ProxySettingDlg::ProxyType)(GlobalSettings::proxyType()));
	dlg.setProxyAddress(GlobalSettings::proxyAddress());
	dlg.setProxyPort(GlobalSettings::proxyPort());
	dlg.setProxyUser(GlobalSettings::proxyUser());
	dlg.setProxyPassword(GlobalSettings::proxyPassword());
	if (dlg.exec())
	{
		ProxySettingDlg::ProxyType proxyType = dlg.proxyType();
		QString address = dlg.proxyAddress();
		int port = dlg.proxyPort();
		QString user = dlg.proxyUser();
		QString password = dlg.proxyPassword();

		// save settings
		GlobalSettings::setProxyType((int)(proxyType));
		GlobalSettings::setProxyAddress(address);
		GlobalSettings::setProxyPort(port);
		GlobalSettings::setProxyUser(user);
		GlobalSettings::setProxyPassword(password);

		// set application proxy
		QNetworkProxy proxy;
		if (proxyType == ProxySettingDlg::NoProxy)
		{
			proxy.setType(QNetworkProxy::NoProxy);
		}
		if (proxyType == ProxySettingDlg::HttpProxy)
		{
			proxy.setType(QNetworkProxy::HttpProxy);
		}
		else if (proxyType == ProxySettingDlg::Socks5Proxy)
		{
			proxy.setType(QNetworkProxy::Socks5Proxy);
		}

		if (proxyType != ProxySettingDlg::NoProxy)
		{
			proxy.setHostName(address);
			proxy.setPort(port);
			if (!user.isEmpty())
				proxy.setUser(user);
			if (!password.isEmpty())
				proxy.setPassword(password);
		}
		QNetworkProxy::setApplicationProxy(proxy);
	}
}

void CLoginDlg::on_btnPhoneNext_clicked()
{
	if (m_loginState != None && m_loginState != GetingPasswordStatus)
		return;

	if (m_loginState == GetingPasswordStatus)
	{
		qPmApp->getCompanyLoginManager()->cancelPhonePassStatus();
		setLoginState(None);
		return;
	}

	// check phone
	QString sPhone = ui->leditPhone->text();
	if (sPhone.length() <= 0)
	{
		ui->leditPhone->setFocus();

		m_pToolTip->setText(tr("Phone can't be empty"));
		QPoint p = ui->leditPhone->mapToGlobal(QPoint(0, ui->leditPhone->height()/2));
		m_pToolTip->move(mapFromGlobal(p));
		m_pToolTip->show();
		return;
	}

	Account *pAccount = qPmApp->getAccount();
	Q_ASSERT(pAccount != NULL);

	pAccount->setCountryCode(m_countryCode);
	QString loginPhone = loginPhoneFromCountryCodeAndPhone(m_countryCode, sPhone);
	pAccount->setLoginPhone(loginPhone);
	pAccount->setId("");
	pAccount->setPwd("");
	pAccount->setCryptoPwd("");
	pAccount->setAutologin(false);

	qDebug() << Q_FUNC_INFO << loginPhone;

	m_operation = OpLogin;
	setLoginState(GetingPasswordStatus);

	checkUpdating();
}

void CLoginDlg::on_btnPwdNext_clicked()
{
	if (m_loginState != GotPasswordStatus && m_loginState != GetingCompany)
		return;

	if (m_loginState == GetingCompany)
	{
		qPmApp->GetLoginMgr()->dislogin();
		setLoginState(GotPasswordStatus);
		return;
	}

	// check password
	QString sPwd = ui->leditPwd->text();
	if (sPwd.length()<= 0)
	{
		ui->leditPwd->setFocus();
		ui->leditPwd->selectAll();

		m_pToolTip->setText(tr("Password can't be empty"));
		QPoint p = ui->leditPwd->mapToGlobal(QPoint(0, ui->leditPwd->height()/2));
		m_pToolTip->move(mapFromGlobal(p));
		m_pToolTip->show();
		return;
	}

	Account *pAccount = qPmApp->getAccount();
	Q_ASSERT(pAccount != NULL);
	pAccount->setPwd(sPwd);
	pAccount->setCryptoPwd("");

	qDebug() << Q_FUNC_INFO << pAccount->loginPhone();

	setLoginState(GetingCompany);

	qPmApp->GetLoginMgr()->login();
}

void CLoginDlg::on_btnEnterCompany_clicked()
{
	QString uid = ui->comboBoxCompany->currentData().toString();
	if (uid.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "no company selected";
		return;
	}

	if (m_loginState != GotCompany && m_loginState != LoginingCompany)
		return;

	if (m_loginState == LoginingCompany)
	{
		qPmApp->GetLoginMgr()->dislogin();
		setLoginState(GotPasswordStatus);
		return;
	}

	Account *pAccount = Account::instance();
	QList<CompanyInfo> companyInfos = pAccount->companyInfos();
	int selIndex = 0;
	foreach (CompanyInfo companyInfo, companyInfos)
	{
		if (companyInfo.uid == uid)
		{
			if (companyInfo.userState == 0)
			{
				QDialogButtonBox::StandardButton ret = 
					PMessageBox::question(this, tr("Tip"), tr("This phone haven't been active in this corporation, active right now"),
					QDialogButtonBox::Yes|QDialogButtonBox::No);
				if (ret == QDialogButtonBox::Yes)
				{
					// active phone
					PhoneActiveDlg phoneActiveDlg(pAccount->loginPhone(), countryShowName()+QString(" ")+ui->leditPhone->text(), uid, this);
					phoneActiveDlg.exec();

					if (phoneActiveDlg.isActived())
					{
						// update company info
						companyInfo.userState = 1;
						companyInfos[selIndex] = companyInfo;
						pAccount->setCompanyInfos(companyInfos);

						// update text
						ui->comboBoxCompany->clear();
						foreach (CompanyInfo companyInfoNew, companyInfos)
						{
							QString companyName = companyInfoNew.companyName;
							if (companyInfoNew.userState == 0)
							{
								companyName.prepend(tr("[Unactive] "));
							}
							else if (companyInfoNew.frozen)
							{
								companyName.prepend(tr("[Frozen] "));
							}
							ui->comboBoxCompany->addItem(companyName, companyInfoNew.uid);
						}
						ui->comboBoxCompany->setCurrentIndex(selIndex);
					}
				}
				return;
			}
			else if (companyInfo.frozen)
			{
				PMessageBox::information(this, tr("Tip"), tr("You are frozen in this corporation, please contact manager"));
				return;
			}
		}
		++selIndex;
	}

	pAccount->setId(uid);
	pAccount->storeUserIds();
	pAccount->setUserIdDefault();
	setLoginState(LoginingCompany);
	
	// check if id logined
	checkLoginUser(pAccount->id());
}

void CLoginDlg::on_btnErrorBack_clicked()
{
	setLoginState(None);
}

void CLoginDlg::on_leditPhone_textChanged(const QString &text)
{
	// first check if open pm console
	if (text == QString("##lt##"))
	{
		CDebugDlg::getDebugDlg()->slot_show();
		ui->leditPhone->clear();
		return;
	}

	if (m_pToolTip->isVisible())
		m_pToolTip->setVisible(false);

	ui->leditPwd->setText("");
}

void CLoginDlg::on_comboBoxCompany_currentIndexChanged(int index)
{
	QString uid = ui->comboBoxCompany->itemData(index).toString();
	if (uid.isEmpty())
		return;

	QPixmap avatar = getLoginAvatar(uid);
	ui->avatarWidget->setPixmap(avatar);
}

void CLoginDlg::on_leditPwd_textChanged(const QString &text)
{
	Q_UNUSED(text);
	if (m_pToolTip->isVisible())
		m_pToolTip->hide();
}

void CLoginDlg::on_checkRemeberPwd_clicked(bool bChecked)
{
	Q_UNUSED(bChecked);
}

void CLoginDlg::on_labelForgetPwd_clicked()
{
	GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/pmuserlogin/resetUserPassword").arg(curConfig.managerUrl);
	QDesktopServices::openUrl(QUrl::fromUserInput(urlStr));
}

void CLoginDlg::on_labelRegister_clicked()
{
	RegisterDialog dlg(this);
	dlg.exec();
}

void CLoginDlg::onSwitchAccount()
{
	dislogin();
	ui->leditPhone->setFocus();
	ui->leditPhone->selectAll();
}

void CLoginDlg::slot_phonePwdEditFinished()
{
	if (m_pToolTip->isVisible())
		m_pToolTip->hide();
}

void CLoginDlg::initUI()
{
	setWindowTitle(GlobalSettings::title());

	QString country = countryShowName();
	ui->labelCountry->setText(country);
	
	ui->leditPwd->setText("");
	ui->leditPwd->setButtonVisible(FilterLineEdit::Left, false);
	ui->leditPwd->setPlaceholderText(tr("Please input password"));
	ui->leditPwd->setButtonPixmap(FilterLineEdit::Right, QPixmap(":/images/login_passedit_normal.png"));
	connect(ui->leditPwd, SIGNAL(editingFinished()), this, SLOT(slot_phonePwdEditFinished()));

	ui->leditPhone->setText("");
	ui->leditPhone->setButtonVisible(FilterLineEdit::Left, false);
	ui->leditPhone->setPlaceholderText(tr("Please input phone"));
	ui->leditPhone->setButtonPixmap(FilterLineEdit::Right, QPixmap(":/images/login_passedit_normal.png"));
	connect(ui->leditPhone, SIGNAL(editingFinished()), this, SLOT(slot_phonePwdEditFinished()));

	// tool tip
	m_pToolTip = new CToolTip(this);
	m_pToolTip->hide();

	// avatar widget
	QPixmap avatarBorderPixmap(":/images/Icon_101.png");
	WidgetBorder avatarBorder;
	avatarBorder.left = 4;
	avatarBorder.top = 4;
	avatarBorder.right = 4;
	avatarBorder.bottom = 4;
	avatarBorder.width = 0;
	avatarBorder.height = 0;
	avatarBorder.radius = 0;
	ui->avatarWidget->setBorder(avatarBorderPixmap, avatarBorder);

	// set languages
	ui->labelLanguages->setFontAtt(QColor(0, 120, 216), 9, false);
	QFile languageFile(":/languages/languages.csv");
	languageFile.open(QFile::ReadOnly);
	QByteArray csvData = languageFile.readAll();
	m_languages = QString::fromUtf8(csvData).split(",");
	languageFile.close();
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		ui->labelLanguages->setText(m_languages[0]);
	else
		ui->labelLanguages->setText(m_languages[1]);
	connect(ui->labelLanguages, SIGNAL(clicked()), this, SLOT(onLanguagesClicked()));

	// set version
	ui->labelVersion->setText(APP_VERSION);

	// loading animation
	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);

	ui->labelRegister->setFontAtt(QColor(0, 120, 216), 9, false);
	ui->labelForgetPwd->setFontAtt(QColor(0, 120, 216), 9, false);

	ui->labelSwitchAccount1->setFontAtt(QColor(0, 120, 216), 9, false);
	ui->labelSwitchAccount2->setFontAtt(QColor(0, 120, 216), 9, false);
	connect(ui->labelSwitchAccount1, SIGNAL(clicked()), this, SLOT(onSwitchAccount()));
	connect(ui->labelSwitchAccount2, SIGNAL(clicked()), this, SLOT(onSwitchAccount()));

	ui->centerStackedWidget->setCurrentIndex(kPagePhone);
}

void CLoginDlg::getLoginInfo()
{
	Account *pAccout = qPmApp->getAccount();
	DB::UserDB &db = pAccout->getUserDBref();
	QList<DB::UserDB::PhoneItem> infos = db.getAccountsByUpdateTimeDesc();
	if (infos.isEmpty())
		return;

	foreach (DB::UserDB::PhoneItem info, infos)
	{
		if (info.phone.isEmpty())
			continue;

		m_countryCode = info.countryCode;
		QString country = countryShowName();
		ui->labelCountry->setText(country);
		
		QString phone = phoneFromLoginPhoneAndCountryCode(info.phone, info.countryCode);
		ui->leditPhone->setText(phone);

		if (info.storePasswd)
		{
			ui->leditPwd->setText(info.passwd);
		}
		else
		{
			ui->leditPwd->setText("");
		}
		break;
	}
}

QPixmap CLoginDlg::getLoginAvatar(const QString &loginUser)
{
	QPixmap defaultAvatar = QPixmap::fromImage(ModelManager::avatarDefaultIcon());
	if (loginUser.isEmpty())
	{
		return defaultAvatar;
	}

	QString loginPhone = loginPhoneFromCountryCodeAndPhone(m_countryCode, ui->leditPhone->text());
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString storeHome = loginConfig.storeHome;
	QString avatarPath = storeHome + "/" + loginPhone + "/" + loginUser + "/" + DIR_AVATAR_NAME + "/";
	QDir avatarDir(avatarPath);
	if (!avatarDir.exists(avatarPath))
	{
		return defaultAvatar;
	}

	QString filePath = AvatarUtil::avatarName(loginUser);
	filePath = avatarDir.absoluteFilePath(filePath);
	QImage img;
	if (img.load(filePath))
	{
		QPixmap pixmap = QPixmap::fromImage(img);
		return pixmap;
	}
	else
	{
		return defaultAvatar;
	}
}

void CLoginDlg::checkLoginUser(const QString &id)
{
	connect(qPmApp, SIGNAL(userLoginCheckFinished(bool)), this, SLOT(userLoginCheckFinished(bool)), Qt::UniqueConnection);
	qPmApp->checkIfUserLogined(id);
}

void CLoginDlg::userLoginCheckFinished(bool logined)
{
	disconnect(qPmApp, SIGNAL(userLoginCheckFinished(bool)), this, SLOT(userLoginCheckFinished(bool)));

	if (m_operation == OpLogin)
	{
		if (logined)
		{
			slot_idLockedError();
		}
		else
		{
			doLogin();
		}
	}
	else if (m_operation == OpDeleteUser)
	{
		if (logined)
		{
			PMessageBox::information(this, tr("Tip"), tr("Your account is in use, please retry after quit"));
		}
		else
		{
			if (m_deleteCheckUids.isEmpty())
			{
				deleteUserAccount(m_deleteUser);
				deleteUserPath(m_deleteUser);
				m_deleteUser = "";
			}
			else
			{
				// check first
				QString checkUid = m_deleteCheckUids.takeFirst();
				checkLoginUser(checkUid);
			}
		}
	}
}

void CLoginDlg::checkUpdating()
{
	connect(qPmApp, SIGNAL(updateCheckFinished(bool)), this, SLOT(onUpdatingChecked(bool)), Qt::UniqueConnection);
	qPmApp->checkIfUpdating();
}

void CLoginDlg::onUpdatingChecked(bool update)
{
	disconnect(qPmApp, SIGNAL(updateCheckFinished(bool)), this, SLOT(onUpdatingChecked(bool)));
	
	if (update)
	{
		setLoginState(None);
		hide();

		// someone is updating
		PMessageBox::information(0, 
			tr("Upgrade"), 
			tr("Finding upgrade on this PC.\nPress ok to quit this program."),
			QDialogButtonBox::Ok);
		qPmApp->quit();
	}
	else
	{
		connect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassStatusOK(int)), 
			this, SLOT(onPhonePassStatusOK(int)), Qt::UniqueConnection);
		connect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassStatusFailed(int, QString)), 
			this, SLOT(onPhonePassStatusFailed(int, QString)), Qt::UniqueConnection);
		qPmApp->getCompanyLoginManager()->phonePassStatus(Account::instance()->loginPhone());
	}
}

void CLoginDlg::deleteUser(const QString &user)
{
	UserDeleteDialog dlg(user, this);
	if (dlg.exec()) 
	{
		UserDeleteDialog::DeleteOption option = dlg.deleteOption();
	
		if (option == UserDeleteDialog::DeleteUser)
		{
			deleteUserAccount(user);
			return;
		}

		// check if can delete user files
		m_operation = OpDeleteUser;
		m_deleteCheckUids.clear();
		Account *pAccout = qPmApp->getAccount();
		DB::UserDB &db = pAccout->getUserDBref();
		QList<DB::UserDB::UserIdItem> idItems = db.userIdsForPhone(user);
		foreach (DB::UserDB::UserIdItem idItem, idItems)
		{
			m_deleteCheckUids.append(idItem.uid);
		}
		m_deleteUser = user;

		if (m_deleteCheckUids.isEmpty())
		{
			// delete directly
			deleteUserAccount(m_deleteUser);
			deleteUserPath(m_deleteUser);
			m_deleteUser = "";
		}
		else
		{
			// check first
			QString checkUid = m_deleteCheckUids.takeFirst();
			checkLoginUser(checkUid);
		}
	}
}

void CLoginDlg::deleteUserPath(const QString &user)
{
	if (user.isEmpty())
		return;

	// delete all the user related files
	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString storeHome = loginConfig.storeHome;
	QString userPath = storeHome + "/" + user + "/";
	FileUtil::removePath(userPath);

	QDir topDir;
	topDir.rmdir(userPath);
}

void CLoginDlg::deleteUserAccount(const QString &user)
{
	if (user.isEmpty())
		return;

	// delete the user from db and reload
	Account *pAccout = qPmApp->getAccount();
	DB::UserDB &db = pAccout->getUserDBref();
	db.deleteAccount(user);
	getLoginInfo();
}

void CLoginDlg::dislogin()
{
	qPmApp->GetLoginMgr()->dislogin();
	setLoginState(None);
}

void CLoginDlg::onLanguagesClicked()
{
	QString curStr = ui->labelLanguages->text();
	if (curStr == m_languages[1])
	{
		curStr = m_languages[0];
		qPmApp->setLanguageChinese();
	}
	else
	{
		curStr = m_languages[1];
   		qPmApp->setLanguageEnglish();
	}

	retranslateUi();

	ui->labelLanguages->setText(curStr);
}

void CLoginDlg::onPhonePassStatusOK(int status)
{
	disconnect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassStatusOK(int)), 
		this, SLOT(onPhonePassStatusOK(int)));
	disconnect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassStatusFailed(int, QString)), 
		this, SLOT(onPhonePassStatusFailed(int, QString)));

	CompanyLoginManager::PhoneStatus phoneStatus = (CompanyLoginManager::PhoneStatus)status;
	switch (phoneStatus)
	{
	case CompanyLoginManager::NoRegister:
		{
			setLoginState(None);
			PMessageBox::information(this, tr("Tip"), tr("This phone haven't register yet"));
		}
		break;
	case CompanyLoginManager::NoAuditNoPass:
		{
			setLoginState(None);
			PMessageBox::information(this, tr("Tip"), tr("This phone haven't been approved yet"));
		}
		break;
	case CompanyLoginManager::NoPass:
	case CompanyLoginManager::AuditNoPass:
		{
			setLoginState(None);
			QDialogButtonBox::StandardButton ret = PMessageBox::question(
				this, tr("Tip"), tr("This phone haven't set password before, set right now"), 
				QDialogButtonBox::Yes|QDialogButtonBox::No);
			if (ret == QDialogButtonBox::Yes)
			{
				PhonePassSetDlg passSetDlg(Account::instance()->loginPhone(), countryShowName()+QString(" ")+ui->leditPhone->text(), this);
				passSetDlg.exec();
			}
		}
		break;
	case CompanyLoginManager::HavePass:
		{
			ui->labelPhone->setText(countryShowName()+QString(" ")+ui->leditPhone->text());
			setLoginState(GotPasswordStatus);
		}
		break;
	default:
		break;
	}
}

void CLoginDlg::onPhonePassStatusFailed(int retCode, const QString &desc)
{
	disconnect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassStatusOK(int)), 
		this, SLOT(onPhonePassStatusOK(int)));
	disconnect(qPmApp->getCompanyLoginManager(), SIGNAL(phonePassStatusFailed(int, QString)), 
		this, SLOT(onPhonePassStatusFailed(int, QString)));

	ui->labelErrorDesc->setText(QString("%1(%2)").arg(desc).arg(retCode));
	setLoginState(LoginError);
}

void CLoginDlg::doLogin()
{
	StatusChanger *pStatusChanger = qPmApp->getStatusChanger();
	if (pStatusChanger)
	{
		pStatusChanger->reset();
		pStatusChanger->setStatus(StatusChanger::Status_Online);
	}
}

void CLoginDlg::setLoginState(LoginState state)
{
	m_loginState = state;
	if (m_loginState == GetingPasswordStatus)
	{
		ui->labelLoading->setVisible(true);
		ui->labelLoading->movie()->start();
		ui->leditPhone->setEnabled(false);
		ui->labelRegister->setEnabled(false);
		ui->tBtnCountry->setEnabled(false);
		ui->btnPhoneNext->setText(tr("Cancel"));
		ui->labelLanguages->setVisible(false);
	}
	else if (m_loginState == GotPasswordStatus)
	{
		ui->labelLoading->movie()->stop();
		ui->labelLoading->setVisible(false);
		ui->leditPwd->setEnabled(true);
		ui->labelSwitchAccount1->setEnabled(true);
		ui->labelForgetPwd->setEnabled(true);
		ui->btnPwdNext->setText(tr("Next"));
		ui->centerStackedWidget->setCurrentIndex(kPagePassword);
	}
	else if (m_loginState == GetingCompany)
	{
		qPmApp->getCompanyLoginManager()->reset();
		ui->labelLoading->setVisible(true);
		ui->labelLoading->movie()->start();
		ui->labelSwitchAccount1->setEnabled(false);
		ui->leditPwd->setEnabled(false);
		ui->labelForgetPwd->setEnabled(false);
		ui->btnPwdNext->setText(tr("Cancel"));
		ui->labelLanguages->setVisible(false);
	}
	else if (m_loginState == GotCompany)
	{
		ui->labelLoading->movie()->stop();
		ui->labelLoading->setVisible(false);
		ui->labelLanguages->setVisible(false);
		ui->labelSwitchAccount2->setEnabled(true);
		ui->centerStackedWidget->setCurrentIndex(kPageCompany);
	}
	else if (m_loginState == LoginingCompany)
	{
		ui->labelLoading->setVisible(true);
		ui->labelLoading->movie()->start();
		ui->comboBoxCompany->setEnabled(false);
		ui->labelSwitchAccount2->setEnabled(false);
		ui->btnEnterCompany->setText(tr("Cancel"));
	}
	else if (m_loginState == None)
	{
		disconnect(qPmApp, SIGNAL(updateCheckFinished(bool)), this, SLOT(onUpdatingChecked(bool)));
		disconnect(qPmApp, SIGNAL(userLoginCheckFinished(bool)), this, SLOT(userLoginCheckFinished(bool)));

		ui->labelLoading->movie()->stop();
		ui->labelLoading->setVisible(false);
		ui->tBtnCountry->setEnabled(true);
		ui->leditPhone->setEnabled(true);
		ui->leditPhone->setFocus();
		ui->leditPwd->setEnabled(true);
		ui->labelRegister->setEnabled(true);
		ui->labelForgetPwd->setEnabled(true);
		ui->btnPhoneNext->setText(tr("Next"));
		ui->btnPwdNext->setText(tr("Next"));
		ui->comboBoxCompany->setEnabled(true);
		ui->btnEnterCompany->setText(tr("Enter Corporation"));
		ui->labelSwitchAccount1->setEnabled(true);
		ui->labelSwitchAccount2->setEnabled(true);
		ui->labelLanguages->setVisible(true);

		ui->centerStackedWidget->setCurrentIndex(kPagePhone);
	}
	else if (m_loginState == LoginError)
	{
		disconnect(qPmApp, SIGNAL(updateCheckFinished(bool)), this, SLOT(onUpdatingChecked(bool)));
		disconnect(qPmApp, SIGNAL(userLoginCheckFinished(bool)), this, SLOT(userLoginCheckFinished(bool)));

		ui->labelLoading->movie()->stop();
		ui->labelLoading->setVisible(false);
		ui->centerStackedWidget->setCurrentIndex(kPageError);
	}
}

void CLoginDlg::retranslateUi()
{
	ui->retranslateUi(this);

	setWindowTitle(GlobalSettings::title());
	ui->labelCountry->setText(countryShowName());
	ui->leditPhone->setPlaceholderText(tr("Please input phone"));
	ui->leditPwd->setPlaceholderText(tr("Please input password"));
	ui->labelVersion->setText(APP_VERSION);
}

QString CLoginDlg::countryCode2CountryName(const QString &code) const
{
	QString countryName;
	if (code.isEmpty())
		return countryName;

	QFile codeFile;
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		codeFile.setFileName(":/countrycode/chineseCountryJson.txt");
	else
		codeFile.setFileName(":/countrycode/englishCountryJson.txt");
	codeFile.open(QIODevice::ReadOnly);
	QByteArray content = codeFile.readAll();
	codeFile.close();

	bool ok = false;
	QVariant v = QtJson::parse(QString::fromUtf8(content), ok);
	if (!ok)
		return false;

	// read all code items
	QMap<QString, QStandardItem *> codeGroups;
	QVariantList listV = v.toMap().value("data").toList();
	foreach (QVariant itemV, listV)
	{
		QVariantMap codeItem = itemV.toMap();
		QString countryCode = codeItem["phoneCode"].toString();
		if (countryCode == code)
		{
			countryName = codeItem["countryName"].toString();
			break;
		}
	}
	return countryName;
}

QString CLoginDlg::countryShowName() const
{
	QString countryName = countryCode2CountryName(m_countryCode);
	QString showName = QString("%1 (+%2)").arg(countryName).arg(m_countryCode);
	return showName;
}