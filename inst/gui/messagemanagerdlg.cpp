#include <QDebug>
#include "PmApp.h"
#include "pmessagebox.h"
#include "common/datetime.h"
#include "buddymgr.h"
#include "model/ModelManager.h"
#include "model/rostermodeldef.h"
#include "model/groupitemdef.h"
#include "model/orgstructitemdef.h"
#include "model/groupmodeldef.h"
#include "model/orgstructmodeldef.h"
#include "model/DiscussModeldef.h"
#include "model/DiscussItemdef.h"
#include "dateperiodpicker.h"
#include "pmessagepopup.h"
#include "editfiltertreeview.h"
#include "messagemanagerdlg.h"
#include "ui_messagemanagerdlg.h"

static const int SearchLastOneWeek      = 0;
static const int SearchLastOneMonth     = 1;
static const int SearchLastThreeMonthes = 2;
static const int SearchLastOneYear      = 3;
static const int SearchAll              = 4;
static const int SearchDatePeriod       = 5;

MessageManagerDlg *MessageManagerDlg::s_instance = 0;

MessageManagerDlg::MessageManagerDlg(QWidget *parent)
	: FramelessDialog(parent), m_sourceType(SelectChatWidget::SourceInvalid)
{
	ui = new Ui::MessageManagerDlg();
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(912, 592);
	setResizeable(true);

	initUI();

	initSignals();

	setSkin();
}

MessageManagerDlg::~MessageManagerDlg()
{
	s_instance = 0;
	delete ui;
}

void MessageManagerDlg::init(const QString &id /*= ""*/, int msgType /*= bean::Message_Chat*/, const QString &extra /*= ""*/)
{
	if (isVisible() && id.isEmpty())
		return;

	SelectChatWidget::Source sourceType = SelectChatWidget::SourceInvalid;
	if (msgType == bean::Message_Chat)
	{
		if (extra.isEmpty())
		{
			ModelManager *modelManager = qPmApp->getModelManager();
			RosterModel *rosterModel = modelManager->rosterModel();
			if (id.isEmpty() || rosterModel->containsRoster(id))
			{
				sourceType = SelectChatWidget::RosterSource;
			}
			else
			{
				sourceType = SelectChatWidget::OsSource;
			}
		}
		else
		{
			sourceType = SelectChatWidget::OsSource;
		}
	}
	else if (msgType == bean::Message_GroupChat)
	{
		sourceType = SelectChatWidget::GroupSource;
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		sourceType = SelectChatWidget::DiscussSource;
	}

	ui->selectChatWidget->select(id, sourceType, extra);
}

MessageManagerDlg *MessageManagerDlg::instance()
{
	if (!s_instance)
	{
		s_instance = new MessageManagerDlg();
	}
	return s_instance;
}

void MessageManagerDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("QLabel {color: white; font-size: 12pt;}");
	ui->searchBar->setStyleSheet("QWidget#searchBar {background-color: rgb(232, 232, 232); border-bottom: 1px solid rgb(219, 219, 219);}");
	ui->msgHistoryPanel->setStyleSheet("QWidget#msgHistoryPanel {background-color: rgb(247, 247, 247);}");
}

void MessageManagerDlg::on_btnSearch_clicked()
{
	QString text = ui->lineEditWord->text();
	if (text.isEmpty())
	{
		QPoint p = ui->lineEditWord->mapToGlobal(QPoint(0, 0));
		PMessagePopup::information(tr("Please input key word"), p);
		return;
	}

	QString begindate = "";
	QString enddate = "";
	QDateTime datetime = CDateTime::currentDateTime();
	int comboidx = ui->comboBoxDate->currentIndex();
	switch (comboidx)
	{
	case SearchLastOneWeek:
		datetime = datetime.addDays(-7);
		break;
	case SearchLastOneMonth:
		datetime = datetime.addMonths(-1);
		break;
	case SearchLastThreeMonthes:
		datetime = datetime.addMonths(-3);
		break;
	case SearchLastOneYear:
		datetime = datetime.addYears(-1);
		break;
	default:
		break;
	}

	if (comboidx == SearchAll)
	{
		begindate = "";
		enddate = "";
	}
	else if (comboidx == SearchDatePeriod)
	{
		begindate = m_dateFrom.toString("yyyy-MM-dd 00:00:00");
		QDate tmpDate = m_dateTo.addDays(1);
		enddate = tmpDate.toString("yyyy-MM-dd 00:00:00");
	}
	else
	{
		begindate = datetime.toString("yyyy-MM-dd 00:00:00");
		enddate = "";
	}

	if (!begindate.isEmpty())
		begindate = CDateTime::utcDateTimeStringFromLocalDateTimeString(begindate);
	if (!enddate.isEmpty())
		enddate = CDateTime::utcDateTimeStringFromLocalDateTimeString(enddate);

	ui->msgHistoryWidget->search("", bean::Message_Invalid, begindate, enddate, text);
}

void MessageManagerDlg::onMaximizeStateChanged(bool isMaximized)
{
	if (isMaximized)
	{
		ui->btnMaximize->setChecked(true);
		ui->btnMaximize->setToolTip(tr("Restore"));
	}
	else
	{
		ui->btnMaximize->setChecked(false);
		ui->btnMaximize->setToolTip(tr("Maximize"));
	}
}

void MessageManagerDlg::initUI()
{
	ui->lineEditWord->setPlaceholderText(tr("Message Key Word"));

	ui->comboBoxDate->addItem(tr("Last Week"), SearchLastOneWeek);
	ui->comboBoxDate->addItem(tr("Last Month"), SearchLastOneMonth);
	ui->comboBoxDate->addItem(tr("Last Three Months"), SearchLastThreeMonthes);
	ui->comboBoxDate->addItem(tr("Last Year"), SearchLastOneYear);
	ui->comboBoxDate->addItem(tr("All"), SearchAll);
	ui->comboBoxDate->addItem(tr("A Period"), SearchDatePeriod);
	ui->comboBoxDate->setCurrentIndex(4);
	connect(ui->comboBoxDate, SIGNAL(activated(int)), this, SLOT(onComboBoxDateActivated(int)));

	m_dateTo = QDate::currentDate();
	m_dateFrom = m_dateTo.addDays(-7);

	// select widget
	ui->selectChatWidget->init(tr("Double click to view chat history"));

	// filter line edit
	QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
		QLatin1String("edit-clear-locationbar-rtl") :
	QLatin1String("edit-clear-locationbar-ltr"),
		QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/images/Icon_105.png"))));
	ui->leditFilter->setButtonPixmap(FilterLineEdit::Left, icon.pixmap(16));
	ui->leditFilter->setButtonVisible(FilterLineEdit::Left, true);
	ui->leditFilter->setAutoHideButton(FilterLineEdit::Left, false);
	ui->leditFilter->setPlaceholderText(tr("Search"));
	ui->leditFilter->setKeyDelegate(ui->selectChatWidget->searchWidget()->treeViewSearch());

	// message history
	ui->msgHistoryWidget->setShowTitle(true);
	ui->msgHistoryWidget->setShowSearchButton(false);
	ui->msgHistoryWidget->setShowMsgManagerButton(false);
	ui->msgHistoryWidget->setShowSearchMsgSource(true);
}

void MessageManagerDlg::initSignals()
{
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->btnMaximize, SIGNAL(clicked()), this, SLOT(triggerMaximize()));

	connect(ui->msgHistoryWidget, SIGNAL(messageIdTypeChanged(int, QString)), this, SLOT(onMessageIdTypeChanged(int, QString)));

	connect(ui->leditFilter, SIGNAL(filterChanged(QString)), ui->selectChatWidget, SLOT(editFilterChanged(QString)));
	connect(ui->selectChatWidget, SIGNAL(selectChanged(QString, int)), this, SLOT(chatSelected(QString, int)));

	connect(qPmApp->getBuddyMgr(), SIGNAL(sendSecretMessageRead(QString, QString)), ui->msgHistoryWidget, 
		SLOT(onSendSecretMessageRead(QString, QString)));

	connect(qPmApp->getBuddyMgr(), SIGNAL(recvSecretMessageRead(QString, QString)), ui->msgHistoryWidget, 
		SLOT(onRecvSecretMessageRead(QString, QString)));
}

void MessageManagerDlg::chatSelected(const QString &id, int source)
{
	ui->leditFilter->clear();

	if (id == m_sourceId && source == m_sourceType)
	{
		if (id.isEmpty() || source == SelectChatWidget::SourceInvalid)
		{
			ui->msgHistoryWidget->init(id, bean::Message_Invalid);
		}
		return;
	}

	m_sourceId = id;
	m_sourceType = source;

	bean::MessageType msgType = bean::Message_Invalid;
	if (source == SelectChatWidget::RosterSource) // roster
	{
		msgType = bean::Message_Chat;
	}
	if (source == SelectChatWidget::OsSource) // organization structure
	{
		msgType = bean::Message_Chat;
	}
	else if (source == SelectChatWidget::GroupSource) // group
	{
		msgType = bean::Message_GroupChat;
	}
	else if (source == SelectChatWidget::DiscussSource) // discuss
	{
		msgType = bean::Message_DiscussChat;
	}

	ui->msgHistoryWidget->init(id, msgType);
}

void MessageManagerDlg::onComboBoxDateActivated(int index)
{
	if (index == SearchDatePeriod)
	{
		DatePeriodPicker picker(m_dateFrom, m_dateTo, this);
		picker.setWindowModality(Qt::WindowModal);
		if (picker.exec())
		{
			m_dateFrom = picker.from();
			m_dateTo = picker.to();
		}
	}
}

void MessageManagerDlg::onMessageIdTypeChanged(int msgType, const QString &id)
{
	SelectChatWidget::Source sourceType = SelectChatWidget::SourceInvalid;
	if (msgType == bean::Message_Chat)
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		RosterModel *rosterModel = modelManager->rosterModel();
		if (id.isEmpty() || rosterModel->containsRoster(id))
		{
			sourceType = SelectChatWidget::RosterSource;
		}
		else
		{
			sourceType = SelectChatWidget::OsSource;
		}
	}
	else if (msgType == bean::Message_GroupChat)
	{
		sourceType = SelectChatWidget::GroupSource;
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		sourceType = SelectChatWidget::DiscussSource;
	}

	disconnect(ui->selectChatWidget, SIGNAL(selectChanged(QString, int)), this, SLOT(chatSelected(QString, int)));
	ui->selectChatWidget->select(id, sourceType);
	connect(ui->selectChatWidget, SIGNAL(selectChanged(QString, int)), this, SLOT(chatSelected(QString, int)));
}

void MessageManagerDlg::on_lineEditWord_returnPressed()
{
	on_btnSearch_clicked();
}
