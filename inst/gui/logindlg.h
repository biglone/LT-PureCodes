#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QPointer>
#include <QScopedPointer>
#include <QTimer>
#include "framelessdialog.h"

class CToolTip;
class QDataWidgetMapper;
class QMenu;
class QStandardItemModel;
class LoginSettingDlg;

namespace Ui {
    class CLoginDlg;
}

class CLoginDlg : public FramelessDialog
{
    Q_OBJECT
		
	enum Operation
	{
		OpLogin,
		OpDeleteUser
	};

	enum LoginState
	{
		None,
		GetingPasswordStatus,
		GotPasswordStatus,
		GetingCompany,
		GotCompany,
		LoginingCompany,
		LoginError
	};

public:
	explicit CLoginDlg(QWidget *parent = 0);
    ~CLoginDlg();

	void loginWithId(const QString &uid);

	static QString loginPhoneFromCountryCodeAndPhone(const QString &countryCode, const QString &phone);
	static QString phoneFromLoginPhoneAndCountryCode(const QString &loginPhone, const QString &countryCode);

public slots:
	virtual void setSkin();

protected:
	void keyPressEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent *);

private slots:
	void slot_loginStatus(const QString &rsStatus);
	void slot_loginError(const QString &rsError);
	void slot_idLockedError();
	void slot_beLogined();
	void slot_validateFailed();
	void slot_gotCompany();
	
	void on_tBtnCountry_clicked();

	void on_btnSetting_clicked();

	void on_btnPhoneNext_clicked();

	void on_btnPwdNext_clicked();

	void on_btnEnterCompany_clicked();

	void on_btnErrorBack_clicked();

	void on_leditPhone_textChanged(const QString &text);

	void on_comboBoxCompany_currentIndexChanged(int index);

	void on_leditPwd_textChanged(const QString &text);
	
	void on_checkRemeberPwd_clicked(bool bChecked);

	void on_labelForgetPwd_clicked();

	void on_labelRegister_clicked();

	void onSwitchAccount();

	void slot_phonePwdEditFinished();
	
	void checkLoginUser(const QString &id);
	void userLoginCheckFinished(bool logined);

	void checkUpdating();
	void onUpdatingChecked(bool update);

	void deleteUser(const QString &user);
	void deleteUserPath(const QString &user);
	void deleteUserAccount(const QString &user);

	void dislogin();

	void onLanguagesClicked();

	void onPhonePassStatusOK(int status);
	void onPhonePassStatusFailed(int retCode, const QString &desc);
	
private:
	void initUI();
	void getLoginInfo();
	void configLoginAvatar();
	QPixmap getLoginAvatar(const QString &loginUser);
	void doLogin();
	void setLoginState(LoginState state);
	void retranslateUi();
	QString countryCode2CountryName(const QString &code) const;
	QString countryShowName() const;

private:
    Ui::CLoginDlg                      *ui;
	Operation                           m_operation;
	LoginState                          m_loginState;
	CToolTip                           *m_pToolTip;
	QString                             m_deleteUser;
	QStringList                         m_deleteCheckUids;
	QStringList                         m_languages;
	QString                             m_countryCode;
	QString                             m_switchId;
};

#endif // LOGINDLG_H
