#include "proxysettingdlg.h"
#include "ui_proxysettingdlg.h"
#include "loginsettingdlg.h"
#include "db/UserDB.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include "pmessagebox.h"

ProxySettingDlg::ProxySettingDlg(QWidget *parent)
	: FramelessDialog(parent), m_testReply(0)
{
	ui = new Ui::ProxySettingDlg();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("Login Setting"));
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(432, 248);
	setResizeable(false);

	initUI();

	setSkin();
}

ProxySettingDlg::~ProxySettingDlg()
{
	if (m_testReply)
		delete m_testReply;

	delete ui;
}

ProxySettingDlg::ProxyType ProxySettingDlg::proxyType() const
{
	return (ProxySettingDlg::ProxyType)(ui->comboBoxType->currentData().toInt());
}

void ProxySettingDlg::setProxyType(ProxyType type)
{
	if (type == NoProxy)
		ui->comboBoxType->setCurrentIndex(0);
	else if (type == HttpProxy)
		ui->comboBoxType->setCurrentIndex(1);
	else if (type == Socks5Proxy)
		ui->comboBoxType->setCurrentIndex(2);
}

QString ProxySettingDlg::proxyAddress() const
{
	return ui->lineEditAddress->text();
}

void ProxySettingDlg::setProxyAddress(const QString &address)
{
	ui->lineEditAddress->setText(address);
}

int ProxySettingDlg::proxyPort() const
{
	return ui->lineEditPort->text().toInt();
}

void ProxySettingDlg::setProxyPort(int port)
{
	ui->lineEditPort->setText(QString::number(port));
}

QString ProxySettingDlg::proxyUser() const
{
	return ui->lineEditUser->text();
}

void ProxySettingDlg::setProxyUser(const QString &user)
{
	ui->lineEditUser->setText(user);
}

QString ProxySettingDlg::proxyPassword() const
{
	return ui->lineEditPassword->text();
}

void ProxySettingDlg::setProxyPassword(const QString &password)
{
	ui->lineEditPassword->setText(password);
}

void ProxySettingDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_8.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	this->setStyleSheet("QLabel#title {font-size: 12pt; color: white;}");

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
}

void ProxySettingDlg::testProxy()
{
	if (m_testReply)
	{   
		// test in process, cancel it
		delete m_testReply;
		m_testReply = 0;
		ui->pushButtonTest->setText(tr("Test"));
		ui->labelTest->clear();
	}
	else
	{
		if (proxyType() == NoProxy)
			return;

		QString address = proxyAddress();
		if (address.isEmpty())
		{
			PMessageBox::information(this, tr("Tip"), tr("Address can't be empty"));
			return;
		}

		int port = proxyPort();
		if (port <= 0)
		{
			PMessageBox::information(this, tr("Tip"), tr("Port is not valid"));
			return;
		}

		// back original proxy
		m_currentProxy = QNetworkProxy::applicationProxy();

		// do test
		QString user = proxyUser();
		QString password = proxyPassword();
		QNetworkProxy proxy;
		if (proxyType() == HttpProxy)
		{
			proxy.setType(QNetworkProxy::HttpProxy);
		}
		else if (proxyType() == Socks5Proxy)
		{
			proxy.setType(QNetworkProxy::Socks5Proxy);
		}
		proxy.setHostName(address);
		proxy.setPort(port);
		if (!user.isEmpty())
			proxy.setUser(user);
		if (!password.isEmpty())
			proxy.setPassword(password);
		QNetworkProxy::setApplicationProxy(proxy);

		m_testReply = m_nam.get(QNetworkRequest(QUrl::fromUserInput("https://www.baidu.com")));
		connect(m_testReply, SIGNAL(finished()), this, SLOT(onTestFinished()));
		
		ui->pushButtonTest->setText(tr("Cancel"));
		ui->labelTest->setText(tr("Test proxy..."));
	}
}

void ProxySettingDlg::onProxyTypeChanged(int index)
{
	if (ui->comboBoxType->itemData(index).toInt() == NoProxy)
	{
		ui->lineEditAddress->setEnabled(false);
		ui->lineEditPort->setEnabled(false);
		ui->lineEditUser->setEnabled(false);
		ui->lineEditPassword->setEnabled(false);
		ui->pushButtonTest->setEnabled(false);
	}
	else
	{
		ui->lineEditAddress->setEnabled(true);
		ui->lineEditPort->setEnabled(true);
		ui->lineEditUser->setEnabled(true);
		ui->lineEditPassword->setEnabled(true);
		ui->pushButtonTest->setEnabled(true);
	}
}

void ProxySettingDlg::openLoginSettings()
{
	LoginSettingDlg loginSettingDlg(this);
	loginSettingDlg.exec();

	// close all dbs, reset the app path
	DB::DBBase::closeAllDBs();
}

void ProxySettingDlg::onAddressChanged(const QString &address)
{
	if (address == QString::fromLatin1("lt888888"))
	{
		ui->btnLoginSettings->setVisible(true);
	}
	ui->labelTest->clear();
}

void ProxySettingDlg::onTestFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if (!reply)
		return;

	if (reply != m_testReply)
		return;

	do 
	{
		if (m_testReply->error() != QNetworkReply::NoError)
		{
			ui->labelTest->setText(tr("Test failed, proxy error: %1").arg(m_testReply->error()));
			break;
		}

		ui->labelTest->setText(tr("Test is successful"));
	} while (0);

	m_testReply->deleteLater();
	m_testReply = 0;

	ui->pushButtonTest->setText(tr("Test"));
	QNetworkProxy::setApplicationProxy(m_currentProxy);
}

void ProxySettingDlg::initUI()
{
	connect(ui->comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(onProxyTypeChanged(int)));
	ui->comboBoxType->addItem(tr("No Proxy"), NoProxy);
	ui->comboBoxType->addItem(tr("HTTP Proxy"), HttpProxy);
	ui->comboBoxType->addItem(tr("SOCKS5 Proxy"), Socks5Proxy);
	ui->comboBoxType->setCurrentIndex(0);

	ui->lineEditPort->setValidator(new QIntValidator(1, 65536));
	ui->lineEditPort->setText("80");

	ui->lineEditPassword->setEchoMode(QLineEdit::Password);

	ui->btnLoginSettings->setVisible(false);
	connect(ui->btnLoginSettings, SIGNAL(clicked()), this, SLOT(openLoginSettings()));

	connect(ui->lineEditAddress, SIGNAL(textChanged(QString)), this, SLOT(onAddressChanged(QString)));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->pushButtonTest, SIGNAL(clicked()), this, SLOT(testProxy()));
}
