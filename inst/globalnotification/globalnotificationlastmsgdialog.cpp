#include "globalnotificationlastmsgdialog.h"
#include "ui_globalnotificationlastmsgdialog.h"
#include "globalnotificationmodel.h"
#include "globalnotificationlastmsgmodel.h"
#include "globalnotificationlastmsgitemdelegate.h"
#include "lastcontactmodeldef.h"
#include "ModelManager.h"
#include "PmApp.h"
#include <QMenu>
#include <QAction>
#include "Constants.h"

QPointer<GlobalNotificationLastMsgDialog> GlobalNotificationLastMsgDialog::s_dialog;

GlobalNotificationLastMsgDialog::GlobalNotificationLastMsgDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::GlobalNotificationLastMsgDialog();
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

GlobalNotificationLastMsgDialog::~GlobalNotificationLastMsgDialog()
{
	delete ui;
}

GlobalNotificationLastMsgDialog *GlobalNotificationLastMsgDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new GlobalNotificationLastMsgDialog();
	}
	return s_dialog.data();
}

bool GlobalNotificationLastMsgDialog::hasDialog()
{
	return !s_dialog.isNull();
}

void GlobalNotificationLastMsgDialog::setSkin()
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

void GlobalNotificationLastMsgDialog::onItemDoubleClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QStandardItem *item = m_listModel->itemFromIndex(index);
	if (!item)
		return;

	GlobalNotificationLastMsgModel::LastMsgItem lastMsg = GlobalNotificationLastMsgModel::modelItem2LastMsg(*item);
	emit openGlobalNotificationMsg(lastMsg.subId);
}

void GlobalNotificationLastMsgDialog::onGlobalNotificationLogoChanged(const QString &globalNotificationId)
{
	Q_UNUSED(globalNotificationId);
	ui->listView->update();
}

void GlobalNotificationLastMsgDialog::onMsgAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	emit openGlobalNotificationMsg(globalNotificationId);
}

void GlobalNotificationLastMsgDialog::onDetailAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	emit openGlobalNotificationDetail(globalNotificationId);
}

void GlobalNotificationLastMsgDialog::onDeleteAction()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString globalNotificationId = action->data().toString();
	if (globalNotificationId.isEmpty())
		return;

	m_listModel->removeLastMsg(globalNotificationId);
}

void GlobalNotificationLastMsgDialog::contextMenu(const QPoint &position)
{
	QModelIndex index = ui->listView->indexAt(position);
	if (index.isValid())
	{
		QStandardItem *item = m_listModel->itemFromIndex(index);
		if (!item)
			return;

		GlobalNotificationLastMsgModel::LastMsgItem lastMsg = GlobalNotificationLastMsgModel::modelItem2LastMsg(*item);
		
		QString globalNotificationId = lastMsg.subId;
		m_msgAction->setData(globalNotificationId);
		m_detailAction->setData(globalNotificationId);
		m_deleteAction->setData(globalNotificationId);

		// show menu
		QMenu menu(this);
		menu.addAction(m_msgAction);
		menu.addAction(m_detailAction);
		menu.addAction(m_deleteAction);
		menu.exec(QCursor::pos());
	}
}

void GlobalNotificationLastMsgDialog::onGlobalNotificationLastMsgChanged()
{
	ui->listView->update();

	if (m_listModel->rowCount() > 0)
	{
		ui->stackedWidget->setCurrentIndex(0);
	}
	else
	{
		// remove from last contact model
		qPmApp->getModelManager()->lastContactModel()->onRemoveChat(QString(GLOBALNOTIFICATION_ROSTER_ID));

		// close this dialog
		ui->stackedWidget->setCurrentIndex(1);
		close();
	}
}

void GlobalNotificationLastMsgDialog::initUI()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	m_listModel = modelManager->globalNotificationLastMsgModel();
	
	// list view
	ui->listView->setModel(m_listModel);
	GlobalNotificationLastMsgItemDelegate *itemDelegate = new GlobalNotificationLastMsgItemDelegate(m_listModel.data(), ui->listView);
	ui->listView->setItemDelegate(itemDelegate);
	ui->listView->setMouseTracking(true);
	ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->listView->setSelectionMode(QListView::NoSelection);
	ui->listView->setEditTriggers(QListView::NoEditTriggers);
	ui->listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	connect(m_listModel, SIGNAL(globalNotificationLastMsgChanged()), this, SLOT(onGlobalNotificationLastMsgChanged()));

	if (m_listModel->rowCount() > 0)
		ui->stackedWidget->setCurrentIndex(0);
	else
		ui->stackedWidget->setCurrentIndex(1);

	connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
	connect(ui->listView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(const QPoint&)));
	connect(modelManager->globalNotificationModel(), SIGNAL(globalNotificationLogoChanged(QString)), this, SLOT(onGlobalNotificationLogoChanged(QString)));
	
	m_msgAction = new QAction(tr("Enter GlobalNotification"), this);
	connect(m_msgAction, SIGNAL(triggered()), this, SLOT(onMsgAction()));

	m_detailAction = new QAction(tr("Detail"), this);
	connect(m_detailAction, SIGNAL(triggered()), this, SLOT(onDetailAction()));

	m_deleteAction = new QAction(tr("Delete Chat"), this);
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(onDeleteAction()));
}

