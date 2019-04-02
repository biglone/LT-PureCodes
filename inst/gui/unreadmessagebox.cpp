#include "unreadmessagebox.h"
#include "model/unreadmsgitem.h"
#include "model/unreadmsgmodel.h"
#include "model/unreadmessagesortfiltermodel.h"
#include <QApplication>
#include <QDesktopWidget>
#include "systemtray.h"
#include <QTimer>
#include <QDebug>
#include "bean/bean.h"
#include "Account.h"
#include "settings/AccountSettings.h"

static const int kRowHeight    = 30;
static const int kMarginHeight = 76;

UnreadMessageBox::UnreadMessageBox(UnreadMsgModel *model, CSystemTray *trayIcon, QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_unreadMessageModel(model), m_trayIcon(trayIcon)
{
	Q_ASSERT(m_unreadMessageModel);
	Q_ASSERT(m_trayIcon);

	ui.setupUi(this);
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::WindowStaysOnTopHint;
	flags |= Qt::Popup;
	flags |= Qt::Dialog;
	setWindowFlags(flags);
	setMouseTracking(true);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setAttribute(Qt::WA_Hover, true);
	setMaximizeable(false);
	setMainLayout(ui.verticalLayoutMain);
	setMoveAble(false);

	ui.labelViewAll->setFontAtt(QColor(0, 120, 216), 10, false);
	ui.labelIgnore->setFontAtt(QColor(0, 120, 216), 10, false);

	ui.listView->setMinimumHeight(kRowHeight);
	setFixedWidth(227);

	m_unreadMessageFilterModel = m_unreadMessageModel->filterModel();
	ui.listView->setModel(m_unreadMessageFilterModel);
	connect(m_trayIcon, SIGNAL(hoverEnter(QRect)), SLOT(onTrayIconHoverEnter(QRect)));
	connect(m_trayIcon, SIGNAL(hoverLeave()), SLOT(onTrayIconHoverLeave()));
	connect(m_unreadMessageModel, SIGNAL(unreadItemCountChanged()), SLOT(onUnreadItemCountChanged()));
	connect(m_unreadMessageModel, SIGNAL(preTakeMsg(QModelIndex)), SLOT(onPreTakeMsg(QModelIndex)));

	m_preHideTimer = new QTimer(this);
	m_preHideTimer->setInterval(1200);
	m_preHideTimer->setSingleShot(true);

	connect(m_preHideTimer, SIGNAL(timeout()), SLOT(hide()));
	connect(ui.labelIgnore, SIGNAL(clicked()), this, SLOT(ignore()));
	connect(ui.labelViewAll, SIGNAL(clicked()), this, SLOT(viewAll()));
	connect(ui.listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));

	setSkin();
}

UnreadMessageBox::~UnreadMessageBox()
{
}

void UnreadMessageBox::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_5.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 24;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui.labelTitle->setStyleSheet("QLabel {color: white;}");
}

void UnreadMessageBox::onItemClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		QModelIndex sourceIndex = m_unreadMessageFilterModel->mapToSource(index);
		if (sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_Chat)
			emit openChat(sourceIndex.data(UnreadMsgItem::IdRole).toString());
		else if (sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_GroupChat)
			emit openGroupChat(sourceIndex.data(UnreadMsgItem::IdRole).toString());
		else if (sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_DiscussChat)
			emit openDiscuss(sourceIndex.data(UnreadMsgItem::IdRole).toString());
	}
}

void UnreadMessageBox::ignoreItem(const QModelIndex &sourceIndex)
{
	if (sourceIndex.isValid())
	{
		bean::MessageType msgType = (bean::MessageType)sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt();
		QString id = sourceIndex.data(UnreadMsgItem::IdRole).toString();
		m_unreadMessageModel->takeMsg(id, msgType);
	}
}

void UnreadMessageBox::onTrayIconHoverEnter(const QRect &trayIconGeometry)
{
	m_unreadMessageFilterModel->invalidate();
	if (m_unreadMessageFilterModel->rowCount() > 0 && !isVisible())
	{
		int newHeight = kMarginHeight + kRowHeight * m_unreadMessageFilterModel->rowCount();
		QRect geo = geometry();
		QRect rcAvailable =  QApplication::desktop()->availableGeometry();
		if (newHeight > rcAvailable.height())
		{
			newHeight = rcAvailable.height();
		}
		geo.setHeight(newHeight);

		setGeometry(geo);

		geo = geometry();

		geo.moveLeft(trayIconGeometry.center().x() - geo.width() / 2);
		geo.moveTop(trayIconGeometry.center().y() - geo.height() / 2);
		
		if (geo.right() > rcAvailable.right())
		{
			geo.moveRight(rcAvailable.right());
		}
		
		if (geo.bottom() > rcAvailable.bottom())
		{
			geo.moveBottom(rcAvailable.bottom());
		}

		if (geo.left() < rcAvailable.left())
		{
			geo.moveLeft(rcAvailable.left());
		}

		if (geo.top() < rcAvailable.top())
		{
			geo.moveTop(rcAvailable.top());
		}

		setGeometry(geo);

		show();
		raise();
		activateWindow();
	}
	m_preHideTimer->stop();
}

void UnreadMessageBox::onTrayIconHoverLeave()
{
	if (!frameGeometry().contains(QCursor::pos()) && isVisible() && !Account::settings()->unreadBoxAutoShow())
	{
		m_preHideTimer->start();
	}
}

void UnreadMessageBox::enterEvent(QEvent *e)
{
	m_preHideTimer->stop();
	QDialog::enterEvent(e);
}

void UnreadMessageBox::leaveEvent(QEvent *e)
{
	if (!Account::settings()->unreadBoxAutoShow())
	{
		hide();
	}
	QDialog::leaveEvent(e);
}

void UnreadMessageBox::onPreTakeMsg(const QModelIndex &sourceIndex)
{
	if (sourceIndex.isValid())
	{
		ui.listView->setIndexWidget(sourceIndex, 0);
	}
}

void UnreadMessageBox::readdItemWidget()
{
	for (int i = 0; i < m_unreadMessageFilterModel->rowCount(); i++)
	{
		QModelIndex index = m_unreadMessageFilterModel->index(i, 0);
		QModelIndex sourceIndex = m_unreadMessageFilterModel->mapToSource(index);
		if (sourceIndex.isValid())
		{
			UnreadMessageItemWidget *widget = new UnreadMessageItemWidget(sourceIndex, ui.listView);
			ui.listView->setIndexWidget(index, widget);
			connect(widget, SIGNAL(ignoreItem(QModelIndex)), this, SLOT(ignoreItem(QModelIndex)));
		}
	}
}

void UnreadMessageBox::onUnreadItemCountChanged()
{
	m_unreadMessageFilterModel->invalidate();

	if (m_unreadMessageFilterModel->rowCount() == 0)
	{
		ui.listView->update();
		hide();
	}
	else 
	{
		readdItemWidget();
		ui.listView->update();

		if (isVisible())
		{
			int newHeight = kMarginHeight + kRowHeight * m_unreadMessageFilterModel->rowCount();
			QRect rcAvailable = QApplication::desktop()->availableGeometry();
			if (newHeight > rcAvailable.height())
			{
				newHeight = rcAvailable.height();
			}
			QRect geo = frameGeometry();
			geo.setHeight(newHeight);
			geo.moveBottom(rcAvailable.bottom());
			setGeometry(geo);
		}
		else if (Account::settings()->unreadBoxAutoShow())
		{
			onTrayIconHoverEnter(m_trayIcon->geometry());
		}
	}
}

void UnreadMessageBox::ignore()
{
	m_unreadMessageModel->ignoreAll();
	onUnreadItemCountChanged();
}

void UnreadMessageBox::viewAll()
{
	QList<int> msgTypes;
	QStringList ids;
	for (int i=0; i<m_unreadMessageFilterModel->rowCount(); i++)
	{
		QModelIndex index = m_unreadMessageFilterModel->index(i, 0);
		if (index.isValid())
		{
			QModelIndex sourceIndex = m_unreadMessageFilterModel->mapToSource(index);
			if (sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_Chat)
			{
				msgTypes << (int)bean::Message_Chat;
				ids << sourceIndex.data(UnreadMsgItem::IdRole).toString();
			}
			else if (sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_GroupChat)
			{
				msgTypes << (int)bean::Message_GroupChat;
				ids << sourceIndex.data(UnreadMsgItem::IdRole).toString();
			}
			else if (sourceIndex.data(UnreadMsgItem::MsgTypeRole).toInt() == (int)bean::Message_DiscussChat)
			{
				msgTypes << (int)bean::Message_DiscussChat;
				ids << sourceIndex.data(UnreadMsgItem::IdRole).toString();
			}
		}
	}
	hide();

	emit openAllUnreadChats(msgTypes, ids);
}
