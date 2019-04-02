#include "subscriptiondialog.h"
#include "ui_subscriptiondialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "subscriptionmodel.h"
#include "subscriptionitemdelegate.h"
#include <QAction>
#include "subscriptionmanager.h"
#include "pmessagebox.h"
#include "loginmgr.h"
#include "Account.h"
#include "common/datetime.h"
#include <QMenu>
#include <QStringList>

QPointer<SubscriptionDialog> SubscriptionDialog::s_dialog;

SubscriptionDialog::SubscriptionDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::SubscriptionDialog();
	ui->setupUi(this);

	setWindowTitle(ui->title->text());

	setAttribute(Qt::WA_DeleteOnClose, true);
	setMainLayout(ui->verticalLayoutMain);

	initUI();

	setFixedSize(QSize(552, 562));
	setResizeable(false);
	setMaximizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	setSkin();
}

SubscriptionDialog::~SubscriptionDialog()
{
	delete ui;
}

SubscriptionDialog *SubscriptionDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new SubscriptionDialog();
	}
	return s_dialog.data();
}

void SubscriptionDialog::setCurrent(const QString &subscriptionId)
{
	QStandardItem *subscriptionItem = m_subscriptionModel.data()->subscriptionItem(subscriptionId);
	if (subscriptionItem)
	{
		QModelIndex sourceIndex = subscriptionItem->index();
		ui->tableView->scrollTo(sourceIndex, QAbstractItemView::PositionAtTop);

		ui->tableView->selectionModel()->select(sourceIndex, QItemSelectionModel::ClearAndSelect);
	}
}

void SubscriptionDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 0;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {color: white; font-size: 12pt;}");

	// set table view
	ui->tableView->setStyleSheet("QTableView#tableView {"
		"background: transparent;" 
		"border: none;"
		"}"
	);

	// set label
	ui->pageEmpty->setStyleSheet("QLabel#labelText1, QLabel#labelText2, QLabel#labelText3 { color: rgb(128, 128, 128); font: 12pt; }");
}

void SubscriptionDialog::filterChanged(const QString &filter)
{
	m_subscriptionModel.data()->setFilterString(filter);
}

void SubscriptionDialog::subscriptionDoubleClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QStandardItem *item = m_subscriptionModel.data()->itemFromIndex(index);
	if (!item)
		return;

	SubscriptionDetail subscription = SubscriptionModel::modelItem2Subscription(*item);
	emit openSubscriptionMsg(subscription.id());
}

void SubscriptionDialog::onSubscriptionLogoChanged(const QString &subscriptionId)
{
	Q_UNUSED(subscriptionId);
	ui->tableView->update();
}

void SubscriptionDialog::onSubscriptionDataChanged()
{
	if (!m_subscriptionModel.isNull())
	{
		int count = m_subscriptionModel.data()->allSubscriptionIds().count();
		if (count > 0)
		{
			ui->stackedWidget->setCurrentIndex(0);
		}
		else
		{
			ui->stackedWidget->setCurrentIndex(1);
		}
	}
}

void SubscriptionDialog::onMsgAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	emit openSubscriptionMsg(subscriptionId);
}

void SubscriptionDialog::onDetailAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	emit openSubscriptionDetail(subscriptionId);
}

void SubscriptionDialog::onHistoryAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	emit openSubscriptionHistory(subscriptionId);
}

void SubscriptionDialog::onUnsubscribeAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, please try when online"));
		return;
	}

	SubscriptionDetail subscription = m_subscriptionModel.data()->subscription(subscriptionId);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Unfollow"), 
		tr("Do you want to unfollow %1").arg(subscription.name()), QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret == QDialogButtonBox::Yes)
	{
		SubscriptionManager *subscriptionManager = qPmApp->getSubscriptionManager();
		subscriptionManager->unsubscribe(subscriptionId, Account::instance()->id(), CDateTime::currentDateTimeUtcString());
	}
}

void SubscriptionDialog::contextMenu(const QPoint &position)
{
	QModelIndex index = ui->tableView->indexAt(position);
	if (index.isValid())
	{
		QStandardItem *item = m_subscriptionModel.data()->itemFromIndex(index);
		if (!item)
			return;

		SubscriptionDetail subscription = SubscriptionModel::modelItem2Subscription(*item);
		if (!subscription.isValid())
			return;
		
		QString subscriptionId = subscription.id();
		m_msgAction->setData(subscriptionId);
		m_detailAction->setData(subscriptionId);
		m_historyAction->setData(subscriptionId);
		m_unsubscribeAction->setData(subscriptionId);

		// show menu
		QMenu menu(this);
		menu.addAction(m_msgAction);
		menu.addAction(m_detailAction);
		menu.addAction(m_historyAction);
		if (!subscription.special())
			menu.addAction(m_unsubscribeAction);
		menu.exec(QCursor::pos());
	}
}

void SubscriptionDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	m_subscriptionModel = modelManager->subscriptionModel();
	if (m_subscriptionModel.data()->allSubscriptionIds().isEmpty())
	{
		ui->stackedWidget->setCurrentIndex(1);
	}
	else
	{
		ui->stackedWidget->setCurrentIndex(0);
	}

	// empty page
	ui->labelSearch->setFontAtt(QColor(0, 120, 216), 12, false);
	connect(ui->labelSearch, SIGNAL(clicked()), this, SIGNAL(searchSubscription()));

	// table view
	m_subscriptionModel.data()->setFilterString(QString());
	ui->tableView->setModel(m_subscriptionModel.data());
	SubscriptionItemDelegate *itemDelegate = new SubscriptionItemDelegate(m_subscriptionModel.data(), ui->tableView);
	ui->tableView->setItemDelegate(itemDelegate);
	ui->tableView->setMouseTracking(true);
	ui->tableView->setShowGrid(false);
	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->tableView->setSelectionMode(QTableView::NoSelection);
	ui->tableView->setEditTriggers(QTableView::NoEditTriggers);
	ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui->tableView->horizontalHeader()->setVisible(false);
	ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->resizeColumnsToContents();
	ui->tableView->setFocus();

	connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(subscriptionDoubleClicked(QModelIndex)));
	connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(const QPoint&)));
	connect(m_subscriptionModel.data(), SIGNAL(subscriptionLogoChanged(QString)), this, SLOT(onSubscriptionLogoChanged(QString)));
	connect(m_subscriptionModel.data(), SIGNAL(subscriptionDataChanged()), this, SLOT(onSubscriptionDataChanged()));

	m_msgAction = new QAction(tr("Enter Subscription"), this);
	connect(m_msgAction, SIGNAL(triggered()), this, SLOT(onMsgAction()));

	m_detailAction = new QAction(tr("Detail"), this);
	connect(m_detailAction, SIGNAL(triggered()), this, SLOT(onDetailAction()));

	m_historyAction = new QAction(tr("View Chat History"), this);
	connect(m_historyAction, SIGNAL(triggered()), this, SLOT(onHistoryAction()));

	m_unsubscribeAction = new QAction(tr("Unfollow"), this);
	connect(m_unsubscribeAction, SIGNAL(triggered()), this, SLOT(onUnsubscribeAction()));
}

