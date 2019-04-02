#include <QDebug>
#include <QStandardItemModel>
#include <QRadioButton>
#include <QStandardPaths>

#include "loginsettingdlg.h"
#include "ui_loginsettingdlg.h"
#include "plaintextinput.h"
#include "loginsettingeditdlg.h"
#include "pmessagebox.h"

LoginSettingDlg::LoginSettingDlg(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::LoginSettingDlg();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, false);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("Login Settings"));
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(474, 366);
	setResizeable(false);

	initUI();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));

	setSkin();
}

LoginSettingDlg::~LoginSettingDlg()
{
	delete ui;
}

void LoginSettingDlg::on_btnCancel_clicked()
{
	close();
}

void LoginSettingDlg::onListItemRadioToggled(bool checked)
{
	if (checked)
	{
		for (int i = 0; i < m_loginConfigModel->rowCount(); i++)
		{
			QStandardItem *item = m_loginConfigModel->item(i);
			QRadioButton *radioButton = qobject_cast<QRadioButton *>(ui->listView->indexWidget(item->index()));
			if (radioButton == sender())
			{
				ui->listView->selectionModel()->select(item->index(), QItemSelectionModel::ClearAndSelect);
				
				// update UI
				GlobalSettings::LoginConfig config = getLoginConfig(*item);
				if (config.removable)
					ui->pushButtonDelete->setEnabled(true);
				else
					ui->pushButtonDelete->setEnabled(false);

				QString managerAddress = config.managerUrl;
				ui->leManagerAddress->setText(managerAddress);

				ui->leCacheAddress->setText(config.storeHome);

				// update current login setting
				GlobalSettings::setCurrentLoginKey(config.name);
			}
		}
	}
}

void LoginSettingDlg::onListViewItemClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		QRadioButton *radioButton = qobject_cast<QRadioButton *>(ui->listView->indexWidget(index));
		if (radioButton)
			radioButton->setChecked(true);
	}
}

void LoginSettingDlg::addLoginConfig()
{
	// get login setting name
	PlainTextInput input(this);
	input.init(tr("Add Login Setting"), tr("Login setting name(less than 10 characters, please don't use special character)"), 10);
	if (QDialog::Rejected == input.exec())
		return;

	// check if has the same name
	QString newName = input.getInputText();
	if (newName.isEmpty())
		return;

	for (int i = 0; i < m_loginConfigModel->rowCount(); i++)
	{
		QStandardItem *item = m_loginConfigModel->item(i);
		GlobalSettings::LoginConfig config = getLoginConfig(*item);
		if (config.name == newName)
		{
			PMessageBox::warning(this, tr("Input Error"), tr("This login setting name has bean already used"));
			return;
		}
	}

	LoginSettingEditDlg editDlg(newName, this);
	if (editDlg.exec() == LoginSettingEditDlg::Accepted)
	{
		QString managerAddress = editDlg.managerAddress();
		if (!isValidAddress(managerAddress))
		{
			PMessageBox::warning(this, tr("Input Error"), tr("Server address input error"));
			return;
		}

		GlobalSettings::LoginConfig newConfig;
		newConfig.name = newName;
#ifndef NDEBUG
		newConfig.storeHome = QCoreApplication::applicationDirPath() + "/"  + newConfig.name + "/" + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName();
#else
		newConfig.storeHome = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName() + "/" + newConfig.name;
#endif // NDEBUG
		newConfig.removable = true;
		newConfig.managerUrl = managerAddress;
		
		// add to list
		QStandardItem *item = new QStandardItem();
		setLoginConfig(*item, newConfig);
		m_loginConfigModel->appendRow(item);

		QRadioButton *radioButton = new QRadioButton(newConfig.name, ui->listView);
		ui->listView->setIndexWidget(item->index(), radioButton);
		connect(radioButton, SIGNAL(toggled(bool)), SLOT(onListItemRadioToggled(bool)));

		// add to settings
		GlobalSettings::appendLoginConfig(newConfig.name, newConfig);
	}
}

void LoginSettingDlg::removeLoginConfig()
{
	int selIndex = 0;

	// remove the row
	for (int i = 0; i < m_loginConfigModel->rowCount(); i++)
	{
		QStandardItem *item = m_loginConfigModel->item(i);
		QRadioButton *radioButton = qobject_cast<QRadioButton *>(ui->listView->indexWidget(item->index()));
		if (radioButton->isChecked())
		{
			if (QDialogButtonBox::Cancel == PMessageBox::question(this, tr("Delete Login Setting"), tr("Are you sure to delete %1").arg(radioButton->text()), QDialogButtonBox::Ok|QDialogButtonBox::Cancel))
				return;

			QString configKey = radioButton->text();

			// remove list item
			ui->listView->setIndexWidget(item->index(), 0);
			m_loginConfigModel->takeRow(i);
			delete item;
			item = 0;

			// remove settings
			GlobalSettings::removeLoginConfig(configKey);

			selIndex = i;
			break;
		}
	}

	// set the new select index
	while (selIndex >= m_loginConfigModel->rowCount())
	{
		selIndex--;
	}

	if (selIndex >= 0 && selIndex < m_loginConfigModel->rowCount())
	{
		QStandardItem *item = m_loginConfigModel->item(selIndex);
		QRadioButton *radioButton = qobject_cast<QRadioButton *>(ui->listView->indexWidget(item->index()));
		radioButton->setChecked(true);
	}
}

void LoginSettingDlg::editLoginConfig()
{
	for (int i = 0; i < m_loginConfigModel->rowCount(); i++)
	{
		QStandardItem *item = m_loginConfigModel->item(i);
		QRadioButton *radioButton = qobject_cast<QRadioButton *>(ui->listView->indexWidget(item->index()));
		if (radioButton->isChecked())
		{
			LoginSettingEditDlg editDlg(radioButton->text(), this);
			editDlg.setManagerAddress(ui->leManagerAddress->text());
			if (editDlg.exec() == LoginSettingEditDlg::Accepted)
			{
				QString managerAddress = editDlg.managerAddress();
				if (!isValidAddress(managerAddress))
				{
					PMessageBox::warning(this, tr("Input Error"), tr("Server address input error"));
					return;
				}

				GlobalSettings::LoginConfig newConfig = getLoginConfig(*item);
				newConfig.managerUrl = managerAddress;

				// update to list
				setLoginConfig(*item, newConfig);

				// update to settings
				GlobalSettings::updateLoginConfig(newConfig.name, newConfig);

				// update ui
				ui->leManagerAddress->setText(managerAddress);
			}
		}
	}
}

void LoginSettingDlg::initUI()
{
	ui->listView->setStyleSheet("QListView{"
		"background: transparent;"
		"}"
		"QListView::item {"
		"height: 28px;"
		"background: transparent;"
		"}"
		"QRadioButton {"
		"padding-left: 8px;"
		"}"
		);

	
	ui->listView->setSelectionMode(QListView::SingleSelection);
	ui->listView->setEditTriggers(QListView::NoEditTriggers);

	QList<GlobalSettings::LoginConfig> loginConfigs = GlobalSettings::allLoginConfigs();
	m_loginConfigModel = new QStandardItemModel(this);
	ui->listView->setModel(m_loginConfigModel);
	QString curKey = GlobalSettings::getCurrentLoginKey();
	int index = 0;
	int i = 0;
	foreach (GlobalSettings::LoginConfig config, loginConfigs)
	{
		QStandardItem *item = new QStandardItem();
		setLoginConfig(*item, config);
		m_loginConfigModel->appendRow(item);

		QRadioButton *radioButton = new QRadioButton(config.name, ui->listView);
		ui->listView->setIndexWidget(item->index(), radioButton);
		connect(radioButton, SIGNAL(toggled(bool)), SLOT(onListItemRadioToggled(bool)));

		if (curKey == config.name)
			index = i;

		++i;
	}

	// select current auto reply text
	if (index >= 0 && index < loginConfigs.count())
	{
		QStandardItem *selItem = m_loginConfigModel->item(index);

		QRadioButton *radioButton = qobject_cast<QRadioButton *>(ui->listView->indexWidget(selItem->index()));
		radioButton->setChecked(true);

		ui->listView->scrollTo(selItem->index());
	}

	// init signals
	connect(ui->listView, SIGNAL(clicked(QModelIndex)), SLOT(onListViewItemClicked(QModelIndex)));
	connect(ui->pushButtonAdd, SIGNAL(clicked()), SLOT(addLoginConfig()));
	connect(ui->pushButtonDelete, SIGNAL(clicked()), SLOT(removeLoginConfig()));
	connect(ui->pushButtonEdit, SIGNAL(clicked()), SLOT(editLoginConfig()));
}

void LoginSettingDlg::setLoginConfig(QStandardItem &item, GlobalSettings::LoginConfig loginConfig)
{
	item.setData(loginConfig.name, Qt::UserRole+1);
	item.setData(loginConfig.storeHome, Qt::UserRole+6);
	item.setData(loginConfig.removable, Qt::UserRole+7);
	item.setData(loginConfig.managerUrl, Qt::UserRole+8);
}

GlobalSettings::LoginConfig LoginSettingDlg::getLoginConfig(const QStandardItem &item) const
{
	GlobalSettings::LoginConfig loginConfig;
	loginConfig.name = item.data(Qt::UserRole+1).toString();
	loginConfig.storeHome = item.data(Qt::UserRole+6).toString();
	loginConfig.removable = item.data(Qt::UserRole+7).toBool();
	loginConfig.managerUrl = item.data(Qt::UserRole+8).toString();
	return loginConfig;
}

bool LoginSettingDlg::isValidAddress(const QString &address)
{
	if (address.isEmpty())
		return false;

	if (!address.startsWith(QString::fromLatin1("https://")) && !address.startsWith(QString::fromLatin1("http://")))
		return false;

	return true;

	/*
	QStringList parts = address.split(":");
	if (parts.count() != 2)
		return false;

	QString ip = parts[0];
	QString port = parts[1];
	if (ip.isEmpty() || port.isEmpty())
		return false;

	QStringList ipParts = ip.split(".");
	if (ipParts.count() != 4)
		return false;

	bool changeOK = false;
	foreach (QString ipPart, ipParts)
	{
		if (ipPart.isEmpty())
			return false;

		int ipInt = ipPart.toInt(&changeOK);
		if (!changeOK)
			return false;

		if (ipInt < 0 || ipInt > 255)
			return false;
	}

	port.toInt(&changeOK);
	if (!changeOK)
		return false;

	return true;
	*/
}

void LoginSettingDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_8.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	this->setStyleSheet("QLabel#title {font-size: 12pt; color: white;}"
		"QPushButton#pushButtonAdd, QPushButton#pushButtonEdit, QPushButton#pushButtonDel {padding-left: 0px; padding-right: 0px;}"
		"QWidget#leftPanel {border: none; background-color: rgb(232, 232, 232);}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

