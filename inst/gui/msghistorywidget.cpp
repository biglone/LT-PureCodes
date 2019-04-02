#include <QValidator>
#include <QtGui>
#include <QWebPage>
#include <QWebFrame>

#include "application/Logger.h"
using namespace application;

#include "message4js.h"
#include "common/datetime.h"

#include "PmApp.h"
#include "ModelManager.h"
#include "MessageDBStore.h"
#include "db/ChatMessageDB.h"
#include "db/GroupMessageDB.h"

#include "pmessagebox.h"
#include "msghistorywidget.h"
#include "ui_msghistorywidget.h"

#include "util/AmrPlayUtil.h"

#include "Account.h"
#include "buddymgr.h"
#include "dateperiodpicker.h"
#include "MessageManagerDlg.h"
#include "widgetmanager.h"
#include "msghistorydeletedialog.h"
#include "pmessagepopup.h"
#include "chatinputbox.h"

static const int SearchLastOneWeek      = 0;
static const int SearchLastOneMonth     = 1;
static const int SearchLastThreeMonthes = 2;
static const int SearchLastOneYear      = 3;
static const int SearchAll              = 4;
static const int SearchDatePeriod       = 5;

MsgHistoryWidget::MsgHistoryWidget(QWidget *parent)
	: QWidget(parent)
	, m_eType(bean::Message_Invalid)
	, m_pMessage4js(0)
	, m_nCurPage(0)
	, m_nMaxPage(0)
	, m_pSearchMessage4js(0)
	, m_searchType(bean::Message_Invalid)
	, m_nSearchCurPage(0)
	, m_nSearchMaxPage(0)
	, m_contextType(bean::Message_Invalid)
	, m_nContextCurPage(0)
	, m_nContextMaxPage(0)
	, m_pageDirection(PagePrev)
	, m_historyState(StateHistory)
	, m_nSearchId(-1)
	, m_nDeleteId(-1)
	, m_titleVisibility(false)
	, m_nCopyId(-1)
{
	ui = new Ui::MsgHistoryWidget();
	ui->setupUi(this);

	initUI();

	setSkin();
}

MsgHistoryWidget::~MsgHistoryWidget()
{
	delete ui;
}

void MsgHistoryWidget::setShowTitle(bool show)
{
	m_titleVisibility = show;
}

bool MsgHistoryWidget::isShowTitle() const
{
	return m_titleVisibility;
}

void MsgHistoryWidget::setShowSearchButton(bool show)
{
	ui->tBtnSearchBar->setVisible(show);
}

void MsgHistoryWidget::setShowMsgManagerButton(bool show)
{
	ui->tBtnMsgManager->setVisible(show);
}

void MsgHistoryWidget::setShowSearchMsgSource(bool show)
{
	m_pSearchMessage4js->setEnableMsgSource(show);
}

void MsgHistoryWidget::hideSearchBar()
{
	ui->tBtnSearchBar->setChecked(false);
}

bool MsgHistoryWidget::init(const QString &id, bean::MessageType type)
{
	resetHistory();
	switchHistoryState(StateHistory);

	m_qsId = id;
	m_eType = type;
	configContextLabel(m_eType, m_qsId);

	ui->labelSearchResultTip->clear();
	
	ui->contextInfoLine->setVisible(m_titleVisibility);
	ui->labelContextTip->setVisible(m_titleVisibility);
	ui->tBtnContextBack->setVisible(false);

	ui->labPageCount->setText(QString("%1").arg(m_nMaxPage));
	ui->leditPage->setText(QString("%1").arg(m_nCurPage));

	if (ui->msgBrowser->isLoadFinished())
	{
		if (m_qsId.isEmpty() || m_eType == bean::Message_Invalid)
		{
			ui->stackedWidget->setCurrentIndex(1);
			m_pMessage4js->showNoMessage();
			configBottomBar(m_nCurPage, m_nMaxPage);
			return false;
		}
		else
		{
			// search to the last page
			changeCurPage();
			return true;
		}
	}
	else
	{
		return true;
	}
}

void MsgHistoryWidget::search(const QString &id, bean::MessageType type, const QString &begindate, const QString &enddate, const QString &keyword)
{
	m_searchId = id;
	m_searchType = type;
	m_searchBeginDate = begindate;
	m_searchEndDate = enddate;
	m_searchKeyword = keyword;
	m_nSearchCurPage = 0;
	m_nSearchMaxPage = 0;
	switchHistoryState(StateSearch);

	ui->labPageCount->setText(QString("%1").arg(m_nSearchMaxPage));
	ui->leditPage->setText(QString("%1").arg(m_nSearchCurPage));

	changeCurPage();
}

void MsgHistoryWidget::setSkin()
{
	// set font and color
	ui->contextInfoLine->setStyleSheet("QWidget#contextInfoLine {border-bottom: 1px solid rgb(219, 219, 219);}");
	ui->searchInfoLine->setStyleSheet("QWidget#searchInfoLine {border-bottom: 1px solid rgb(219, 219, 219);}");
	ui->labelSearchResultTip->setStyleSheet("QLabel {color: black; font-size: 9pt;}");
	ui->labelContextTip->setStyleSheet("QLabel {color: black; font-size: 9pt;}");
	this->setStyleSheet(QString("QWidget#bottomBar, QWidget#searchBar {background-color: rgb(232, 232, 232);}"));
}

bool MsgHistoryWidget::canDragImage(const QWebElement &imageElement)
{
	QString secretFlag = imageElement.attribute("secret", "0");
	if (secretFlag != "1") // secret message can't drag
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MsgHistoryWidget::addMenuAction(QMenu *menu, const QWebElement &webElement)
{
	if (!menu)
		return;

	menu->clear();

	// image actions
	if (webElement.hasClass("img_autodisplay"))
	{
		QString secretFlag = webElement.attribute("secret", "0");
		if (secretFlag != "1")
		{
			QString pathUrl = webElement.attribute("origsrc", "");
			m_imageCopyAction->setData(pathUrl);
			m_imageSaveAction->setData(pathUrl);
			menu->addAction(m_imageCopyAction);
			menu->addAction(m_imageSaveAction);
		}
		return;
	}

	// remove actions
	QWebElement ele = webElement;
	do {
		if (ele.hasClass("msgContainer") || ele.hasClass("msgContainer_focus"))
		{
			QString sMsgType = ele.attribute("msgType");
			bean::MessageType msgType = bean::Message_Invalid;
			if (sMsgType == bean::kszChat)
				msgType = bean::Message_Chat;
			else if (sMsgType == bean::kszGroup)
				msgType = bean::Message_GroupChat;
			else if (sMsgType == bean::kszDiscuss)
				msgType = bean::Message_DiscussChat;
			if (msgType == bean::Message_Invalid)
				break;

			QString sMsgId = ele.attribute("id");
			int msgId = sMsgId.toInt();

			if (m_historyState == StateSearch)
				m_pSearchMessage4js->setMessageFocused(sMsgId);
			else
				m_pMessage4js->setMessageFocused(sMsgId);

			QString arg = QString("%1:%2").arg((int)msgType).arg(msgId);
			m_messageCopyAction->setData(arg);
			m_removeOneAction->setData(arg);
			m_removeMoreAction->setData(arg);

			QString secretFlag = ele.attribute("secret", "0");
			if (secretFlag != "1")
			{
				menu->addAction(m_messageCopyAction);
				menu->addSeparator();
			}

			menu->addAction(m_removeOneAction);

			if (m_historyState != StateSearch)
				menu->addAction(m_removeMoreAction);
			
			break;
		}
		ele = ele.parent();
	} while (!ele.isNull());
}

void MsgHistoryWidget::onSendSecretMessageRead(const QString &uid, const QString &stamp)
{
	if (uid == m_qsId && m_eType == bean::Message_Chat)
	{
		emit sendSecretMessageRead(stamp);
	}
	else if (uid == m_contextId && m_contextType == bean::Message_Chat)
	{
		emit sendSecretMessageRead(stamp);
	}
}

void MsgHistoryWidget::onRecvSecretMessageRead(const QString &uid, const QString &stamp)
{
	if (uid == m_qsId && m_eType == bean::Message_Chat)
	{
		emit recvSecretMessageRead(stamp);
	}
	else if (uid == m_contextId && m_contextType == bean::Message_Chat)
	{
		emit recvSecretMessageRead(stamp);
	}
}

void MsgHistoryWidget::onMsgBrowserLoadFinished()
{
	if (m_qsId.isEmpty() || m_eType == bean::Message_Invalid)
	{
		ui->stackedWidget->setCurrentIndex(1);
		m_pMessage4js->showNoMessage();
		configBottomBar(m_nCurPage, m_nMaxPage);
	}
	else
	{
		// search to the last page
		changeCurPage();
	}
}

void MsgHistoryWidget::on_pBtnBegin_clicked()
{
	if (m_historyState == StateHistory)
		m_nCurPage = 1;
	else if (m_historyState == StateSearch)
		m_nSearchCurPage = 1;
	else if (m_historyState == StateContext)
		m_nContextCurPage = 1;

	m_pageDirection = PageNext;
	changeCurPage();
}

void MsgHistoryWidget::on_pBtnPrevious_clicked()
{
	if (m_historyState == StateHistory)
	{
		if (m_nCurPage - 1 < 1)
			return;
		--m_nCurPage;
	}
	else if (m_historyState == StateSearch)
	{
		if (m_nSearchCurPage - 1 < 1)
			return;
		--m_nSearchCurPage;
	}
	else if (m_historyState == StateContext)
	{
		if (m_nContextCurPage - 1 < 1)
			return;
		--m_nContextCurPage;
	}

	m_pageDirection = PagePrev;
	changeCurPage();
}

void MsgHistoryWidget::on_pBtnNext_clicked()
{
	if (m_historyState == StateHistory)
	{
		if (m_nCurPage + 1 > m_nMaxPage)
			return;
		++m_nCurPage;
	}
	else if (m_historyState == StateSearch)
	{
		if (m_nSearchCurPage + 1 > m_nSearchMaxPage)
			return;
		++m_nSearchCurPage;
	}
	else if (m_historyState == StateContext)
	{
		if (m_nContextCurPage + 1 > m_nContextMaxPage)
			return;
		++m_nContextCurPage;
	}

	m_pageDirection = PageNext;
	changeCurPage();
}

void MsgHistoryWidget::on_pBtnEnd_clicked()
{
	if (m_historyState == StateHistory)
		m_nCurPage = m_nMaxPage;
	else if (m_historyState == StateSearch)
		m_nSearchCurPage = m_nSearchMaxPage;
	else if (m_historyState == StateContext)
		m_nContextCurPage = m_nContextMaxPage;

	m_pageDirection = PagePrev;
	changeCurPage();
}

void MsgHistoryWidget::on_leditPage_returnPressed()
{
	int nPage = ui->leditPage->text().toInt();
	QPoint pt = ui->leditPage->mapToGlobal(QPoint(0, 0));

	if (m_historyState == StateHistory)
	{
		if (nPage < 1 || nPage > m_nMaxPage)
		{
			PMessagePopup::information(tr("Please input correct page number"), pt);
			return;
		}
		m_nCurPage = nPage;
	}
	else if (m_historyState == StateSearch)
	{
		if (nPage < 1 || nPage > m_nSearchMaxPage)
		{
			PMessagePopup::information(tr("Please input correct page number"), pt);
			return;
		}
		m_nSearchCurPage = nPage;
	}
	else if (m_historyState == StateContext)
	{
		if (nPage < 1 || nPage > m_nContextMaxPage)
		{
			PMessagePopup::information(tr("Please input correct page number"), pt);
			return;
		}
		m_nContextCurPage = nPage;
	}

	m_pageDirection = PageNext;
	changeCurPage();
}

void MsgHistoryWidget::slot_removeOneFromDB(int nMsgId, int msgType)
{
	if (msgType == bean::Message_Invalid)
		return;

	PMessageBox msgBox(PMessageBox::Question, tr("Are you sure to delete selected messages"), 
		QDialogButtonBox::Yes|QDialogButtonBox::No, tr("Delete Chat History"), this);
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.exec();
	if (msgBox.clickedButton() == QDialogButtonBox::Yes)
	{
		startOperation();
		ui->stackedWidget->setCurrentIndex(0);
		m_nDeleteId = qPmApp->getMessageDBStore()->removeMsgByMsgId((bean::MessageType)msgType, nMsgId);
		qDebug() << Q_FUNC_INFO << msgType << nMsgId << m_nDeleteId;
	}
}

void MsgHistoryWidget::slot_removeMoreFromDB(int msgType)
{
	if (msgType == bean::Message_Invalid)
		return;

	if (m_historyState == StateSearch)
		return;

	MsgHistoryDeleteDialog dlg(this->window());
	dlg.setWindowModality(Qt::WindowModal);
	if (dlg.exec())
	{
		MsgHistoryDeleteDialog::DeleteAction delAction = dlg.deleteAction();

		QDateTime dateTime = CDateTime::currentDateTime();
		QString beginTime;
		QString endTime;
		switch (delAction)
		{
		case MsgHistoryDeleteDialog::DeleteOneDayBefore:
			dateTime.setTime(QTime(0,0));
			endTime = CDateTime::QDateTimeToString(dateTime);
			break;
		case MsgHistoryDeleteDialog::DeleteOneWeekBefore:
			dateTime = dateTime.addDays(-7);
			dateTime.setTime(QTime(0,0));
			endTime = CDateTime::QDateTimeToString(dateTime);
			break;
		case MsgHistoryDeleteDialog::DeleteOneMonthBefore:
			dateTime = dateTime.addMonths(-1);
			dateTime.setTime(QTime(0,0));
			endTime = CDateTime::QDateTimeToString(dateTime);
			break;
		case MsgHistoryDeleteDialog::DeleteThreeMonthBefore:
			dateTime = dateTime.addMonths(-3);
			dateTime.setTime(QTime(0,0));
			endTime = CDateTime::QDateTimeToString(dateTime);
			break;
		case MsgHistoryDeleteDialog::DeleteSixMonthBefore:
			dateTime = dateTime.addMonths(-6);
			dateTime.setTime(QTime(0,0));
			endTime = CDateTime::QDateTimeToString(dateTime);
			break;
		case MsgHistoryDeleteDialog::DeleteOneYearBefore:
			dateTime = dateTime.addYears(-1);
			dateTime.setTime(QTime(0,0));
			endTime = CDateTime::QDateTimeToString(dateTime);
			break;
		case MsgHistoryDeleteDialog::DeleteDatePeriod:
			{
				QDate dtFrom;
				QDate dtTo;
				dlg.getDatePeriod(dtFrom, dtTo);
				dtTo = dtTo.addDays(1);
				beginTime = dtFrom.toString("yyyy-MM-dd 00:00:00");
				endTime = dtTo.toString("yyyy-MM-dd 00:00:00");
			}
			break;
		}

		if (!beginTime.isEmpty())
			beginTime = CDateTime::utcDateTimeStringFromLocalDateTimeString(beginTime);
		if (!endTime.isEmpty())
			endTime = CDateTime::utcDateTimeStringFromLocalDateTimeString(endTime);

		startOperation();
		ui->stackedWidget->setCurrentIndex(0);

		if (m_historyState == StateHistory)
		{
			m_nDeleteId = qPmApp->getMessageDBStore()->removeMsgByDate((bean::MessageType)msgType, m_qsId, beginTime, endTime);
			qDebug() << Q_FUNC_INFO << msgType << m_qsId << beginTime << endTime << m_nDeleteId;
		}
		else if (m_historyState == StateContext)
		{
			m_nDeleteId = qPmApp->getMessageDBStore()->removeMsgByDate((bean::MessageType)msgType, m_contextId, beginTime, endTime);
			qDebug() << Q_FUNC_INFO << msgType << m_contextId << beginTime << endTime << m_nDeleteId;
		}
	}
}

void MsgHistoryWidget::onGotMessages(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs, int sum)
{
	if (seq != m_nSearchId)
		return;

	qDebug() << Q_FUNC_INFO << seq << curPage << maxPage << msgs.count();
	
	endOperation();

	if (m_historyState == StateSearch)
	{
		m_nSearchCurPage = curPage;
		m_nSearchMaxPage = maxPage;

		// add messages
		if (m_nSearchMaxPage <= 0)
		{
			ui->labelSearchResultTip->setText(tr("No search result"));

			m_pSearchMessage4js->showNoMessage();
		}
		else
		{
			ui->labelSearchResultTip->setText(tr("Searched %1 items").arg(sum));

			m_pSearchMessage4js->removeAllMsgs();
			m_pSearchMessage4js->setMessages(msgs);
			if (m_pageDirection == PagePrev)
				m_pSearchMessage4js->scrollMsgsToBottom();
			else
				m_pSearchMessage4js->scrollMsgsToTop();

			m_pSearchMessage4js->setKeywordHighlighted(m_searchKeyword);
		}

		ui->stackedWidget->setCurrentIndex(2); // search page

		configBottomBar(m_nSearchCurPage, m_nSearchMaxPage);
	}
	else
	{
		if (m_historyState == StateHistory)
		{
			m_nCurPage = curPage;
			m_nMaxPage = maxPage;
		}
		else if (m_historyState == StateContext)
		{
			m_nContextCurPage = curPage;
			m_nContextMaxPage = maxPage;
		}

		if (m_nMaxPage <= 0)
		{
			m_pMessage4js->showNoMessage();
		}
		else
		{
			m_pMessage4js->removeAllMsgs();
			m_pMessage4js->setMessages(msgs);
			if (m_pageDirection == PagePrev)
				m_pMessage4js->scrollMsgsToBottom();
			else
				m_pMessage4js->scrollMsgsToTop();

			// check secret state
			checkSecretState(msgs);
		}

		ui->stackedWidget->setCurrentIndex(1); // message page

		configBottomBar(curPage, maxPage);
	}
}

void MsgHistoryWidget::onGotContextMessages(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs, int sum)
{
	Q_UNUSED(sum);

	if (seq != m_nSearchId)
		return;

	qDebug() << Q_FUNC_INFO << seq << curPage << maxPage << msgs.count();

	endOperation();

	m_nContextCurPage = curPage;
	m_nContextMaxPage = maxPage;

	if (m_nContextMaxPage <= 0)
	{
		m_pMessage4js->showNoMessage();
	}
	else
	{
		// qint64 t1 = QDateTime::currentMSecsSinceEpoch();

		m_pMessage4js->removeAllMsgs();
		m_pMessage4js->setMessages(msgs);

		// scroll to that message
		ui->msgBrowser->page()->mainFrame()->scrollToAnchor(m_contextMsgId);

		// focus that message
		m_pMessage4js->setMessageFocused(m_contextMsgId);

		// qDebug() << Q_FUNC_INFO << " show message ms: " << QDateTime::currentMSecsSinceEpoch() - t1;

		// check secret state
		checkSecretState(msgs);
	}

	ui->stackedWidget->setCurrentIndex(1); // message page

	configBottomBar(m_nContextCurPage, m_nContextMaxPage);
}

void MsgHistoryWidget::onMsgRemoved(qint64 seq)
{
	if (seq != m_nDeleteId)
		return;

	qDebug() << Q_FUNC_INFO << seq;

	endOperation();

	changeCurPage();
}

void MsgHistoryWidget::onGotMessageOfMsgId(qint64 seq, int msgId, const bean::MessageBody &msg)
{
	Q_UNUSED(msgId);
	if (seq != m_nCopyId)
		return;

	if (!msg.isValid())
		return;

	doMessageCopy(msg);
}

void MsgHistoryWidget::changeCurPage()
{
	startOperation();
	ui->stackedWidget->setCurrentIndex(0);
	MessageDBStore *messageDBStore = qPmApp->getMessageDBStore();
	if (m_historyState == StateHistory)
	{
		m_nSearchId = messageDBStore->getMessages(m_eType, m_qsId, m_nCurPage);
		qDebug() << Q_FUNC_INFO << m_eType << m_qsId << m_nCurPage << m_nSearchId;
	}
	else if (m_historyState == StateSearch)
	{
		m_nSearchId = messageDBStore->getMessages(m_searchType, m_searchId, m_nSearchCurPage, 60, m_searchBeginDate, m_searchEndDate, m_searchKeyword);
		qDebug() << Q_FUNC_INFO << m_searchType << m_searchId << m_nSearchCurPage << m_searchKeyword << m_searchBeginDate << m_searchEndDate;
	}
	else if (m_historyState == StateContext)
	{
		m_nSearchId = messageDBStore->getMessages(m_contextType, m_contextId, m_nContextCurPage);
		qDebug() << Q_FUNC_INFO << m_contextType << m_contextId << m_nContextCurPage;
	}
}

void MsgHistoryWidget::openMsgContext(const QString &msgId, const QString &strMsgType, const QString &otherId)
{
	bool ok = false;
	int nMsgId = msgId.toInt(&ok);
	if (!ok || nMsgId < 0)
	{
		qWarning() << Q_FUNC_INFO << "msg id is invalid: " << msgId;
		return;
	}

	bean::MessageType msgType = bean::Message_Invalid;
	if (strMsgType == bean::kszChat)
		msgType = bean::Message_Chat;
	else if (strMsgType == bean::kszGroup)
		msgType = bean::Message_GroupChat;
	else if (strMsgType == bean::kszDiscuss)
		msgType = bean::Message_DiscussChat;
	if (msgType == bean::Message_Invalid)
	{
		qWarning() << Q_FUNC_INFO << "msg type is invalid: " << strMsgType;
		return;
	}

	if (otherId.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "id is empty: " << otherId << strMsgType;
		return;
	}

	m_contextType = msgType;
	m_contextId = otherId;
	m_contextMsgId = msgId;
	m_nContextCurPage = 0;
	m_nContextMaxPage = 0;
	configContextLabel(m_contextType, m_contextId);
	switchHistoryState(StateContext);

	startOperation();
	ui->stackedWidget->setCurrentIndex(0);
	m_nSearchId = qPmApp->getMessageDBStore()->getContextMessages(m_contextType, m_contextId, nMsgId);
	qDebug() << Q_FUNC_INFO << m_contextType << m_contextId << nMsgId << m_nSearchId;

	emit messageIdTypeChanged(m_contextType, m_contextId);
}

void MsgHistoryWidget::contextMsgBack()
{
	// back to search page
	switchHistoryState(StateSearch);
	ui->stackedWidget->setCurrentIndex(2);
	configBottomBar(m_nSearchCurPage, m_nSearchMaxPage);
}

void MsgHistoryWidget::on_tBtnSearchBar_toggled(bool checked)
{
	ui->searchBar->setVisible(checked);

	ui->comboBoxDate->setCurrentIndex(4);
	ui->lineEditWord->clear();
}

void MsgHistoryWidget::on_tBtnSearch_clicked()
{
	QString text = ui->lineEditWord->text().trimmed();
	if (text.isEmpty())
	{
		QPoint p = ui->lineEditWord->mapToGlobal(QPoint(0, 0));
		PMessagePopup::information(tr("Please input key word"), p);
		return;
	}

	m_searchKeyword = text;

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
		m_searchBeginDate = "";
		m_searchEndDate = "";
	}
	else if (comboidx == SearchDatePeriod)
	{
		m_searchBeginDate = m_dateFrom.toString("yyyy-MM-dd 00:00:00");
		QDate tmpDate = m_dateTo.addDays(1);
		m_searchEndDate = tmpDate.toString("yyyy-MM-dd 00:00:00");
	}
	else
	{
		m_searchBeginDate = datetime.toString("yyyy-MM-dd 00:00:00");
		m_searchEndDate = "";
	}

	if (!m_searchBeginDate.isEmpty())
		m_searchBeginDate = CDateTime::utcDateTimeStringFromLocalDateTimeString(m_searchBeginDate);
	if (!m_searchEndDate.isEmpty())
		m_searchEndDate = CDateTime::utcDateTimeStringFromLocalDateTimeString(m_searchEndDate);

	m_searchId = m_qsId;
	m_searchType = m_eType;
	m_nSearchCurPage = 0;
	m_nSearchMaxPage = 0;
	switchHistoryState(StateSearch);

	changeCurPage();
}

void MsgHistoryWidget::onComboBoxDateActivated(int index)
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

void MsgHistoryWidget::on_tBtnSearchBack_clicked()
{
	m_nCurPage = 0;
	m_nMaxPage = 0;
	m_pageDirection = PagePrev;
	switchHistoryState(StateHistory);

	changeCurPage();

	emit messageIdTypeChanged(m_eType, m_qsId);
}

void MsgHistoryWidget::on_tBtnMsgManager_clicked()
{
	MessageManagerDlg *pDlg = MessageManagerDlg::instance();
	pDlg->init(m_qsId, m_eType);
	WidgetManager::showActivateRaiseWindow(pDlg);
}

void MsgHistoryWidget::on_lineEditWord_returnPressed()
{
	on_tBtnSearch_clicked();
}

void MsgHistoryWidget::onCopyImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString pathUrl = action->data().toString();
	if (pathUrl.isEmpty())
		return;

	if (m_historyState == StateSearch)
	{
		m_pSearchMessage4js->copyImage(pathUrl);
	}
	else
	{
		m_pMessage4js->copyImage(pathUrl);
	}
}

void MsgHistoryWidget::onSaveImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString pathUrl = action->data().toString();
	if (pathUrl.isEmpty())
		return;

	if (m_historyState == StateSearch)
	{
		m_pSearchMessage4js->saveImage(pathUrl);
	}
	else
	{
		m_pMessage4js->saveImage(pathUrl);
	}
}

void MsgHistoryWidget::onCopyMessage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString arg = action->data().toString();
	QStringList args = arg.split(":");
	if (args.count() != 2)
		return;

	int msgType = args[0].toInt();
	int msgId = args[1].toInt();
	m_nCopyId = qPmApp->getMessageDBStore()->getMessageByMsgId((bean::MessageType)msgType, msgId);
}

void MsgHistoryWidget::onRemoveOne()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString arg = action->data().toString();
	QStringList args = arg.split(":");
	if (args.count() != 2)
		return;

	int msgType = args[0].toInt();
	int msgId = args[1].toInt();
	slot_removeOneFromDB(msgId, msgType);
}

void MsgHistoryWidget::onRemoveMore()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString arg = action->data().toString();
	QStringList args = arg.split(":");
	if (args.count() != 2)
		return;

	int msgType = args[0].toInt();
	slot_removeMoreFromDB(msgType);
}

void MsgHistoryWidget::initUI()
{
	// page edit validator
	QRegExpValidator *pValidator = new QRegExpValidator(this);
	pValidator->setRegExp(QRegExp("[0-9]+"));
	ui->leditPage->setValidator(pValidator);

	// key word
	ui->lineEditWord->setPlaceholderText(tr("Key Word"));

	// web view related
	QString tag = QString("historymsg_webview");
	ui->msgBrowser->setTag(tag);
	ui->msgBrowser->setImageDragDelegate(this);
	ui->msgBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->msgBrowser->setMenuDelegate(this);
	ui->msgBrowser->setUrl(QUrl("qrc:/html/historymsglist.html"));

	m_pMessage4js = ui->msgBrowser->message4Js();
	m_pMessage4js->setEnableOpenContextMsg(false);
	connect(this, SIGNAL(sendSecretMessageRead(QString)), m_pMessage4js, SIGNAL(setSendSecretRead(QString)));
	connect(this, SIGNAL(recvSecretMessageRead(QString)), m_pMessage4js, SIGNAL(onRecvSecretRead(QString)));
	connect(ui->msgBrowser, SIGNAL(loadFinished()), this, SLOT(onMsgBrowserLoadFinished()));

	// load page
	QMovie *movie = new QMovie(":/images/loading_image_small.gif");
	movie->setParent(this);
	ui->labLoading->setMovie(movie);
	movie->start();

	ui->contextInfoLine->setVisible(false);
	connect(ui->tBtnContextBack, SIGNAL(clicked()), this, SLOT(contextMsgBack()));

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

	// search page
	tag = QString("historymsg_search_webview");
	ui->searchBrowser->setTag(tag);
	ui->searchBrowser->setImageDragDelegate(this);
	ui->searchBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->searchBrowser->setMenuDelegate(this);
	ui->searchBrowser->setUrl(QUrl("qrc:/html/historymsglist.html"));

	m_pSearchMessage4js = ui->searchBrowser->message4Js();
	m_pSearchMessage4js->setEnableOpenContextMsg(true);
	connect(m_pSearchMessage4js, SIGNAL(openMsgContext(QString, QString, QString)), SLOT(openMsgContext(QString, QString, QString)));

	ui->tBtnSearchBar->setToolTip(tr("Open Search Bar"));
	ui->tBtnMsgManager->setToolTip(tr("Open Message Manager"));

	MessageDBStore *messageDBStore = qPmApp->getMessageDBStore();
	connect(messageDBStore, SIGNAL(gotMessages(qint64, int, int, bean::MessageBodyList, int)), 
		this, SLOT(onGotMessages(qint64, int, int, bean::MessageBodyList, int)));
	connect(messageDBStore, SIGNAL(gotContextMessages(qint64, int, int, bean::MessageBodyList, int)),
		this, SLOT(onGotContextMessages(qint64, int, int, bean::MessageBodyList, int)));
	connect(messageDBStore, SIGNAL(removed(qint64)), this, SLOT(onMsgRemoved(qint64)));
	connect(messageDBStore, SIGNAL(gotMessageOfMsgId(qint64, int, bean::MessageBody)),
		this, SLOT(onGotMessageOfMsgId(qint64, int, bean::MessageBody)));

	m_imageCopyAction = new QAction(tr("Copy"), this);
	connect(m_imageCopyAction, SIGNAL(triggered()), this, SLOT(onCopyImage()));

	m_imageSaveAction = new QAction(tr("Save..."), this);
	connect(m_imageSaveAction, SIGNAL(triggered()), this, SLOT(onSaveImage()));

	m_messageCopyAction = new QAction(tr("Copy"), this);
	connect(m_messageCopyAction, SIGNAL(triggered()), this, SLOT(onCopyMessage()));

	m_removeOneAction = new QAction(tr("Remove One"), this);
	connect(m_removeOneAction, SIGNAL(triggered()), this, SLOT(onRemoveOne()));

	m_removeMoreAction = new QAction(tr("Remove More"), this);
	connect(m_removeMoreAction, SIGNAL(triggered()), this, SLOT(onRemoveMore()));
}

void MsgHistoryWidget::checkSecretState(const bean::MessageBodyList &msgs)
{
	// check secret state
	foreach (bean::MessageBody msg, msgs)
	{
		if (msg.messageType() == bean::Message_Chat && msg.ext().type() == bean::MessageExt_Secret && msg.readState() == 0)
		{
			if (msg.isSend())
			{
				qPmApp->getBuddyMgr()->checkSendSecretMessageDestroy(msg.stamp(), msg.to());
			}
			else
			{
				qPmApp->getBuddyMgr()->checkRecvSecretMessageDestory(msg.stamp(), msg.to());
			}
		}
	}
}

void MsgHistoryWidget::switchHistoryState(HistoryState state)
{
	if (m_historyState != state)
	{
		m_historyState = state;

		if (m_historyState == StateHistory)
		{
			ui->contextInfoLine->setVisible(m_titleVisibility);
			ui->labelContextTip->setVisible(m_titleVisibility);
			ui->tBtnContextBack->setVisible(false);
			configContextLabel(m_eType, m_qsId);
		
			// clear search & context
			m_searchId.clear();
			m_searchType = bean::Message_Invalid;
			m_searchBeginDate.clear();
			m_searchEndDate.clear();
			m_searchKeyword.clear();
			m_nSearchMaxPage = 0;
			m_nSearchCurPage = 0;
			m_dateTo = CDateTime::currentDateTime().date();
			m_dateFrom = m_dateTo.addDays(-7);

			m_contextMsgId.clear();
			m_contextId.clear();
			m_contextType = bean::Message_Invalid;
			m_nContextMaxPage = 0;
			m_nContextCurPage = 0;
		}
		else if (m_historyState == StateSearch)
		{
			// clear context
			m_contextMsgId.clear();
			m_contextId.clear();
			m_contextType = bean::Message_Invalid;
			m_nContextMaxPage = 0;
			m_nContextCurPage = 0;
		}
		else if (m_historyState == StateContext)
		{
			ui->contextInfoLine->setVisible(true);
			ui->labelContextTip->setVisible(m_titleVisibility);
			ui->tBtnContextBack->setVisible(true);
			configContextLabel(m_contextType, m_contextId);
		}
	}
}

void MsgHistoryWidget::resetHistory()
{
	m_pageDirection = PagePrev;
	m_nSearchId = -1;
	m_nDeleteId = -1;
	
	m_qsId.clear();
	m_eType = bean::Message_Invalid;
	m_nMaxPage = 0;
	m_nCurPage = 0;
	
	m_searchId.clear();
	m_searchType = bean::Message_Invalid;
	m_searchBeginDate.clear();
	m_searchEndDate.clear();
	m_searchKeyword.clear();
	m_nSearchMaxPage = 0;
	m_nSearchCurPage = 0;
	m_dateTo = CDateTime::currentDateTime().date();
	m_dateFrom = m_dateTo.addDays(-7);

	m_contextMsgId.clear();
	m_contextId.clear();
	m_contextType = bean::Message_Invalid;
	m_nContextMaxPage = 0;
	m_nContextCurPage = 0;
}

void MsgHistoryWidget::configBottomBar(int curPage, int maxPage)
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

void MsgHistoryWidget::configContextLabel(bean::MessageType msgType, const QString &id)
{
	if (msgType == bean::Message_Invalid || id.isEmpty())
	{
		ui->labelContextTip->clear();
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	if (msgType == bean::Message_Chat)
	{
		QString name = modelManager->userName(id);
		if (!modelManager->hasUserItem(id))
		{
			name = tr("Invalid contact");
		}
		QString title;
		if (!name.isEmpty())
		{
			title = tr("%1's chat history").arg(name);
		}
		ui->labelContextTip->setText(title);
	}
	else if (msgType == bean::Message_GroupChat)
	{
		QString name = modelManager->groupName(id);
		QString title;
		if (!name.isEmpty())
		{
			title = tr("%1's chat history").arg(name);
		}
		else
		{
			title = tr("Quited group's chat history");
		}
		ui->labelContextTip->setText(title);
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		QString name = modelManager->discussName(id);
		QString title;
		if (!name.isEmpty())
		{
			title = tr("%1's chat history").arg(name);
		}
		else
		{
			title = tr("Quited discuss' chat history");
		}
		ui->labelContextTip->setText(title);
	}
}

void MsgHistoryWidget::startOperation()
{
	ui->pBtnBegin->setEnabled(false);
	ui->pBtnPrevious->setEnabled(false);
	ui->pBtnNext->setEnabled(false);
	ui->pBtnEnd->setEnabled(false);
	ui->leditPage->setEnabled(false);
	ui->tBtnSearchBar->setEnabled(false);
	ui->comboBoxDate->setEnabled(false);
	ui->lineEditWord->setEnabled(false);
	ui->tBtnSearch->setEnabled(false);
	ui->tBtnSearchBack->setEnabled(false);
	ui->tBtnContextBack->setEnabled(false);
}

void MsgHistoryWidget::endOperation()
{
	ui->pBtnBegin->setEnabled(true);
	ui->pBtnPrevious->setEnabled(true);
	ui->pBtnNext->setEnabled(true);
	ui->pBtnEnd->setEnabled(true);
	ui->leditPage->setEnabled(true);
	ui->tBtnSearchBar->setEnabled(true);
	ui->comboBoxDate->setEnabled(true);
	ui->lineEditWord->setEnabled(true);
	ui->tBtnSearch->setEnabled(true);
	ui->tBtnSearchBack->setEnabled(true);
	ui->tBtnContextBack->setEnabled(true);
}

void MsgHistoryWidget::doMessageCopy(const bean::MessageBody &msg)
{
	if (!msg.isValid())
		return;

	QList<bean::AttachItem> attachs = msg.attachs();
	if (!attachs.isEmpty())
	{
		bean::AttachItem attach = attachs[0];
		if (attach.transferResult() != bean::AttachItem::Transfer_Successful)
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Attach was not transfer OK, can't copy"));
			return;
		}

		QString filePath = attach.filepath();
		if (!QFile::exists(filePath))
		{
			PMessageBox::warning(this->window(), tr("Tip"), tr("Attach was deleted or moved to other place, can't copy"));
			return;
		}
	}

	QString title;
	QString fromName = msg.fromName();
	QString toName = msg.toName();
	if (msg.ext().type() == bean::MessageExt_Tip ||
		msg.ext().type() == bean::MessageExt_Interphone ||
		msg.ext().type() == bean::MessageExt_Session)
	{
		title.append(tr("Tip"));
	}
	else if (msg.messageType() == bean::Message_Chat)
	{
		if (msg.isSend())
			title.append(fromName);
		else
			title.append(toName);
	}
	else
	{
		title.append(fromName);
	}
	title.append(" ");
	title.append(msg.time());

	QString attachPath;
	QString dirPath;
	QString imagePath;
	QString msgText = msg.body();
	if (!attachs.isEmpty())
	{
		msgText.clear();

		bean::AttachItem attach = attachs[0];
		if (attach.transferType() == bean::AttachItem::Type_AutoDisplay)
		{
			imagePath = attach.filepath();
		}
		else if (attach.transferType() == bean::AttachItem::Type_Default)
		{
			attachPath = attach.filepath();
		}
		else if (attach.transferType() == bean::AttachItem::Type_Dir)
		{
			dirPath = attach.filepath();
		}
	}

	QMimeData *mimeData = CChatInputBox::msgToCopyMimeData(title, msgText, imagePath, attachPath, dirPath);
	if (mimeData)
	{
		QApplication::clipboard()->setMimeData(mimeData);
	}
}
