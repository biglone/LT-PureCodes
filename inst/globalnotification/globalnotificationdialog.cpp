#include "globalnotificationdialog.h"
#include "ui_globalnotificationdialog.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "globalnotificationmodel.h"
#include "globalnotificationitemdelegate.h"
#include <QAction>
#include "globalnotificationmanager.h"
#include "pmessagebox.h"
#include "loginmgr.h"
#include "Account.h"
#include "common/datetime.h"
#include <QMenu>
#include <QStringList>

QPointer<GlobalNotificationDialog> GlobalNotificationDialog::s_dialog;

GlobalNotificationDialog::GlobalNotificationDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::GlobalNotificationDialog();
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

GlobalNotificationDialog::~GlobalNotificationDialog()
{
	delete ui;
}

GlobalNotificationDialog *GlobalNotificationDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new GlobalNotificationDialog();
	}
	return s_dialog.data();
}

void GlobalNotificationDialog::setCurrent(const QString &globalNotificationId)
{
	QStandardItem *globalNotificationItem = m_globalNotificationModel.data()->globalNotificationItem(globalNotificationId);
	if (globalNotificationItem)
	{
		QModelIndex sourceIndex = globalNotificationItem->index();
		ui->tableView->scrollTo(sourceIndex, QAbstractItemView::PositionAtTop);

		ui->tableView->selectionModel()->select(sourceIndex, QItemSelectionModel::ClearAndSelect);
	}
}

void GlobalNotificationDialog::setSkin()
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

void GlobalNotificationDialog::filterChanged(const QString &filter)
{
	m_globalNotificationModel.data()->setFilterString(filter);
}

void GlobalNotificationDialog::globalNotificationDoubleClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QStandardItem *item = m_globalNotificationModel.data()->itemFromIndex(index);
	if (!item)
		return;

	GlobalNotificationDetail globalNotification = GlobalNotificationModel::modelItem2GlobalNotification(*item);
	emit openGlobalNotificationMsg(globalNotification.id());
}

void GlobalNotificationDialog::onGlobalNotificationLogoChanged(const QString &globalNotificationId)
{
	Q_UNUSED(globalNotificationId);
	ui->tableView->update();
}

void GlobalNotificationDialog::onGlobalNotificationDataChanged()
{
	if (!m_globalNotificationModel.isNull())
	{
		int count = m_globalNotificationModel.data()->allGlobalNotificationIds().count();
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

void GlobalNotificationDialog::onMsgAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	emit openGlobalNotificationMsg(globalNotificationId);
}

void GlobalNotificationDialog::onDetailAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	emit openGlobalNotificationDetail(globalNotificationId);
}

void GlobalNotificationDialog::onHistoryAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	emit openGlobalNotificationHistory(globalNotificationId);
}

void GlobalNotificationDialog::onUnsubscribeAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are offline, please try when online"));
		return;
	}

	GlobalNotificationDetail subscription = m_globalNotificationModel.data()->globalNotification(globalNotificationId);
	QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Unfollow"), 
		tr("Do you want to unfollow %1").arg(subscription.name()), QDialogButtonBox::Yes|QDialogButtonBox::No);
	if (ret == QDialogButtonBox::Yes)
	{
		GlobalNotificationManager *globalNotificationManager = qPmApp->getGlobalNotificationManager();
		globalNotificationManager->unsubscribe(globalNotificationId, Account::instance()->id(), CDateTime::currentDateTimeUtcString());
	}
}

void GlobalNotificationDialog::contextMenu(const QPoint &position)
{
	QModelIndex index = ui->tableView->indexAt(position);
	if (index.isValid())
	{
		QStandardItem *item = m_globalNotificationModel.data()->itemFromIndex(index);
		if (!item)
			return;

		GlobalNotificationDetail globalNotification = GlobalNotificationModel::modelItem2GlobalNotification(*item);
		if (!globalNotification.isValid())
			return;
		
		QString globalNotificationId = globalNotification.id();
		m_msgAction->setData(globalNotificationId);
		m_detailAction->setData(globalNotificationId);
		m_historyAction->setData(globalNotificationId);
		m_unsubscribeAction->setData(globalNotificationId);

		// show menu
		QMenu menu(this);
		menu.addAction(m_msgAction);
		menu.addAction(m_detailAction);
		menu.addAction(m_historyAction);
		if (!globalNotification.special())
			menu.addAction(m_unsubscribeAction);
		menu.exec(QCursor::pos());
	}
}

void GlobalNotificationDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	m_globalNotificationModel = modelManager->globalNotificationModel();
	if (m_globalNotificationModel.data()->allGlobalNotificationIds().isEmpty())
	{
		ui->stackedWidget->setCurrentIndex(1);
	}
	else
	{
		ui->stackedWidget->setCurrentIndex(0);
	}

	// empty page
	ui->labelSearch->setFontAtt(QColor(0, 120, 216), 12, false);
	connect(ui->labelSearch, SIGNAL(clicked()), this, SIGNAL(searchGlobalNotification()));

	// table view
	m_globalNotificationModel.data()->setFilterString(QString());
	ui->tableView->setModel(m_globalNotificationModel.data());
	GlobalNotificationItemDelegate *itemDelegate = new GlobalNotificationItemDelegate(m_globalNotificationModel.data(), ui->tableView);
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

	connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(globalNotificationDoubleClicked(QModelIndex)));
	connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(const QPoint&)));
	connect(m_globalNotificationModel.data(), SIGNAL(globalNotificationLogoChanged(QString)), this, SLOT(onGlobalNotificationLogoChanged(QString)));
	connect(m_globalNotificationModel.data(), SIGNAL(globalNotificationDataChanged()), this, SLOT(onGlobalNotificationDataChanged()));

	m_msgAction = new QAction(tr("Enter GlobalNotification"), this);
	connect(m_msgAction, SIGNAL(triggered()), this, SLOT(onMsgAction()));

	m_detailAction = new QAction(tr("Detail"), this);
	connect(m_detailAction, SIGNAL(triggered()), this, SLOT(onDetailAction()));

	m_historyAction = new QAction(tr("View Chat History"), this);
	connect(m_historyAction, SIGNAL(triggered()), this, SLOT(onHistoryAction()));

	m_unsubscribeAction = new QAction(tr("Unfollow"), this);
	connect(m_unsubscribeAction, SIGNAL(triggered()), this, SLOT(onUnsubscribeAction()));
}

