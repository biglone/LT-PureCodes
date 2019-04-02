#include "unionchatdialog.h"
#include "ui_unionchatdialog.h"
#include "guiconstants.h"
#include <QStandardItemModel>
#include <QDebug>
#include "PmApp.h"
#include "ModelManager.h"
#include "chatbasedialog.h"
#include "unionchatlistitemdelegate.h"
#include "unionchatlistitemwidget.h"
#include "widgetmanager.h"
#include "pmessagebox.h"
#include "unionchatclosetipdialog.h"
#include "Account.h"
#include "settings/AccountSettings.h"
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QtWin>

#ifndef UNICODE
#define UNICODE

#include <Windows.h>
#include <Shobjidl.h>
#include <Dwmapi.h>

const int kChatListMsgTypeRole  = Qt::UserRole + 1;
const int kChatListMsgIdRole    = Qt::UserRole + 2;
const int kChatListMsgCountRole = Qt::UserRole + 3;

static const QSize kChatListIconSize = QSize(40, 40);

static const int kBorderWidth = 5;
static const int kMaxListViewWidth = 198;
static const int kMinListViewWidth = 84;

static HMODULE s_dwmapiModule = NULL;

typedef HRESULT (__stdcall *MyDwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
typedef HRESULT (__stdcall *MyDwmSetIconicThumbnail)(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
typedef HRESULT (__stdcall *MyDwmSetIconicLivePreviewBitmap)(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);

static MyDwmSetWindowAttribute myDwmSetWindowAttribute = NULL;
static MyDwmSetIconicThumbnail myDwmSetIconicThumbnail = NULL;
static MyDwmSetIconicLivePreviewBitmap myDwmSetIconicLivePreviewBitmap = NULL;

UnionChatDialog::UnionChatDialog(QWidget *parent)
	: FramelessDialog(parent)
	, m_pTl3(0)
	, m_previousIndex(-1)
	, m_taskbarBtnCreatedMsg(-1)
	, m_lastFlashWidget(0)
	, m_blockingClose(false)
	, m_splitter(0)
{
	ui = new Ui::UnionChatDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	m_mainLayout = new QHBoxLayout(this);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);
	
	m_splitter = new MiniSplitter(Qt::Horizontal);
	m_splitter->addWidget(ui->listView);
	m_splitter->addWidget(ui->stackedWidget);
	m_splitter->setStretchFactor(0, 0);
	m_splitter->setStretchFactor(1, 1);
	ui->listView->setVisible(false);
	m_splitter->handle(1)->setVisible(false);
	m_mainLayout->addWidget(m_splitter);

	setMainLayout(m_mainLayout);
	setResizeable(true);
	setMaximizeable(true);

	initUI();

	resize(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth+2*kBorderWidth, 
		GuiConstants::WidgetSize::ChatHeight+2*kBorderWidth);
	setMinimumSize(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth+2*kBorderWidth, 
		GuiConstants::WidgetSize::ChatHeight+2*kBorderWidth);

	setSkin();

	if (ChatProxy::isSupportTaskbarApi())
	{
		s_dwmapiModule = LoadLibraryW(L"dwmapi.dll");
		if (s_dwmapiModule != NULL)
		{
			myDwmSetWindowAttribute = (MyDwmSetWindowAttribute)GetProcAddress(s_dwmapiModule, "DwmSetWindowAttribute");
			if (myDwmSetWindowAttribute == NULL)
			{
				qDebug() << "GetProcAddress DwmSetWindowAttribute failed";
			}
			myDwmSetIconicThumbnail = (MyDwmSetIconicThumbnail)GetProcAddress(s_dwmapiModule, "DwmSetIconicThumbnail");
			if (myDwmSetIconicThumbnail == NULL)
			{
				qDebug() << "GetProcAddress DwmSetIconicThumbnail failed";
			}
			myDwmSetIconicLivePreviewBitmap = (MyDwmSetIconicLivePreviewBitmap)GetProcAddress(s_dwmapiModule, "DwmSetIconicLivePreviewBitmap");
			if (myDwmSetIconicLivePreviewBitmap == NULL)
			{
				qDebug() << "GetProcAddress DwmSetIconicLivePreviewBitmap failed";
			}
		}
		else
		{
			qDebug() << "LoadLibrary dwmapi.dll failed";
		}

		m_taskbarBtnCreatedMsg = RegisterWindowMessageW(L"TaskbarButtonCreated");
	}
}

UnionChatDialog::~UnionChatDialog()
{
	if (m_pTl3)
		m_pTl3->Release();

	if (s_dwmapiModule != NULL)
		FreeLibrary(s_dwmapiModule);

	delete ui;
}

void UnionChatDialog::clearChatUnread(ChatBaseDialog *chatWidget)
{
	// clear unread
	clearUnreadMessage(chatWidget);

	// if is minimized, need to set to next unread
	if (isMinimized() || (isVisible() && !isActiveWindow()))
	{
		ChatBaseDialog *curChatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->currentWidget());
		if (curChatWidget)
		{
			QStandardItem *item = m_chatListModel->item(ui->stackedWidget->indexOf(curChatWidget));
			setTaskbarIconAndText(this, item);

			if (curChatWidget->unreadMessageCount() > 0)
			{
				flashTaskBar(curChatWidget);
			}
			else // current no unread message, flash next unread
			{
				flashNextUnread();
			}
		}
	}
}

void UnionChatDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_10.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 95;
	setBG(bgPixmap, bgSizes);

	ui->listView->setStyleSheet("QListView#listView {background: rgb(72, 167, 243); border: none; padding: 0px;}");
}

void UnionChatDialog::triggerMaximize()
{
	int listViewWidth = 0;
	if (ui->stackedWidget->count() > 1)
	{
		QList<int> sizes = m_splitter->sizes();
		listViewWidth = sizes.at(0);
	}

	FramelessDialog::triggerMaximize();

	if (listViewWidth > 0)
	{
		QList<int> sizes;
		sizes << listViewWidth << (this->width() - listViewWidth - m_splitter->handleWidth());
		m_splitter->setSizes(sizes);
	}
}

void UnionChatDialog::insertChatAtTop(bean::MessageType msgType, 
									  const QString &id, 
									  ChatBaseDialog *chatWidget,
									  bool makeCurrent /*= true*/)
{
	if (!chatWidget)
		return;

	if (hasChatWidget(chatWidget))
	{
		qDebug() << Q_FUNC_INFO << "already has chat widget:" << msgType << id; 
		return;
	}

	// add item and widget
	if (ui->stackedWidget->count() == 0)
		chatWidget->setUnionState(ChatBaseDialog::Single);
	else
		chatWidget->setUnionState(ChatBaseDialog::Union);
	connectChatWidget(chatWidget);
	QStandardItem *item = createListItem(msgType, id);
	m_chatListModel->insertRow(0, item);
	addListItemWidget(item);

	ChatProxy *proxy = createChatProxy();
	chatWidget->setChatProxy(proxy);
	ui->stackedWidget->insertWidget(0, chatWidget);

	// add task bar tab
	QWidget *beforeWidget = 0;
	if (ui->stackedWidget->count() > 1)
		beforeWidget = ui->stackedWidget->widget(1);
	insertTaskbarTab(chatWidget, beforeWidget);
	setTaskbarIconAndText(chatWidget, item);

	if (makeCurrent)
	{
		// make current
		setCurrentChat(chatWidget);

		// clear unread message
		clearUnreadMessage(chatWidget);
	}
	else
	{
		if (chatWidget->unreadMessageCount() > 0)
		{
			QMetaObject::invokeMethod(chatWidget, "chatUnreadMessageCountChanged", Qt::DirectConnection, 
				Q_ARG(int, chatWidget->unreadMessageCount()));
		}
	}

	// check if need to show list
	if (ui->stackedWidget->count() == 2)
	{
		for (int i = 0; i < ui->stackedWidget->count(); ++i)
		{
			ChatBaseDialog *openedChat = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(i));
			openedChat->setUnionState(ChatBaseDialog::Union);
		}

		if (ui->listView->width() <= 0)
			showList();

		if (!makeCurrent)
		{
			ui->listView->setCurrentIndex(m_chatListModel->item(1)->index());
		}
	}
	else if (ui->stackedWidget->count() == 1)
	{
		setTaskbarIconAndText(this, item);
	}
}

void UnionChatDialog::insertMultiChatAtTop(const QString &id, 
										   const QStringList &members, 
										   ChatBaseDialog *chatWidget,
										   bool makeCurrent /*= true*/)
{
	if (!chatWidget)
		return;

	if (hasChatWidget(chatWidget))
	{
		qDebug() << Q_FUNC_INFO << "already has multi chat widget:" << id << members; 
		return;
	}

	// add item and widget
	if (ui->stackedWidget->count() == 0)
		chatWidget->setUnionState(ChatBaseDialog::Single);
	else
		chatWidget->setUnionState(ChatBaseDialog::Union);
	connectChatWidget(chatWidget);
	QStandardItem *item = createMultiChatListItem(id, members);
	m_chatListModel->insertRow(0, item);

	ChatProxy *proxy = createChatProxy();
	chatWidget->setChatProxy(proxy);
	addListItemWidget(item);
	ui->stackedWidget->insertWidget(0, chatWidget);

	// add task bar tab
	QWidget *beforeWidget = 0;
	if (ui->stackedWidget->count() > 1)
		beforeWidget = ui->stackedWidget->widget(1);
	insertTaskbarTab(chatWidget, beforeWidget);
	setTaskbarIconAndText(chatWidget, item);

	if (makeCurrent)
	{
		// make current
		setCurrentChat(chatWidget);

		// clear unread message
		clearUnreadMessage(chatWidget);
	}
	else
	{
		if (chatWidget->unreadMessageCount() > 0)
		{
			QMetaObject::invokeMethod(chatWidget, "chatUnreadMessageCountChanged", Qt::DirectConnection, 
				Q_ARG(int, chatWidget->unreadMessageCount()));
		}
	}

	// check if need to show list
	if (ui->stackedWidget->count() == 2)
	{
		for (int i = 0; i < ui->stackedWidget->count(); ++i)
		{
			ChatBaseDialog *openedChat = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(i));
			openedChat->setUnionState(ChatBaseDialog::Union);
		}

		if (ui->listView->width() <= 0)
			showList();

		if (!makeCurrent)
		{
			ui->listView->setCurrentIndex(m_chatListModel->item(1)->index());
		}
	}
	else if (ui->stackedWidget->count() == 1)
	{
		setTaskbarIconAndText(this, item);
	}
}

bool UnionChatDialog::removeChat(QWidget *chatWidget)
{
	if (!chatWidget)
		return false;

	int index = ui->stackedWidget->indexOf(chatWidget);
	if (index == -1)
	{
		qDebug() << Q_FUNC_INFO << "do not has chat widget";
		return false;
	}

	if (m_blockingClose && ui->stackedWidget->count() == 1)
	{
		return false;
	}

	if (!((ChatBaseDialog *)chatWidget)->canClose())
	{
		return false;
	}

	bool isExpanded = ((ChatBaseDialog *)chatWidget)->isExpanded();
	int unexpandedWidth = ((ChatBaseDialog *)chatWidget)->unExpandedWidth();

	// remove from task bar
	removeTaskbarTab(chatWidget);

	// remove from stack
	ui->stackedWidget->removeWidget(chatWidget);
	chatWidget->close();

	// remove list item
	QStandardItem *item = m_chatListModel->item(index);
	bool isCurrentRemoved = (ui->listView->currentIndex() == item->index());
	UnionChatListItemWidget *itemWidget = qobject_cast<UnionChatListItemWidget *>(ui->listView->indexWidget(item->index()));
	delete itemWidget;
	itemWidget = 0;
	ui->listView->setIndexWidget(item->index(), 0);
	m_chatListModel->takeRow(index);
	delete item;
	item = 0;

	// find current
	if (isCurrentRemoved)
	{
		if (index >= ui->stackedWidget->count())
			index = ui->stackedWidget->count() - 1;
		if (index >= 0)
		{
			QWidget *currentChat = ui->stackedWidget->widget(index);
			if (currentChat)
			{
				setCurrentChat(currentChat);

				// need clear unread message
				clearUnreadMessage(((ChatBaseDialog *)currentChat));
			}
		}
	}

	// set current tab
	QWidget *currentChat = ui->stackedWidget->currentWidget();
	if (currentChat)
	{
		setActiveTaskbarTab(currentChat);

		if (!isCurrentRemoved)
		{
			int currentIndex = ui->stackedWidget->indexOf(currentChat);
			QStandardItem *currentItem = m_chatListModel->item(currentIndex);
			setTaskbarIconAndText(this, currentItem);
		}
	}

	if (ui->stackedWidget->count() == 0)
	{
		// check if need close this dialog
		close();
		return true;
	}
	
	if (isExpanded)
	{
		// re-config dialog size
		chatMinimumWidthChanged(0);
		if (!isShowMaximized())
		{
			chatWidthChanged(unexpandedWidth);
		}
	}

	// check if need to hide list
	if (ui->stackedWidget->count() == 1)
	{
		ChatBaseDialog *lastChatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(0));
		lastChatWidget->setUnionState(ChatBaseDialog::Single);

		if (ui->listView->width() > 0)
			hideList();
	}

	return true;
}

void UnionChatDialog::setCurrentChat(QWidget *chatWidget)
{
	if (!chatWidget)
		return;

	if (!hasChatWidget(chatWidget))
	{
		qDebug() << Q_FUNC_INFO << "do not have chat widget"; 
		return;
	}

	int index = ui->stackedWidget->indexOf(chatWidget);
	QStandardItem *item = m_chatListModel->item(index);
	
	// set current list item
	ui->listView->setCurrentIndex(item->index());
	UnionChatListItemWidget *w = qobject_cast<UnionChatListItemWidget *>(ui->listView->indexWidget(item->index()));
	w->update();

	// set current chat
	ui->stackedWidget->setCurrentIndex(index);
	
	// set current task bar
	setActiveTaskbarTab(chatWidget);
	setTaskbarIconAndText(this, item);
}

void UnionChatDialog::flashChat(QWidget *chatWidget)
{
	if (!chatWidget)
		return;

	if (!hasChatWidget(chatWidget))
	{
		qDebug() << Q_FUNC_INFO << "do not have chat widget"; 
		return;
	}

	QStandardItem *item = m_chatListModel->item(ui->stackedWidget->indexOf(chatWidget));
	setTaskbarIconAndText(this, item);

	if (isVisible() && isActiveWindow() && !isMinimized() && (chatWidget == ui->stackedWidget->currentWidget()))
	{
		// do not flash, this is current chat
	}
	else
	{
		// flash
		ChatProxy *chatProxy = ((ChatBaseDialog *)chatWidget)->chatProxy();
		if (chatProxy)
		{
			flashTaskBar(chatProxy);
		}
	}
	
	flashTaskBar(this);

	// record last flash widget
	if (!ChatProxy::isSupportTaskbarApi())
	{
		if (isMinimized())
		{
			m_lastFlashWidget = chatWidget;
		}
	}
}

bool UnionChatDialog::hasChatWidget(QWidget *chatWidget)
{
	if (ui->stackedWidget->indexOf(chatWidget) == -1)
		return false;
	else
		return true;
}

bool UnionChatDialog::isMaximumState() const
{
	return isShowMaximized();
}

void UnionChatDialog::chatWidthChanged(int width)
{
	QSize origSize = ui->stackedWidget->size();
	if (origSize.width() == width)
		return;

	QSize adjustSize = this->size();
	if (origSize.width() < width)
	{
		adjustSize.setWidth(adjustSize.width()-origSize.width()+width);
		resize(adjustSize);
	}
	else // if (origSize.width() > width)
	{
		if (width > ui->stackedWidget->minimumWidth())
		{		
			adjustSize.setWidth(adjustSize.width()-origSize.width()+width);
			resize(adjustSize);
		}
		else
		{
			adjustSize.setWidth(adjustSize.width()-origSize.width()+ui->stackedWidget->minimumWidth());
			resize(adjustSize);
		}
	}
}

bool UnionChatDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	const QByteArray kWindowsEventTypeId("windows_generic_MSG");
	if (eventType == kWindowsEventTypeId)
	{
		MSG *msg = (MSG *)message;
		return winEvent(msg, result);
	}
	return false;
}

bool UnionChatDialog::winEvent(MSG *message, long *result)
{
	if (!message)
	{
		return false;
	}

	if (message->message == m_taskbarBtnCreatedMsg && ChatProxy::isSupportTaskbarApi())
	{
		qDebug() << Q_FUNC_INFO << "taskbarBtnCreatedMsg message " << ui->stackedWidget->count() << "tabs";

		ITaskbarList3 *pTl3 = 0;
		CoCreateInstance(CLSID_TaskbarList,
			NULL, 
			CLSCTX_ALL, 
			__uuidof(ITaskbarList3), 
			reinterpret_cast<void **>(&pTl3));

		if (!pTl3)
		{
			qDebug() << "create ITaskbarList3 interface failed";
		}
		else
		{
			pTl3->HrInit();

			bool tabOK = true;
			HWND mainHwnd = (HWND)this->winId();
			QList<HWND> tabHwnds;
			int i = 0;
			for (i = 0; i < ui->stackedWidget->count(); ++i)
			{
				// add task bar tab
				QWidget *w = ui->stackedWidget->widget(i);
				ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(w);
				HWND hwnd = chatWidget->chatProxyWinId();
				if (hwnd == NULL)
					break;

				if (FAILED(pTl3->RegisterTab(hwnd, mainHwnd)))
				{
					qDebug() << "register tab failed";
					tabOK = false;
					break;
				}

				if (FAILED(pTl3->SetTabOrder(hwnd, NULL)))
				{
					qDebug() << "set tab order failed";
					break;
				}

				QStandardItem *item = m_chatListModel->item(i);
				setTaskbarIconAndText(w, item);

				if (item->index() == ui->listView->currentIndex())
				{
					if (FAILED(pTl3->SetTabActive(hwnd, mainHwnd, 0)))
					{
						qDebug() << "set tab active failed";
					}
				}
				else
				{
					if (chatWidget->unreadMessageCount() > 0)
						flashTaskBar(chatWidget->chatProxy());
				}
			}

			if (!tabOK)
			{
				pTl3->Release();
				m_pTl3 = 0;
			}
			else
			{
				m_pTl3 = pTl3;
			}
		}

		*result = 0;
		return true;
	}

	return false;
}

void UnionChatDialog::closeEvent(QCloseEvent *ev)
{
	if (ui->stackedWidget->count() <= 0)
	{
		ev->accept();
		return;
	}

	UnionChatCloseTipDialog::CloseResult closeResult = UnionChatCloseTipDialog::CloseAll;
	if (!(qPmApp->isShutingDown() || qPmApp->isLogout())) // if not logout or exit, need query to close session
	{
		if (ui->stackedWidget->count() > 1 && !Account::settings()->chatAlwaysCloseAll())
		{
			UnionChatCloseTipDialog dlg(this);
			dlg.setWindowModality(Qt::WindowModal);
			m_blockingClose = true;
			if (!dlg.exec())
			{
				m_blockingClose = false;
				ev->ignore();
				return;
			}

			m_blockingClose = false;

			if (dlg.isAlwaysCloseAllChecked())
				Account::settings()->setChatAlwaysCloseAll(true);
			else
				Account::settings()->setChatAlwaysCloseAll(false);

			closeResult = dlg.closeResult();
			if (ui->stackedWidget->count() == 1)
				closeResult = UnionChatCloseTipDialog::CloseAll;
		}
	}

	if (closeResult == UnionChatCloseTipDialog::CloseCurrent)
	{
		QWidget *chatWidget = ui->stackedWidget->currentWidget();
		removeChat(chatWidget);
		ev->ignore();
	}
	else // close all
	{
		for (int i = ui->stackedWidget->count()-1; i >= 0; --i)
		{
			QWidget *chatWidget = ui->stackedWidget->widget(i);
			if (!removeChat(chatWidget))
			{
				ev->ignore();
				return;
			}
		}

		ev->accept();
	}
}

void UnionChatDialog::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		if (ui->stackedWidget->currentIndex() != -1)
		{
			removeChat(ui->stackedWidget->currentWidget());
		}
		else
		{
			close();
		}
		return;
	}
	else if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::ControlModifier) // switch to next chat
	{
		if (ui->stackedWidget->count() > 1)
		{
			int index = ui->stackedWidget->currentIndex();
			if (index != -1)
			{
				++index;
				if (index >= ui->stackedWidget->count())
					index = 0;
				QWidget *nextWidget = ui->stackedWidget->widget(index);
				setCurrentChat(nextWidget);
				return;
			}
		}
	}

	FramelessDialog::keyPressEvent(event);
}

void UnionChatDialog::changeEvent(QEvent *ev)
{
	if (ev->type() == QEvent::WindowStateChange)
	{
		QWindowStateChangeEvent *stateEvent = (QWindowStateChangeEvent *)ev;
		Qt::WindowStates oldState = stateEvent->oldState();
		Qt::WindowStates newState = this->windowState();
		if ((oldState & Qt::WindowMinimized) && !(newState & Qt::WindowMinimized))
		{
			if (!ChatProxy::isSupportTaskbarApi() && m_lastFlashWidget)
			{
				if (ui->stackedWidget->indexOf(m_lastFlashWidget) != -1)
				{
					if (ui->stackedWidget->currentWidget() != m_lastFlashWidget)
					{
						setCurrentChat(m_lastFlashWidget);
					}
				}
				m_lastFlashWidget = 0;
			}
		}
		else if ((newState & Qt::WindowMinimized) && !(oldState & Qt::WindowMinimized))
		{
			// if is minimized, need to set to next unread
			ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->currentWidget());
			if (chatWidget && chatWidget->unreadMessageCount() <= 0)
			{
				// if current chat has no unread message, other widgets which have unread message need to be checked and shown at top
				flashNextUnread();
			}
		}
	}

	if (ev->type() == QEvent::ActivationChange)
	{
		// inactive --> active
		Qt::WindowStates state = this->windowState();
		if (isActiveWindow() && !(state & Qt::WindowMinimized) && ui->stackedWidget->count() > 0)
		{
			m_clearUnreadTimer.stop();
			m_clearUnreadTimer.start();
		}
	}

	FramelessDialog::changeEvent(ev);
}

void UnionChatDialog::onMaximizeStateChanged(bool isMaximized)
{
	for (int i = 0; i < ui->stackedWidget->count(); ++i)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(i));
		if (chatWidget)
		{
			chatWidget->setMaximizeState(isMaximized);
		}
	}
}

void UnionChatDialog::onListItemClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	// clear previous unread message
	if (m_previousIndex >= 0 && m_previousIndex < ui->stackedWidget->count())
	{
		ChatBaseDialog *previousChatWidget =  qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(m_previousIndex));
		clearUnreadMessage(previousChatWidget);
	}

	// set current
	QWidget *w = ui->stackedWidget->widget(index.row());
	if (ui->stackedWidget->currentWidget() != w)
	{
		setCurrentChat(w);
	}

	// clear current unread message
	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(w);
	clearUnreadMessage(chatWidget);
}

void UnionChatDialog::removeChat()
{
	UnionChatListItemWidget *removedItem = qobject_cast<UnionChatListItemWidget *>(sender());
	if (!removedItem)
		return;

	for (int i = 0; i < m_chatListModel->rowCount(); ++i)
	{
		QStandardItem *item = m_chatListModel->item(i);
		UnionChatListItemWidget *w = (UnionChatListItemWidget *)ui->listView->indexWidget(item->index());
		if (w == removedItem)
		{
			ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(i));
			removeChat(chatWidget);
			break;
		}
	}
}

void UnionChatDialog::chatIconChanged(const QPixmap &pixmap)
{
	if (pixmap.isNull())
		return;

	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
	if (!chatWidget)
		return;

	int index = ui->stackedWidget->indexOf(chatWidget);
	if (index == -1)
		return;

	// refresh list item
	QStandardItem *item = m_chatListModel->item(index);
	QIcon icon = pixmap;
	QPixmap taskbarPixmap = pixmap;
	item->setIcon(icon);
	UnionChatListItemWidget *w = qobject_cast<UnionChatListItemWidget *>(ui->listView->indexWidget(item->index()));
	w->refresh();

	setTaskbarIcon(chatWidget, taskbarPixmap);

	// if is current, need update windows icon
	if (item->index() == ui->listView->currentIndex())
	{
		setTaskbarIcon(this, taskbarPixmap);
	}
}

void UnionChatDialog::chatNameChanged(const QString &name)
{
	if (name.isEmpty())
		return;

	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
	if (!chatWidget)
		return;

	int index = ui->stackedWidget->indexOf(chatWidget);
	if (index == -1)
		return;

	QStandardItem *item = m_chatListModel->item(index);
	QString origName = item->text();
	if (name == origName)
		return;

	// refresh list item
	item->setText(name);
	UnionChatListItemWidget *w = qobject_cast<UnionChatListItemWidget *>(ui->listView->indexWidget(item->index()));
	w->refresh();

	int unreadCount = chatWidget->unreadMessageCount()/*item->data(kChatListMsgCountRole).toInt()*/;
	QString title = name;
	if ((!this->isVisible() || this->isMinimized()) && unreadCount > 0)
		title.append(tr("(%1 unread)").arg(unreadCount));
	setTaskbarText(chatWidget, title);

	// if is current, need update windows text
	if (item->index() == ui->listView->currentIndex())
	{
		setTaskbarText(this, name);
	}
}

void UnionChatDialog::activateFromTaskbar()
{
	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
	if (!chatWidget)
		return;

	int index = ui->stackedWidget->indexOf(chatWidget);
	if (index == -1)
		return;

	if (!isVisible() || isMinimized())
	{
		disconnect(chatWidget, SIGNAL(activateFromTaskbar()), this, SLOT(activateFromTaskbar()));
		WidgetManager::showActivateRaiseWindow(this);
		connect(chatWidget, SIGNAL(activateFromTaskbar()), this, SLOT(activateFromTaskbar()), Qt::UniqueConnection);
	}
	else if (isVisible())
	{
		disconnect(chatWidget, SIGNAL(activateFromTaskbar()), this, SLOT(activateFromTaskbar()));
		WidgetManager::showActivateRaiseWindow(this);
		connect(chatWidget, SIGNAL(activateFromTaskbar()), this, SLOT(activateFromTaskbar()), Qt::UniqueConnection);

		// clear previous unread message
		if (m_previousIndex >= 0 && m_previousIndex < ui->stackedWidget->count())
		{
			ChatBaseDialog *previousChatWidget =  qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(m_previousIndex));
			clearUnreadMessage(previousChatWidget);
		}
	}

	// set to current
	if (ui->stackedWidget->currentWidget() != chatWidget)
	{
		setCurrentChat(chatWidget);
	}
	else
	{
		setActiveTaskbarTab(chatWidget);
	}

	// clear current unread message
	clearUnreadMessage(chatWidget);
}

void UnionChatDialog::closeFromTaskbar()
{
	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
	if (!chatWidget)
		return;

	removeChat(chatWidget);

	delete chatWidget;
	chatWidget = 0;
}

void UnionChatDialog::chatUnreadMessageCountChanged(int count)
{
	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
	if (!chatWidget)
		return;

	int index = ui->stackedWidget->indexOf(chatWidget);
	if (index == -1)
		return;

	QStandardItem *item = m_chatListModel->item(index);
	QString title = item->text();
	item->setData(count, kChatListMsgCountRole);
	if (item->index() == ui->listView->currentIndex())
	{
		if ((!this->isVisible() || this->isMinimized() || !this->isActiveWindow()) && count > 0)
		{
			title.append(tr("(%1 unread)").arg(count));

			if (this->isVisible() && !this->isMinimized() && !this->isActiveWindow())
			{
				item->setData(0, kChatListMsgCountRole);
			}
		}
		else if (this->isVisible())
		{
			chatWidget->clearUnreadMessageCount();
			item->setData(0, kChatListMsgCountRole);
		}
	}
	else
	{
		if (count > 0)
			title.append(tr("(%1 unread)").arg(count));
	}
	ui->listView->update();

	setTaskbarText(chatWidget, title);
}

void UnionChatDialog::setChatThumbnail(int maxWidth, int maxHeight)
{
	if (ChatProxy::isSupportTaskbarApi() && myDwmSetIconicThumbnail && m_pTl3)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
		if (!chatWidget)
			return;

		ChatProxy *chatProxy = chatWidget->chatProxy();
		if (!chatProxy)
			return;

		if (chatWidget->size() != ui->stackedWidget->size())
			chatWidget->resize(ui->stackedWidget->size());

		QPixmap pixmap = QPixmap::grabWidget(chatWidget);
		pixmap = pixmap.scaled(QSize(maxWidth, maxHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		HBITMAP hbm = QtWin::toHBITMAP(pixmap);
		if (hbm)
		{
			HWND hwnd = (HWND)chatProxy->winId();
			if (FAILED(myDwmSetIconicThumbnail(hwnd, hbm, 0)))
			{
				qDebug() << "DwmSetIconicThumbnail failed";
			}
			DeleteObject(hbm);
		}
	}
}

void UnionChatDialog::setChatLivePreview()
{
	if (ChatProxy::isSupportTaskbarApi() && myDwmSetIconicLivePreviewBitmap && m_pTl3)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
		if (!chatWidget)
			return;

		ChatProxy *chatProxy = chatWidget->chatProxy();
		if (!chatProxy)
			return;

		if (chatWidget->size() != ui->stackedWidget->size())
			chatWidget->resize(ui->stackedWidget->size());

		HWND hwnd = (HWND)chatProxy->winId();
		POINT pt;
		QPoint pos = ui->stackedWidget->pos();
		pos = ((QWidget *)ui->stackedWidget->parent())->mapTo(this, pos);
		pt.x = pos.x();
		pt.y = pos.y();
		QPixmap pixmap = QPixmap::grabWidget(chatWidget);
		HBITMAP hbm = QtWin::toHBITMAP(pixmap);
		if (hbm)
		{
			HRESULT ret;
			if (FAILED(ret = myDwmSetIconicLivePreviewBitmap(hwnd, hbm, &pt, 0)))
			{
				qDebug() << "DwmSetIconicLivePreviewBitmap failed";
			}
			DeleteObject(hbm);
		}
	}
}

void UnionChatDialog::startShake()
{
	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(sender());
	if (!chatWidget)
		return;

	if (!hasChatWidget(chatWidget))
		return;

	// show chat dialog directly
	if (!this->isVisible() || this->isMinimized())
	{
		WidgetManager::showActivateRaiseWindow(this);
	}

	// make current
	if (ui->stackedWidget->currentWidget() != chatWidget)
	{
		setCurrentChat(chatWidget);
	}

	// start shake
	m_shakingFrameBak = frameGeometry();
	m_shakingCount = 0;
	m_shakingTimer->start(20);
}

void UnionChatDialog::shakingTimeout()
{
	m_shakingTimer->stop();
	QRect rect = m_shakingFrameBak;
	rect.translate(m_shakingPosList.at(m_shakingCount));
	setGeometry(rect);
	m_shakingCount++;
	if(m_shakingCount < m_shakingPosList.size())
	{
		m_shakingTimer->start(20);
	}
}

void UnionChatDialog::hideListFinished()
{
	ui->listView->setVisible(false);
	m_splitter->handle(1)->setVisible(false);
}

void UnionChatDialog::clearCurrentUnreadMessage()
{
	if (ui->stackedWidget->count() > 0)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->currentWidget());
		if (chatWidget)
		{
			// need clear unread message
			clearUnreadMessage(chatWidget);
		}
	}
}

void UnionChatDialog::insertMimeData(const QModelIndex &index, const QMimeData *source)
{
	if (!index.isValid())
		return;

	// set current
	QWidget *w = ui->stackedWidget->widget(index.row());
	ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(w);
	if (!chatWidget)
		return;

	chatWidget->insertMimeData(source);
}

void UnionChatDialog::onCurrentChatChanged(int index)
{
	// update previous index
	m_previousIndex = index;

	// focus to input
	if (index >= 0 && index < ui->stackedWidget->count())
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(index));
		if (chatWidget)
		{
			chatWidget->focusToEdit();
		}
	}
}

void UnionChatDialog::onChatRemoved(int index)
{
	// update previous index
	if (index != -1 && m_previousIndex != -1)
	{
		if (m_previousIndex == index)
		{
			m_previousIndex = -1;
		}
		else if (m_previousIndex > index)
		{
			--m_previousIndex;
		}
	}

	/*
	// check if need to hide list
	if (ui->stackedWidget->count() == 1)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(0));
		chatWidget->setUnionState(ChatBaseDialog::Single);

		if (ui->listView->width() > 0)
			hideList();
	}
	*/
}

void UnionChatDialog::chatMinimumWidthChanged(int minWidth)
{
	int maxMinWidth = minWidth;
	for (int i = 0; i < ui->stackedWidget->count(); ++i)
	{
		QWidget *chatWidget = ui->stackedWidget->widget(i);
		if (chatWidget->minimumWidth() > maxMinWidth)
			maxMinWidth = chatWidget->minimumWidth();
	}

	ui->stackedWidget->setMinimumWidth(maxMinWidth);
	if (ui->stackedWidget->count() > 1)
		this->setMinimumWidth(kMinListViewWidth+m_splitter->handleWidth()+maxMinWidth+2*kBorderWidth);
	else
		this->setMinimumWidth(maxMinWidth+2*kBorderWidth);
}

void UnionChatDialog::initUI()
{
	ui->listView->setMinimumWidth(0);
	ui->listView->setMaximumWidth(0);
	m_chatListModel = new QStandardItemModel(this);
	ui->listView->setModel(m_chatListModel);
	ui->listView->setItemDelegate(new UnionChatListItemDelegate(ui->listView));
	ui->listView->setDragEnabled(false);
	ui->listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onListItemClicked(QModelIndex)));
	connect(ui->listView, SIGNAL(dragMoveToIndex(QModelIndex)), this, SLOT(onListItemClicked(QModelIndex)));
	connect(ui->listView, SIGNAL(insertMimeData(const QModelIndex &, const QMimeData *)), 
		this, SLOT(insertMimeData(const QModelIndex &, const QMimeData *)));

	ui->stackedWidget->setMinimumWidth(GuiConstants::WidgetSize::GroupChatMainWidth+GuiConstants::WidgetSize::GroupChatMemberWidth);
	connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentChatChanged(int)));
	connect(ui->stackedWidget, SIGNAL(widgetRemoved (int)), this, SLOT(onChatRemoved(int)));

	m_clearUnreadTimer.setSingleShot(true);
	m_clearUnreadTimer.setInterval(20);
	connect(&m_clearUnreadTimer, SIGNAL(timeout()), this, SLOT(clearCurrentUnreadMessage()));

	initShake();
}

void UnionChatDialog::initShake()
{
	// shake time, every 20ms change a pos
	m_shakingTimer = new QTimer(this);
	m_shakingTimer->setInterval(20);

	// init position list
	m_shakingPosList.clear();
	m_shakingPosList.append(QPoint(5, 0));
	m_shakingPosList.append(QPoint(5, -5));
	m_shakingPosList.append(QPoint(-5, -5));
	m_shakingPosList.append(QPoint(-5, 5));
	m_shakingPosList.append(QPoint(5, 5));
	m_shakingPosList.append(QPoint(5, -5));
	m_shakingPosList.append(QPoint(-5, -5));
	m_shakingPosList.append(QPoint(-5, 5));
	m_shakingPosList.append(QPoint(5, 5));
	m_shakingPosList.append(QPoint(5, -5));
	m_shakingPosList.append(QPoint(-5, -5));
	m_shakingPosList.append(QPoint(-5, 5));
	m_shakingPosList.append(QPoint(5, 5));
	m_shakingPosList.append(QPoint(0, 0));

	connect(m_shakingTimer, SIGNAL(timeout()), SLOT(shakingTimeout()));
}

QStandardItem * UnionChatDialog::createListItem(bean::MessageType msgType, const QString &id)
{
	QStandardItem *item = 0;
	if (msgType == bean::Message_Invalid || id.isEmpty())
		return item;

	ModelManager *modelManager = qPmApp->getModelManager();
	QIcon icon;
	QString name;
	if (msgType == bean::Message_GroupChat)
	{
		icon = modelManager->getGroupLogo(id);
		name = modelManager->groupName(id);
	}
	else if (msgType == bean::Message_DiscussChat)
	{
		icon = modelManager->discussLogo(id);
		name = modelManager->discussName(id);
	}
	else
	{
		icon = modelManager->getUserAvatar(id);
		name = modelManager->userName(id);
	}

	item = new QStandardItem(icon, name);
	item->setData((int)msgType, kChatListMsgTypeRole);
	item->setData(id, kChatListMsgIdRole);
	
	return item;
}

QStandardItem * UnionChatDialog::createMultiChatListItem(const QString &id, const QStringList &members)
{
	QStandardItem *item = 0;
	if (id.isEmpty() || members.isEmpty())
		return item;

	QStringList sortedMembers = members;
	qSort(sortedMembers);
	ModelManager *modelManager = qPmApp->getModelManager();
	QPixmap pixmap = modelManager->getMultiAvatar(48, sortedMembers);
	QIcon icon(pixmap);
	QString name = modelManager->getMultiName(sortedMembers);

	item = new QStandardItem(icon, name);
	item->setData(bean::Message_Invalid, kChatListMsgTypeRole);
	item->setData(id, kChatListMsgIdRole);

	return item;
}

void UnionChatDialog::addListItemWidget(QStandardItem *item)
{
	if (!item)
		return;

	UnionChatListItemWidget *w = new UnionChatListItemWidget(item, ui->listView, this);
	connect(w, SIGNAL(closeChat()), this, SLOT(removeChat()), Qt::QueuedConnection);
	ui->listView->setIndexWidget(item->index(), w);
}

void UnionChatDialog::flashTaskBar(QWidget *w)
{
	if (!w)
		return;

	DWORD timeOut = GetCaretBlinkTime();
	if (timeOut <= 0)
	{
		timeOut = 250;
	}

	UINT flashCount = 1;

	FLASHWINFO info;
	info.cbSize = sizeof(info);
	info.hwnd = (HWND)w->winId();
	info.dwFlags = FLASHW_ALL;
	info.dwTimeout = timeOut;
	info.uCount = flashCount;

	FlashWindowEx(&info);
}

void UnionChatDialog::unflashTaskBar(QWidget *w)
{
	if (!w)
		return;

	FLASHWINFO info;
	info.cbSize = sizeof(info);
	info.hwnd = (HWND)w->winId();
	info.dwFlags = FLASHW_STOP;
	info.dwTimeout = 0;
	info.uCount = 0;

	FlashWindowEx(&info);
}

void UnionChatDialog::connectChatWidget(ChatBaseDialog *chatWidget)
{
	if (!chatWidget)
		return;

	chatWidget->setUnionChatDelegate(this);

	connect(chatWidget, SIGNAL(requestClose()), this, SLOT(close()), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(requestMinimized()), this, SLOT(showMinimized()), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(requestMaximized()), this, SLOT(triggerMaximize()), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(chatIconChanged(QPixmap)), this, SLOT(chatIconChanged(QPixmap)), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(chatNameChanged(QString)), this, SLOT(chatNameChanged(QString)), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(activateFromTaskbar()), this, SLOT(activateFromTaskbar()), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(closeFromTaskbar()), this, SLOT(closeFromTaskbar()), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(chatUnreadMessageCountChanged(int)), this, SLOT(chatUnreadMessageCountChanged(int)), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(setThumbnail(int, int)), this, SLOT(setChatThumbnail(int, int)), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(setLivePreview()), this, SLOT(setChatLivePreview()), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(removeChat(QWidget *)), this, SLOT(removeChat(QWidget *)), Qt::UniqueConnection);
	connect(chatWidget, SIGNAL(shaking()), this, SLOT(startShake()), Qt::UniqueConnection);
}

void UnionChatDialog::setTaskbarIconAndText(QWidget *w, QStandardItem *item)
{
	if (!item || !w)
		return;

	// set window icon
	QPixmap pixmap = item->icon().pixmap(kChatListIconSize);
	setTaskbarIcon(w, pixmap);

	// set window text
	QString name = item->text();
	int unreadCount = item->data(kChatListMsgCountRole).toInt();
	if (w != this)
	{
		unreadCount = ((ChatBaseDialog *)w)->unreadMessageCount();
	}
	QString title = name;
	if (w != this && ui->listView->currentIndex() != item->index() && unreadCount > 0)
		title.append(tr("(%1 unread)").arg(unreadCount));
	setTaskbarText(w, title);
}

void UnionChatDialog::setTaskbarIcon(QWidget *w, const QPixmap &pixmap)
{
	if (!w || pixmap.isNull())
		return;

	HWND hwnd = NULL;
	if (w == this)
	{
		hwnd = (HWND)w->winId();
	}
	else
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(w);
		if (chatWidget)
			hwnd = chatWidget->chatProxyWinId();
	}

	if (hwnd != NULL)
	{
		QPixmap icon = pixmap.scaled(QSize(32, 32), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		HICON hIcon = QtWin::toHICON(icon);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
}

void UnionChatDialog::setTaskbarText(QWidget *w, const QString &text)
{
	if (!w)
		return;

	HWND hwnd = (HWND)w->winId();
	wchar_t titleBuf[128];

	if (w != this)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(w);
		if (chatWidget)
			hwnd = chatWidget->chatProxyWinId();

		int len = text.toWCharArray(titleBuf);
		titleBuf[len] = L'\0';
	}
	else
	{
		hwnd = (HWND)w->winId();

		int chatCount = ui->stackedWidget->count();
		if (chatCount > 1)
		{
			QString tooltip = tr("%1 and total %2 sessions").arg(text).arg(chatCount);
			int len = tooltip.toWCharArray(titleBuf);
			titleBuf[len] = L'\0';
		}
		else
		{
			int len = text.toWCharArray(titleBuf);
			titleBuf[len] = L'\0';
		}

		setWindowTitle(QString::fromWCharArray(titleBuf));
	}

	if (hwnd != NULL)
	{
		SetWindowTextW(hwnd, titleBuf);
	}
}

bool UnionChatDialog::insertTaskbarTab(QWidget *insertWidget, QWidget *beforeWidget)
{
	if (ChatProxy::isSupportTaskbarApi())
	{
		if (!m_pTl3)
			return false;

		if (!insertWidget)
			return false;

		ChatBaseDialog *insertChatWidget = qobject_cast<ChatBaseDialog *>(insertWidget);
		if (!insertChatWidget)
			return false;

		HWND hwnd = insertChatWidget->chatProxyWinId();
		if (hwnd == NULL)
			return false;

		HWND beforeHwnd = NULL;
		if (beforeWidget)
		{
			ChatBaseDialog *beforeChatWidget = qobject_cast<ChatBaseDialog *>(beforeWidget);
			beforeHwnd = beforeChatWidget->chatProxyWinId();
		}

		HWND mainHwnd = (HWND)this->winId();
		ITaskbarList3 *pTl3 = (ITaskbarList3 *)m_pTl3;
		if (FAILED(pTl3->RegisterTab(hwnd, mainHwnd)))
		{
			qDebug() << Q_FUNC_INFO << "register tab failed";
			return false;
		}

		if (FAILED(pTl3->SetTabOrder(hwnd, beforeHwnd)))
		{
			qDebug() << Q_FUNC_INFO << "set tab order failed";
			return false;
		}

		if (insertChatWidget->unreadMessageCount() > 0)
			flashTaskBar(insertChatWidget->chatProxy());

		return true;
	}

	return false;
}

bool UnionChatDialog::removeTaskbarTab(QWidget *widget)
{
	if (ChatProxy::isSupportTaskbarApi())
	{
		if (!m_pTl3)
			return false;

		if (!widget)
			return false;

		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(widget);
		if (!chatWidget)
			return false;

		HWND hwnd = chatWidget->chatProxyWinId();
		if (hwnd == NULL)
			return false;

		ITaskbarList3 *pTl3 = (ITaskbarList3 *)m_pTl3;
		if (FAILED(pTl3->UnregisterTab(hwnd)))
		{
			qDebug() << Q_FUNC_INFO << "unregister tab failed";
			return false;
		}

		return true;
	}

	return false;
}

bool UnionChatDialog::setActiveTaskbarTab(QWidget *widget)
{
	if (ChatProxy::isSupportTaskbarApi())
	{
		if (!m_pTl3)
			return false;

		if (!widget)
			return false;

		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(widget);
		if (!chatWidget)
			return false;

		HWND hwnd = chatWidget->chatProxyWinId();
		if (hwnd == NULL)
			return false;

		HWND mainHwnd = (HWND)this->winId();
		ITaskbarList3 *pTl3 = (ITaskbarList3 *)m_pTl3;
		if (FAILED(pTl3->SetTabActive(hwnd, mainHwnd, 0)))
		{
			qDebug() << Q_FUNC_INFO << "set tab active failed";
			return false;
		}

		unflashTaskBar(chatWidget->chatProxy());

		return true;
	}

	return false;
}

void UnionChatDialog::clearUnreadMessage(ChatBaseDialog *chatWidget)
{
	if (!chatWidget)
		return;

	int index = ui->stackedWidget->indexOf(chatWidget);
	if (index == -1)
		return;

	/*
	if (chatWidget->unreadMessageCount() <= 0)
		return;
	*/

	// current widget need clear unread message count
	chatWidget->clearUnreadMessageCount();
	QStandardItem *item = m_chatListModel->item(index);
	item->setData(0, kChatListMsgCountRole);
	ui->listView->update();

	QString title = item->text();
	setTaskbarText(chatWidget, title);
}

bool UnionChatDialog::flashNextUnread()
{
	for (int i = 0; i < ui->stackedWidget->count(); ++i)
	{
		ChatBaseDialog *chatWidget = qobject_cast<ChatBaseDialog *>(ui->stackedWidget->widget(i));
		if (chatWidget->unreadMessageCount() > 0)
		{
			QStandardItem *item = m_chatListModel->item(ui->stackedWidget->indexOf(chatWidget));
			setTaskbarIconAndText(this, item);
			flashTaskBar(this);
			return true;
		}
	}

	return false;
}

void UnionChatDialog::showList()
{
	if (!isShowMaximized())
	{
		QRect geo = this->geometry();
		QRect endGeo = geo;
		endGeo.setWidth(geo.width() + kMaxListViewWidth + m_splitter->handleWidth());
		this->setGeometry(endGeo);
	}

	// do not use animation
	ui->listView->setVisible(true);
	ui->listView->setMinimumWidth(kMinListViewWidth);
	ui->listView->setMaximumWidth(kMaxListViewWidth);
	m_splitter->handle(1)->setVisible(true);
	m_splitter->setSizes(QList<int>() << kMaxListViewWidth << ui->stackedWidget->width());
	this->setMinimumWidth(kMinListViewWidth + m_splitter->handleWidth() + ui->stackedWidget->minimumWidth() + 2*kBorderWidth);
}

void UnionChatDialog::hideList()
{
	ui->listView->setMinimumWidth(0);
	this->setMinimumWidth(ui->stackedWidget->minimumWidth() + 2*kBorderWidth);

	QParallelAnimationGroup *groupAnimation = new QParallelAnimationGroup(this);
	connect(groupAnimation, SIGNAL(finished()), this, SLOT(hideListFinished()));

	QList<int> sizes = m_splitter->sizes();
	int listViewWidth = sizes[0];

	QPropertyAnimation *widthAnimation = new QPropertyAnimation(ui->listView, "width", this);
	widthAnimation->setStartValue(listViewWidth);
	widthAnimation->setEndValue(0);
	widthAnimation->setDuration(200);
	groupAnimation->addAnimation(widthAnimation);
	
	if (!isShowMaximized())
	{
		QPropertyAnimation *geoAnimation = new QPropertyAnimation(this, "geometry");
		QRect geo = this->geometry();
		QRect endGeo = geo;
		endGeo.setWidth(geo.width() - listViewWidth);
		geoAnimation->setStartValue(geo);
		geoAnimation->setEndValue(endGeo);
		geoAnimation->setDuration(200);
		groupAnimation->addAnimation(geoAnimation);
	}
	
	groupAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

ChatProxy *UnionChatDialog::createChatProxy()
{
	ChatProxy *chatProxy = new ChatProxy();
	if (ChatProxy::isSupportTaskbarApi() && myDwmSetWindowAttribute)
	{
		HWND hwnd = (HWND)chatProxy->winId();
		BOOL fForceIconic = TRUE;
		BOOL fHasIconicBitmap = TRUE;
		HRESULT ret = S_OK;
		if (FAILED(ret = myDwmSetWindowAttribute(
			hwnd,
			DWMWA_FORCE_ICONIC_REPRESENTATION,
			&fForceIconic,
			sizeof(fForceIconic))))
		{
			qDebug() << "DwmSetWindowAttribute DWMWA_FORCE_ICONIC_REPRESENTATION failed";
		}

		if (FAILED(ret = myDwmSetWindowAttribute(
			hwnd,
			DWMWA_HAS_ICONIC_BITMAP,
			&fHasIconicBitmap,
			sizeof(fHasIconicBitmap))))
		{
			qDebug() << "DwmSetWindowAttribute DWMWA_HAS_ICONIC_BITMAP failed";
		}
	}
	return chatProxy;
}

#endif // UNICODE