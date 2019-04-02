#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "PmApp.h"
#include <QMovie>
#include "pmessagebox.h"
#include "searchitemwidget.h"
#include "ModelManager.h"
#include "widgetmanager.h"
#include "subscriptionmanager.h"
#include "subscriptionitemwidget.h"
#include "Account.h"
#include "common/datetime.h"
#include "settings/GlobalSettings.h"
#include "model/orgstructitemdef.h"

static const int kPageRow   = 2;
static const int kPageCol   = 3;
static const int kPageCount = kPageRow*kPageCol;

SearchDialog *SearchDialog::s_instance = 0;

SearchDialog *SearchDialog::getInstance()
{
	if (!s_instance)
		s_instance = new SearchDialog();
	return s_instance;
}

SearchDialog::SearchDialog(QWidget *parent)
	: FramelessDialog(parent), m_index(1), m_pages(0), m_count(0), m_tableItemHeight(116)
{
	ui = new Ui::SearchDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(662, 582);
	setFixedSize(662, 582);
	setResizeable(false);

	initUI();

	initSignals();

	setSkin();

	s_instance = this;
}

SearchDialog::~SearchDialog()
{
	s_instance = 0;

	delete ui;
}

void SearchDialog::setSearchType(SearchType type)
{
	if (type == SearchPeople)
	{
		ui->tabSearchPeople->setChecked(true);
		ui->tabSearchSubscription->setChecked(false);
		ui->searchStackedWidget->setCurrentIndex(0);
	}
	else if (type == SearchSubscription)
	{
		ui->tabSearchPeople->setChecked(false);
		ui->tabSearchSubscription->setChecked(true);
		ui->searchStackedWidget->setCurrentIndex(1);
	}
}

void SearchDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	ui->frame->setStyleSheet("border: none; border-top: 1px solid rgb(219, 219, 219);");

	ui->searchTabWidget->setStyleSheet("border: none; background-color: rgb(0, 120, 216);");

	QString tabStyleSheet = QString("QPushButton{"
		"border-top: 0px;"
		"border-left: 2px;"
		"border-right: 2px;"
		"border-bottom: 0px;"
		"font-size: 13pt;"
		"color: white;"
		"border-image: url(:/theme/maintab/main_tab_normal.png);"
		"}"
		"QPushButton:hover{"
		"border-image: url(:/theme/maintab/main_tab_normal.png);"
		"}"
		"QPushButton:checked{"
		"border-image: url(:/theme/maintab/main_tab_selected.png);"
		"}"
		);
	ui->tabSearchPeople->setStyleSheet(tabStyleSheet);
	ui->tabSearchSubscription->setStyleSheet(tabStyleSheet);

	ui->btnSearchPeople->setStyleSheet(QString("font-size: 13pt; color: white;"));
	ui->btnSearchSubscription->setStyleSheet(QString("font-size: 13pt; color: white;"));

	ui->tableWidget->setStyleSheet("QTableWidget#tableWidget {"
		"background: transparent;" 
		"border: none;"
		"}"
		"QTableWidget#tableWidget::item {"
		"background: transparent;" 
		"border: none;"
		"}"
		);
	
	// bottom bar button style
	QFile qssFile;
	QStringList fileNames;
	fileNames << ":/theme/qss/lineedit2_skin.qss" << ":/theme/qss/combobox2_skin.qss";
	QString dlgQss;
	foreach (QString fileName, fileNames)
	{
		qssFile.setFileName(fileName);
		if (qssFile.open(QIODevice::ReadOnly))
		{
			QString qss = qssFile.readAll();
			dlgQss += qss;
			qssFile.close();
		}
	}
	setStyleSheet(dlgQss);
}

void SearchDialog::onSearchPeopleFinished(bool ok)
{
	ui->btnSearchPeople->setEnabled(true);
	ui->btnSearchSubscription->setEnabled(true);

	if (!ok)
	{
		QString errorDesc = qPmApp->getSearchManager()->lastError();
		QString errorTip = tr("Search failed (%1), please try again").arg(errorDesc);
		ui->labelTip->setText(errorTip);
		ui->labelLoading->setVisible(false);
		return;
	}

	SearchManager::SearchResult *searchResult = qPmApp->getSearchManager()->searchResult();
	if (searchResult->m_items.count() > 0)
	{
		ui->resultStackedWidget->setCurrentIndex(0);
	}
	else
	{
		ui->resultStackedWidget->setCurrentIndex(3);
	}

	m_count = searchResult->m_count;
	m_index = searchResult->m_currentPage;
	m_pages = m_count/kPageCount;
	if ((m_count%kPageCount) != 0)
	{
		m_pages += 1;
	}
	setSearchPeopleResult(searchResult);

	setPageInfo();
}

void SearchDialog::onSearchSubscriptionFinished(bool ok, int currentPage, int /*pageSize*/, int rowCount, 
												int /*totalPage*/, const QList<SubscriptionDetail> &subscriptions)
{
	ui->btnSearchPeople->setEnabled(true);
	ui->btnSearchSubscription->setEnabled(true);

	if (!ok)
	{
		QString errorTip = tr("Search failed, please try again");
		ui->labelTip->setText(errorTip);
		ui->labelLoading->setVisible(false);
		return;
	}

	if (rowCount > 0)
	{
		ui->resultStackedWidget->setCurrentIndex(0);
	}
	else
	{
		ui->resultStackedWidget->setCurrentIndex(3);
	}

	m_count = rowCount;
	m_index = currentPage;
	m_pages = m_count/kPageCount;
	if ((m_count%kPageCount) != 0)
	{
		m_pages += 1;
	}
	setSearchSubscriptionResult(subscriptions);

	setPageInfo();
}

void SearchDialog::on_tabSearchPeople_clicked()
{
	if (!ui->btnSearchPeople->isChecked())
	{
		ui->tabSearchPeople->setChecked(true);
		ui->tabSearchSubscription->setChecked(false);
		ui->searchStackedWidget->setCurrentIndex(0);
		ui->resultStackedWidget->setCurrentIndex(2);
		ui->lineEditName->clear();
	}
}

void SearchDialog::on_tabSearchSubscription_clicked()
{
	if (!ui->btnSearchSubscription->isChecked())
	{
		ui->tabSearchPeople->setChecked(false);
		ui->tabSearchSubscription->setChecked(true);
		ui->searchStackedWidget->setCurrentIndex(1);
		ui->resultStackedWidget->setCurrentIndex(2);
		ui->lineEditSubscription->clear();
	}
}

void SearchDialog::on_btnSearchSubscription_clicked()
{
	QString keyword = ui->lineEditSubscription->text().trimmed();
	searchSubscription(keyword, 1, kPageCount);
}

void SearchDialog::on_btnSearchPeople_clicked()
{
	QString name = ui->lineEditName->text().trimmed();
	if (name.isEmpty())
	{
		PMessageBox::information(this, tr("Tip"), tr("Search can't be empty"));
		return;
	}

	searchPeople(name, 1, kPageCount);
}

void SearchDialog::onTableViewItemClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
}

void SearchDialog::on_pBtnBegin_clicked()
{
	m_index = 1;
	if (ui->searchStackedWidget->currentIndex() == 0) // search people
	{
		SearchManager::SearchResult *searchResult = qPmApp->getSearchManager()->searchResult();
		searchPeople(searchResult->m_searchName, m_index, kPageCount);
	}
	else if (ui->searchStackedWidget->currentIndex() == 1) // search subscription
	{
		searchSubscription(ui->labelSearchCondition->text(), m_index, kPageCount);
	}
}

void SearchDialog::on_pBtnPrevious_clicked()
{
	m_index--;
	if (m_index < 1)
	{
		m_index = 1;
	}

	if (ui->searchStackedWidget->currentIndex() == 0) // search people
	{
		SearchManager::SearchResult *searchResult = qPmApp->getSearchManager()->searchResult();
		searchPeople(searchResult->m_searchName, m_index, kPageCount);
	}
	else if (ui->searchStackedWidget->currentIndex() == 1) // search subscription
	{
		searchSubscription(ui->labelSearchCondition->text(), m_index, kPageCount);
	}
}

void SearchDialog::on_pBtnNext_clicked()
{
	m_index++;
	if (m_index > m_pages)
	{
		m_index = m_pages;
	}

	if (ui->searchStackedWidget->currentIndex() == 0) // search people
	{
		SearchManager::SearchResult *searchResult = qPmApp->getSearchManager()->searchResult();
		searchPeople(searchResult->m_searchName, m_index, kPageCount);
	}
	else if (ui->searchStackedWidget->currentIndex() == 1) // search subscription
	{
		searchSubscription(ui->labelSearchCondition->text(), m_index, kPageCount);
	}
}

void SearchDialog::on_pBtnEnd_clicked()
{
	m_index = m_pages;
	if (m_index < 1)
	{
		m_index = 1;
	}

	if (ui->searchStackedWidget->currentIndex() == 0) // search people
	{
		SearchManager::SearchResult *searchResult = qPmApp->getSearchManager()->searchResult();
		searchPeople(searchResult->m_searchName, m_index, kPageCount);
	}
	else if (ui->searchStackedWidget->currentIndex() == 1) // search subscription
	{
		searchSubscription(ui->labelSearchCondition->text(), m_index, kPageCount);
	}
}

void SearchDialog::onAddFriend(const QString &id)
{
	SearchManager::SearchResult *result = qPmApp->getSearchManager()->searchResult();
	foreach (SearchManager::SearchItem item, result->m_items)
	{
		if (id == item.m_id)
		{
			// send add request
			emit addFriendRequest(item.m_id, item.m_name);
			return;
		}
	}
}

void SearchDialog::subscribe(const SubscriptionDetail &subscription)
{
	if (!subscription.isValid())
		return;

	if (qPmApp->getModelManager()->hasSubscriptionItem(subscription.id()))
	{
		PMessageBox::information(this, tr("Tip"), tr("You have followed %1, can't follow again").arg(subscription.name()));
		return;
	}

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, please follow when online"));
		return;
	}

	QString selfId = Account::instance()->id();
	QString createTime = CDateTime::currentDateTimeUtcString();
	qPmApp->getSubscriptionManager()->subscribe(subscription.id(), selfId, createTime);

	m_subscribeMap.insert(subscription.id(), subscription);
}

void SearchDialog::onSubscribeFinished(bool ok, const QString &subscriptionId, const SubscriptionMsg &msg)
{
	if (!m_subscribeMap.contains(subscriptionId))
		return;

	SubscriptionDetail subscription = m_subscribeMap.take(subscriptionId);
	if (!ok)
	{
		PMessageBox::warning(this, tr("Tip"), tr("Follow %1 failed, please try again").arg(subscription.name()));
		return;
	}

	emit subscriptionSubscribed(subscription, msg);
	emit subscribeSucceed(subscriptionId);
	PMessageBox::information(this, tr("Tip"), tr("Congratulations, follow %1 successfully").arg(subscription.name()));
}

void SearchDialog::keyPressEvent(QKeyEvent *e)
{
	switch(e->key())
	{
	case Qt::Key_Escape:
		return;
	case Qt::Key_Return:
	case Qt::Key_Enter:
		{
			if (ui->searchStackedWidget->currentIndex() == 0)
			{
				if (ui->btnSearchPeople->isEnabled())
				{
					on_btnSearchPeople_clicked();
				}
			}
			else
			{
				if (ui->btnSearchSubscription->isEnabled())
				{
					on_btnSearchSubscription_clicked();
				}
			}
		}
		break;
	default:
		break;
	}

	return QWidget::keyPressEvent(e);
}

void SearchDialog::initUI()
{
	ui->tabSearchPeople->setCheckable(true);
	ui->tabSearchSubscription->setCheckable(true);
	ui->tabSearchPeople->setChecked(true);
	ui->searchStackedWidget->setCurrentIndex(0);

	// check if need show subscription tab
	if (GlobalSettings::subscriptionDisabled())
	{
		ui->tabSearchSubscription->setVisible(false);
	}
	else
	{
		ui->tabSearchSubscription->setVisible(true);
	}

	ui->resultStackedWidget->setCurrentIndex(2);

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);
	movie->start();
	ui->labelTip->setText(tr("Searching..."));

	ui->lineEditSubscription->setPlaceholderText(tr("Search directly or input key word to search"));
	ui->lineEditName->setPlaceholderText(tr("Input phone/name to search"));
	
	ui->tableWidget->setFocusPolicy(Qt::NoFocus);
	ui->tableWidget->setShowGrid(false);
	ui->tableWidget->setSelectionBehavior(QTableView::SelectItems);
	ui->tableWidget->setSelectionMode(QTableView::SingleSelection);
	ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
	ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableWidget->horizontalHeader()->setVisible(false);
	ui->tableWidget->verticalHeader()->setVisible(false);
}

void SearchDialog::initSignals()
{
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(qPmApp->getSearchManager(), SIGNAL(searchFinished(bool)), this, SLOT(onSearchPeopleFinished(bool)));
	connect(ui->tableWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onTableViewItemClicked(QModelIndex)));
	connect(qPmApp->getSubscriptionManager(), SIGNAL(searchSubscriptionFinished(bool, int, int, int, int, QList<SubscriptionDetail>)),
		this, SLOT(onSearchSubscriptionFinished(bool, int, int, int, int, QList<SubscriptionDetail>)));
	connect(qPmApp->getSubscriptionManager(), SIGNAL(subscribeFinished(bool, QString, SubscriptionMsg)),
		this, SLOT(onSubscribeFinished(bool, QString, SubscriptionMsg)));
}

void SearchDialog::setSearchText(const QString &keyword)
{
	ui->labelSearchCondition->setText(keyword);
}

void SearchDialog::setSearchPeopleResult(SearchManager::SearchResult *searchResult)
{
	if (!searchResult)
	{
		return;
	}

	ui->tableWidget->clear();
	ui->tableWidget->setRowCount(kPageRow);
	ui->tableWidget->setColumnCount(kPageCol);

	QList<SearchManager::SearchItem> tableItems;
	int i = 0;
	for (; i < searchResult->m_items.count(); i++)
	{
		tableItems.append(searchResult->m_items[i]);
	}

	int row = 0;
	int col = 0;
	ModelManager *modelManager = qPmApp->getModelManager();
	for (i = 0; i < tableItems.count(); i++)
	{
		SearchManager::SearchItem item = tableItems[i];

		// insert row
		if ((col%kPageCol) ==0)
		{
			ui->tableWidget->setRowHeight(row, m_tableItemHeight);
		}
		ui->tableWidget->setColumnWidth(col, ui->tableWidget->width()/ui->tableWidget->columnCount()-2);

		SearchItemWidget *widget = new SearchItemWidget(ui->tableWidget);
		QPixmap avatar = modelManager->getUserAvatar(item.m_id);
		QString name = item.m_name;
		QColor nameColor = QColor(0, 0, 0);
		if (item.m_userState == OrgStructContactItem::USER_STATE_INACTIVE)
		{
			name.append(tr("(unregistered)"));
			nameColor = QColor(136, 136, 136);
		}
		widget->setAvatar(avatar);
		widget->setId(item.m_id);
		// widget->setSex(item.m_sex);
		widget->setName(name, nameColor);
		widget->setDepart(item.m_depart);
		widget->setPhone(item.m_phone);
		if (modelManager->hasFriendItem(item.m_id) || Account::instance()->id() == item.m_id)
			widget->setAddEnabled(false);
		else
			widget->setAddEnabled(true);
		connect(widget, SIGNAL(addFriend(QString)), this, SLOT(onAddFriend(QString)));
		connect(widget, SIGNAL(showMaterial(QString)), this, SIGNAL(showMaterial(QString)));
		ui->tableWidget->setCellWidget(row, col, widget);

		++col;
		if ((col%kPageCol) == 0)
		{
			++row;
			col = 0;
		}
	}
}

void SearchDialog::setSearchSubscriptionResult(const QList<SubscriptionDetail> &subscriptions)
{
	if (subscriptions.isEmpty())
	{
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();

	ui->tableWidget->clear();
	ui->tableWidget->setRowCount(kPageRow);
	ui->tableWidget->setColumnCount(kPageCol);

	int row = 0;
	int col = 0;
	for (int i = 0; i < subscriptions.count(); i++)
	{
		SubscriptionDetail subscription = subscriptions[i];

		// insert row
		if ((col%kPageCol) ==0)
		{
			ui->tableWidget->setRowHeight(row, m_tableItemHeight);
		}
		ui->tableWidget->setColumnWidth(col, ui->tableWidget->width()/ui->tableWidget->columnCount()-2);

		SubscriptionItemWidget *widget = new SubscriptionItemWidget(ui->tableWidget);
		widget->setNetworkAccessManager(&m_networkAccessManager);
		widget->setSubscription(subscription);
		if (modelManager->hasSubscriptionItem(subscription.id()))
			widget->setSubscribeEnabled(false);
		else
			widget->setSubscribeEnabled(true);
		connect(widget, SIGNAL(subscribe(SubscriptionDetail)), this, SLOT(subscribe(SubscriptionDetail)));
		connect(this, SIGNAL(subscribeSucceed(QString)), widget, SLOT(onSucribeSucceed(QString)));

		ui->tableWidget->setCellWidget(row, col, widget);

		++col;
		if ((col%kPageCol) == 0)
		{
			++row;
			col = 0;
		}
	}
}

void SearchDialog::setPageInfo()
{
	ui->labPageCount->setText(QString::number(m_pages));
	if (m_pages > 0)
	{
		ui->leditPage->setText(QString::number(m_index));
		ui->pBtnBegin->setEnabled(true);
		ui->pBtnEnd->setEnabled(true);
	}
	else
	{
		ui->leditPage->setText("0");
		ui->pBtnBegin->setEnabled(false);
		ui->pBtnEnd->setEnabled(false);
	}

	if (m_index <= 1)
	{
		ui->pBtnPrevious->setEnabled(false);
	}
	else
	{
		ui->pBtnPrevious->setEnabled(true);
	}

	if (m_index >= m_pages)
	{
		ui->pBtnNext->setEnabled(false);
	}
	else
	{
		ui->pBtnNext->setEnabled(true);
	}

	if (m_pages <= 1)
	{
		ui->pageBar->setVisible(false);
	}
	else
	{
		ui->pageBar->setVisible(true);
	}
}

void SearchDialog::searchPeople(const QString &name, int page, int size)
{
	if (name.isEmpty())
		return;

	qPmApp->getSearchManager()->conditionSearch(name, QString(), QString(), page, size);
	ui->resultStackedWidget->setCurrentIndex(1);
	ui->labelLoading->setVisible(true);
	ui->labelTip->setText(tr("Searching..."));
	ui->btnSearchPeople->setEnabled(false);
	ui->btnSearchSubscription->setEnabled(false);
	
	setSearchText(name);
}

void SearchDialog::searchSubscription(const QString &keyword, int page, int size)
{
	/*
	if (keyword.isEmpty())
		return;
	*/

	qPmApp->getSubscriptionManager()->searchSubscription(keyword, page, size);
	ui->resultStackedWidget->setCurrentIndex(1);
	ui->labelLoading->setVisible(true);
	ui->labelTip->setText(tr("Searching..."));
	ui->btnSearchPeople->setEnabled(false);
	ui->btnSearchSubscription->setEnabled(false);
	setSearchText(keyword);
}
