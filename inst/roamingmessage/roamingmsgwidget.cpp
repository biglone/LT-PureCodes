#include "roamingmsgwidget.h"
#include "ui_roamingmsgwidget.h"
#include "PmApp.h"
#include "roamingmsgmanager.h"
#include "Account.h"
#include "common/datetime.h"
#include "message4js.h"
#include <QDebug>
#include "dateperiodpicker.h"
#include <QMovie>

static const int     kPageSize          = 60;
static const int SearchLastOneWeek      = 0;
static const int SearchLastOneMonth     = 1;
static const int SearchLastThreeMonthes = 2;
static const int SearchLastOneYear      = 3;
static const int SearchAll              = 4;
static const int SearchDatePeriod       = 5;

RoamingMsgWidget::RoamingMsgWidget(QWidget *parent)
	: QWidget(parent)
	, m_eType(bean::Message_Invalid)
	, m_pMessage4js(0)
	, m_pageDirection(PagePrev)
	, m_nCurPage(0)
	, m_nMaxPage(0)
	, m_inited(false)
{
	ui = new Ui::RoamingMsgWidget();
	ui->setupUi(this);

	initUI();

	setSkin();
}

RoamingMsgWidget::~RoamingMsgWidget()
{
	delete ui;
}

bool RoamingMsgWidget::init(const QString &id, bean::MessageType type)
{
	if (m_inited)
		return true;

	resetRoaming();

	m_qsId = id;
	m_eType = type;
	ui->labPageCount->setText(QString("%1").arg(m_nMaxPage));
	ui->leditPage->setText(QString("%1").arg(m_nCurPage));

	if (m_qsId.isEmpty() || m_eType == bean::Message_Invalid)
	{
		ui->stackedWidget->setCurrentIndex(1);
		m_pMessage4js->showNoMessage();
		return false;
	}

	changeCurPage();

	m_inited = true;

	return true;
}

void RoamingMsgWidget::setSkin()
{
	this->setStyleSheet(QString("QWidget#bottomBar, QWidget#searchBar {background-color: rgb(232, 232, 232);}"));
}

void RoamingMsgWidget::on_pBtnBegin_clicked()
{
	m_nCurPage = 1;
	m_pageDirection = PageNext;
	changeCurPage();
}

void RoamingMsgWidget::on_pBtnPrevious_clicked()
{
	if (m_nCurPage - 1 < 1)
		return;
	--m_nCurPage;

	m_pageDirection = PagePrev;
	changeCurPage();
}

void RoamingMsgWidget::on_pBtnNext_clicked()
{
	if (m_nCurPage + 1 > m_nMaxPage)
		return;
	++m_nCurPage;

	m_pageDirection = PageNext;
	changeCurPage();
}

void RoamingMsgWidget::on_pBtnEnd_clicked()
{
	m_nCurPage = m_nMaxPage;
	
	m_pageDirection = PagePrev;
	changeCurPage();
}

void RoamingMsgWidget::changeCurPage()
{
	startOperation();
	ui->stackedWidget->setCurrentIndex(0);
	RoamingMsgManager *roamingMsgManager = qPmApp->getRoamingMsgManager();
	if (m_eType == bean::Message_Chat)
	{
		roamingMsgManager->getChatRoamingMessage(m_qsId, Account::instance()->id(), kPageSize, m_nCurPage, m_beginDate, m_endDate);
	}
	else if (m_eType == bean::Message_GroupChat)
	{
		roamingMsgManager->getGroupRoamingMessage(m_qsId, kPageSize, m_nCurPage, m_beginDate, m_endDate);
	}
	else if (m_eType == bean::Message_DiscussChat)
	{
		roamingMsgManager->getDiscussRoamingMessage(m_qsId, kPageSize, m_nCurPage, m_beginDate, m_endDate);
	}
}

void RoamingMsgWidget::onComboBoxDateActivated(int index)
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

void RoamingMsgWidget::on_tBtnSearch_clicked()
{
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
		m_beginDate = "";
		m_endDate = "";
	}
	else if (comboidx == SearchDatePeriod)
	{
		m_beginDate = m_dateFrom.toString("yyyy-MM-dd 00:00:00");
		QDate tmpDate = m_dateTo.addDays(1);
		m_endDate = tmpDate.toString("yyyy-MM-dd 00:00:00");
	}
	else
	{
		m_beginDate = datetime.toString("yyyy-MM-dd 00:00:00");
		m_endDate = "";
	}

	if (!m_beginDate.isEmpty())
		m_beginDate = CDateTime::utcDateTimeStringFromLocalDateTimeString(m_beginDate);
	if (!m_endDate.isEmpty())
		m_endDate = CDateTime::utcDateTimeStringFromLocalDateTimeString(m_endDate);

	m_nCurPage = 0;
	m_nMaxPage = 0;

	changeCurPage();
}

void RoamingMsgWidget::on_tBtnRefresh_clicked()
{
	changeCurPage();
}

void RoamingMsgWidget::on_tBtnSearchBar_toggled(bool checked)
{
	ui->searchBar->setVisible(checked);
	ui->comboBoxDate->setCurrentIndex(4);
}

void RoamingMsgWidget::gotChatRoamingMessageOK(const QString &uid, int pageSize, int currentPage, int totalPage,
	const bean::MessageBodyList &messages)
{
	if (m_eType == bean::Message_Chat && m_qsId == uid)
	{
		onGotMessageOK(pageSize, currentPage, totalPage, messages);
	}
}

void RoamingMsgWidget::gotChatRoamingMessageFailed(const QString &uid)
{
	if (m_eType == bean::Message_Chat && m_qsId == uid)
	{
		onGotMessageFailed();
	}
}

void RoamingMsgWidget::gotGroupRoamingMessageOK(const QString &groupId, int pageSize, int currentPage, int totalPage,
	const bean::MessageBodyList &messages)
{
	if (m_eType == bean::Message_GroupChat && m_qsId == groupId)
	{
		onGotMessageOK(pageSize, currentPage, totalPage, messages);
	}
}

void RoamingMsgWidget::gotGroupRoamingMessageFailed(const QString &groupId)
{
	if (m_eType == bean::Message_GroupChat && m_qsId == groupId)
	{
		onGotMessageFailed();
	}
}

void RoamingMsgWidget::gotDiscussRoamingMessageOK(const QString &discussId, int pageSize, int currentPage, int totalPage,
	const bean::MessageBodyList &messages)
{
	if (m_eType == bean::Message_DiscussChat && m_qsId == discussId)
	{
		onGotMessageOK(pageSize, currentPage, totalPage, messages);
	}
}

void RoamingMsgWidget::gotDiscussRoamingMessageFailed(const QString &discussId)
{
	if (m_eType == bean::Message_DiscussChat && m_qsId == discussId)
	{
		onGotMessageFailed();
	}
}

void RoamingMsgWidget::initUI()
{
	// page edit validator
	QRegExpValidator *pValidator = new QRegExpValidator(this);
	pValidator->setRegExp(QRegExp("[0-9]+"));
	ui->leditPage->setValidator(pValidator);

	// web view related
	QString tag = QString("roamingmsg");
	ui->msgBrowser->setTag(tag);
	ui->msgBrowser->setContextMenuPolicy(Qt::NoContextMenu);
	ui->msgBrowser->setUrl(QUrl("qrc:/html/roamingmsglist.html"));
	m_pMessage4js = ui->msgBrowser->message4Js();
	
	// load page
	QMovie *movie = new QMovie(":/images/loading_image_small.gif");
	movie->setParent(this);
	ui->labLoading->setMovie(movie);
	movie->start();

	// search bar initialize
	ui->searchBar->setVisible(false);

	ui->comboBoxDate->addItem(tr("Last One Week"), SearchLastOneWeek);
	ui->comboBoxDate->addItem(tr("Last One Month"), SearchLastOneMonth);
	ui->comboBoxDate->addItem(tr("Last Three Months"), SearchLastThreeMonthes);
	ui->comboBoxDate->addItem(tr("Last One Year"), SearchLastOneYear);
	ui->comboBoxDate->addItem(tr("All"), SearchAll);
	ui->comboBoxDate->addItem(tr("A Period"), SearchDatePeriod);
	ui->comboBoxDate->setCurrentIndex(4);
	connect(ui->comboBoxDate, SIGNAL(activated(int)), this, SLOT(onComboBoxDateActivated(int)));

	RoamingMsgManager *roamingMsgManager = qPmApp->getRoamingMsgManager();
	connect(roamingMsgManager, SIGNAL(gotChatRoamingMessageOK(QString, int, int, int, bean::MessageBodyList)),
		this, SLOT(gotChatRoamingMessageOK(QString, int, int, int, bean::MessageBodyList)));
	connect(roamingMsgManager, SIGNAL(gotChatRoamingMessageFailed(QString)),
		this, SLOT(gotChatRoamingMessageFailed(QString)));
	connect(roamingMsgManager, SIGNAL(gotGroupRoamingMessageOK(QString, int, int, int, bean::MessageBodyList)),
		this, SLOT(gotGroupRoamingMessageOK(QString, int, int, int, bean::MessageBodyList)));
	connect(roamingMsgManager, SIGNAL(gotGroupRoamingMessageFailed(QString)),
		this, SLOT(gotGroupRoamingMessageFailed(QString)));
	connect(roamingMsgManager, SIGNAL(gotDiscussRoamingMessageOK(QString, int, int, int, bean::MessageBodyList)),
		this, SLOT(gotDiscussRoamingMessageOK(QString, int, int, int, bean::MessageBodyList)));
	connect(roamingMsgManager, SIGNAL(gotDiscussRoamingMessageFailed(QString)),
		this, SLOT(gotDiscussRoamingMessageFailed(QString)));
}

void RoamingMsgWidget::startOperation()
{
	ui->pBtnBegin->setEnabled(false);
	ui->pBtnPrevious->setEnabled(false);
	ui->pBtnNext->setEnabled(false);
	ui->pBtnEnd->setEnabled(false);
	ui->leditPage->setEnabled(false);
	ui->tBtnRefresh->setEnabled(false);
	ui->tBtnSearchBar->setEnabled(false);
	ui->comboBoxDate->setEnabled(false);
}

void RoamingMsgWidget::endOperation()
{
	ui->pBtnBegin->setEnabled(true);
	ui->pBtnPrevious->setEnabled(true);
	ui->pBtnNext->setEnabled(true);
	ui->pBtnEnd->setEnabled(true);
	ui->leditPage->setEnabled(true);
	ui->tBtnRefresh->setEnabled(true);
	ui->tBtnSearchBar->setEnabled(true);
	ui->comboBoxDate->setEnabled(true);
}

void RoamingMsgWidget::configBottomBar(int curPage, int maxPage)
{
	// bottom bar
	ui->labPageCount->setText(QString("%1").arg(maxPage));
	ui->leditPage->setText(QString("%1").arg(curPage));

	if (curPage > 1)
	{
		ui->pBtnBegin->setEnabled(true);
		ui->pBtnPrevious->setEnabled(true);
	}
	else
	{
		ui->pBtnBegin->setEnabled(false);
		ui->pBtnPrevious->setEnabled(false);
	}

	if (curPage < maxPage)
	{
		ui->pBtnNext->setEnabled(true);
		ui->pBtnEnd->setEnabled(true);
	}
	else
	{
		ui->pBtnNext->setEnabled(false);
		ui->pBtnEnd->setEnabled(false);
	}
}

void RoamingMsgWidget::resetRoaming()
{
	m_pageDirection = PagePrev;
	m_qsId = QString();
	m_eType = bean::Message_Invalid;
	m_nMaxPage = 0;
	m_nCurPage = 0;
	m_dateTo = CDateTime::currentDateTime().date();
	m_dateFrom = m_dateTo.addDays(-7);
	m_beginDate.clear();
	m_endDate.clear();
}

void RoamingMsgWidget::onGotMessageOK(int pageSize, int currentPage, int totalPage, const bean::MessageBodyList &messages)
{
	Q_UNUSED(pageSize);

	endOperation();

	m_nCurPage = currentPage;
	m_nMaxPage = totalPage;
	
	if (m_nMaxPage <= 0)
	{
		m_pMessage4js->showNoMessage();
	}
	else
	{
		m_pMessage4js->removeAllMsgs();
		m_pMessage4js->setMessages(messages);
		/*
		if (m_pageDirection == PagePrev)
			m_pMessage4js->scrollMsgsToBottom();
		else
			m_pMessage4js->scrollMsgsToTop();
		*/
	}

	ui->stackedWidget->setCurrentIndex(1); // message page

	configBottomBar(m_nCurPage, m_nMaxPage);
}

void RoamingMsgWidget::onGotMessageFailed()
{
	endOperation();

	ui->stackedWidget->setCurrentIndex(1); // message page
	m_pMessage4js->showError();
}