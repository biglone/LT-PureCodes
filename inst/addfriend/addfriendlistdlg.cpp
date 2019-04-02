#include "addfriendlistdlg.h"
#include "ui_addfriendlistdlg.h"
#include <QMovie>
#include "PmApp.h"
#include "Account.h"
#include "plaintextlineinput.h"
#include "loginmgr.h"
#include "pmessagebox.h"
#include "addfriendlistitemwidget.h"
#include "ModelManager.h"
#include <QStandardItem>
#include <QStandardItemModel>
#include "rostermodeldef.h"
#include "buddymgr.h"
#include <QDebug>
#include <Windows.h>

AddFriendListDlg *AddFriendListDlg::s_instance = 0;

bool addFriendItemLessThan(const AddFriendManager::Item &item1, const AddFriendManager::Item &item2)
{
	QString selfId = Account::instance()->id();
	if (item1.m_toId == selfId && item2.m_toId == selfId)
	{
		if (item1.m_action == AddFriendManager::Request && item2.m_action != AddFriendManager::Request)
		{
			return true;
		}
		else if (item1.m_action != AddFriendManager::Request && item2.m_action == AddFriendManager::Request)
		{
			return false;
		}
		else
		{
			return item1.m_time > item2.m_time;
		}
	}
	else if (item1.m_toId == selfId && item2.m_toId != selfId)
	{
		if (item1.m_action == AddFriendManager::Request)
		{
			return true;
		}
		else
		{
			return item1.m_time > item2.m_time;
		}
	}
	else if (item1.m_toId != selfId && item2.m_toId == selfId)
	{
		if (item2.m_action == AddFriendManager::Request)
		{
			return false;
		}
		else
		{
			return item1.m_time > item2.m_time;
		}
	}
	else
	{
		return item1.m_time > item2.m_time;
	}
}

AddFriendListDlg *AddFriendListDlg::instance()
{
	if (!s_instance)
	{
		s_instance = new AddFriendListDlg();
	}
	return s_instance;
}

AddFriendListDlg::AddFriendListDlg(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::AddFriendListDlg();
	ui->setupUi(this);

	setWindowTitle(ui->title->text());

	setAttribute(Qt::WA_DeleteOnClose, true);
	setMainLayout(ui->verticalLayoutMain);

	initUI();

	setFixedSize(QSize(609, 587));
	setResizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));

	AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
	connect(addFriendManager, SIGNAL(refreshOK()), this, SLOT(onListRefreshOK()), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(refreshFailed(QString)), this, SLOT(onListRefreshFailed(QString)), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(addFriendRequestOK(QString, int, QString, QString)), 
		this, SLOT(onAddFriendRequestOK(QString, int, QString, QString)), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(addFriendRequestFailed(QString, QString)), 
		this, SLOT(onAddFriendRequestFailed(QString, QString)), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(addFriendDeleteOK(QString)), this, SLOT(onListDeleteOK(QString)), Qt::UniqueConnection);
	connect(addFriendManager, SIGNAL(addFriendDeleteFailed(QString, QString)), this, SLOT(onListDeleteFailed(QString, QString)), Qt::UniqueConnection);

	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)), Qt::UniqueConnection);
	
	setSkin();
}

AddFriendListDlg::~AddFriendListDlg()
{
	delete ui;

	s_instance = 0;
}

void AddFriendListDlg::refreshList()
{
	AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
	if (addFriendManager->requestAddFriendList())
	{
		ui->stackedWidgetTip->setCurrentIndex(0);
		ui->stackedWidgetTip->setVisible(true);
		ui->listView->setEnabled(false);
	}
}

void AddFriendListDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 0;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	ui->centralPanel->setStyleSheet("QWidget#centralPanel {background: rgb(223, 242, 229);}");

	ui->pushButtonRoster->setStyleSheet("QPushButton {"
		"border-image: none;"
		"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(0, 120, 216), stop: 0.6 rgb(35, 139, 218), stop: 1.0 rgb(223, 242, 229));"
		"color: white;"
		"border: none;"
		"font: bold;"
		"}");
	ui->tabBar->setStyleSheet("QWidget#tabBar {background: rgb(0, 120, 216);}");
}

void AddFriendListDlg::onListRefreshOK()
{
	ui->stackedWidgetTip->setVisible(false);
	ui->listView->setEnabled(true);

	// add all items to list
	AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
	QList<AddFriendManager::Item> origItems = addFriendManager->refreshItems();
	m_items.clear();
	m_itemIndice.clear();
	ui->listView->clear();
	
	// do data analyze, only keep request from others or which is solved
	foreach (AddFriendManager::Item origItem, origItems)
	{
		bool add = true;
		if (origItem.m_fromId == Account::instance()->id() && origItem.m_action == AddFriendManager::Request)
		{
			add = false;
		}
		else
		{
			for (int k = 0; k < m_items.count(); k++)
			{
				AddFriendManager::Item item = m_items[k];
				if (origItem.m_sId == item.m_sId)
				{
					if (origItem.m_action != AddFriendManager::Request && item.m_action == AddFriendManager::Request)
					{
						m_items[k] = origItem;
					}
					add = false;
					break;
				}
				else // different session id
				{
					// previous request do not show
					if (((origItem.m_fromId == item.m_fromId && origItem.m_toId == item.m_toId) || (origItem.m_fromId == item.m_toId && origItem.m_toId == item.m_fromId))
						&& origItem.m_action == AddFriendManager::Request)
					{
						add = false;
						break;
					}
				}
			}
		}

		if (add)
		{
			m_items.append(origItem);
		}
	}

	// remove request from black list
	QList<int> blackIds;
	int i = 0;
	for (i = 0; i < m_items.count(); i++)
	{
		AddFriendManager::Item item = m_items[i];
		if (item.m_action == AddFriendManager::Request && qPmApp->getModelManager()->isInBlackList(item.m_fromId))
			blackIds.insert(0, i);
	}
	for (i = 0; i < blackIds.count(); i++)
	{
		int blackId = blackIds[i];
		m_items.removeAt(blackId);
	}

	// sort to put unsolved message to topmost
	qSort(m_items.begin(), m_items.end(), addFriendItemLessThan);

	// add to list
	foreach (AddFriendManager::Item item, m_items)
	{
		addListItem(item);
	}

	ui->listView->update();

	if (this->isVisible())
	{
		// flash this window
		DWORD timeOut = GetCaretBlinkTime();
		if (timeOut <= 0)
			timeOut = 250;

		UINT flashCount = 1;

		FLASHWINFO info;
		info.cbSize = sizeof(info);
		info.hwnd = (HWND)this->winId();
		info.dwFlags = FLASHW_ALL;
		info.dwTimeout = timeOut;
		info.uCount = flashCount;

		FlashWindowEx(&info);
	}
}

void AddFriendListDlg::onListRefreshFailed(const QString &desc)
{
	ui->labelTipIcon->setPixmap(QPixmap(":/messagebox/failed.png").scaled(QSize(24,24), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	ui->labelTipText->setText(tr("Getting validation information failed(%1)").arg(desc));
	ui->stackedWidgetTip->setVisible(true);
	ui->stackedWidgetTip->setCurrentIndex(1);
	ui->listView->setEnabled(true);
}

void AddFriendListDlg::on_labelRefresh_clicked()
{
	refreshList();
}

void AddFriendListDlg::onAccept(int index)
{
	if (index >= 0 && index < m_items.count())
	{
		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::information(0, tr("Tip"), tr("You are offline, can't add friends"));
			return;
		}

		// send accept
		QString group1 = RosterModel::defaultGroupName();
		AddFriendManager::Item item = m_items[index];
		AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
		addFriendManager->addFriendAction(AddFriendManager::Accept, item.m_fromId, item.m_toId, item.m_sId, item.m_message, item.m_group, group1);
	}
}

void AddFriendListDlg::onRefuse(int index)
{
	if (index >= 0 && index < m_items.count())
	{
		PlainTextLineInput refuseInput(this);
		refuseInput.setWindowModality(Qt::WindowModal);
		refuseInput.init(tr("Confirm"), tr("Please input reason"), 32, PlainTextLineInput::ModeUnicode, QString(), true);
		if (QDialog::Rejected == refuseInput.exec())
			return;

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::information(0, tr("Tip"), tr("You are offline, can't add friends"));
			return;
		}

		// send refuse
		QString reason = refuseInput.getInputText();
		AddFriendManager::Item item = m_items[index];
		AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
		addFriendManager->addFriendAction(AddFriendManager::Refuse, item.m_fromId, item.m_toId, item.m_sId, reason, item.m_group);
	}
}

void AddFriendListDlg::onAddFriendRequestOK(const QString &sId, int action, const QString &group, const QString &group1)
{
	Q_UNUSED(group);
	
	bool needRefresh = false;
	foreach (AddFriendManager::Item item, m_items)
	{
		if (item.m_sId == sId)
		{
			if (action == AddFriendManager::Accept)
			{
				// add friend
				QString name = qPmApp->getModelManager()->userName(item.m_fromId);
				emit addFriendOK(item.m_fromId, name, group1);

				// open chat
				qPmApp->getBuddyMgr()->openChat(item.m_fromId);

				// add a message to chat dialog
				bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
				msgBody.setSend(false);
				msgBody.setFrom(item.m_toId);
				msgBody.setTo(item.m_fromId);
				msgBody.setTime(CDateTime::currentDateTimeUtcString());
				msgBody.setBody(tr("We are friends now"));
				bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
				ext.setData("level", "info");
				ext.setData(bean::EXT_DATA_LASTCONTACT_NAME, true);
				ext.setData(bean::EXT_DATA_HISTORY_NAME, true);
				msgBody.setExt(ext);
				qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
			}

			needRefresh = true;
			break;
		}
	}

	if (needRefresh)
	{
		refreshList();
	}
}

void AddFriendListDlg::onAddFriendRequestFailed(const QString &sId, const QString &desc)
{
	bool showErrMsg = false;
	QString selfId = Account::instance()->id();
	foreach (AddFriendManager::Item item, m_items)
	{
		if (item.m_sId == sId && item.m_toId == selfId)
		{
			showErrMsg = true;
			break;
		}
	}

	if (showErrMsg)
	{
		PMessageBox::warning(this, tr("Error"), tr("Sending confirm information failed: %1").arg(desc));
	}
}

void AddFriendListDlg::onDetailChanged(const QString &id)
{
	if (m_itemIndice.contains(id))
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		bean::DetailItem *detail = modelManager->detailItem(id);

		QList<int> rows = m_itemIndice.values(id);
		QStandardItemModel *listModel = ui->listView->itemModel();
		foreach (int row, rows)
		{
			QStandardItem *listItem = listModel->item(row);
			if (!listItem)
				continue;

			AddFriendListItemWidget *w = (AddFriendListItemWidget *)ui->listView->indexWidget(listItem->index());
			if (!w)
				continue;

			QPixmap avatar = modelManager->getUserAvatar(id);
			QString name = modelManager->userName(id);
			w->setAvatar(avatar);
			w->setName(name);
			w->setSex(detail->sex());
			w->setDepart(detail->depart());
			w->setPhone(detail->phone1());
		}

		ui->listView->update();
	}
}

void AddFriendListDlg::onViewMaterial(int index)
{
	if (index >= 0 && index < m_items.count())
	{
		AddFriendManager::Item item = m_items[index];
		QString selfId = Account::instance()->id();
		QString id = item.m_fromId;
		if (id == selfId)
			id = item.m_toId;
		emit viewMaterial(id);
	}
}

void AddFriendListDlg::onDeleteItem(int index)
{
	if (index >= 0 && index < m_items.count())
	{
		QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Delete"), 
			tr("Are you sure to delete this record"), QDialogButtonBox::Yes|QDialogButtonBox::No);
		if (ret != QDialogButtonBox::Yes)
			return;

		AddFriendManager::Item item = m_items[index];
		AddFriendManager *addFriendManager = qPmApp->getAddFriendManager();
		addFriendManager->addFriendDelete(item.m_sId, Account::instance()->id());
	}
}

void AddFriendListDlg::onListDeleteOK(const QString &sId)
{
	qDebug() << Q_FUNC_INFO << "---------------------------------- item count: " << m_items.count();

	int index = 0;
	int row = 0;
	foreach (AddFriendManager::Item item, m_items)
	{
		if (item.m_sId == sId)
		{
			m_items[index].setDelete(true);
			ui->listView->removeRow(row);
			break;
		}

		if (!item.isDelete())
		{
			++row;
		}

		++index;
	}

	bool hasRequest = false;
	foreach (AddFriendManager::Item item, m_items)
	{
		if ((item.m_action == AddFriendManager::Request) && (!item.isDelete()))
		{
			hasRequest = true;
			break;
		}
	}
	emit setUnhandleFlag(hasRequest);
}

void AddFriendListDlg::onListDeleteFailed(const QString &sId, const QString &desc)
{
	Q_UNUSED(sId);
	PMessageBox::warning(this, tr("Error"), tr("Delete record failed: %1").arg(desc));
}

void AddFriendListDlg::initUI()
{
	ui->stackedWidgetTip->setCurrentIndex(0);
	ui->stackedWidgetTip->setVisible(false);
	ui->stackedWidget->setCurrentIndex(0);

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);
	movie->start();

	ui->labelRefresh->setFontAtt(QColor(0, 120, 216), 10, false);
}

void AddFriendListDlg::addListItem(const AddFriendManager::Item &item)
{
	if (item.m_action == AddFriendManager::Request)
	{
		// set read
		qPmApp->getAddFriendManager()->addFriendRead(item.m_sId);
	}

	QStandardItemModel *listModel = ui->listView->itemModel();
	QString selfId = Account::instance()->id();
	QString otherId = item.m_fromId;
	QString otherName = item.m_fromName;
	if (otherId == selfId)
	{
		otherId = item.m_toId;
		otherName = item.m_toName;
	}
	QStandardItem *listItem = new QStandardItem();
	listItem->setText(otherId);
	listModel->appendRow(listItem);

	AddFriendListItemWidget *w = new AddFriendListItemWidget(this);

	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *detail = modelManager->detailItem(otherId);
	QPixmap avatar = modelManager->getUserAvatar(otherId);
	w->setAvatar(avatar);
	w->setIndex(listItem->index().row());
	w->setName(otherName);
	w->setSex(detail->sex());
	w->setDepart(detail->depart());
	w->setPhone(detail->phone1());
	w->setDate(item.m_time);
	w->setAction(item.m_fromId, item.m_toId, Account::instance()->id(), item.m_action);
	w->setMessage(item.m_message);
	w->setDeletable(true/*item.m_status == 1*/);
	ui->listView->setIndexWidget(listItem->index(), w);

	m_itemIndice.insert(otherId, listItem->index().row());

	bool connectOK = false;
	connectOK = connect(w, SIGNAL(accept(int)), this, SLOT(onAccept(int)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);
	connectOK = connect(w, SIGNAL(refuse(int)), this, SLOT(onRefuse(int)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);
	connectOK = connect(w, SIGNAL(viewMaterial(int)), this, SLOT(onViewMaterial(int)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);
	connectOK = connect(w, SIGNAL(deleteItem(int)), this, SLOT(onDeleteItem(int)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);
}

