#include "addfriendrequestdlg.h"
#include "ui_addfriendrequestdlg.h"
#include "ModelManager.h"
#include "PmApp.h"
#include <QMovie>
#include "addfriendmanager.h"
#include <QUuid>
#include "Account.h"
#include "loginmgr.h"
#include "pmessagebox.h"
#include "rostermodeldef.h"
#include "plaintextlineinput.h"
#include <QDebug>

QMap<QString, QPointer<AddFriendRequestDlg>> AddFriendRequestDlg::s_addFriendRequestDlgs;

AddFriendRequestDlg *AddFriendRequestDlg::getAddFriendRequestDlg(const QString &id, const QString &name)
{
	if (s_addFriendRequestDlgs.contains(id))
	{
		QPointer<AddFriendRequestDlg> dlg = s_addFriendRequestDlgs[id];
		if (!dlg.isNull())
		{
			return dlg.data();
		}
	}

	AddFriendRequestDlg *pDlg = new AddFriendRequestDlg(id, name);
	QPointer<AddFriendRequestDlg> dlg(pDlg);
	s_addFriendRequestDlgs.insert(id, dlg);
	return pDlg;
}

AddFriendRequestDlg::AddFriendRequestDlg(const QString &id, const QString &name, QWidget *parent)
	: FramelessDialog(parent), m_uid(id), m_name(name)
{
	ui = new Ui::AddFriendRequestDlg();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(tr("%1 - Add Friends").arg(m_name));
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(459, 349);
	setResizeable(false);

	initUI();

	connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)));

	bool connectOK = false;
	AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
	connectOK = connect(addFriendManager, SIGNAL(addFriendRequestOK(QString, int, QString, QString)), this, SLOT(onAddRequestSendOK(QString)));
	Q_ASSERT(connectOK);
	connectOK = connect(addFriendManager, SIGNAL(addFriendRequestFailed(QString, QString)), this, SLOT(onAddRequestSendFailed(QString, QString)));
	Q_ASSERT(connectOK);

	setSkin();
}

AddFriendRequestDlg::~AddFriendRequestDlg()
{
	delete ui;
}

void AddFriendRequestDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 31;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	ui->nameLabel->setStyleSheet("color: black;");
	ui->deptLabel->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
	ui->phoneLabel->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
	ui->leftPanel->setStyleSheet("QWidget#leftPanel {background: rgb(232, 232, 232);}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pushButtonAdd->setStyleSheet(qss);
		ui->pushButtonClose->setStyleSheet(qss);
		qssFile.close();
	}
}

void AddFriendRequestDlg::onAddRequestSendOK(const QString &sId)
{
	if (m_sId == sId)
	{
		ui->stackedWidget->setCurrentIndex(2);
		ui->labelResultIcon->setPixmap(QPixmap(":/messagebox/success.png"));
		ui->labelResultText->setText(tr("Your request sent ok. Please wait for acceptance."));
	}
}

void AddFriendRequestDlg::onAddRequestSendFailed(const QString &sId, const QString &desc)
{
	if (m_sId == sId)
	{
		ui->stackedWidget->setCurrentIndex(2);
		ui->labelResultIcon->setPixmap(QPixmap(":/messagebox/failed.png"));
		QString resultText = tr("Your request sent failed(%1)").arg(desc);
		ui->labelResultText->setText(resultText);
	}
}

void AddFriendRequestDlg::on_pushButtonAdd_clicked()
{
	if (ui->stackedWidget->currentIndex() == 0)
	{
		QString reason = ui->textEditReason->toPlainText();
		if (reason.count() > 48)
		{
			PMessageBox::information(this, tr("Tip"), tr("Message can't exceed 48 character"));
			return;
		}

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::information(this, tr("Tip"), tr("You are offline, can't add friends"));
			return;
		}

		// send add friend request
		m_sId = QUuid::createUuid().toString();
		m_sId = m_sId.mid(1, m_sId.length()-2);
		QString groupName = RosterModel::defaultGroupName();
		AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
		bool send = addFriendManager->addFriendAction(AddFriendManager::Request, Account::instance()->id(), m_uid, 
			m_sId, ui->textEditReason->toPlainText(), groupName);
		if (send)
		{
			ui->stackedWidget->setCurrentIndex(1); // loading
			ui->pushButtonAdd->setVisible(false); // hide add button
		}
		else
		{
			qWarning() << Q_FUNC_INFO << Account::instance()->id() << m_uid << m_sId << ui->textEditReason->toPlainText() << groupName;
		}
	}
}

void AddFriendRequestDlg::on_avatarLabel_clicked()
{
	emit viewMaterial(m_uid);
}

void AddFriendRequestDlg::on_stackedWidget_currentChanged(int index)
{
	ui->pushButtonAdd->setText(tr("Send"));
	if (index == 1)
	{
		QMovie *movie = ui->labelLoading->movie();
		movie->start();
	}
	else
	{
		QMovie *movie = ui->labelLoading->movie();
		movie->stop();
	}
}

void AddFriendRequestDlg::onDetailChanged(const QString &id)
{
	if (id == m_uid)
	{
		setDetail();
	}
}

void AddFriendRequestDlg::initUI()
{
	setDetail();

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);

	ui->stackedWidget->setCurrentIndex(0);
}

void AddFriendRequestDlg::setDetail()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *detail = modelManager->detailItem(m_uid);
	m_name = modelManager->userName(m_uid);
	int sex = detail->sex();
	QString dept = detail->depart();
	QString phone = detail->phone1();

	ui->avatarLabel->setClickable(true);
	QPixmap avatar = modelManager->getUserAvatar(m_uid);
	ui->avatarLabel->setPixmap(avatar);

	ui->nameLabel->setText(m_name);
	if (sex == 0)
	{
		// female
		ui->sexLabel->setPixmap(QPixmap(":/images/Icon_95.png"));
	}
	else if (sex == 1)
	{
		// male
		ui->sexLabel->setPixmap(QPixmap(":/images/Icon_94.png"));
	}
	else
	{
		// secret
		ui->sexLabel->setPixmap(QPixmap());
	}
	ui->deptLabel->setText(tr("Dept: %1").arg(dept));

	ui->phoneLabel->setText(tr("Phone: %1").arg(phone));
}

