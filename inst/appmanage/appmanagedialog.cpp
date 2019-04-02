#include "appmanagedialog.h"
#include "ui_appmanagedialog.h"
#include "settings/AccountSettings.h"
#include <QDesktopServices>
#include <QStandardItemModel>
#include "appmanageitemdelegate.h"
#include "Account.h"
#include <QUrl>
#include "appmanagemodel.h"
#include "appmanageaddappdialog.h"
#include "pmessagebox.h"
#include <QMenu>

static const int kMaxAppCount = 8;

QPointer<AppManageDialog> AppManageDialog::s_dialog;

AppManageDialog::AppManageDialog(QWidget *parent /*= 0*/)
	: FramelessDialog(parent)
{
	ui = new Ui::AppManageDialog();
	ui->setupUi(this);

	setWindowTitle(ui->title->text());

	setAttribute(Qt::WA_DeleteOnClose, true);
	setMainLayout(ui->verticalLayoutMain);

	initUI();

	setFixedSize(QSize(358, 458));
	setResizeable(false);
	setMaximizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	setSkin();
}

AppManageDialog::~AppManageDialog()
{
	delete ui;
}

AppManageDialog *AppManageDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new AppManageDialog();
	}
	return s_dialog.data();
}

void AppManageDialog::setSkin()
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
		"padding: 1px;"
		"}"
	);
}

void AppManageDialog::closeEvent(QCloseEvent *e)
{
	emit appChanged();

	e->accept();
}

void AppManageDialog::appItemClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	int actualData = index.data(AppManageModel::kActualDataRole).toInt();
	if (actualData == AppManageModel::Add)
	{
		// add application
		AppManageAddAppDialog dlg(AppManageAddAppDialog::AddMode, this);
		dlg.setWindowModality(Qt::WindowModal);
		if (dlg.exec())
		{
			QString name = dlg.appName();
			QString path = dlg.appPath();

			// add to model
			QList<AccountSettings::AppInfo> infos = Account::settings()->appInfos();
			int i = infos.count();
			AppManageModel *model = qobject_cast<AppManageModel *>(ui->tableView->model());
			model->setAppItem(i, name, path);
			
			// save to account settings
			AccountSettings::AppInfo info(name, path);
			infos.append(info);
			Account::settings()->setAppInfos(infos);

			// move 'add' sign to next
			if (infos.count() < kMaxAppCount)
			{
				// add an 'add' item
				model->setAddItem(infos.count());
			}

			emit appChanged();
		}
	}
}

void AppManageDialog::appItemDoubleClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	int actualData = index.data(AppManageModel::kActualDataRole).toInt();
	if (actualData == AppManageModel::Data)
	{
		// open application
		QString path = index.data(AppManageModel::kPathRole).toString();
		openApp(path);
	}
}

void AppManageDialog::onAppItemsChanged()
{
	AppManageModel *model = qobject_cast<AppManageModel *>(ui->tableView->model());
	QList<AccountSettings::AppInfo> infos;
	int row = 0;
	int col = 0;
	for (int i = 0; i < kMaxAppCount; ++i)
	{
		row = i/model->columnCount();
		col = i%model->columnCount();
		QStandardItem *item = model->item(row, col);
		if (!item)
			break;

		QString path = item->data(AppManageModel::kPathRole).toString();
		if (path.isEmpty())
			break;

		int actualData = item->data(AppManageModel::kActualDataRole).toInt();
		if (actualData == AppManageModel::Data)
		{
			QString name = item->text();
			AccountSettings::AppInfo appInfo(name, path);
			infos.append(appInfo);
		}
	}

	Account::settings()->setAppInfos(infos);

	emit appChanged();
}

void AppManageDialog::contextMenu(const QPoint &position)
{
	QModelIndex index = ui->tableView->indexAt(position);
	if (index.isValid())
	{
		int actualData = index.data(AppManageModel::kActualDataRole).toInt();
		if (actualData != AppManageModel::Data)
			return;

		AppManageModel *model = qobject_cast<AppManageModel *>(ui->tableView->model());
		int dataIndex = index.row()*model->columnCount() + index.column();
		m_openAppAction->setData(dataIndex);
		m_editAppNameAction->setData(dataIndex);

		// show menu
		QMenu menu(this);
		menu.addAction(m_openAppAction);
		menu.addSeparator();
		menu.addAction(m_editAppNameAction);
		menu.exec(QCursor::pos());
		return;
	}
}

void AppManageDialog::editAppName()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action != m_editAppNameAction)
		return;

	int index = m_editAppNameAction->data().toInt();
	AppManageModel *model = qobject_cast<AppManageModel *>(ui->tableView->model());
	int row = index/model->columnCount();
	int col = index%model->columnCount();
	QStandardItem *item = model->item(row, col);
	if (!item)
		return;

	QString name = item->text();
	QString path = item->data(AppManageModel::kPathRole).toString();

	AppManageAddAppDialog dlg(AppManageAddAppDialog::EditMode, this);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.setAppName(name);
	dlg.setAppPath(path);
	if (dlg.exec())
	{
		name = dlg.appName();
		path = dlg.appPath();

		// update to model
		model->setAppItem(index, name, path);

		// save to account settings
		QList<AccountSettings::AppInfo> infos = Account::settings()->appInfos();
		AccountSettings::AppInfo info(name, path);
		infos[index] = info;
		Account::settings()->setAppInfos(infos);

		emit appChanged();
	}
}

void AppManageDialog::openApp()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action != m_openAppAction)
		return;

	int index = m_editAppNameAction->data().toInt();
	AppManageModel *model = qobject_cast<AppManageModel *>(ui->tableView->model());
	int row = index/model->columnCount();
	int col = index%model->columnCount();
	QStandardItem *item = model->item(row, col);
	if (!item)
		return;

	QString path = item->data(AppManageModel::kPathRole).toString();
	openApp(path);
}

void AppManageDialog::initUI()
{
	AppManageModel *model = new AppManageModel(this);
	QList<AccountSettings::AppInfo> infos = Account::settings()->appInfos();
	int i = 0;
	foreach (AccountSettings::AppInfo info, infos)
	{
		model->setAppItem(i, info.name, info.path);
		++i;
	}

	if (infos.count() < kMaxAppCount)
	{
		// add an 'add' item
		model->setAddItem(infos.count());
	}

	model->setDelItem();

	// table view
	ui->tableView->setModel(model);
	AppManageItemDelegate *itemDelegate = new AppManageItemDelegate(ui->tableView);
	ui->tableView->setItemDelegate(itemDelegate);
	ui->tableView->setMouseTracking(true);
	ui->tableView->setShowGrid(false);
	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->tableView->setSelectionMode(QTableView::SingleSelection);
	ui->tableView->setEditTriggers(QTableView::NoEditTriggers);
	ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableView->horizontalHeader()->setVisible(false);
	ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->resizeColumnsToContents();
	ui->tableView->resizeRowsToContents();
	ui->tableView->setFocus();

	ui->tableView->setDragEnabled(true);
	ui->tableView->setAcceptDrops(true);
	ui->tableView->setDropIndicatorShown(true);

	connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(appItemClicked(QModelIndex)));
	connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(appItemDoubleClicked(QModelIndex)));
	connect(model, SIGNAL(appItemsChanged()), this, SLOT(onAppItemsChanged()));
	connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));

	m_editAppNameAction = new QAction(tr("Edit Name"), this);
	connect(m_editAppNameAction, SIGNAL(triggered()), this, SLOT(editAppName()));

	m_openAppAction = new QAction(tr("Open App"), this);
	connect(m_openAppAction, SIGNAL(triggered()), this, SLOT(openApp()));
}

void AppManageDialog::openApp(const QString &path)
{
	if (path.isEmpty())
		return;

	// check path
	QFileInfo fi(path);
	if (!fi.exists())
	{
		PMessageBox::warning(this, tr("Tip"), tr("App is not existing, please add again"));
		return;
	}

	// open application
	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
	{
		PMessageBox::warning(this, tr("Tip"), tr("Can't find program to open this app"));
		return;
	}
}