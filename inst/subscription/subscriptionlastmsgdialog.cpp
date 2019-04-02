#include "subscriptionlastmsgdialog.h"
#include "ui_subscriptionlastmsgdialog.h"
#include "subscriptionmodel.h"
#include "subscriptionlastmsgmodel.h"
#include "subscriptionlastmsgitemdelegate.h"
#include "lastcontactmodeldef.h"
#include "ModelManager.h"
#include "PmApp.h"
#include <QMenu>
#include <QAction>
#include "Constants.h"

QPointer<SubscriptionLastMsgDialog> SubscriptionLastMsgDialog::s_dialog;

SubscriptionLastMsgDialog::SubscriptionLastMsgDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::SubscriptionLastMsgDialog();
	ui->setupUi(this);

	setWindowTitle(ui->title->text());

	setAttribute(Qt::WA_DeleteOnClose, true);
	setMainLayout(ui->verticalLayoutMain);

	initUI();

	setFixedSize(QSize(482, 562));
	setResizeable(false);
	setMaximizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	setSkin();
}

SubscriptionLastMsgDialog::~SubscriptionLastMsgDialog()
{
	delete ui;
}

SubscriptionLastMsgDialog *SubscriptionLastMsgDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new SubscriptionLastMsgDialog();
	}
	return s_dialog.data();
}

bool SubscriptionLastMsgDialog::hasDialog()
{
	return !s_dialog.isNull();
}

void SubscriptionLastMsgDialog::setSkin()
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

	// set empty label style
	ui->labelEmpty->setStyleSheet("QLabel {color: rgb(128, 128, 128);}");

	// set table view
	ui->listView->setStyleSheet("QListView#listView {"
		"background: transparent;" 
		"border: none;"
		"}"
	);
}

void SubscriptionLastMsgDialog::onItemDoubleClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QStandardItem *item = m_listModel->itemFromIndex(index);
	if (!item)
		return;

	SubscriptionLastMsgModel::LastMsgItem lastMsg = SubscriptionLastMsgModel::modelItem2LastMsg(*item);
	emit openSubscriptionMsg(lastMsg.subId);
}

void SubscriptionLastMsgDialog::onSubscriptionLogoChanged(const QString &subscriptionId)
{
	Q_UNUSED(subscriptionId);
	ui->listView->update();
}

void SubscriptionLastMsgDialog::onMsgAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	emit openSubscriptionMsg(subscriptionId);
}

void SubscriptionLastMsgDialog::onDetailAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	emit openSubscriptionDetail(subscriptionId);
}

void SubscriptionLastMsgDialog::onDeleteAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString subscriptionId = action->data().toString();
	if (subscriptionId.isEmpty())
		return;

	m_listModel->removeLastMsg(subscriptionId);
}

void SubscriptionLastMsgDialog::contextMenu(const QPoint &position)
{
	QModelIndex index = ui->listView->indexAt(position);
	if (index.isValid())
	{
		QStandardItem *item = m_listModel->itemFromIndex(index);
		if (!item)
			return;

		SubscriptionLastMsgModel::LastMsgItem lastMsg = SubscriptionLastMsgModel::modelItem2LastMsg(*item);
		
		QString subscriptionId = lastMsg.subId;
		m_msgAction->setData(subscriptionId);
		m_detailAction->setData(subscriptionId);
		m_deleteAction->setData(subscriptionId);

		// show menu
		QMenu menu(this);
		menu.addAction(m_msgAction);
		menu.addAction(m_detailAction);
		menu.addAction(m_deleteAction);
		menu.exec(QCursor::pos());
	}
}

void SubscriptionLastMsgDialog::onSubscriptionLastMsgChanged()
{
	ui->listView->update();

	if (m_listModel->rowCount() > 0)
	{
		ui->stackedWidget->setCurrentIndex(0);
	}
	else
	{
		// remove from last contact model
		qPmApp->getModelManager()->lastContactModel()->onRemoveChat(QString(SUBSCRIPTION_ROSTER_ID));

		// close this dialog
		ui->stackedWidget->setCurrentIndex(1);
		close();
	}
}

void SubscriptionLastMsgDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	m_listModel = modelManager->subscriptionLastMsgModel();
	
	// list view
	ui->listView->setModel(m_listModel);
	SubscriptionLastMsgItemDelegate *itemDelegate = new SubscriptionLastMsgItemDelegate(m_listModel.data(), ui->listView);
	ui->listView->setItemDelegate(itemDelegate);
	ui->listView->setMouseTracking(true);
	ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->listView->setSelectionMode(QListView::NoSelection);
	ui->listView->setEditTriggers(QListView::NoEditTriggers);
	ui->listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	connect(m_listModel, SIGNAL(subscriptionLastMsgChanged()), this, SLOT(onSubscriptionLastMsgChanged()));

	if (m_listModel->rowCount() > 0)
		ui->stackedWidget->setCurrentIndex(0);
	else
		ui->stackedWidget->setCurrentIndex(1);

	connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
	connect(ui->listView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(const QPoint&)));
	connect(modelManager->subscriptionModel(), SIGNAL(subscriptionLogoChanged(QString)), this, SLOT(onSubscriptionLogoChanged(QString)));
	
	m_msgAction = new QAction(tr("Enter Subscription"), this);
	connect(m_msgAction, SIGNAL(triggered()), this, SLOT(onMsgAction()));

	m_detailAction = new QAction(tr("Detail"), this);
	connect(m_detailAction, SIGNAL(triggered()), this, SLOT(onDetailAction()));

	m_deleteAction = new QAction(tr("Delete Chat"), this);
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(onDeleteAction()));
}

