#ifndef PROXYSETTINGDLG_H
#define PROXYSETTINGDLG_H

#include "framelessdialog.h"
#include "settings/GlobalSettings.h"
#include <QNetworkProxy>
#include <QNetworkAccessManager>

namespace Ui {class ProxySettingDlg;};

class QNetworkReply;

class ProxySettingDlg : public FramelessDialog
{
	Q_OBJECT

public:
	enum ProxyType
	{
		NoProxy,
		HttpProxy,
		Socks5Proxy
	};

public:
	ProxySettingDlg(QWidget *parent = 0);
	~ProxySettingDlg();

	ProxyType proxyType() const;
	void setProxyType(ProxyType type);

	QString proxyAddress() const;
	void setProxyAddress(const QString &address);

	int proxyPort() const;
	void setProxyPort(int port);

	QString proxyUser() const;
	void setProxyUser(const QString &user);

	QString proxyPassword() const;
	void setProxyPassword(const QString &password);

public Q_SLOTS:
	virtual void setSkin();

private Q_SLOTS:
	void testProxy();
	void onProxyTypeChanged(int index);
	void openLoginSettings();
	void onAddressChanged(const QString &address);
	void onTestFinished();

private:
	void initUI();

private:
	Ui::ProxySettingDlg *ui;

	QNetworkAccessManager m_nam;
	QNetworkProxy         m_currentProxy;
	QNetworkReply        *m_testReply;
};

#endif // PROXYSETTINGDLG_H
